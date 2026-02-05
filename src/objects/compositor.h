#pragma once

#include "../wl_types.h"
#include "../wl_state.h"

#include "surface.h"

class wl_compositor {

    wl_new_id id;

    public:

    wl_compositor(const wl_new_id id) : id(id) {

    }

    wl_surface* create_surface(const wl_fd_t socket) {
        wl_surface* surface = new wl_surface(wl_id_assigner.get_id());
        wl_id_map.create(*surface);

        wl_request client_msg(send_queue_alloc, id, 0, 1);
        wl_request::writer writer(client_msg);

        writer.write(surface->id);

        return surface;
    }

    void create_region(const wl_fd_t socket) {

    }
};