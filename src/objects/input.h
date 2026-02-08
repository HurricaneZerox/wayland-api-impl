#pragma once

#include "../wl_utils/wl_types.h"
#include "../wl_utils/wl_state.h"
#include "surface.h"

/**
    @brief Keyboard
*/
class wl_keyboard : public wl_obj {
    wl_object id;

    static constexpr wl_uint RELEASE_OPCODE = 0;

    static constexpr wl_uint EV_KEYMAP_OPCODE = 0;
    static constexpr wl_uint EV_ENTER_OPCODE = 1;
    static constexpr wl_uint EV_LEAVE_OPCODE = 2;
    static constexpr wl_uint EV_KEY_OPCODE = 3;
    static constexpr wl_uint EV_MODIFIERS_OPCODE = 4;
    static constexpr wl_uint EV_REPEAT_INFO_OPCODE = 5;

    public:

    enum class key_state : wl_uint {
        released,
        pressed,
        repeated,
    };

    enum class keymap_format : wl_uint {
        no_keymap,
        xkb_v1,
    };

    struct listener {
        
        void (*key)(wl_uint serial, wl_uint time, wl_uint key, key_state state);
    };

    listener* listener = nullptr;

    wl_keyboard(const wl_new_id id) : id(id) {

    }

    wl_object ID() const noexcept override {
        return id;
    }

    void handle_event(uint16_t opcode, wl_message::reader reader) override {
        if (!listener) {
            throw std::runtime_error("No listener supplied for wl_keybard.");
        }

        if (opcode == EV_KEYMAP_OPCODE) {
            //std::cout << "KEYMAP\n";
        } else if (opcode == EV_ENTER_OPCODE) {
            //std::cout << "ENTER\n";
        } else if (opcode == EV_LEAVE_OPCODE) {
            //std::cout << "LEAVE\n";
        } else if (opcode == EV_KEY_OPCODE) {
            const wl_uint serial = reader.read_uint();
            const wl_uint time = reader.read_uint();
            const wl_uint key = reader.read_uint();
            const key_state state = static_cast<key_state>(reader.read_uint());

            listener->key(serial, time, key, state);
        } else if (opcode == EV_MODIFIERS_OPCODE) {
            //std::cout << "MOD\n";
        } else if (opcode == EV_REPEAT_INFO_OPCODE) {
            //std::cout << "REPEAT\n";
        }
    }
};

/**
    @brief Mouse
*/
class wl_pointer : public wl_obj {
    wl_object id;

    static constexpr wl_uint SET_CURSOR_OPCODE = 0;
    static constexpr wl_uint RELEASE_OPCODE = 1;

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
        void (*axis)(wl_uint time, enum axis axis, wl_fixed value);
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

    /**
        @brief Sets the pointer image (cursor).

        If @p surface is `nullptr`, then this request will
        hide the cursor.
    */
    void set_cursor(wl_uint serial, const wl_surface* surface, wl_uint hotspot_x, wl_uint hotspot_y) {
        const wl_object id = surface != nullptr ? surface->ID() : NULL_OBJ_ID;

        wl_message client_msg(this->id, SET_CURSOR_OPCODE, 4);
        wl_message::writer writer = client_msg.new_writer(send_queue_alloc);

        writer.write(serial);
        writer.write(id);
        writer.write(hotspot_x);
        writer.write(hotspot_y);
    }

    void release() {
        wl_message client_msg(this->id, RELEASE_OPCODE, 0);
        wl_message::writer writer = client_msg.new_writer(send_queue_alloc);
    }

