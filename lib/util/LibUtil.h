/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <sstream>
#include <string>

#include <X11/Xlib.h>

namespace Util {

std::string x_request_code_to_string(unsigned char);

std::string x_event_code_to_string(const XEvent&);

template<typename T>
struct Size {
    T width, height;

    Size() = default;
    Size(T w, T h)
        : width(w)
        , height(h)
    {
    }

    std::string to_string() const;
};

template<typename T>
std::string Size<T>::to_string() const
{
    std::ostringstream out;
    out << width << 'x' << height;
    return out.str();
}

template<typename T>
std::ostream& operator<<(std::ostream& out, const Size<T>& size)
{
    return out << size.to_string();
}

template<typename T>
struct Position {
    T x, y;

    Position() = default;
    Position(T _x, T _y)
        : x(_x)
        , y(_y)
    {
    }

    std::string to_string() const;
};

template<typename T>
std::string Position<T>::to_string() const
{
    std::ostringstream out;
    out << "(" << x << ", " << y << ")";
    return out.str();
}

template<typename T>
std::ostream& operator<<(std::ostream& out, const Position<T>& pos)
{
    return out << pos.to_string();
}

}
