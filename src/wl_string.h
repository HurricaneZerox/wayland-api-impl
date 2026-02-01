#pragma once

#include <cstddef>
#include "wl_types.h"

class wl_string {
    static size_t total_alloc;

    wl_uint size = 0;
    char* str = nullptr;

    void str_free() {
        if (!str) { return; }
        free(str);
        str = nullptr;
    }

    public:

    wl_string() : size(0) {
        str = static_cast<char*>(malloc(size));
    }

    wl_string(const size_t o_size, const char* data) : size(o_size) {
        str = static_cast<char*>(malloc(size));
        memcpy(str, data, size);
    }

    wl_string(const char* data) : wl_string(read_wl_uint(data), data + 4) {}

    ~wl_string() {
        str_free();
    }

    wl_string& operator=(const wl_string& other) {
        if (this == &other) { return *this; }

        str_free();
        size = other.size;
        str = static_cast<char*>(malloc(size));
        memcpy(str, other.str, size);

        return *this;
    }

    operator const char*() const {
        return str;
    }

    char* Data() noexcept {
        return str;
    }

    wl_uint Size() const noexcept {
        return size;
    }

    wl_uint WordSize() const noexcept {
        return wl_align(size) / WL_WORD_SIZE;
    }

    wl_uint SerialisedSize() const noexcept {
        return wl_align(size) + WL_WORD_SIZE;
    }

    int Compare(const char* other) const noexcept {
        return strcmp(str, other);
    }
};