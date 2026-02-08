#pragma once

#include "wl_id.h"
#include "../buffers/queue.h"

inline wl_id_assigner wl_id_assigner;
inline wl_id_map wl_id_map;

inline wl::recv_queue recv_queue;
inline wl::send_queue send_queue;

inline void* send_queue_alloc(size_t bytes) {
    return send_queue.Allocate(bytes);
}