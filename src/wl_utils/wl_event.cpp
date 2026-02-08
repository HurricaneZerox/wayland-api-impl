#include "wl_event.h"


wl_message::wl_message(
    wl_object object_id, 
    wl_uint16 opcode, 
    wl_uint16 size
) : object_id(object_id), opcode(opcode), size((size * WL_WORD_SIZE) + WL_EVENT_HEADER_SIZE) {}
    
wl_message::wl_message(
    wl_object object_id, 
    wl_uint16 opcode, 
    wl_uint16 size, 
    char* payload
) : wl_message(object_id, opcode, size) {
    this->payload = payload;
}

void wl_message::reader::advance_cursor(const wl_uint bytes) {
    cursor += bytes;
    if (cursor - data > size) {
        lumber::err("[Wayland::ERR]: Exceeded message payload boundary.");
    }
}

wl_message::reader::reader(const value_ptr data, const size_type payload_size) : data(data), size(payload_size), cursor(data) {}

wl_uint wl_message::reader::read_uint() {
    wl_uint value = read_wl_uint(cursor);
    advance_cursor(WL_UINT_SIZE);
    return value;
}

wl_int wl_message::reader::read_int() {
    const wl_int value = read_wl_int(cursor);
    advance_cursor(WL_INT_SIZE);
    return value;
}

wl_fixed wl_message::reader::read_fixed() {
    wl_fixed value = read_wl_fixed(cursor);
    advance_cursor(WL_FIXED_SIZE);
    return value;
}

wl_object wl_message::reader::read_object() {
    wl_object value = read_wl_uint(cursor);
    advance_cursor(WL_OBJECT_SIZE);
    return value;
}

wl_string wl_message::reader::read_string() {
    wl_string value(cursor);
    advance_cursor(value.serialised_size());
    return value;
}

void wl_message::writer::advance_cursor(const wl_uint bytes) {
    cursor += bytes;
    if (cursor - data > size) {
        lumber::err("[Wayland::ERR]: Exceeded message payload boundary.");
    }
}

wl_message::writer::writer(const wl_message& request, char* data) : size(request.size), data(data), cursor(data + WL_EVENT_HEADER_SIZE) {

    if (!data) {
        lumber::err("Attempt to create writer from nullptr");
        exit(1);
    }

    if (!cursor) {
        lumber::err("Cursor was nullptr");
        exit(1);
    }
}

void wl_message::writer::write(const wl_uint val) {
    from_uint(val, cursor);
    advance_cursor(WL_UINT_SIZE);
}

void wl_message::writer::write(const char* string) {
    const wl_uint str_len = strlen(string) + 1;
    const wl_uint padded_str_len = wl_align(str_len - 1);

    from_uint(str_len, cursor);
    memcpy(cursor + WL_UINT_SIZE, string, str_len);
    
    advance_cursor(wl_align(str_len + WL_UINT_SIZE));
}

void wl_message::writer::write(const wl_string& string) {
    const wl_uint str_len = string.size();
    const wl_uint padded_str_len = string.serialised_size();

    from_uint(str_len, cursor);
    memcpy(cursor + WL_UINT_SIZE, string, str_len);
    
    advance_cursor(string.serialised_size());
}

wl_message::writer wl_message::new_writer(void*(*allocator)(size_t bytes)) {
    char* writeout = (char*)allocator(size);

    from_uint(object_id, writeout);
    from_uint(((size << (sizeof(wl_uint16) * 8)) | opcode), writeout + WL_OBJECT_SIZE);
    return writer(*this, writeout);
}