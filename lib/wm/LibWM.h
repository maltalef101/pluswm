/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <LibClient.h>
#include <LibUtil.h>
#include <X11/XF86keysym.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <map>
#include <memory>
#include <unordered_map>
#include <vector>

using Util::Position;
using Util::Size;

enum WMAtom { WMProtocols = 0,
    WMDelete,
    WMState,
    WMTakeFocus
};

enum class Colors {
	WindowBorderActive = 0,
	WindowBorderInactive,
};

enum NetAtom { NetActiveWindow = 0,
    NetName,
    NetFullscreen,
    NetState
};

enum Cursors {
    LeftPointing = 0,
    Sizing,
    Fleur
};

struct Gaps {
    Gaps(unsigned int _in_h, unsigned int _in_v, unsigned int _out_h,
        unsigned int _out_v)
        : in_h(_in_h)
        , in_v(_in_v)
        , out_h(_out_h)
        , out_v(_out_v)
    {
    }

    unsigned int in_h;
    unsigned int in_v;
    unsigned int out_h;
    unsigned int out_v;
};

struct Rule {
    const char* win_class;
    const char* win_instance;
    const char* win_title;
    unsigned int tag;
    bool is_floating;
    bool is_terminal;
    bool no_swallow;
    bool monitor;
};

struct Monitor {
    int screen;
    Util::Size<int> size;
};

struct WMProps {
    double master_size; // value between 0 and 1 that determines the proportion of the
                     // master area in comparison to the stack area
};

class WinMan {
public:
    WinMan(const WinMan&) = delete;
    WinMan operator=(const WinMan&) = delete;

    static WinMan& get();

    void run();

    ~WinMan();

    Display* display() const;
    Window root_window() const;

    Atom wm_atom(WMAtom);
    Atom net_atom(NetAtom);

    Client window_client_map_at(Window) const;
    Cursor cursor(Cursors);

    Monitor monitor() const;

    Client currently_focused() const;

private:
    WinMan(Display*);

    static int on_wm_detected(Display*, XErrorEvent*);
    static int on_x_error(Display*, XErrorEvent*);

    void grab_keys();
    void grab_buttons();

    void on_CreateNotify(const XCreateWindowEvent&);
    void on_DestroyNotify(const XDestroyWindowEvent&);

    void on_MapRequest(const XMapRequestEvent&);
    void on_MapNotify(const XMapEvent&);

    void on_UnmapNotify(const XUnmapEvent&);

    void on_ConfigureRequest(const XConfigureRequestEvent&);
    void on_ConfigureNotify(const XConfigureEvent&);

    void on_KeyPress(const XKeyPressedEvent&);
    void on_KeyRelease(const XKeyReleasedEvent&);

    void on_EnterNotify(const XEnterWindowEvent&);
    void on_LeaveNotify(const XLeaveWindowEvent&);

    void on_ButtonPress(const XButtonPressedEvent&);
    void on_ButtonRelease(const XButtonReleasedEvent&);

    void tile();

    Display* m_display;
    const Window m_root_window;

    Monitor m_monitor;

    std::vector<Client> m_stack;
    std::unordered_map<Window, Client> m_window_to_client_map;
    std::unordered_map<Cursors, Cursor> m_cursors;

    inline static bool m_wm_detected = false;

    std::unordered_map<WMAtom, Atom> m_wmatom;
    std::unordered_map<NetAtom, Atom> m_netatom;

	Colormap m_colormap;

	std::unordered_map<Colors, XColor> m_colors;
};
