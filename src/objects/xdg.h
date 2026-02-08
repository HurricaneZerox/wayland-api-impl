#pragma once

#include "../wl_utils/wl_types.h"
#include "../wl_utils/wl_state.h"

#include "surface.h"

class xdg_toplevel : public wl_obj {

    wl_object id;

    static constexpr wl_uint DESTROY_OPCODE = 0;
    static constexpr wl_uint SET_PARENT_OPCODE = 1;
    static constexpr wl_uint SET_TITLE_OPCODE = 2;
    static constexpr wl_uint SET_APP_ID_OPCODE = 3;
    static constexpr wl_uint SHOW_WINDOW_MENU_OPCODE = 4;
    static constexpr wl_uint MOVE_OPCODE = 5;
    static constexpr wl_uint RESIZE_OPCODE = 6;
    static constexpr wl_uint SET_MAX_SIZE_OPCODE = 7;
    static constexpr wl_uint SET_MIN_SIZE_OPCODE = 8;
    static constexpr wl_uint SET_MAXIMISED_OPCODE = 9;
    static constexpr wl_uint UNSET_MAXIMISED_OPCODE = 10;
    static constexpr wl_uint SET_FULLSCREEN_OPCODE = 11;
    static constexpr wl_uint UNSET_FULLSCREEN_OPCODE = 12;
    static constexpr wl_uint SET_MINIMISED_OPCODE = 13;

    static constexpr wl_uint EV_CONFIGURE_OPCODE = 0;
    static constexpr wl_uint EV_CLOSE_OPCODE = 1;
    static constexpr wl_uint EV_CONFIGURE_BOUNDS_OPCODE = 2;
    static constexpr wl_uint EV_WM_CAPABILITIES_OPCODE = 3;

    public:

    struct listener {
        void (*configure)(int width, int height);
        void (*close)();
        void (*configure_bounds)(int width, int height);
        void (*wm_capabilities)();
    };

    listener* listener;

    xdg_toplevel(const wl_new_id id) : id(id) {}

    wl_object ID() const noexcept override {
        return id;
    }

    void handle_event(uint16_t opcode, wl_message::reader reader) override {
        if (!listener) {
            throw std::runtime_error("No event listener supplied for xdg_toplevel");
        }

        if (opcode == EV_CONFIGURE_OPCODE) {
            const wl_int width = reader.read_int();
            const wl_int height = reader.read_int();

            listener->configure(width, height);
        } else if (opcode == EV_CLOSE_OPCODE) {
            listener->close();
        } else if (opcode == EV_CONFIGURE_BOUNDS_OPCODE) {
            const wl_int width = reader.read_int();
            const wl_int height = reader.read_int();

            listener->configure_bounds(width, height);
        } else if (opcode == EV_WM_CAPABILITIES_OPCODE) {
            listener->wm_capabilities();
        }
    }

    void destroy() {
        wl_message client_msg(id, DESTROY_OPCODE, 0);
        wl_message::writer writer = client_msg.new_writer(send_queue_alloc);
    }

    void set_parent(const wl_object parent) {
        
    }

    void set_title(const char* title) {
        const wl_string str(strlen(title) + 1, title);

        wl_message client_msg(id, SET_TITLE_OPCODE, str.word_size() + WL_WORD_SIZE);
        wl_message::writer writer = client_msg.new_writer(send_queue_alloc);

        writer.write(str);
    }

    void set_app_id(const wl_string& app_id) {
        
    }

    void set_maximised() {
        wl_message client_msg(id, SET_MAXIMISED_OPCODE, 0);
        wl_message::writer writer = client_msg.new_writer(send_queue_alloc);
    }

    void unset_maximised() {
        wl_message client_msg(id, UNSET_MAXIMISED_OPCODE, 0);
        wl_message::writer writer = client_msg.new_writer(send_queue_alloc);
    }

