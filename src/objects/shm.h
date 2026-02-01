#pragma once

#include "../wl_types.h"
#include "../wl_state.h"

#include "buffer.h"

class wl_shm_pool {

    wl_object id;

    static constexpr wl_uint CREATE_BUFFER_OPCODE = 0;
    static constexpr wl_uint DESTROY_OPCODE = 1;
    static constexpr wl_uint RESIZE_OPCODE = 2;

    public:

    wl_shm_pool(const wl_new_id id) : id(id) {
        
    }

    wl_object ID() const noexcept {
        return id;
    }

    wl_buffer* create_buffer(fd_t socket, wl_int offset, wl_int width, wl_int height, wl_int stride, wl_uint format) {
        wl_buffer* buffer = new wl_buffer(wl_id_assigner.get_id());

        WaylandMessage client_msg(send_queue_alloc, id, CREATE_BUFFER_OPCODE, 6);
        client_msg.Write(buffer->ID());
        client_msg.Write(offset);
        client_msg.Write(width);
        client_msg.Write(height);
        client_msg.Write(stride);
        client_msg.Write(format);

        return buffer;
    }

    void destroy() {
        WaylandMessage client_msg(send_queue_alloc, id, DESTROY_OPCODE, 0);
    }

    /**
        Remaps the backing server-side memory for the pool
        with size @p bytes. This function can only be used
        to make the pool bigger. It is the clients responsibility
        to keep the backing file (pointed to by fd) as the correct
        size.
    */
    void resize(wl_int bytes) {
        WaylandMessage client_msg(send_queue_alloc, id, RESIZE_OPCODE, 1);
        client_msg.Write(bytes);
    }
};

/**
    @brief Shared memory support.

    Wayland clients and servers use shared memory to display
    an image created by the client as a window shown by the
    server.
*/
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

    wl_shm_pool* create_pool(const fd_t socket, fd_t fd, size_t size) {
        wl_shm_pool* pool = new wl_shm_pool(wl_id_assigner.get_id());

        WaylandMessage client_msg(send_queue_alloc, id, 0, 2);
        client_msg.Write(pool->ID());
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