#pragma once

#include "../wl_types.h"
#include "../wl_state.h"

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

    void handle_event(uint16_t opcode, void *data, size_t size) override {
        if (!listener) {
            throw std::runtime_error("No event listener supplied for xdg_toplevel");
        }

        if (opcode == EV_CONFIGURE_OPCODE) {
            listener->configure(read_wl_int(data), read_wl_int((char*)data + 4));
        } else if (opcode == EV_CLOSE_OPCODE) {
            listener->close();
        } else if (opcode == EV_CONFIGURE_BOUNDS_OPCODE) {
            listener->configure_bounds(read_wl_int(data), read_wl_int((char*)data + 4));
        } else if (opcode == EV_WM_CAPABILITIES_OPCODE) {
            listener->wm_capabilities();
        }
    }

    void destroy() {
        WaylandMessage client_msg(send_queue_alloc, id, DESTROY_OPCODE, 0);
    }

    void set_parent(const wl_object parent) {
        
    }

    void set_title(const char* title) {
        const wl_string str(strlen(title) + 1, title);

        WaylandMessage client_msg(send_queue_alloc, id, SET_TITLE_OPCODE, str.WordSize() + WL_WORD_SIZE);
        client_msg.Write(str);
    }

    void set_app_id(const wl_string& app_id) {
        
    }

    void set_maximised() {
        WaylandMessage client_msg(send_queue_alloc, id, SET_MAXIMISED_OPCODE, 0);
    }

    void unset_maximised() {
        WaylandMessage client_msg(send_queue_alloc, id, UNSET_MAXIMISED_OPCODE, 0);
    }

    void set_fullscreen(const wl_object output) {
        WaylandMessage client_msg(send_queue_alloc, id, SET_FULLSCREEN_OPCODE, 1);
        client_msg.Write(output);
    }

    void unset_fullscreen() {
        WaylandMessage client_msg(send_queue_alloc, id, UNSET_FULLSCREEN_OPCODE, 0);
    }
};

class xdg_surface : public wl_obj {

    wl_new_id id;
    fd_t socket;

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

    xdg_toplevel* get_toplevel(const fd_t socket) {
        this->socket = socket;
        xdg_toplevel* toplevel = new xdg_toplevel(wl_id_assigner.get_id());
        wl_id_map.create(*toplevel);

        WaylandMessage client_msg(send_queue_alloc, id, 1, 1);
        client_msg.Write(toplevel->ID());

        return toplevel;
    }

    void ack_configure(int serial) {
        WaylandMessage client_msg(send_queue_alloc, id, 4, 1);
        client_msg.Write(serial);
    }

    void handle_event(uint16_t opcode, void *data, size_t size) override {
        if (!listener) {
            throw std::runtime_error("No event listener supplied for xdg_surface");
        }
        
        if (opcode == 0) {
            listener->configure(*this, read_wl_int(data));
        }
    }
};

class xdg_wm_base : public wl_obj {

    wl_object id;

    fd_t socket;

    static constexpr wl_uint DESTROY_OPCODE = 0;
    static constexpr wl_uint CREATE_POSITIONER_OPCODE = 1;
    static constexpr wl_uint GET_XDG_SURFACE_OPCODE = 2;
    static constexpr wl_uint PONG_OPCODE = 3;

    static constexpr wl_uint EV_PING_OPCODE = 0;

    public:

    xdg_wm_base(const wl_new_id id) : id(id) {

    }

    void destroy();

    void create_positioner();

    xdg_surface* get_xdg_surface(const fd_t socket, wl_surface& surface) {
        xdg_surface* x_surface = new xdg_surface(wl_id_assigner.get_id());
        wl_id_map.create(*x_surface);

        WaylandMessage client_msg(send_queue_alloc, id, GET_XDG_SURFACE_OPCODE, 2);
        client_msg.Write(x_surface->ID());
        client_msg.Write(surface.id);

        return x_surface;
    }

    void pong(const wl_uint serial) {
        WaylandMessage client_msg(send_queue_alloc, id, PONG_OPCODE, 1);
        client_msg.Write(serial);
    }

    void handle_event(uint16_t opcode, void *data, size_t size) override {
        if (opcode == EV_PING_OPCODE) {
            if (size != WL_UINT_SIZE) {
                throw std::runtime_error("Server sent invalid event");
            }

            const wl_uint serial = read_wl_uint(data);
            pong(serial);
        }
    }

    wl_object ID() const noexcept override {
        return id;
    }
};