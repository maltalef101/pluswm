/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <X11/Xlib.h>
#include <LibUtil.h>

using Util::Position;
using Util::Size;

class Client {
public:
    ~Client() = default;
    Client(Display*, Window);

    Window window() const;

    Position<int> position() const;
    Size<unsigned int> size() const;

    void resize(Size<unsigned int>);
    void move(Position<int>);

private:
    Window m_window;
    Display* m_display;

    Position<int> m_position;
    Size<unsigned int> m_size;

    bool is_floating, is_fullscreen, is_terminal, is_sticky;
    bool is_mapped;
};
