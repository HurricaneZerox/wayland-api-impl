#pragma once

#include <cstdio>
#include <iostream>
#include <printf.h>

#define RED_FG 31
#define YELLOW_FG 33

namespace lumber {

inline void info(const char* msg) {

}

inline void warn(const char* msg) {
    std::cout << "\033[" << YELLOW_FG << "m" << msg << "\033[0m\n";
}

inline void err(const char* msg) {
    std::cout << "\033[" << RED_FG << "m" << msg << "\033[0m\n";
}

}