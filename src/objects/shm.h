#pragma once

#include "../wl_types.h"
#include "../wl_state.h"
#include "../wl_enums.h"

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

    wl_buffer* create_buffer(wl_fd_t socket, wl_int offset, wl_int width, wl_int height, wl_int stride, Format format) {
        wl_buffer* buffer = new wl_buffer(wl_id_assigner.get_id());

        wl_message client_msg(id, CREATE_BUFFER_OPCODE, 6);
        wl_message::writer writer = client_msg.new_writer(send_queue_alloc);
        
        writer.write(buffer->ID());
        writer.write(offset);
        writer.write(width);
        writer.write(height);
        writer.write(stride);
        writer.write(static_cast<wl_uint>(format));

        return buffer;
    }

    void destroy() {
        wl_message client_msg(id, DESTROY_OPCODE, 0);
        wl_message::writer writer = client_msg.new_writer(send_queue_alloc);
    }

    /**
        Remaps the backing server-side memory for the pool
        with size @p bytes. This function can only be used
        to make the pool bigger. It is the clients responsibility
        to keep the backing file (pointed to by fd) as the correct
        size.
    */
    void resize(wl_int bytes) {
        wl_message client_msg(id, RESIZE_OPCODE, 1);
        wl_message::writer writer = client_msg.new_writer(send_queue_alloc);

        writer.write(bytes);
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

    wl_shm_pool* create_pool(const wl_fd_t socket, wl_fd_t fd, size_t size) {
        wl_shm_pool* pool = new wl_shm_pool(wl_id_assigner.get_id());

        wl_message client_msg(id, 0, 2);
        wl_message::writer writer = client_msg.new_writer(send_queue_alloc);
        
        writer.write(pool->ID());
        writer.write(size);

        send_queue.SetAncillary(fd);

        return pool;
    }

    void handle_event(uint16_t opcode, wl_message::reader reader) override {
        if (!listener) {
            throw std::runtime_error("No listener supplied for wl_shm.");
        }

        if (opcode == 0) {
            listener->format(reader.read_uint());
        }
    }
};