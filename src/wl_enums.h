#pragma once

enum class Format {
    ARGB8888 = 0,
    XRGB8888 = 1,
    C8 = 0x20203843,
    RGB332 = 0x38424752,
    BGR233 = 0x38524742,
    XRGB4444 = 0x32315258,
    XBGR4444 = 0x32314258,

};

inline const char* format_to_str(const Format format) {
    switch (format) {
        case Format::ARGB8888: return "ARGB8888";
        case Format::XRGB8888: return "XRGB8888";
        case Format::C8: return "C8";
        case Format::RGB332: return "RGB332";
        case Format::BGR233: return "BGR233";
        case Format::XRGB4444: return "XRGB4444";
        default: return "Unknown format";
    }
}