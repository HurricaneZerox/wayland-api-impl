#include <cstddef>
#include <filesystem>
#include <map>

#include <set>
#include <stdexcept>
#include <unistd.h>
#include <syscall.h>
#include <sys/mman.h>
#include <sys/un.h>

#include "lumber.h"
#include "wl_enums.h"
#include "wl_types.h"

#include "buffers/recv.h"
#include "buffers/send.h"

#include "wl_string.h"
#include "wl_event.h"

using fd_t = uintmax_t;

#define DISPLAY_OBJ_ID 1

#define WL_NEW_ID_MIN 2
#define WL_NEW_ID_MAX 0xFEFFFFFF

EventQueue event_queue(4096);
SendQueue send_queue;

void* send_queue_alloc(size_t bytes) {
    return send_queue.Allocate(bytes);
}

struct WaylandGlobal {
    const wl_uint name;
    const wl_string interface;
    const wl_uint version;

    WaylandGlobal(const char* data)
      : name(read_wl_uint(data)),
        interface(data + WL_UINT_SIZE),
        version(read_wl_uint(data + wl_align((2 * WL_UINT_SIZE) + interface.Size())))
    {}
};

std::filesystem::path get_wayland_socket_path() {
    const std::filesystem::path parent_path = getenv("XDG_RUNTIME_DIR");
    const std::filesystem::path filename = getenv("WAYLAND_DISPLAY");
    return parent_path / filename;
}

