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

Keybind::Keybind(unsigned int modmask, KeySym keysym, const char* action, Arg params)
    : m_modmask(modmask)
    , m_keysym(keysym)
    , m_action(action)
    , m_params(params)
{
    m_init_actions_map();
}

void Keybind::execute()
{
    switch (m_actions[this->action()]) {
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

void Keybind::m_init_actions_map()
{
    m_actions["spawn"] = Action::Spawn;
    m_actions["kill_client"] = Action::KillClient;
    m_actions["stack_focus"] = Action::StackFocus;
    m_actions["stack_push"] = Action::StackPush;
    m_actions["make_master"] = Action::MakeMaster;
    m_actions["inc_master_size"] = Action::IncMasterSize;
    m_actions["dec_master_size"] = Action::DecMasterSize;
    m_actions["inc_master_count"] = Action::IncMasterCount;
    m_actions["dec_master_count"] = Action::DecMasterCount;
    m_actions["tag_view"] = Action::TagView;
    m_actions["tag_toggle"] = Action::TagToggle;
    m_actions["tag_move"] = Action::TagMoveTo;
    m_actions["toggle_float"] = Action::ToggleFloat;
    m_actions["toggle_aot"] = Action::ToggleAOT;
    m_actions["toggle_sticky"] = Action::ToggleSticky;
    m_actions["toggle_fullscreen"] = Action::ToggleFullscreen;
}

unsigned int Keybind::modmask() const
{
    return m_modmask;
}

KeySym Keybind::keysym() const
{
    return m_keysym;
}

std::string Keybind::action() const
{
    return m_action;
}

Arg Keybind::params() const
{
    return m_params;
}

void Keybind::m_spawn(const char* command)
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

    LOG(INFO) << "Spawned command \"" << command << "\"";
}

void Keybind::m_kill_client()
{
    WinMan::get().currently_focused().kill();
}

void Keybind::m_stack_focus() { }

void Keybind::m_stack_push() { }

void Keybind::m_tag_view(unsigned int) { }

void Keybind::m_tag_toggle(unsigned int) { }

void Keybind::m_tag_move_to(unsigned int) { }

void Keybind::m_make_master() { }

void Keybind::m_inc_master_size(float) { }

void Keybind::m_dec_master_size(float) { }

void Keybind::m_inc_master_count(int) { }

void Keybind::m_dec_master_count(int) { }

void Keybind::m_toggle_float() { }

void Keybind::m_toggle_aot() { }

void Keybind::m_toggle_sticky() { }

void Keybind::m_toggle_fullscreen()
{
    WinMan::get().currently_focused().toggle_fullscreen();
}

void Keybind::m_undefined() { }
