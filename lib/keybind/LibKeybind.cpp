/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <LibClient.h>
#include <LibKeybind.h>
#include <LibWM.h>
#include <algorithm>
#include <glog/logging.h>

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
    switch (m_actions_map[this->action()]) {
    case ActionType::Spawn:
        m_spawn(m_params.s);
        break;
    case ActionType::KillClient:
        m_kill_client();
        break;
    case ActionType::StackFocus:
        m_stack_focus();
        break;
    case ActionType::StackPush:
        m_stack_push();
        break;
    case ActionType::MakeMaster:
        m_make_master();
        break;
    case ActionType::IncMasterSize:
        m_inc_master_size(m_params.f);
        break;
    case ActionType::DecMasterSize:
        m_dec_master_size(m_params.f);
        break;
    case ActionType::IncMasterCount:
        m_inc_master_size(m_params.i);
        break;
    case ActionType::DecMasterCount:
        m_dec_master_size(m_params.i);
        break;
    case ActionType::TagView:
        m_tag_view(m_params.ui);
        break;
    case ActionType::TagToggle:
        m_tag_toggle(m_params.ui);
        break;
    case ActionType::TagMoveTo:
        m_tag_move_to(m_params.ui);
        break;
    case ActionType::ToggleFloat:
        m_toggle_float();
        break;
    case ActionType::ToggleAOT:
        m_toggle_aot();
        break;
    case ActionType::ToggleSticky:
        m_toggle_sticky();
        break;
    case ActionType::Undefined:
        m_undefined();
        break;
    }
}

void Keybind::m_init_actions_map()
{
    m_actions_map["spawn"] = ActionType::Spawn;
    m_actions_map["kill_client"] = ActionType::KillClient;
    m_actions_map["stack_focus"] = ActionType::StackFocus;
    m_actions_map["stack_push"] = ActionType::StackPush;
    m_actions_map["make_master"] = ActionType::MakeMaster;
    m_actions_map["inc_master_size"] = ActionType::IncMasterSize;
    m_actions_map["dec_master_size"] = ActionType::DecMasterSize;
    m_actions_map["inc_master_count"] = ActionType::IncMasterCount;
    m_actions_map["dec_master_count"] = ActionType::DecMasterCount;
    m_actions_map["tag_view"] = ActionType::TagView;
    m_actions_map["tag_toggle"] = ActionType::TagToggle;
    m_actions_map["tag_move"] = ActionType::TagMoveTo;
    m_actions_map["toggle_float"] = ActionType::ToggleFloat;
    m_actions_map["toggle_aot"] = ActionType::ToggleAOT;
    m_actions_map["toggle_sticky"] = ActionType::ToggleSticky;
}

unsigned int Keybind::modmask() const { return m_modmask; }

KeySym Keybind::keysym() const { return m_keysym; }

std::string Keybind::action() const { return m_action; }

Arg Keybind::params() const { return m_params; }

void Keybind::m_spawn(const char*) { }

void Keybind::m_kill_client()
{
    Display* dpy = WindowManager::get().display();

    int revert_to_return;
    Window curr_focused_win;
    XGetInputFocus(dpy, &curr_focused_win, &revert_to_return);

    Client client = WindowManager::get().window_client_map_at(curr_focused_win);
    client.kill();
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

void Keybind::m_undefined() { }