    void set_fullscreen(const wl_object output) {
        wl_message client_msg(id, SET_FULLSCREEN_OPCODE, 1);
        wl_message::writer writer = client_msg.new_writer(send_queue_alloc);

        writer.write(output);
    }

    void unset_fullscreen() {
        wl_message client_msg(id, UNSET_FULLSCREEN_OPCODE, 0);
        wl_message::writer writer = client_msg.new_writer(send_queue_alloc);
        
    }
};

class xdg_positioner : public wl_obj {
    wl_object id;
    wl_fd_t socket;

    public:

    xdg_positioner(const wl_new_id id, wl_fd_t socket) : id(id), socket(socket) {}

    void handle_event(uint16_t opcode, wl_message::reader reader) override {

    }

    wl_object ID() const noexcept override {
        return id;
    }
};

class xdg_surface : public wl_obj {

    wl_new_id id;
    wl_fd_t socket;

    public:
    
    struct listener {
        void (*configure)(xdg_surface& surface, int serial);
    };

    listener* listener;

    xdg_surface(const wl_new_id id) : id(id) {

    }

    wl_object ID() const noexcept override {
        return id;
    }

    void destroy();

    xdg_toplevel& get_toplevel(const wl_fd_t socket) {
        this->socket = socket;
        xdg_toplevel* toplevel = new xdg_toplevel(wl_id_assigner.request_id());
        wl_id_map.create(*toplevel);

        wl_message client_msg(id, 1, 1);
        wl_message::writer writer = client_msg.new_writer(send_queue_alloc);
        
        writer.write(toplevel->ID());

        return *toplevel;
    }

    void get_popup() {

    }

    void ack_configure(int serial) {
        wl_message client_msg(id, 4, 1);
        wl_message::writer writer = client_msg.new_writer(send_queue_alloc);

        writer.write(serial);
    }

    void handle_event(uint16_t opcode, wl_message::reader reader) override {
        if (!listener) {
            throw std::runtime_error("No event listener supplied for xdg_surface");
        }
        
        if (opcode == 0) {
            listener->configure(*this, reader.read_uint());
        }
    }
};

class xdg_wm_base : public wl_obj {
    const wl_object id;
    wl_fd_t socket;

    static constexpr wl_uint DESTROY_OPCODE = 0;
    static constexpr wl_uint CREATE_POSITIONER_OPCODE = 1;
    static constexpr wl_uint GET_XDG_SURFACE_OPCODE = 2;
    static constexpr wl_uint PONG_OPCODE = 3;

    static constexpr wl_uint EV_PING_OPCODE = 0;

    public:

    xdg_wm_base(const wl_new_id id) : id(id) {}

    void destroy();

    xdg_positioner& create_positioner() {
        xdg_positioner* positioner = new xdg_positioner(wl_id_assigner.request_id(), socket);
        wl_id_map.create(*positioner);

        wl_message client_msg(id, CREATE_POSITIONER_OPCODE, 1);
        wl_message::writer writer = client_msg.new_writer(send_queue_alloc);

        writer.write(positioner->ID());

        return *positioner;
    }

    xdg_surface& get_xdg_surface(const wl_fd_t socket, wl_surface& surface) {
        xdg_surface* x_surface = new xdg_surface(wl_id_assigner.request_id());
        wl_id_map.create(*x_surface);

        wl_message client_msg(id, GET_XDG_SURFACE_OPCODE, 2);
        wl_message::writer writer = client_msg.new_writer(send_queue_alloc);

        writer.write(x_surface->ID());
        writer.write(surface.id);

        return *x_surface;
    }

    void pong(const wl_uint serial) {
        wl_message client_msg(id, PONG_OPCODE, 1);
        wl_message::writer writer = client_msg.new_writer(send_queue_alloc);

        writer.write(serial);
    }

    void handle_event(uint16_t opcode, wl_message::reader reader) override {

        if (opcode == EV_PING_OPCODE) {
            const wl_uint serial = reader.read_uint();
            pong(serial);
        }
    }

    wl_object ID() const noexcept override {
        return id;
    }
};