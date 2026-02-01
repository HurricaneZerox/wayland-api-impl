#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <sys/socket.h>

#include "../wl_event.h"
#include "../wl_types.h"

class EventQueue {
    char* buffer = nullptr;
    size_t current_size = 0;
    size_t max_alloc = 4096;
    uintptr_t offset = 0;

    public:

    /**
        Creates a new receive buffer with
        @p prealloc bytes already allocated.
    */
    EventQueue(const size_t prealloc) : buffer(static_cast<char*>(malloc(prealloc))), max_alloc(prealloc) {
        
    }

    void Recv(const int socket) {
        offset = 0;
        current_size = recv(socket, buffer, max_alloc, 0);
    }

    std::optional<wl_event> PopEvent() {
        if (offset >= current_size) {
            return std::nullopt;
        }

        char* const current = buffer + offset;

        wl_uint size = *reinterpret_cast<const wl_uint16* const>(current + 6);
        offset += size;

        wl_event ev {
            .object_id = read_wl_uint(current),
            .size = size,
            .opcode = *reinterpret_cast<const wl_uint16* const>(current + 4),
            .payload = current + WL_EVENT_HEADER_SIZE,
        };

        return ev;
    }
};