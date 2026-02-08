#pragma once

#include "../wl_utils/wl_types.h"
#include "../wl_utils/wl_state.h"

#include "surface.h"

class wl_compositor {

    wl_new_id id;

    public:

    wl_compositor(const wl_new_id id) : id(id) {

    }

    wl_surface* create_surface(const wl_fd_t socket) {
        wl_surface* surface = new wl_surface(wl_id_assigner.request_id());
        wl_id_map.create(*surface);

        wl_message client_msg(id, 0, 1);
        wl_message::writer writer = client_msg.new_writer(send_queue_alloc);

        writer.write(surface->id);

        return surface;
    }

    void create_region(const wl_fd_t socket) {

    }
};