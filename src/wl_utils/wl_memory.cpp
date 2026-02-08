#include "wl_types.h"

wl_uint wl_align(const wl_uint addr) {
    return (addr - 1) - ((addr - 1) % WL_WORD_SIZE) + WL_WORD_SIZE;
}

bool is_aligned(const wl_uint addr) {
    return addr % WL_WORD_SIZE == 0;
}