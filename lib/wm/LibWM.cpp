/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <LibClient.h>
#include <LibUtil.h>
#include <LibWM.h>
#include <X11/XKBlib.h>
#include <X11/Xutil.h>
#include <algorithm>
#include <glog/logging.h>
#include <stdio.h>
#include <unistd.h>

#include <config.h>

#define CLEANMASK(mask) (mask & ~(LockMask) & (ShiftMask | ControlMask | Mod1Mask | Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask))

WinMan& WinMan::get()
{
    static WinMan instance(XOpenDisplay(nullptr));
    return instance;
}

WinMan::WinMan(Display* display)
    : m_display(CHECK_NOTNULL(display))
    , m_root_window(DefaultRootWindow(m_display))
{
    // init atoms
    m_wmatom[WMAtom::Protocols] = XInternAtom(m_display, "WM_PROTOCOLS", false);
    m_wmatom[WMAtom::Delete] = XInternAtom(m_display, "WM_DELETE_WINDOW", false);
    m_wmatom[WMAtom::State] = XInternAtom(m_display, "WM_STATE", false);
    m_wmatom[WMAtom::TakeFocus] = XInternAtom(m_display, "WM_TAKE_FOCUS", false);
    m_netatom[NetAtom::ActiveWindow] = XInternAtom(m_display, "_NET_ACTIVE_WINDOW", false);
    m_netatom[NetAtom::WMFullscreen] = XInternAtom(m_display, "_NET_WM_STATE_FULLSCREEN", false);
    m_netatom[NetAtom::WMName] = XInternAtom(m_display, "_NET_WM_NAME", false);
    //init cursor map
    m_cursors[Cursors::LeftPointing] = XCreateFontCursor(m_display, XC_left_ptr);
    m_cursors[Cursors::Fleur] = XCreateFontCursor(m_display, XC_fleur);
    m_cursors[Cursors::Sizing] = XCreateFontCursor(m_display, XC_sizing);
    // init monitor
    m_monitor.screen = DefaultScreen(m_display);
    m_monitor.size = Size<int>(DisplayWidth(m_display, m_monitor.screen),
        DisplayHeight(m_display, m_monitor.screen));
}

WinMan::~WinMan()
{
    for (unsigned int i = 0; i < sizeof(Cursors); i++) {
        XFreeCursor(m_display, m_cursors[static_cast<Cursors>(i)]);
    }
    XCloseDisplay(m_display);
}

Display* WinMan::display() const { return m_display; }

Window WinMan::root_window() const
{
    return m_root_window;
}

Atom WinMan::wm_atom(WMAtom atom) const { return m_wmatom[atom]; }

Atom WinMan::net_atom(NetAtom atom) const
{
    return m_netatom[atom];
}

Client WinMan::window_client_map_at(Window window_at) const
{
    return m_window_to_client_map.at(window_at);
}

Cursor WinMan::cursor(Cursors cursor)
{
    return m_cursors[cursor];
}

int WinMan::on_wm_detected(Display*, XErrorEvent* err)
{
    CHECK_EQ(static_cast<int>(err->error_code), BadAccess);
    m_wm_detected = true;
    return 0;
}

int WinMan::on_x_error(Display* display, XErrorEvent* err)
{
    constexpr int MAX_ERROR_TEXT_LENGTH = 1024;
    char error_text[MAX_ERROR_TEXT_LENGTH];
    XGetErrorText(display, err->error_code, error_text, sizeof(error_text));

    LOG(FATAL) << "Recieved X error:\n"
               << "\tRequest: " << int(err->request_code) << " - "
               << Util::x_request_code_to_string(err->request_code) << "\n"
               << "\tError code: " << int(err->error_code) << " - " << error_text
               << "\n"
               << "\tResource ID: " << err->resourceid;

    return 0;
}

