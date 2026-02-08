#pragma once

#include <filesystem>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/un.h>

#include "../wl_utils/wl_types.h"
#include "../wl_utils/wl_state.h"

#include "../lumber.h"



inline std::filesystem::path get_wayland_socket_path() {
    const std::filesystem::path parent_path = getenv("XDG_RUNTIME_DIR");
    const std::filesystem::path filename = getenv("WAYLAND_DISPLAY");
    return parent_path / filename;
}

inline wl_fd_t create_wayland_socket() {
    const wl_fd_t handle = socket(AF_UNIX, SOCK_STREAM, 0);

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

/**
    @brief Represents a connection to the Wayland compositor.

    All requests, events, and enums have been implemented
    apart from `sync`.

    @warning This should be used only by Wayland clients.
*/
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

    wl_fd_t socket;

    wl_display() : socket(create_wayland_socket()) {}

    wl_registry& get_registry() {
        const wl_new_id registry_id = wl_id_assigner.request_id();

        wl_message client_msg(DISPLAY_OBJ_ID, GET_REGISTRY_OPCODE, 1);
        wl_message::writer writer = client_msg.new_writer(send_queue_alloc);
        
        writer.write(registry_id);

        wl_registry* registry = create_wl_registry(registry_id);

        wl_id_map.create(*reinterpret_cast<wl_obj*>(registry));

        return *registry;
    }

    void sync() {

    }

    /**
        @brief Reads messages on all non-empty recv 
        queues.
    */
    void read_queues() {
        recv_queue.Recv(socket);

        for (const wl_message msg : recv_queue) {
            if (msg.object_id == NULL_OBJ_ID) {
                lumber::err("[Wayland::ERR]: Event was dispatched to null object.");
                exit(1);
            }

            if (msg.object_id == DISPLAY_OBJ_ID && msg.opcode == EV_ERROR_OPCODE) {
                wl_object err_object_id = read_wl_object(msg.payload);
                wl_uint err_opcode = read_wl_uint(msg.payload + 4);
                wl_string err_msg(msg.payload + 8);

                std::string output_msg("[Wayland::ERR]: Ran into an error:\n");
                output_msg += "\tMessage: " + std::string(err_msg);
                
                lumber::err(output_msg.c_str());
                
                exit(1);
            } else if (msg.object_id == 1 && msg.opcode == EV_DELETE_ID_OPCODE) {
                wl_object id = read_wl_object(msg.payload);
                wl_id_assigner.release_id(id);
                wl_id_map.destroy(id);
                continue;
            }

            std::shared_ptr<wl_obj*> object = wl_id_map.get(msg.object_id);

            if (!object) {
                const std::string warning_msg = "[Wayland::WARN]: Received event for unregistered object. (id: " + std::to_string(msg.object_id) + ")";
                lumber::warn(warning_msg.c_str());
                continue;
            }

            (*object)->handle_event(msg.opcode, wl_message::reader(msg.payload, msg.size - WL_EVENT_HEADER_SIZE));
        }
    }

    /**
        @brief Dispatches messages on the send queue without
        reading from the event queue.

        @returns The number of messages dispatched.
    */
    size_t dispatch_pending() {
        if (!send_queue.Empty()) {
            return send_queue.Send(socket);
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
        read_queues();
    }
};