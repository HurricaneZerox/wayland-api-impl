#pragma once

#include "../wl_types.h"
#include "../wl_state.h"

#include "buffer.h"

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
    static constexpr wl_uint SET_BUFFER_SCALE_OPCODE = 8;
    static constexpr wl_uint DAMAGE_BUFFER_OPCODE = 9;
    static constexpr wl_uint OFFSET_OPCODE = 10;

    static constexpr wl_uint EV_ENTER_OPCODE = 0;
    static constexpr wl_uint EV_LEAVE_OPCODE = 1;
    static constexpr wl_uint EV_PREFERRED_BUFFER_SCALE_OPCODE = 2;
    static constexpr wl_uint EV_PREFERRED_BUFFER_TRANSFORM_OPCODE = 3;

    public:

    struct listener {
        void (*enter)();
        void (*leave)();
        void (*preferred_buffer_scale)();
        void (*preferred_buffer_transform)();
    };

    wl_surface(const wl_new_id id) : id(id) {

    }

    void attach(wl_fd_t socket, wl_buffer& buffer, wl_int x, wl_int y) {
        wl_request client_msg(send_queue_alloc, id, ATTACH_OPCODE, 3);
        wl_request::writer writer(client_msg);

        writer.write(buffer.ID());
        writer.write(x);
        writer.write(y);
    }

    void commit(wl_fd_t socket) {
        wl_request client_msg(send_queue_alloc, id, COMMIT_OPCODE, 0);
    }

    void handle_event(uint16_t opcode, wl_message::reader reader) override {

    }

    wl_object ID() const noexcept override {
        return id;
    }
};