/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <LibUtil.h>
#include <X11/Xlib.h>

using Util::Position;
using Util::Size;

class Client {
public:
    Client(Display*, Window);

    Window window() const;

    Position<int> position() const;
    Size<unsigned int> size() const;

    bool is_focused() const;

    void kill();

    void resize(Size<unsigned int>);
    void move(Position<int>);

    void focus();
    void unfocus();

    void map();
    void unmap();

    void select_input(long);

    bool find_atom(Atom);

private:
    Window m_window;
    Display* m_display;

    Position<int> m_position;
    Size<unsigned int> m_size;

    bool m_is_floating;
    bool m_is_fullscreen;
    bool m_is_terminal;
    bool m_is_sticky;
    bool m_is_focused;
    bool m_is_mapped;
};
