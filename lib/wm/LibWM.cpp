/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <LibClient.h>
#include <LibUtil.h>
#include <LibWM.h>
#include <X11/X.h>
#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <algorithm>
#include <exception>
#include <glog/logging.h>
#include <unistd.h>
#include <cassert>

#include <config.h>

inline unsigned int clean_mask(const unsigned int mask)
{
    return (mask & ~(LockMask) & (ShiftMask | ControlMask | Mod1Mask | Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask));
}

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
    m_wmatom[WMAtom::WMProtocols] = XInternAtom(m_display, "WM_PROTOCOLS", false);
    m_wmatom[WMAtom::WMDelete] = XInternAtom(m_display, "WM_DELETE_WINDOW", false);
    m_wmatom[WMAtom::WMState] = XInternAtom(m_display, "WM_STATE", false);
    m_wmatom[WMAtom::WMTakeFocus] = XInternAtom(m_display, "WM_TAKE_FOCUS", false);
    m_netatom[NetAtom::NetActiveWindow] = XInternAtom(m_display, "_NET_ACTIVE_WINDOW", false);
    m_netatom[NetAtom::NetState] = XInternAtom(m_display, "_NET_WM_STATE", false);
    m_netatom[NetAtom::NetFullscreen] = XInternAtom(m_display, "_NET_WM_STATE_FULLSCREEN", false);
    m_netatom[NetAtom::NetName] = XInternAtom(m_display, "_NET_WM_NAME", false);
    // init cursor map
    m_cursors[Cursors::LeftPointing] = XCreateFontCursor(m_display, XC_left_ptr);
	m_cursors[Cursors::Hand] = XCreateFontCursor(m_display, XC_hand2);
    m_cursors[Cursors::Fleur] = XCreateFontCursor(m_display, XC_fleur);
    m_cursors[Cursors::Sizing] = XCreateFontCursor(m_display, XC_sizing);
    // init monitor
    m_monitor.screen = DefaultScreen(m_display);
    m_monitor.size = Size<int>{DisplayWidth(m_display, m_monitor.screen),
        DisplayHeight(m_display, m_monitor.screen)};

	// Color stuff
	m_colormap = XCreateColormap(m_display, m_root_window, DefaultVisual(m_display, m_monitor.screen), AllocNone);

	for (const auto& [color, value] : Config::colors) {
		XColor xcolor;
		XParseColor(m_display, m_colormap, value, &xcolor);
		XAllocColor(m_display, m_colormap, &xcolor);
		m_colors[color] = xcolor;
	}
}

WinMan::~WinMan()
{
    for (unsigned int i = 0; i < sizeof(Cursors); i++) {
        XFreeCursor(m_display, m_cursors[static_cast<Cursors>(i)]);
    }

    XCloseDisplay(m_display);
}

Display* WinMan::display() const
{
    return m_display;
}

Window WinMan::root_window() const
{
    return m_root_window;
}

Atom WinMan::wm_atom(WMAtom atom)
{
    return m_wmatom[atom];
}

Atom WinMan::net_atom(NetAtom atom)
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

Monitor WinMan::monitor() const
{
    return m_monitor;
}

Client WinMan::currently_focused() const
{
    int n;
    Window currently_focused;
    XGetInputFocus(m_display, &currently_focused, &n);

    return window_client_map_at(currently_focused);
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

void WinMan::run()
{
    XSetErrorHandler(&WinMan::on_wm_detected);

    unsigned int mask = SubstructureNotifyMask | SubstructureRedirectMask | ButtonPressMask;
    XSelectInput(m_display, m_root_window, mask);

    XSetWindowAttributes wa;
    wa.cursor = this->cursor(Cursors::LeftPointing);
    XChangeWindowAttributes(m_display, m_root_window, CWCursor, &wa);

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
        case ButtonPress:
            on_ButtonPress(e.xbutton);
            break;
        default:
            LOG(WARNING) << "[!!!] Non-implemented event " << Util::x_event_code_to_string(e) << " (" << e.type << ")";
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

void WinMan::on_CreateNotify(const XCreateWindowEvent&)
{
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
    LOG(INFO) << "Created window " << e.window;

    Client client { m_display, e.window };

    // insert the window into the stack
    m_stack.insert(m_stack.begin(), client);
    // insert into the map

    // FIXME: Use the [] operator.
    m_window_to_client_map.emplace(e.window, client);

    // Get the XEnterWindow and XLeaveWindow events to manage focus
    XSelectInput(m_display, e.window, EnterWindowMask | LeaveWindowMask);

	// Set window border
	XSetWindowBorderWidth(m_display, e.window, Config::border_width_in_px);
	XSetWindowBorder(m_display, e.window, m_colors[Colors::WindowBorderActive].pixel);

	client.grab_input();

    tile();

    client.map();

    client.focus();
}

void WinMan::on_MapNotify(const XMapEvent& e)
{
    LOG(INFO) << "Mapped window " << e.window;
}

void WinMan::on_UnmapNotify(const XUnmapEvent& e)
{
    if (!m_window_to_client_map.contains(e.window)) {
        LOG(INFO) << "Ignore UnmapNotify for non-client window " << e.window;
        return;
    }

    auto to_delete = std::find(m_stack.begin(), m_stack.end(), m_window_to_client_map[e.window]);
    m_stack.erase(to_delete);

    m_window_to_client_map.erase(e.window);

    LOG(INFO) << "Unmapped window " << e.window;

    tile();
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
            && clean_mask(Config::keybinds[i].modmask()) == clean_mask(e.state)) {
            Config::keybinds[i].execute();
        }
    }
}

void WinMan::on_KeyRelease(const XKeyReleasedEvent&)
{
}

void WinMan::on_EnterNotify(const XEnterWindowEvent& e)
{
    // FIXME: Use the [] operator.
	Client& client = m_window_to_client_map.at(e.window);
	if (currently_focused() != client) {
		currently_focused().unfocus();
		client.focus();
	}
}

void WinMan::on_ButtonPress(const XButtonPressedEvent&)
{

}

void WinMan::tile()
{
	// XWindowChanges wc;

	// Raise the always-on-top window
	for(auto c : m_stack) {
		if(c.is_aot()) {
			c.raise_to_top();
			break;
		}
	}
}

XColor WinMan::color(Colors color) const
{
	return m_colors.at(color);
}
