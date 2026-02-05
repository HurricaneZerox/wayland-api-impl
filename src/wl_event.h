#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>

#include "lumber.h"
#include "wl_types.h"
#include "wl_string.h"

/**
    @brief Represents a message or event sent between
    the client and server.

    wl_messages are created by buffers and
    therefore don't own the data they point to.
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

    wl_message(wl_object object_id, wl_uint16 opcode, wl_uint16 size) : object_id(object_id), opcode(opcode), size((size * WL_WORD_SIZE) + WL_EVENT_HEADER_SIZE) {}
    wl_message(wl_object object_id, wl_uint16 opcode, wl_uint16 size, char* payload) : wl_message(object_id, opcode, size) {
        this->payload = payload;
    }

    class reader;
    class writer;

    writer new_writer(void*(*allocator)(size_t));
};

class wl_message::reader {

    const value_ptr data;
    const size_type size = 0;
    value_ptr cursor;

    void advance_cursor(const wl_uint bytes) {
        cursor += bytes;
        if (cursor - data > size) {
            lumber::err("[Wayland::ERR]: Exceeded message payload boundary.");
        }
    }

    public:

    reader(const value_ptr data, const size_type payload_size) : data(data), size(payload_size), cursor(data) {}

    wl_uint read_uint() {
        wl_uint value = read_wl_uint(cursor);
        advance_cursor(WL_UINT_SIZE);
        return value;
    }

    wl_int read_int() {
        const wl_int value = read_wl_int(cursor);
        advance_cursor(WL_INT_SIZE);
        return value;
    }

    wl_fixed read_fixed() {
        wl_fixed value = read_wl_fixed(cursor);
        advance_cursor(WL_FIXED_SIZE);
        return value;
    }

    wl_object read_object() {
        wl_object value = read_wl_uint(cursor);
        advance_cursor(WL_OBJECT_SIZE);
        return value;
    }

    wl_string read_string() {
        wl_string value(cursor);
        advance_cursor(value.SerialisedSize());
        return value;
    }

};

class wl_message::writer {

    const wl_uint16 size = 0;
    const char* data = nullptr;
    char* cursor = nullptr;

    void advance_cursor(const wl_uint bytes) {
        cursor += bytes;
        if (cursor - data > size) {
            lumber::err("[Wayland::ERR]: Exceeded message payload boundary.");
        }
    }

    public:

    writer(const wl_message& request, char* data) : size(request.size), data(data), cursor(data + WL_EVENT_HEADER_SIZE) {

        if (!data) {
            lumber::err("Attempt to create writer from nullptr");
            exit(1);
        }

        if (!cursor) {
            lumber::err("Cursor was nullptr");
            exit(1);
        }
    }

    void write(const wl_uint val) {
        from_uint(val, cursor);
        advance_cursor(WL_UINT_SIZE);
    }

    void write(const char* string) {
        const wl_uint str_len = strlen(string) + 1;
        const wl_uint padded_str_len = wl_align(string);

        from_uint(str_len, cursor);
        memcpy(cursor + WL_UINT_SIZE, string, str_len);
        
        advance_cursor(wl_align(str_len + WL_UINT_SIZE));
    }

    void write(const wl_string& string) {
        const wl_uint str_len = string.Size();
        const wl_uint padded_str_len = string.SerialisedSize();

        from_uint(str_len, cursor);
        memcpy(cursor + WL_UINT_SIZE, string, str_len);
        
        advance_cursor(string.SerialisedSize());
    }
};

inline wl_message::writer wl_message::new_writer(void*(*allocator)(size_t bytes)) {
    char* writeout = (char*)allocator(size);

    from_uint(object_id, writeout);
    from_uint(((size << (sizeof(wl_uint16) * 8)) | opcode), writeout + WL_OBJECT_SIZE);
    return writer(*this, writeout);
}