    void handle_event(uint16_t opcode, wl_message::reader reader) override {
        if (!listener) {
            throw std::runtime_error("No listener supplied for wl_mouse.");
        }

        if (opcode == EV_ENTER_OPCODE) {
            const wl_uint serial = reader.read_uint();
            const wl_object surface = reader.read_object();
            const wl_fixed surface_x = reader.read_fixed();
            const wl_fixed surface_y = reader.read_fixed();

            listener->enter(serial, surface, surface_x, surface_y);
        } else if (opcode == EV_LEAVE_OPCODE) {
            const wl_uint serial = reader.read_uint();
            const wl_object surface = reader.read_object();

            listener->leave(serial, surface);
        } else if (opcode == EV_MOTION_OPCODE) {
            const wl_uint serial = reader.read_uint();
            const wl_fixed surface_x = reader.read_fixed();
            const wl_fixed surface_y = reader.read_fixed();

            listener->motion(serial, surface_x, surface_y);
        } else if (opcode == EV_BUTTON_OPCODE) {
            const wl_uint serial = reader.read_uint();
            const wl_uint time = reader.read_uint();
            const wl_uint button = reader.read_uint();
            const button_state state = static_cast<button_state>(reader.read_uint());

            listener->button(serial, time, button, state);
        } else if (opcode == EV_AXIS_OPCODE) {
            const wl_uint time = reader.read_uint();
            const enum axis axis = static_cast<enum axis>(reader.read_uint());
            const wl_fixed value = reader.read_fixed();

            listener->axis(time, axis, value);
        } else if (opcode == EV_FRAME_OPCODE) {
            listener->frame();
        } else if (opcode == EV_AXIS_SOURCE_OPCODE) {
            const axis_source source = static_cast<axis_source>(reader.read_uint());

            listener->axis_source(source);
        } else if (opcode == EV_AXIS_DISCRETE_OPCODE) {
            const axis axis = static_cast<enum axis>(reader.read_uint());
            const wl_int discrete = static_cast<wl_int>(reader.read_int());

            listener->axis_discrete(axis, discrete);
        } else if (opcode == EV_AXIS_VALUE120_OPCODE) {
            const axis axis = static_cast<enum axis>(reader.read_uint());
            const wl_int value120 = static_cast<wl_int>(reader.read_int());

            listener->axis_value120(axis, value120);
        } else if (opcode == EV_AXIS_RELATIVE_DIRECTION_OPCODE) {
            const axis axis = static_cast<enum axis>(reader.read_uint());
            const axis_relative_direction relative_direction = static_cast<axis_relative_direction>(reader.read_uint());

            listener->axis_relative_direction(axis, relative_direction);
        }
    }
};

/**
    @brief Group of input devices
*/
class wl_seat : public wl_obj {
    wl_object id;

    static constexpr wl_uint GET_POINTER_OPCODE = 0;
    static constexpr wl_uint GET_KEYBOARD_OPCODE = 1;
    static constexpr wl_uint GET_TOUCH_OPCODE = 2;
    static constexpr wl_uint RELEASE_OPCODE = 3;

    public:

    struct listener {
        
    };

    listener* listener = nullptr;

    wl_seat(const wl_new_id id) : id(id) {

    }

    wl_object ID() const noexcept override {
        return id;
    }

    void handle_event(uint16_t opcode, wl_message::reader reader) override {
        if (!listener) {
            throw std::runtime_error("No listener supplied for wl_seat.");
        }
    }

    wl_pointer* get_mouse() {
        wl_pointer* mouse = new wl_pointer(wl_id_assigner.request_id());

        wl_message client_msg(id, GET_POINTER_OPCODE, 1);
        wl_message::writer writer = client_msg.new_writer(send_queue_alloc);

        writer.write(mouse->ID());

        wl_id_map.create(*mouse);

        return mouse;
    }

    wl_keyboard* get_keyboard() {
        wl_keyboard* keyboard = new wl_keyboard(wl_id_assigner.request_id());

        wl_message client_msg(id, GET_KEYBOARD_OPCODE, 1);
        wl_message::writer writer = client_msg.new_writer(send_queue_alloc);

        writer.write(keyboard->ID());

        wl_id_map.create(*keyboard);

        return keyboard;
    }

    void release() {
        
    }
};