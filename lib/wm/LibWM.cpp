/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <X11/Xutil.h>
#include <glog/logging.h>
#include <stdio.h>
#include <unistd.h>

#include <LibUtil.h>
#include <LibWM.h>

#include <config.h>

std::unique_ptr<WindowManager> WindowManager::get(Display* display)
{
    return std::unique_ptr<WindowManager>(new WindowManager(display));
}

WindowManager::WindowManager(Display* display)
    : m_display(CHECK_NOTNULL(display))
    , m_root_window(DefaultRootWindow(m_display))
{
    // init atoms
    m_wmatom[WMProtocols] = XInternAtom(m_display, "WM_PROTOCOLS", false);
    m_wmatom[WMDelete] = XInternAtom(m_display, "WM_DELETE_WINDOW", false);
    m_wmatom[WMState] = XInternAtom(m_display, "WM_STATE", false);
    m_wmatom[WMTakeFocus] = XInternAtom(m_display, "WM_TAKE_FOCUS", false);
    // init monitor
    m_monitor.screen = DefaultScreen(m_display);
    m_monitor.size = Size<int>(DisplayWidth(m_display, m_monitor.screen),
        DisplayHeight(m_display, m_monitor.screen));
}

WindowManager::~WindowManager()
{
    XCloseDisplay(m_display);
}

int WindowManager::on_wm_detected(Display*, XErrorEvent* err)
{
    CHECK_EQ(static_cast<int>(err->error_code), BadAccess);
    m_wm_detected = true;
    return 0;
}

int WindowManager::on_x_error(Display* display, XErrorEvent* err)
{
    constexpr int MAX_ERROR_TEXT_LENGTH = 1024;
    char error_text[MAX_ERROR_TEXT_LENGTH];
    XGetErrorText(display, err->error_code, error_text, sizeof(error_text));

    LOG(ERROR) << "Recieved X error:\n"
               << "\tRequest: " << int(err->request_code)
               << " - " << Util::x_request_code_to_string(err->request_code) << "\n"
               << "\tError code: " << int(err->error_code)
               << " - " << error_text << "\n"
               << "\tResource ID: " << err->resourceid;

    return 0;
}

void WindowManager::run()
{
    XSetErrorHandler(&WindowManager::on_wm_detected);
    XSelectInput(m_display, m_root_window, SubstructureRedirectMask | SubstructureNotifyMask);
    XSync(m_display, false);

    if (m_wm_detected)
        LOG(ERROR) << "Detected another window manager running on display " << XDisplayString(m_display);

    // Set the error handler for normal execution.
    XSetErrorHandler(&WindowManager::on_x_error);

    // Main event loop.
    for (;;) {
        XEvent e;
        XNextEvent(m_display, &e);

        LOG(INFO) << "Recieved event: " << Util::x_event_code_to_string(e);

        switch (e.type) {
        case CreateNotify:
            on_CreateNotify(e.xcreatewindow);
            break;
        case DestroyNotify:
            on_DestroyNotify(e.xdestroywindow);
            break;
        case MapRequest:
            on_MapRequest(e.xmaprequest);
            break;
        case MapNotify:
            on_MapNotify(e.xmap);
            break;
        case UnmapNotify:
            on_UnmapNotify(e.xunmap);
            break;
        case ConfigureRequest:
            on_ConfigureRequest(e.xconfigurerequest);
            break;
        case ConfigureNotify:
            on_ConfigureNotify(e.xconfigure);
            break;
        case KeyPress:
            on_KeyPress(e.xkey);
            break;
        case KeyRelease:
            on_KeyRelease(e.xkey);
            break;
        case ButtonPress:
            on_ButtonPress(e.xbutton);
            break;
        case ButtonRelease:
            on_ButtonRelease(e.xbutton);
            break;
        }
    }
}

