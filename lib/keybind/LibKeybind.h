/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <X11/XF86keysym.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <map>
#include <string>

union Arg {
    int i;
    unsigned int ui;
    float f;
    const char* s;
    const void* v;
};

class Keybind {
public:
    Keybind(unsigned int, KeySym, const char*, Arg);

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
