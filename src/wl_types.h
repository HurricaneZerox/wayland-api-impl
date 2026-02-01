#pragma once

#include <cmath>
#include <cstdint>
#include <cstring>

/**
    Type definitions for Wayland's protocol-defined
    types.
*/

using wl_int = int32_t;
using wl_uint = uint32_t;
using wl_fixed = float_t;
using wl_object = uint32_t;
using wl_new_id = uint32_t;

/**
    Special case types

    For example, Wayland uses a 16-bit integer to
    represent message opcodes and sizes.
*/

using wl_uint16 = uint16_t;

#define WL_INT_SIZE sizeof(wl_int)
#define WL_UINT_SIZE sizeof(wl_uint)
#define WL_FIXED_SIZE sizeof(wl_fixed)
#define WL_OBJECT_SIZE sizeof(wl_object)
#define WL_NEW_ID_SIZE sizeof(wl_new_id)
#define WL_WORD_SIZE 4

inline wl_int read_wl_int(const void* data) {
    return *reinterpret_cast<const wl_int*>(data);
}

inline wl_uint read_wl_uint(const void* data) {
    return *reinterpret_cast<const wl_uint*>(data);
}

inline wl_object read_wl_object(const void* data) {
    return *reinterpret_cast<const wl_object*>(data);
}

template<class T>
void from_wl(const T val, void* data) {
    memcpy(data, reinterpret_cast<const char*>(&val), WL_NEW_ID_SIZE);
}

inline void from_int(const wl_int sint, void* data) {
    from_wl(sint, data);
}

inline void from_uint(const wl_uint uint, void* data) {
    from_wl(uint, data);
}

inline void from_object(const wl_object object, void* data) {
    from_wl(object, data);
}

inline void from_new_id(const wl_new_id new_id, void* data) {
    from_wl(new_id, data);
}

inline uintmax_t wl_align(const uintmax_t addr) {
    return (addr - 1) - ((addr - 1) % WL_WORD_SIZE) + WL_WORD_SIZE;
}

inline uintmax_t wl_align(const char* str) {
    return wl_align(strlen(str) + 1);
}