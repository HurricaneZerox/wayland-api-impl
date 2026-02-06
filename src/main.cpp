#include <cstddef>
#include <cstdint>
#include <cstring>

#include <stdexcept>
#include <sys/syscall.h>
#include <unistd.h>
#include <syscall.h>
#include <sys/mman.h>
#include <sys/un.h>

#include "wl_enums.h"
#include "wl_event.h"
#include "wl_types.h"
#include "wl_string.h"
#include "wl_state.h"

#include "objects/display.h"
#include "objects/registry.h"
#include "objects/input.h"
#include "objects/xdg.h"
#include "objects/compositor.h"
#include "objects/shm.h"

struct BMPImage {

};

/**
    Use-case example
*/

wl_display display;
wl_surface* surface;
wl_shm* shm;
wl_seat* seat;

int resizes = 0;

wl_buffer* create_buffer(wl_shm_pool& pool, wl_int width, wl_int height) {
    wl_buffer* buffer = pool.create_buffer(display.socket, 0, width, height, width * 4, Format::ARGB8888);
    wl_id_map.create(*buffer);
    surface->commit(display.socket);
    return buffer;
}

void destroy_shared_memory_fd(const int fd) {
    if (fd != -1) {
        close(fd);
    }
}

int create_shared_memory_fd(const size_t size) {
    const int fd = syscall(SYS_memfd_create, "buffer", 0);

    if (fd < 0) {
        throw std::runtime_error("Failed to create buffer file");
    }

    ftruncate(fd, size);
    return fd;
}

struct Framebuffer {
    unsigned char* data;
    int fd;
    wl_shm_pool* pool;
    wl_buffer* buffer;

    bool has_acked_config = false;
    bool has_surface_resized = false;

    Framebuffer() {}

    void Create(const size_t width, const size_t height) {
        int stride = width * 4;
        int size = stride * height;

        fd = create_shared_memory_fd(size);
        data = (unsigned char*)mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

        for (int x = 0; x < width; x++) {
            for (int y = 0; y < height; y++) {
                unsigned char* pixel = data + ((y * width + x) * 4);
                pixel[0] = ((float) x / width) * 255;
                pixel[1] = ((float) y / height) * 255;
                pixel[2] = 0;
                pixel[3] = 255;
            }
        }

        pool = shm->create_pool(display.socket, fd, size);
        buffer = create_buffer(*pool, width, height);
    }

    Framebuffer(const size_t width, const size_t height) {
        Create(width, height);
    }

    void Attach() {
        surface->attach(display.socket, *buffer, 0, 0);
        surface->commit(display.socket);
    }

    void Resize(const size_t width, const size_t height) {
        if (buffer) {
            buffer->destroy();
            buffer = nullptr;
        }

        if (pool) {
            pool->destroy();
            pool = nullptr;
        }

        destroy_shared_memory_fd(fd);

        Create(width, height);

        display.dispatch_pending();
    }
};

Framebuffer framebuffer;

bool has_configured = false;
bool has_resized_buffer = false;
bool should_close = false;

wl_int screen_width = 200;
wl_int screen_height = 200;

struct xdg_surface::listener xdg_surface_listener {
    .configure = [](xdg_surface& surface, int serial) {
        surface.ack_configure(serial);
        framebuffer.Attach();
    }
};

struct xdg_toplevel::listener xdg_toplevel_listener {
    .configure = [](const wl_int x, const wl_int y) {
        if ((wl_uint)x == 0 || (wl_uint)y == 0) { return; }

        screen_width = x;
        screen_height = y;

        framebuffer.Resize(x, y);
    },
    .close = []() {
        std::cout << "Close" << '\n';
        should_close = true;
    },
    .configure_bounds = [](int x, int y) {
        std::cout << "Configure bounds: " << x << ", " << y << '\n';
    },
    .wm_capabilities = []() {
        std::cout << "Capabilities" << '\n';
    },
};

struct wl_seat::listener wl_seat_listener {
    
};

