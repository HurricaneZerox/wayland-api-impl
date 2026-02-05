#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>

#include "wl_types.h"
#include "wl_string.h"

#define WL_EVENT_HEADER_SIZE 2 * WL_WORD_SIZE

/**
    @brief Represents a message or event sent between
    the client and server.
*/
struct wl_message {
    using value_type = char;
    using value_ptr = char*;
    using difference_type = wl_uint16;
    using size_type = wl_uint16;
    
    const wl_object object_id;
    const size_type size;
    const wl_opcode_t opcode;

    value_ptr payload = nullptr;
};

class wl_request {
    uint16_t size;
    uint16_t offset = 0;
    char* data;

    char* Payload() const {
        return data + WL_EVENT_HEADER_SIZE;
    }

    public:

    wl_request(void*(*allocator)(size_t), uint32_t object_id, const uint16_t opcode, const uint16_t msg_size)
      : size((msg_size * WL_WORD_SIZE) + WL_EVENT_HEADER_SIZE), data(static_cast<char*>(allocator(size))) {
        from_uint(object_id, data);
        from_uint(((size << (sizeof(uint16_t) * 8)) | opcode), this->data + WL_WORD_SIZE);
    }

    size_t Size() const noexcept {
        return size;
    }

    char* Data() const {
        return data;
    }

    void Write(wl_uint val) {
        if (offset >= size) {
            throw std::runtime_error("Exceeded message boundaries");
        }

        from_uint(val, Payload() + offset);
        offset += WL_WORD_SIZE;
    }

    void Write(const char* string) {
        if (offset >= size) {
            throw std::runtime_error("Exceeded message boundaries");
        }

        const wl_uint str_len = strlen(string) + 1;
        const wl_uint padded_str_len = wl_align(string);

        from_uint(str_len, Payload() + offset);
        memcpy(Payload() + offset + WL_UINT_SIZE, string, str_len);
        
        offset = wl_align(offset + str_len + WL_UINT_SIZE);
    }

    void Write(const wl_string& string) {
        if (offset >= size) {
            throw std::runtime_error("Exceeded message boundaries");
        }

        const wl_uint str_len = string.Size();
        const wl_uint padded_str_len = string.SerialisedSize();

        from_uint(str_len, Payload() + offset);
        memcpy(Payload() + offset + WL_UINT_SIZE, string, str_len);
        
        offset = wl_align(offset + str_len + WL_UINT_SIZE);
    }
};