#pragma once

#include <filesystem>
#include <sys/socket.h>
#include <sys/un.h>

#include "../wl_types.h"
#include "../wl_state.h"

#include "../lumber.h"


inline std::filesystem::path get_wayland_socket_path() {
    const std::filesystem::path parent_path = getenv("XDG_RUNTIME_DIR");
    const std::filesystem::path filename = getenv("WAYLAND_DISPLAY");
    return parent_path / filename;
}

inline fd_t create_wayland_socket() {
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

class wl_registry;
wl_registry* create_wl_registry(const wl_new_id id);

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
        WaylandMessage client_msg(send_queue_alloc, DISPLAY_OBJ_ID, GET_REGISTRY_OPCODE, 1);
        client_msg.Write(registry_id);

        wl_registry* registry = create_wl_registry(registry_id);

        wl_id_map.create(*reinterpret_cast<wl_obj*>(registry));

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
                wl_id_assigner.destroy_id(id);
                wl_id_map.destroy(id);
                continue;
            }

            std::shared_ptr<wl_obj*> object = wl_id_map.get(ev.object_id);

            if (!object) {
                std::string warning_msg = "[Wayland::WARN]: Received event for non-existent object. (id: ";
                warning_msg += std::to_string(ev.object_id) + ")";
                lumber::warn(warning_msg.c_str());
                continue;
            }

            (*object)->handle_event(ev.opcode, ev.payload, ev.size - WL_EVENT_HEADER_SIZE);
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
        return 0;
    }

    void roundtrip() {
        dispatch_pending();
        read_events();
    }
};