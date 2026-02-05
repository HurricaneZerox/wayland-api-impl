#pragma once

#include "wl_types.h"
#include "wl_event.h"

class wl_obj {

    public:

    virtual void handle_event(uint16_t opcode, wl_message::reader reader) = 0;

    virtual wl_object ID() const noexcept = 0;
};