struct wl_pointer::listener wl_mouse_listener {
    .enter = [](wl_uint serial, wl_object surface, wl_fixed surface_x, wl_fixed surface_y) {
        std::cout << "Mouse entered: " << "surface_x: " << surface_x << ", " << "surface_y: " << surface_y << '\n';
    },
    .leave = [](wl_uint serial, wl_object surface) {
        std::cout << "Mouse left" << '\n';
    },
    .motion = [](wl_uint serial, wl_fixed surface_x, wl_fixed surface_y) {
        //std::cout << "Mouse moved: " << "surface_x: " << surface_x << ", " << "surface_y: " << surface_y << '\n';
    },
    .button = [](wl_uint serial, wl_uint time, wl_uint button, enum wl_pointer::button_state state) {
        std::cout << "Mouse clicked: " << "button: " << button << ", " << "state: " << (wl_uint)state << '\n';
    },
    .axis = [](wl_uint time, enum wl_pointer::axis axis, wl_fixed value) {
        std::cout << "Mouse axis: " << "axis: " << (wl_uint)axis << ", " << "value: " << value << '\n';
    },
    .frame = []() {
        //std::cout << "Mouse frame" << '\n';
    },
    .axis_source = [](enum wl_pointer::axis_source source) {
        //std::cout << "Mouse axis source" << '\n';
    },
    .axis_discrete = [](enum wl_pointer::axis axis, const wl_int discrete) {
        //std::cout << "Mouse axis discrete: " << discrete << '\n';
    },
    .axis_value120 = [](enum wl_pointer::axis axis, const wl_int value120) {
        //std::cout << "Mouse axis value120: " << value120 << '\n';
    },
    .axis_relative_direction = [](enum wl_pointer::axis axis, enum wl_pointer::axis_relative_direction relative_direction) {
        //std::cout << "Mouse axis relative direction: " << (wl_uint)relative_direction << '\n';
    },
};

wl_compositor compositor(0);
xdg_wm_base* wm_base;

void on_global_registered(wl_registry& registry, const wl_uint name, const wl_string& interface, const wl_uint version) {

    if (interface.Compare("wl_shell_surface") == 0) {
        std::cout << "SUPPORTS OLD SHELL INTERFACE\n";
    }

    if (interface.Compare("wl_compositor") == 0) {
        const wl_new_id id = wl_id_assigner.get_id();
        registry.bind(name, interface, version, id);
        compositor = wl_compositor(id);
    } else if (interface.Compare("wl_shm") == 0) {
        const wl_new_id id = wl_id_assigner.get_id();
        registry.bind(name, interface, version, id);
        shm = new wl_shm(id);
        wl_id_map.create(*shm);
    } else if (interface.Compare("xdg_wm_base") == 0) {
        const wl_new_id id = wl_id_assigner.get_id();
        registry.bind(name, interface, version, id);
        wm_base = new xdg_wm_base(id);
        wl_id_map.create(*wm_base);
    } else if (interface.Compare("wl_seat") == 0) {
        const wl_new_id id = wl_id_assigner.get_id();
        registry.bind(name, interface, version, id);
        seat = new wl_seat(id);
        wl_id_map.create(*seat);
    } else if (interface.Compare("xdg_toplvel_icon_manager_v1") == 0) {
        std::cout << "toplevel icon support\n";
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

    struct wl_fixed_t fixed_n(2.5f);
    std::cout << fixed_n.flot() << '\n';

    wl_registry& registry = display.get_registry();
    registry.set_listener(&registry_listener);

    display.roundtrip();

    shm->listener = &wl_shm_listener;
    seat->listener = &wl_seat_listener;

    surface = compositor.create_surface(display.socket);
    
    xdg_surface* xdg_surface = wm_base->get_xdg_surface(display.socket, *surface);
    xdg_surface->listener = &xdg_surface_listener;

    xdg_toplevel* toplevel = xdg_surface->get_toplevel(display.socket);
    toplevel->listener = &xdg_toplevel_listener;

    toplevel->set_title("Test Application");
    toplevel->set_maximised();

    display.dispatch_pending();

    wl_pointer* mouse = seat->get_mouse();
    mouse->listener = &wl_mouse_listener;
    
    framebuffer = Framebuffer(200, 200);

    while (!should_close) {
        display.roundtrip();
    }

    return 0;
}