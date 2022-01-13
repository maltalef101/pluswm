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
	Client() = default;

	bool operator == (const Client& rhs) { return this->window() == rhs.window(); }

	bool operator != (const Client& rhs) { return !(this->window() == rhs.window()); }

    Window window() const;

    Position<int> position() const;
    Size<unsigned int> size() const;
    Size<unsigned int> prev_size() const;

    bool focus_lock() const;

    bool is_focused() const;

    void kill();

    void resize(Size<unsigned int>);
    void move(Position<int>);

    void focus();
    void unfocus();

    void map();
    void unmap();

    void raise_to_top();

    void toggle_fullscreen();

    void select_input(long);

    void toggle_focus_lock();

    bool find_atom(Atom);

private:
    Window m_window;
    Display* m_display;

    Position<int> m_position;
    Size<unsigned int> m_size;
    Size<unsigned int> m_prev_size;

    // bool m_is_floating;
    bool m_is_fullscreen { false };
    // bool m_is_terminal { false };
    // bool m_is_sticky { false };
    bool m_is_focused { false };
    bool m_is_mapped { false };

    bool m_focus_locked { false };
    ;
};
