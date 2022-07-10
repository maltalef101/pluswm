/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <X11/XF86keysym.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <map>
#include <string>
#include <vector>

static constexpr unsigned int nomod = 0;
static constexpr unsigned int winkey = Mod4Mask;
static constexpr unsigned int l_alt = Mod1Mask;

enum class KeyAction {
    Spawn = 0,
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
    ToggleFullscreen,
    IncMasterSize,
    DecMasterSize,
    IncMasterCount,
    DecMasterCount,
    Undefined
};

union Arg {
    int i;
    unsigned int ui;
    float f;
    const void* v;
    const char* s;
};

class Keybind {
public:
    Keybind(unsigned int, KeySym, KeyAction, Arg);

    ~Keybind() = default;

    unsigned int modmask() const;
    KeySym keysym() const;
    KeyAction action() const;
    Arg params() const;

    void execute() const;

private:
    void m_spawn(const char*) const;
    void m_kill_client() const;
    void m_stack_focus() const;
    void m_stack_push() const;
    void m_tag_view(unsigned int) const;
    void m_tag_toggle(unsigned int) const;
    void m_tag_move_to(unsigned int) const;
    void m_make_master() const;
    void m_inc_master_size(float) const;
    void m_dec_master_size(float) const;
    void m_inc_master_count(int) const;
    void m_dec_master_count(int) const;
    void m_toggle_float() const;
    void m_toggle_aot() const;
    void m_toggle_sticky() const;
    void m_toggle_fullscreen() const;
    void m_undefined() const;

    unsigned int m_modmask;
    KeySym m_keysym;
    KeyAction m_action;
    Arg m_params;
};
