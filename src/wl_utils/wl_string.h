#pragma once

#include "wl_types.h"

class wl_string {

    public:

    using value_type = char;
    using size_type = wl_uint;
    using difference_type = wl_int;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;

    private:

    static size_t total_alloc;

    size_type size_n = 0;
    value_type* str = nullptr;

    void str_free();

    public:

    wl_string();

    wl_string(const size_type o_size, const char* const data);

    wl_string(const char* data);

    /**
        @brief Create a new wl_string from a c-style
        string.
    */
    static wl_string from_c_str(const char* const c_str);

    ~wl_string();

    char& at(size_type pos);
    const char& at(size_type pos) const;

    wl_string& operator=(const wl_string& other);

    operator const char*() const;

    pointer data() noexcept;

    const char* const c_str() const noexcept;

    bool empty() const noexcept;

    wl_uint size() const noexcept;

    wl_uint length() const noexcept;

    wl_uint word_size() const noexcept;

    wl_uint serialised_size() const noexcept;

    int compare(const char* other) const noexcept;
};