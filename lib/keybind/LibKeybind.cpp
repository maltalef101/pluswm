/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <LibClient.h>
#include <LibKeybind.h>
#include <LibWM.h>
#include <algorithm>
#include <cerrno>
#include <cstring>
#include <glog/logging.h>
#include <sched.h>
#include <unistd.h>

Keybind::Keybind(unsigned int modmask, KeySym keysym, Action action, Arg params)
    : m_modmask(modmask)
    , m_keysym(keysym)
    , m_action(action)
    , m_params(params)
{
}

void Keybind::execute() const
{
    switch (this->action()) {
    case Action::Spawn:
        m_spawn(m_params.s);
        break;
    case Action::KillClient:
        m_kill_client();
        break;
    case Action::StackFocus:
        m_stack_focus();
        break;
    case Action::StackPush:
        m_stack_push();
        break;
    case Action::MakeMaster:
        m_make_master();
        break;
    case Action::IncMasterSize:
        m_inc_master_size(m_params.f);
        break;
    case Action::DecMasterSize:
        m_dec_master_size(m_params.f);
        break;
    case Action::IncMasterCount:
        m_inc_master_size(m_params.i);
        break;
    case Action::DecMasterCount:
        m_dec_master_size(m_params.i);
        break;
    case Action::TagView:
        m_tag_view(m_params.ui);
        break;
    case Action::TagToggle:
        m_tag_toggle(m_params.ui);
        break;
    case Action::TagMoveTo:
        m_tag_move_to(m_params.ui);
        break;
    case Action::ToggleFloat:
        m_toggle_float();
        break;
    case Action::ToggleAOT:
        m_toggle_aot();
        break;
    case Action::ToggleSticky:
        m_toggle_sticky();
        break;
    case Action::ToggleFullscreen:
        m_toggle_fullscreen();
        break;
    case Action::Undefined:
        m_undefined();
        break;
    }
}

unsigned int Keybind::modmask() const
{
    return m_modmask;
}

KeySym Keybind::keysym() const
{
    return m_keysym;
}

Action Keybind::action() const
{
    return m_action;
}

Arg Keybind::params() const
{
    return m_params;
}

void Keybind::m_spawn(const char* command) const
{
    const char* cmdarg[] = { "/bin/sh", "-c", command, NULL };

    pid_t child = fork();

    if (child == 0) {
        // child
        execvp(cmdarg[0], (char**)cmdarg);
    } else if (child > 0) {
        // parent
    } else {
        // error
        LOG(ERROR) << "Could not fork a child proc: " << strerror(errno) << "(errno=" << errno << ")";
        exit(1);
    }

    LOG(INFO) << "Spawned command `" << command << "`";
}

void Keybind::m_kill_client() const
{
    WinMan::get().currently_focused().kill();
}

void Keybind::m_stack_focus() const { }

void Keybind::m_stack_push() const { }

void Keybind::m_tag_view(unsigned int) const { }

void Keybind::m_tag_toggle(unsigned int) const { }

void Keybind::m_tag_move_to(unsigned int) const { }

void Keybind::m_make_master() const { }

void Keybind::m_inc_master_size(float) const { }

void Keybind::m_dec_master_size(float) const { }

void Keybind::m_inc_master_count(int) const { }

void Keybind::m_dec_master_count(int) const { }

void Keybind::m_toggle_float() const { }

void Keybind::m_toggle_aot() const { }

void Keybind::m_toggle_sticky() const { }

void Keybind::m_toggle_fullscreen() const
{
    WinMan::get().currently_focused().toggle_fullscreen();
}

void Keybind::m_undefined() const
{
	LOG(INFO) << "Action::Undefined used in Keybinds vector.";
}