void WindowManager::on_CreateNotify(const XCreateWindowEvent& e)
{
    LOG(INFO) << "Created window " << e.window;
    // insert the window into the client list
    m_clients[e.window] = e.window;
    // insert the window into the stack
    Client client(m_display, e.window);
    // insert into the map
    m_window_to_client_map[e.window] = client;

    m_stack.insert(m_stack.begin(), client);
    for (unsigned long i = 0; i < m_stack.size(); i++) {
        LOG(INFO) << "STACK :: Position " << i << " = " << m_stack[i].window();
    }
}
void WindowManager::on_DestroyNotify(const XDestroyWindowEvent& e)
{
    LOG(INFO) << "Destoryed window " << e.window;
}

void WindowManager::on_MapRequest(const XMapRequestEvent& e)
{

    XMapWindow(m_display, e.window);
}

void WindowManager::on_MapNotify(const XMapEvent& e)
{
    LOG(INFO) << "Mapped window " << e.window;
}

void WindowManager::on_UnmapNotify(const XUnmapEvent& e)
{
    if (!m_clients.count(e.window)) {
        LOG(INFO) << "Ignore UnmapNotify for non-client window " << e.window;
        return;
    }
    LOG(INFO) << "Unmapped window " << e.window;
}

void WindowManager::on_ConfigureRequest(const XConfigureRequestEvent& e)
{
    // unsigned int value_mask;
    XWindowChanges changes;
    changes.x = e.x;
    changes.y = e.y;
    changes.width = e.width;
    changes.height = e.height;
    changes.border_width = Config::border_width_in_px;
    changes.sibling = e.above;
    changes.stack_mode = e.detail;

    // Grant the request.
    XConfigureWindow(m_display, e.window, e.value_mask, &changes);
    LOG(INFO) << "Resize window  " << e.window << " to " << Size<unsigned int>(e.width, e.height);
}

void WindowManager::on_ConfigureNotify(const XConfigureEvent& e)
{
    LOG(INFO) << "Configured window " << e.window;
}

void WindowManager::on_KeyPress(const XKeyPressedEvent&)
{
}

void WindowManager::on_KeyRelease(const XKeyReleasedEvent&)
{
}

void WindowManager::on_ButtonPress(const XButtonPressedEvent&)
{
}

void WindowManager::on_ButtonRelease(const XButtonReleasedEvent&)
{
}

Client::Client(Display* dpy, Window window)
    : m_window(window)
    , m_display(dpy)
{
    XWindowAttributes attrs;
    XGetWindowAttributes(m_display, m_window, &attrs);

    m_size.height = attrs.height;
    m_size.width = attrs.width;
    m_position.x = attrs.x;
    m_position.y = attrs.y;
}

Window Client::window() const
{
    return m_window;
}

Position<int> Client::position() const
{
    return m_position;
}

Size<unsigned int> Client::size() const
{
    return m_size;
}

void Client::resize(Size<unsigned int> size)
{
    XWindowAttributes attrs;
    XGetWindowAttributes(m_display, m_window, &attrs);

    m_size.width = size.width;
    m_size.height = size.height;

    XMoveResizeWindow(m_display, m_window, attrs.x, attrs.y, size.width, size.height);
    LOG(INFO) << "Resize window " << m_window << " to " << size;
}

void Client::move(Position<int> pos)
{
    m_position.x = pos.x;
    m_position.y = pos.y;

    XMoveWindow(m_display, m_window, pos.x, pos.y);
    LOG(INFO) << "Move window " << m_window << " to " << pos;
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
}

void Keybind::m_kill_client()
{
}

void Keybind::m_stack_focus()
{
}

void Keybind::m_stack_push()
{
}

void Keybind::m_tag_view(unsigned int tag_num)
{
}

void Keybind::m_tag_toggle(unsigned int tag_num)
{
}

void Keybind::m_tag_move_to(unsigned int tag_num)
{
}

void Keybind::m_make_master()
{
}

void Keybind::m_inc_master_size(float value)
{
}

void Keybind::m_dec_master_size(float value)
{
}

void Keybind::m_inc_master_count(int value)
{
}

void Keybind::m_dec_master_count(int value)
{
}

void Keybind::m_toggle_float()
{
}

void Keybind::m_toggle_aot()
{
}

void Keybind::m_toggle_sticky()
{
}

void Keybind::m_undefined()
{
}
