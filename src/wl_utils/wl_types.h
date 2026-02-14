#pragma once

#include <cstdint>

/**
    Type definitions for Wayland's protocol-defined
    types.
*/

using wl_int = int32_t;
using wl_uint = uint32_t;
using wl_fixed = float;
using wl_object = uint32_t;
using wl_new_id = uint32_t;
using wl_dev_t = uint32_t;

/**
    Special case types

    For example, Wayland uses a 16-bit integer to
    represent message opcodes and sizes.
*/

using wl_uint16 = uint16_t;
using wl_opcode_t = wl_uint16;
using wl_fd_t = wl_uint;
using wl_opcode_t = wl_uint16;

#define WL_INT_SIZE sizeof(wl_int)
#define WL_UINT_SIZE sizeof(wl_uint)
#define WL_FIXED_SIZE sizeof(wl_fixed)
#define WL_OBJECT_SIZE sizeof(wl_object)
#define WL_NEW_ID_SIZE sizeof(wl_new_id)
#define WL_FD_SIZE sizeof(wl_fd_t)
#define WL_WORD_SIZE 4
#define WL_EVENT_HEADER_SIZE 2 * WL_WORD_SIZE

#define WL_NEW_ID_MIN 2
#define WL_NEW_ID_MAX 0xFEFFFFFF

/**
    @brief Reads the next four bytes of `data`
    as a wl_int value.
*/
wl_int read_wl_int(const void* data);

/**
    @brief Reads the next four bytes of `data`
    as a wl_uint value.
*/
wl_uint read_wl_uint(const void* data);

/**
    @brief Reads the next four bytes of `data`
    as a wl_object value.
*/
wl_object read_wl_object(const void* data);

/**
    @brief Reads the next four bytes of `data`
    as a wl_fixed value.
*/
wl_fixed read_wl_fixed(const void* data);

/**
    @brief Writes a wl_int value into the next
    four bytes of `data`.
*/
void from_int(const wl_int int_v, void* data);

/**
    @brief Writes a wl_uint value into the next
    four bytes of `data`.
*/
void from_uint(const wl_uint uint, void* data);

/**
    @brief Writes a wl_object value into the next
    four bytes of `data`.
*/
void from_object(const wl_object object, void* data);

/**
    @brief Writes a wl_new_id value into the next
    four bytes of `data`.
*/
void from_new_id(const wl_new_id new_id, void* data);

/**
    @brief Rounds up an address to the nearest multiple
    of four.
*/
wl_uint wl_align(const wl_uint addr);

/**
    @brief Checks whether an address is a multiple of
    `WL_WORD_SIZE`.
*/
bool is_aligned(const wl_uint addr);

#define NULL_OBJ_ID 0
#define DISPLAY_OBJ_ID 1