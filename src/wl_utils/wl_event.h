#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>

#include "../lumber.h"
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

    wl_message(wl_object object_id, wl_uint16 opcode, wl_uint16 size);
    wl_message(wl_object object_id, wl_uint16 opcode, wl_uint16 size, char* payload);

    class reader;
    class writer;

    writer new_writer(void*(*allocator)(size_t));
};

class wl_message::reader {

    const value_ptr data;
    const size_type size = 0;
    value_ptr cursor;

    void advance_cursor(const wl_uint bytes);

    public:

    reader(const value_ptr data, const size_type payload_size);

    wl_uint read_uint();

    wl_int read_int();

    wl_fixed read_fixed();

    wl_object read_object();

    wl_string read_string();

};

class wl_message::writer {

    const wl_uint16 size = 0;
    const char* data = nullptr;
    char* cursor = nullptr;

    void advance_cursor(const wl_uint bytes);

    public:

    writer(const wl_message& request, char* data);

    void write(const wl_uint val);

    void write(const char* string);

    void write(const wl_string& string);
};