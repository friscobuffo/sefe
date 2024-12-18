#ifndef MY_COLOR_H
#define MY_COLOR_H

#include <cassert>

enum class Color {
    RED,
    BLUE,
    BLACK,
    RED_AND_BLUE,
    NONE,
    ANY,
};

inline const char* color2string(const Color color) {
    switch (color) {
        case Color::RED: return "red";
        case Color::BLUE: return "blue";
        case Color::BLACK: return "black";
        default: assert(false);
    }
}

#endif
