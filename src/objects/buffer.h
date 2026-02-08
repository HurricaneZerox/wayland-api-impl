#pragma once

#include "../wl_utils/wl_types.h"
#include "../wl_utils/wl_state.h"

class wl_buffer : public wl_obj {
    wl_object id;
    bool is_invalid = false;

    static constexpr wl_uint DESTROY_OPCODE = 0;

    public:

    wl_buffer(const wl_new_id id) : id(id) {
        
    }

    wl_object ID() const noexcept override {
        return id;
    }

    void destroy() {
        wl_message client_msg(id, DESTROY_OPCODE, 0);
        wl_message::writer writer = client_msg.new_writer(send_queue_alloc);
        is_invalid = true;
    }

    void handle_event(uint16_t opcode, wl_message::reader reader) override {
        
    }
};