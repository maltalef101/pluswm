/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <X11/Xlib.h>
#include <map>
#include <memory>
#include <unordered_map>
#include <vector>

#include <LibUtil.h>

using Util::Position;
using Util::Size;

enum wmatom { WMProtocols = 0,
    WMDelete,
    WMState,
    WMTakeFocus,
    WMLast };

struct Gaps {
    Gaps(unsigned int _in_h, unsigned int _in_v,
        unsigned int _out_h, unsigned int _out_v)
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

union Arg {
    int i;
    unsigned int ui;
    float f;
    const char* s;
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

class Keybind {
public:
    Keybind(unsigned int modmask, KeySym keysym,
        const char* action, Arg params)
        : m_modmask(modmask)
        , m_keysym(keysym)
        , m_action(action)
        , m_params(params)
    {
        m_init_actions_map();
    }

    ~Keybind() = default;

    unsigned int modmask() const;
    KeySym keysym() const;
    std::string action() const;
    Arg params() const;

    void execute();

private:
    void m_init_actions_map();

    void m_spawn(const char*);
    void m_kill_client();
    void m_stack_focus();
    void m_stack_push();
    void m_tag_view(unsigned int);
    void m_tag_toggle(unsigned int);
    void m_tag_move_to(unsigned int);
    void m_make_master();
    void m_inc_master_size(float);
    void m_dec_master_size(float);
    void m_inc_master_count(int);
    void m_dec_master_count(int);
    void m_toggle_float();
    void m_toggle_aot();
    void m_toggle_sticky();
    void m_undefined();

    unsigned int m_modmask;
    KeySym m_keysym;
    const char* m_action;
    Arg m_params;

    enum class ActionType { Spawn = 0,
        KillClient,
        StackFocus,
        StackPush,
        TagView,
        TagToggle,
        TagMoveTo,
        MakeMaster,
        ToggleFloat,
        ToggleAOT,
        ToggleSticky,
        IncMasterSize,
        DecMasterSize,
        IncMasterCount,
        DecMasterCount,
        Undefined };

    std::map<std::string, ActionType> m_actions_map;
};

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

struct Monitor {
    int screen;
    Util::Size<int> size;
};

struct WMProps {
    int master_size; // value between 0 and 1 that determines the proportion of the master area in comparison to the stack area
};

class WindowManager {
public:
    WindowManager(const WindowManager&) = delete;
    WindowManager operator=(const WindowManager&) = delete;

    static WindowManager* get(Display*);

    void run();

    ~WindowManager();

    Display* display() const;

private:
    WindowManager(Display*);

    static int on_wm_detected(Display*, XErrorEvent*);
    static int on_x_error(Display*, XErrorEvent*);

    void on_CreateNotify(const XCreateWindowEvent&);
    void on_DestroyNotify(const XDestroyWindowEvent&);

    void on_MapRequest(const XMapRequestEvent&);
    void on_MapNotify(const XMapEvent&);

    void on_UnmapNotify(const XUnmapEvent&);

    void on_ConfigureRequest(const XConfigureRequestEvent&);
    void on_ConfigureNotify(const XConfigureEvent&);

    void on_KeyPress(const XKeyPressedEvent&);
    void on_KeyRelease(const XKeyReleasedEvent&);

    void on_ButtonPress(const XButtonPressedEvent&);
    void on_ButtonRelease(const XButtonReleasedEvent&);

    Display* m_display;
    const Window m_root_window;

    Monitor m_monitor;

    std::unordered_map<Window, Window> m_clients;
    std::vector<Client> m_stack;
    std::map<Window, Client> m_window_to_client_map;

    inline static bool m_wm_detected = false;

    Atom m_wmatom[WMLast];

    static WindowManager s_wm_instance;
};

WindowManager WindowManager::s_wm_instance(XOpenDisplay(nullptr));