fd_t create_wayland_socket() {
    const fd_t handle = socket(AF_UNIX, SOCK_STREAM, 0);

    if (handle < 0) {
        throw std::runtime_error("Failed to create socket");
    }

    const std::filesystem::path socket_path = get_wayland_socket_path();

    struct sockaddr_un addr {
        .sun_family = AF_UNIX,
    };

    strcpy(addr.sun_path, socket_path.c_str());
    
    if (connect(handle, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        throw std::runtime_error("Failed to bind socket");
    }

    return handle;
}

class wl_id_assigner {
    std::set<wl_object> ids = {};

    public:

    wl_new_id get_id() {
        for (wl_new_id id = WL_NEW_ID_MIN; id < WL_NEW_ID_MAX; id++) {
            if (ids.count(id) != 0) { continue; }
            ids.insert(id);
            return id;
        }

        throw std::runtime_error("Failed to get new ID.");
    }
};

wl_id_assigner wl_id_assigner;

class wl_registry;

class wl_obj {

    public:

    virtual void handle_event(uint16_t opcode, void* data, size_t size) = 0;

    virtual wl_object ID() const noexcept = 0;
};

class wl_id_map {
    std::map<wl_object, wl_obj*> objects;

    public:

    wl_obj* get(const wl_object id) {
        if (objects.count(id) == 0) {
            return nullptr;
        }

        return objects.at(id);
    }

    void set(const wl_object id, wl_obj* object) {
        objects[id] = object;
    }
};

wl_id_map wl_id_map;

class wl_registry : public wl_obj {
    const wl_object id;
    const fd_t socket;

    static constexpr uintmax_t BIND_OPCODE = 0;

    static constexpr wl_uint EV_GLOBAL_OPCODE = 0;
    static constexpr wl_uint EV_GLOBAL_REMOVE_OPCODE = 1;

    public:

    struct listener {
        void (*global)(wl_registry& registry, const WaylandGlobal& global);
        void (*global_remove)(wl_registry& registry, wl_uint name);
    };

    listener* listener = nullptr;

    wl_registry(const wl_new_id id, const fd_t socket) : id(id), socket(socket) {
        
    }

    void set_listener(struct listener* listener) {
        this->listener = listener;
    }

    void handle_event(uint16_t opcode, void* data, size_t size) override {
        if (!listener) {
            std::cout << "[Wayland::WARN]: Missing event listener.\n";
            return;
        }

        if (opcode == EV_GLOBAL_OPCODE) {
            listener->global(*this, WaylandGlobal((char*)data));
        } else if (opcode == EV_GLOBAL_REMOVE_OPCODE) {
            listener->global_remove(*this, read_wl_uint(data));
        }
    }

    /**
        Binds a server-side global to a client-side ID.
    */
    void bind(wl_uint name, const wl_string& interface, wl_uint version, wl_new_id id) {
        WaylandMessage client_msg(send_queue_alloc, this->id, BIND_OPCODE, 3 + (interface.WordSize() + WL_WORD_SIZE));
    
        client_msg.Write(name);
        client_msg.Write(interface);
        client_msg.Write(version);
        client_msg.Write(id);
    }

    wl_object ID() const noexcept override {
        return id;
    }
};

class wl_display {

    static constexpr wl_uint SYNC_OPCODE = 0;
    static constexpr wl_uint GET_REGISTRY_OPCODE = 1;

    static constexpr wl_uint EV_ERROR_OPCODE = 0;
    static constexpr wl_uint EV_DELETE_ID_OPCODE = 1;

    public:

    enum class Error : wl_uint {
        INVALID_OBJECT = 0,
        INVALID_METHOD = 1,
        NO_MEMORY = 2,
        IMPLEMENTATION = 3,
    };

    fd_t socket;

    wl_display() : socket(create_wayland_socket()) {}

    wl_registry& get_registry() {
        const wl_new_id registry_id = wl_id_assigner.get_id();
        std::cout << "Registry ID: " << registry_id << '\n';

        WaylandMessage client_msg(send_queue_alloc, DISPLAY_OBJ_ID, GET_REGISTRY_OPCODE, 1);
        client_msg.Write(registry_id);

        wl_registry* registry = new wl_registry(registry_id, socket);
        wl_obj* object = registry;

        wl_id_map.set(2, object);

        return *registry;
    }

    void sync() {

    }

    void read_events() {
        event_queue.Recv(socket);

        while (true) {
            const std::optional<wl_event> opt_ev = event_queue.PopEvent();
            if (!opt_ev.has_value()) { break; }

            const wl_event ev = opt_ev.value();

            if (ev.object_id == 1 && ev.opcode == EV_ERROR_OPCODE) {
                wl_object err_object_id = read_wl_object(ev.payload);
                wl_uint err_opcode = read_wl_uint(ev.payload + 4);
                wl_string err_msg(ev.payload + 8);

                std::string output_msg("[Wayland::ERR]: Ran into an error:\n");
                output_msg += "\tMessage: " + std::string(err_msg);
                
                lumber::err(output_msg.c_str());
                
                exit(1);
            } else if (ev.object_id == 1 && ev.opcode == EV_DELETE_ID_OPCODE) {
                wl_object id = read_wl_object(ev.payload);
                std::cout << "DELETE ID: " << id << '\n';
                continue;
            }

            wl_obj* object = wl_id_map.get(ev.object_id);

            if (!object) {
                std::string warning_msg = "[Wayland::WARN]: Received event for non-existent object. (id: ";
                warning_msg += std::to_string(ev.object_id) + ")";
                lumber::warn(warning_msg.c_str());
                continue;
            }

            object->handle_event(ev.opcode, ev.payload, ev.size - WL_EVENT_HEADER_SIZE);
        }
    }

    /**
        @brief Dispatches messages on the send queue without
        reading from the event queue.

        @returns The number of messages dispatched.
    */
    size_t dispatch_pending() {
        if (!send_queue.Empty()) {
            return send_queue.Flush(socket);
        }

        return 0;
    }

    /**
        @brief Dispatches messages on the send queue.

        If the event queue is empty, this function blocks
        until there is an event to read from.
    */
    size_t dispatch() {

    }

    void roundtrip() {
        dispatch_pending();
        read_events();
    }
};

class wl_buffer {
    wl_object id;

    public:

    wl_buffer(const wl_new_id id) : id(id) {
        
    }

    wl_object ID() const noexcept {
        return id;
    }

    void destroy() {

    }
};

struct wl_surface : public wl_obj {
    const wl_object id;

    static constexpr wl_uint DESTROY_OPCODE = 0;
    static constexpr wl_uint ATTACH_OPCODE = 1;
    static constexpr wl_uint DAMAGE_OPCODE = 2;
    static constexpr wl_uint FRAME_OPCODE = 3;
    static constexpr wl_uint SET_OPAQUE_REGION_OPCODE = 4;
    static constexpr wl_uint SET_INPUT_REGION_OPCODE = 5;
    static constexpr wl_uint COMMIT_OPCODE = 6;
    static constexpr wl_uint SET_BUFFER_TRANSFORM_OPCODE = 7;

    public:

    wl_surface(const wl_new_id id) : id(id) {

    }

    void attach(fd_t socket, wl_buffer& buffer, wl_int x, wl_int y) {
        WaylandMessage client_msg(send_queue_alloc, id, ATTACH_OPCODE, 3);
        client_msg.Write(buffer.ID());
        client_msg.Write(x);
        client_msg.Write(y);
    }

    void commit(fd_t socket) {
        WaylandMessage client_msg(send_queue_alloc, id, COMMIT_OPCODE, 0);
    }

    void handle_event(uint16_t opcode, void *data, size_t size) override {

    }

    wl_object ID() const noexcept override {
        return id;
    }
};

class wl_compositor {

    wl_new_id id;

    public:

    wl_compositor(const wl_new_id id) : id(id) {

    }

    wl_surface* create_surface(const fd_t socket) {
        wl_surface* surface = new wl_surface(wl_id_assigner.get_id());

        WaylandMessage client_msg(send_queue_alloc, id, 0, 1);
        client_msg.Write(surface->id);

        return surface;
    }

    void create_region(const fd_t socket) {

    }
};

class xdg_toplevel : public wl_obj {

    wl_object id;

    public:

    struct listener {
        void (*configure)(int width, int height);
        void (*close)();
        void (*configure_bounds)(int width, int height);
        void (*wm_capabilities)();
    };

    listener* listener;

    xdg_toplevel(const wl_new_id id) : id(id) {
        
    }

    wl_object ID() const noexcept override {
        return id;
    }

    void handle_event(uint16_t opcode, void *data, size_t size) override {
        if (!listener) {
            throw std::runtime_error("No event listener supplied for xdg_toplevel");
        }

        if (opcode == 0) {
            listener->configure(read_wl_int(data), read_wl_int((char*)data + 4));
        } else if (opcode == 1) {
            listener->close();
        } else if (opcode == 2) {
            listener->configure_bounds(read_wl_int(data), read_wl_int((char*)data + 4));
        } else if (opcode == 3) {
            listener->wm_capabilities();
        }
    }
};

class xdg_surface : public wl_obj {

    wl_new_id id;
    fd_t socket;

    public:
    
    struct listener {
        void (*configure)(xdg_surface& surface, int serial);
    };

    listener* listener;

    xdg_surface(const wl_new_id id) : id(id) {

    }

    wl_object ID() const noexcept override {
        return id;
    }

    void destroy();

    xdg_toplevel* get_toplevel(const fd_t socket) {
        this->socket = socket;
        xdg_toplevel* toplevel = new xdg_toplevel(wl_id_assigner.get_id());
        std::cout << "XDG Toplevel ID: " << toplevel->ID() << '\n';

        WaylandMessage client_msg(send_queue_alloc, id, 1, 1);
        client_msg.Write(toplevel->ID());

        return toplevel;
    }

    void ack_configure(int serial) {
        WaylandMessage client_msg(send_queue_alloc, id, 4, 1);
        client_msg.Write(serial);
    }

    void handle_event(uint16_t opcode, void *data, size_t size) override {
        if (!listener) {
            throw std::runtime_error("No event listener supplied for xdg_surface");
        }
        
        if (opcode == 0) {
            listener->configure(*this, read_wl_int(data));
        }
    }
};

class xdg_wm_base : public wl_obj {

    wl_object id;

    fd_t socket;

    static constexpr wl_uint DESTROY_OPCODE = 0;
    static constexpr wl_uint CREATE_POSITIONER_OPCODE = 1;
    static constexpr wl_uint GET_XDG_SURFACE_OPCODE = 2;
    static constexpr wl_uint PONG_OPCODE = 3;

    static constexpr wl_uint EV_PING_OPCODE = 0;

    public:

    xdg_wm_base(const wl_new_id id) : id(id) {

    }

    void destroy();

    void create_positioner();

    xdg_surface* get_xdg_surface(const fd_t socket, wl_surface& surface) {
        xdg_surface* x_surface = new xdg_surface(wl_id_assigner.get_id());
        std::cout << "XDG Surface ID: " << x_surface->ID() << '\n';

        WaylandMessage client_msg(send_queue_alloc, id, GET_XDG_SURFACE_OPCODE, 2);
        client_msg.Write(x_surface->ID());
        client_msg.Write(surface.id);

        return x_surface;
    }

    void pong(const wl_uint serial) {
        WaylandMessage client_msg(send_queue_alloc, id, PONG_OPCODE, 1);
        client_msg.Write(serial);
    }

    void handle_event(uint16_t opcode, void *data, size_t size) override {
        if (opcode == EV_PING_OPCODE) {
            if (size != WL_UINT_SIZE) {
                throw std::runtime_error("Server sent invalid event");
            }

            const wl_uint serial = read_wl_uint(data);
        }
    }

    wl_object ID() const noexcept override {
        return id;
    }
};

class wl_shm_pool {

    wl_object id;

    public:

    wl_shm_pool(const wl_new_id id) : id(id) {
        
    }

    wl_object ID() const noexcept {
        return id;
    }

    wl_buffer create_buffer(fd_t socket, wl_int offset, wl_int width, wl_int height, wl_int stride, wl_uint format) {
        wl_buffer buffer(wl_id_assigner.get_id());
        std::cout << "Buffer ID: " << buffer.ID() << '\n';

        WaylandMessage client_msg(send_queue_alloc, id, 0, 6);
        client_msg.Write(buffer.ID());
        client_msg.Write(offset);
        client_msg.Write(width);
        client_msg.Write(height);
        client_msg.Write(stride);
        client_msg.Write(format);

        return buffer;
    }
};

class wl_shm : public wl_obj {
    wl_object id;

    public:

    struct listener {
        /**
            Informs the client of a supported
            pixel format.
        */
        void (*format)(const wl_uint format);
    };

    listener* listener = nullptr;

    wl_shm(const wl_new_id id) : id(id) {

    }

    wl_object ID() const noexcept override {
        return id;
    }

    wl_shm_pool create_pool(const fd_t socket, fd_t fd, size_t size) {
        wl_shm_pool pool(wl_id_assigner.get_id());
        std::cout << "Pool ID: " << pool.ID() << '\n';

        WaylandMessage client_msg(send_queue_alloc, id, 0, 2);
        client_msg.Write(pool.ID());
        client_msg.Write(size);

        send_queue.SetAncillary(fd);

        return pool;
    }

    void handle_event(uint16_t opcode, void *data, size_t size) override {
        if (!listener) {
            throw std::runtime_error("No listener supplied for wl_shm.");
        }

        if (size != 4) {
            throw std::runtime_error("Server sent unexpected size.");
        }

        if (opcode == 0) {
            listener->format(read_wl_uint(data));
        }
    }
};

/**
    Use-case example
*/

bool has_configured = false;
bool has_attached_buffer = false;

struct xdg_surface::listener xdg_surface_listener {
    .configure = [](xdg_surface& surface, int serial) {
        surface.ack_configure(serial);
        has_configured = true;
    }
};

struct xdg_toplevel::listener xdg_toplevel_listener {
    .configure = [](int x, int y) {
        std::cout << "Configure: " << x << ", " << y << '\n';
    },
    .close = []() {
        std::cout << "Close" << '\n';
    },
    .configure_bounds = [](int x, int y) {
        std::cout << "Configure bounds: " << x << ", " << y << '\n';
    },
    .wm_capabilities = []() {
        std::cout << "Capabilities" << '\n';
    },
};

wl_compositor compositor(0);
xdg_wm_base* wm_base;
wl_shm* shm;

void on_global_registered(wl_registry& registry, const WaylandGlobal& global) {
    //std::cout << "Global: " << global.interface << '\n';

    if (global.interface.Compare("wl_compositor") == 0) {
        const wl_new_id id = wl_id_assigner.get_id();
        registry.bind(global.name, global.interface, global.version, id);
        compositor = wl_compositor(id);
    } else if (global.interface.Compare("wl_shm") == 0) {
        const wl_new_id id = wl_id_assigner.get_id();
        registry.bind(global.name, global.interface, global.version, id);
        shm = new wl_shm(id);
        wl_id_map.set(id, shm);
    } else if (global.interface.Compare("xdg_wm_base") == 0) {
        const wl_new_id id = wl_id_assigner.get_id();
        registry.bind(global.name, global.interface, global.version, id);
        wm_base = new xdg_wm_base(id);
        wl_id_map.set(id, wm_base);
    }
}

struct wl_registry::listener registry_listener {
    .global = on_global_registered,
};

struct wl_shm::listener wl_shm_listener {
    .format = [](const wl_uint format) {
        std::cout << format_to_str((Format)format) << '\n';
    },
};

int main() {

    wl_display display;

    wl_registry& registry = display.get_registry();
    registry.set_listener(&registry_listener);

    // Global population roundtrip (C: Create registry -> S: Announce globals -> C: Bind globals)
    display.roundtrip();

    shm->listener = &wl_shm_listener;

    wl_surface* surface = compositor.create_surface(display.socket);
    wl_id_map.set(surface->id, surface);
    
    xdg_surface* xdg_surface = wm_base->get_xdg_surface(display.socket, *surface);
    wl_id_map.set(xdg_surface->ID(), xdg_surface);
    xdg_surface->listener = &xdg_surface_listener;

    xdg_toplevel* toplevel = xdg_surface->get_toplevel(display.socket);
    wl_id_map.set(toplevel->ID(), toplevel);
    toplevel->listener = &xdg_toplevel_listener;

    //display.dispatch_pending();

    int width = 200;
    int height = 200;
    int stride = width * 4;
    int size = stride * height;

    int fd = syscall(SYS_memfd_create, "buffer", 0);

    if (fd < 0) {
        throw std::runtime_error("Failed to create buffer file");
    }

    ftruncate(fd, size);

    unsigned char* data = (unsigned char*)mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            unsigned char* pixel = data + ((y * width + x) * 4);
            pixel[0] = 0;
            pixel[1] = ((float) x / width) * 255;
            pixel[2] = ((float) y / height) * 255;
            pixel[3] = 255;
        }
    }

    wl_shm_pool pool = shm->create_pool(display.socket, fd, size);
    wl_buffer buffer = pool.create_buffer(display.socket, 0, width, height, stride, 0);

    surface->commit(display.socket);

    while (true) {
        if (has_configured && !has_attached_buffer) {
            surface->attach(display.socket, buffer, 0, 0);
            surface->commit(display.socket);
            has_attached_buffer = true;
        }
        
        std::cout << "Loop\n";
        display.roundtrip();
    }    

    return 0;
}