void WindowManager::run()
{
    XSetErrorHandler(&WindowManager::on_wm_detected);
    unsigned int mask = SubstructureNotifyMask | SubstructureRedirectMask
        | ButtonPressMask | PropertyChangeMask;
    XSelectInput(m_display, m_root_window, mask);
    XSync(m_display, false);

    if (m_wm_detected)
        LOG(FATAL) << "Detected another window manager running on display "
                   << XDisplayString(m_display);

    grab_keys();

    // Set the error handler for normal execution.
    XSetErrorHandler(&WinMan::on_x_error);

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
        case EnterNotify:
            on_EnterNotify(e.xcrossing);
            break;
        case LeaveNotify:
            on_LeaveNotify(e.xcrossing);
            break;
        case ButtonPress:
            on_ButtonPress(e.xbutton);
            break;
        default:
            LOG(WARNING) << "!!! Non-implemented event " << Util::x_event_code_to_string(e) << " (" << e.type << ")";
            break;
        }
    }
}

void WinMan::grab_keys()
{
    XUngrabKey(m_display, AnyKey, AnyModifier, m_root_window);

    KeyCode keycode;

    for (unsigned int i = 0; i < Config::keybinds.size(); i++) {
        if ((keycode = XKeysymToKeycode(m_display, Config::keybinds[i].keysym()))) {
            XGrabKey(m_display, keycode, Config::keybinds[i].modmask(), m_root_window, true, GrabModeAsync, GrabModeAsync);
        }
    }
}

void WinMan::on_CreateNotify(const XCreateWindowEvent& e)
{
    LOG(INFO) << "Created window " << e.window;
    // insert the window into the client list
    m_clients[e.window] = e.window;

    Client client { m_display, e.window };

    // insert the window into the stack
    m_stack.insert(m_stack.begin(), client);
    // insert into the map
    /*
     * For some hideus reason `m_window_to_client_map[e.window] = client;`  does not work
     * Really not having it today bruv.
     */
    m_window_to_client_map.emplace(e.window, client);

    // Get the XEnterWindow and XLeaveWindow events to manage focus
    XSelectInput(m_display, e.window, EnterWindowMask | LeaveWindowMask);

    for (unsigned long i = 0; i < m_stack.size(); i++) {
        LOG(INFO) << "STACK :: Position " << i << " = " << m_stack[i].window();
    }
}
void WinMan::on_DestroyNotify(const XDestroyWindowEvent& e)
{
    LOG(INFO) << "Destoryed window " << e.window;
}

void WinMan::on_MapRequest(const XMapRequestEvent& e)
{

    XMapWindow(m_display, e.window);
}

void WinMan::on_MapNotify(const XMapEvent& e)
{
    LOG(INFO) << "Mapped window " << e.window;
}

void WinMan::on_UnmapNotify(const XUnmapEvent& e)
{
    if (!m_clients.count(e.window)) {
        LOG(INFO) << "Ignore UnmapNotify for non-client window " << e.window;
        return;
    }
    LOG(INFO) << "Unmapped window " << e.window;
}

void WinMan::on_ConfigureRequest(const XConfigureRequestEvent& e)
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

void WinMan::on_ConfigureNotify(const XConfigureEvent& e)
{
    LOG(INFO) << "Configured window " << e.window;
}

void WinMan::on_KeyPress(const XKeyPressedEvent& e)
{
    KeySym key = XkbKeycodeToKeysym(m_display, e.keycode, 0, 0);

    for (unsigned long i = 0; i < Config::keybinds.size(); i++) {
        if (key == Config::keybinds[i].keysym()
            && CLEANMASK(Config::keybinds[i].modmask()) == CLEANMASK(e.state)) {
            Config::keybinds[i].execute();
        }
    }
}

void WinMan::on_KeyRelease(const XKeyReleasedEvent&)
{
}

void WinMan::on_EnterNotify(const XEnterWindowEvent& e)
{
    m_window_to_client_map.at(e.window).focus();
    LOG(INFO) << "Window " << e.window << " focused";
}

void WinMan::on_LeaveNotify(const XLeaveWindowEvent& e)
{
    m_window_to_client_map.at(e.window).unfocus();
    LOG(INFO) << "Window " << e.window << " unfocused";
}

void WinMan::on_ButtonPress(const XButtonPressedEvent&)
{
}
