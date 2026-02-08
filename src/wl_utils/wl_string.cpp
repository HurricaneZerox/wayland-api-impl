#include "../wl_utils/wl_string.h"

#include <stdexcept>
#include <cstring>

void wl_string::str_free() {
    if (!str) { return; }
    free(str);
    str = nullptr;
}

wl_string::wl_string() : size_n(0) {
    str = static_cast<pointer>(malloc(size_n));
}

wl_string::wl_string(const size_type o_size, const char* data) : size_n(o_size) {
    str = static_cast<pointer>(malloc(size_n));
    memcpy(str, data, size_n);
}

wl_string::wl_string(const char* data) : wl_string(read_wl_uint(data), data + 4) {}

/**
    @brief Create a new wl_string from a c-style
    string.
*/
wl_string wl_string::from_c_str(const char* const c_str) {
    if (!c_str) { throw std::runtime_error("Can't pass nullptr"); }
    return wl_string(strlen(c_str), c_str);
}

wl_string::~wl_string() {
    str_free();
}

wl_string& wl_string::operator=(const wl_string& other) {
    if (this == &other) { return *this; }

    str_free();
    size_n = other.size_n;
    str = static_cast<pointer>(malloc(size_n));
    memcpy(str, other.str, size_n);

    return *this;
}

wl_string::operator const char*() const {
    return reinterpret_cast<const char* const>(str);
}

/** ELEMENT ACCESS */

char& wl_string::at(wl_string::size_type pos) {
    if (pos < 0 || pos >= size_n) {
        throw std::out_of_range("Out of range element access");
    }

    return str[pos];
}

const char& wl_string::at(wl_string::size_type pos) const {
    if (pos < 0 || pos >= size_n) {
        throw std::out_of_range("Out of range element access");
    }

    return str[pos];
}


wl_string::pointer wl_string::data() noexcept {
    return str;
}

const char* const wl_string::c_str() const noexcept {
    return reinterpret_cast<const char* const>(str);
}

bool wl_string::empty() const noexcept {
    return size_n == 0;
}

wl_uint wl_string::size() const noexcept {
    return size_n;
}

wl_uint wl_string::length() const noexcept {
    return size();
}

wl_uint wl_string::word_size() const noexcept {
    return wl_align(size_n) / WL_WORD_SIZE;
}

wl_uint wl_string::serialised_size() const noexcept {
    return wl_align(size_n) + WL_WORD_SIZE;
}

int wl_string::compare(const char* other) const noexcept {
    return strcmp(c_str(), other);
}