#pragma once

#include "wl_id.h"
#include "buffers/recv.h"
#include "buffers/send.h"

inline wl_id_assigner wl_id_assigner;
inline wl_id_map wl_id_map;

inline EventQueue event_queue(4096);
inline SendQueue send_queue;

inline void* send_queue_alloc(size_t bytes) {
    return send_queue.Allocate(bytes);
}