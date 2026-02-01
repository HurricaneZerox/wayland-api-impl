#pragma once

#include "../wl_types.h"
#include "../wl_state.h"

/**
    @brief Keyboard
*/
class wl_keyboard : public wl_obj {
    wl_object id;

    public:

    struct listener {
        
    };

    listener* listener = nullptr;

    wl_keyboard(const wl_new_id id) : id(id) {

    }

    wl_object ID() const noexcept override {
        return id;
    }

    void handle_event(uint16_t opcode, void *data, size_t size) override {
        if (!listener) {
            throw std::runtime_error("No listener supplied for wl_seat.");
        }
    }
};

/**
    @brief Mouse
*/
class wl_pointer : public wl_obj {
    wl_object id;

    static constexpr wl_uint EV_ENTER_OPCODE = 0;
    static constexpr wl_uint EV_LEAVE_OPCODE = 1;
    static constexpr wl_uint EV_MOTION_OPCODE = 2;
    static constexpr wl_uint EV_BUTTON_OPCODE = 3;
    static constexpr wl_uint EV_AXIS_OPCODE = 4;
    static constexpr wl_uint EV_FRAME_OPCODE = 5;
    static constexpr wl_uint EV_AXIS_SOURCE_OPCODE = 6;
    static constexpr wl_uint EV_AXIS_STOP_OPCODE = 7;
    static constexpr wl_uint EV_AXIS_DISCRETE_OPCODE = 8;
    static constexpr wl_uint EV_AXIS_VALUE120_OPCODE = 9;
    static constexpr wl_uint EV_AXIS_RELATIVE_DIRECTION_OPCODE = 10;

    public:

    enum class button_state : wl_uint {
        released = 0,
        pressed = 1,
    };

    enum class axis : wl_uint {
        vertical_scroll = 0,
        horizontal_scroll = 1,
    };

    enum class axis_source : wl_uint {
        wheel = 0,
        finger = 1,
        continuous = 2,
        wheel_tilt = 3,
    };

    enum class axis_relative_direction : wl_uint {
        identical = 0,
        inverted = 1,
    };

    struct listener {
        void (*enter)(wl_uint serial, wl_object surface, wl_fixed surface_x, wl_fixed surface_y);
        void (*leave)(wl_uint serial, wl_object surface);
        void (*motion)(wl_uint serial, wl_fixed surface_x, wl_fixed surface_y);
        void (*button)(wl_uint serial, wl_uint time, wl_uint button, enum button_state state);
        void (*axis)(wl_uint time, enum axis axis, wl_uint value);
        void (*frame)();
        void (*axis_source)(enum axis_source axis_source);
        void (*axis_stop)(wl_uint serial, enum axis axis);
        void (*axis_discrete)(enum axis axis, wl_int discrete);
        void (*axis_value120)(enum axis axis, wl_int value120);
        void (*axis_relative_direction)(enum axis axis, enum axis_relative_direction direction);
    };

    listener* listener = nullptr;

    wl_pointer(const wl_new_id id) : id(id) {

    }

    wl_object ID() const noexcept override {
        return id;
    }

    void handle_event(uint16_t opcode, void *data, size_t size) override {
        if (!listener) {
            throw std::runtime_error("No listener supplied for wl_mouse.");
        }

        if (opcode == EV_ENTER_OPCODE) {
            listener->enter(
                read_wl_uint(data),
                read_wl_object((char*)data + 4),
                read_wl_fixed((char*)data + 8),
                read_wl_fixed((char*)data + 12)
            );
        } else if (opcode == EV_LEAVE_OPCODE) {
            listener->leave(read_wl_uint(data), read_wl_object((char*)data + 4));
        } else if (opcode == EV_MOTION_OPCODE) {
            listener->motion(read_wl_uint(data), read_wl_fixed((char*)data + 4), read_wl_fixed((char*)data + 8));
        } else if (opcode == EV_BUTTON_OPCODE) {
            listener->button(read_wl_uint(data), read_wl_uint((char*)data + 4), read_wl_uint((char*)data + 8), (button_state)read_wl_uint((char*)data + 12));
        }
    }
};

/**
    @brief Group of input devices
*/
class wl_seat : public wl_obj {
    wl_object id;

    public:

    struct listener {
        
    };

    listener* listener = nullptr;

    wl_seat(const wl_new_id id) : id(id) {

    }

    wl_object ID() const noexcept override {
        return id;
    }

    void handle_event(uint16_t opcode, void *data, size_t size) override {
        if (!listener) {
            throw std::runtime_error("No listener supplied for wl_seat.");
        }
    }

    wl_pointer* get_mouse() {
        wl_pointer* mouse = new wl_pointer(wl_id_assigner.get_id());

        WaylandMessage client_msg(send_queue_alloc, id, 0, 1);
        client_msg.Write(mouse->ID());

        wl_id_map.create(*mouse);

        return mouse;
    }

    void get_keyboard() {
        
    }

    void release() {
        
    }
};