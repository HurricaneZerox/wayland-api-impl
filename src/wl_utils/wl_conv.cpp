#include "wl_types.h"

#include <cstring>

wl_int read_wl_int(const void* data) {
    return *reinterpret_cast<const wl_int*>(data);
}

wl_uint read_wl_uint(const void* data) {
    return *reinterpret_cast<const wl_uint*>(data);
}

wl_object read_wl_object(const void* data) {
    return *reinterpret_cast<const wl_object*>(data);
}

wl_fixed read_wl_fixed(const void* data) {
    const int32_t bit_data = *(int32_t*)data;
    int32_t integer_part = (bit_data >> 8) & 0x7FFFFF;

    if (bit_data >> 31) {
        integer_part = -((1 << 23) - integer_part);
    }

    return integer_part + static_cast<float>(bit_data & 0xFF) / UINT8_MAX;
}

template<class T>
void from_wl(const T val, void* data) {
    memcpy(data, reinterpret_cast<const char*>(&val), WL_NEW_ID_SIZE);
}

void from_int(const wl_int sint, void* data) {
    from_wl(sint, data);
}

void from_uint(const wl_uint uint, void* data) {
    from_wl(uint, data);
}

void from_object(const wl_object object, void* data) {
    from_wl(object, data);
}

void from_new_id(const wl_new_id new_id, void* data) {
    from_wl(new_id, data);
}