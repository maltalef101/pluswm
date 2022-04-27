/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <LibClient.h>
#include <LibWM.h>
#include <X11/Xatom.h>
#include <algorithm>
#include <glog/logging.h>

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

Size<int> Client::size() const
{
    return m_size;
}

Size<int> Client::prev_size() const
{
    return m_prev_size;
}

bool Client::focus_lock() const
{
    return m_focus_locked;
}

bool Client::is_focused() const
{
    return m_is_focused;
}

bool Client::is_fullscreen() const
{
	return m_is_fullscreen;
}

void Client::kill()
{
    Display* dpy = WinMan::get().display();

    Atom delete_window = WinMan::get().wm_atom(WMAtom::WMDelete);
    Atom wm_protocols = WinMan::get().wm_atom(WMAtom::WMProtocols);

    if (this->find_atom(delete_window)) {
        LOG(INFO) << "Gracefully closing window " << m_window;
        XEvent msg;
        memset(&msg, 0, sizeof(msg));
        msg.xclient.type = ClientMessage;
        msg.xclient.message_type = wm_protocols;
        msg.xclient.window = m_window;
        msg.xclient.format = 32;
        msg.xclient.data.l[0] = delete_window;

        CHECK(XSendEvent(dpy, m_window, false, NoEventMask, &msg));
    } else {
        LOG(INFO) << "Forcefully killing window " << m_window;
        XGrabServer(dpy);
        XKillClient(dpy, m_window);
        XUngrabServer(dpy);
    }
}

void Client::resize(Size<int> size)
{
    m_prev_size.width = m_size.width;
    m_prev_size.height = m_size.height;

    m_size.width = size.width;
    m_size.height = size.height;

    Position<int> pos = position();

    XMoveResizeWindow(m_display, m_window, pos.x, pos.y, size.width, size.height);
    LOG(INFO) << "Resize window " << m_window << " to " << size;
}

void Client::move(Position<int> pos)
{
    m_position.x = pos.x;
    m_position.y = pos.y;

    XMoveWindow(m_display, m_window, pos.x, pos.y);
    LOG(INFO) << "Move window " << m_window << " to " << pos;
}

void Client::focus()
{
    Display* dpy = WinMan::get().display();

    XSetInputFocus(dpy, m_window, RevertToPointerRoot, CurrentTime);
    XChangeProperty(dpy, WinMan::get().root_window(), WinMan::get().net_atom(NetAtom::NetActiveWindow),
        XA_WINDOW, 32, PropModeReplace, (unsigned char*)&m_window, 1);

    Atom take_focus = WinMan::get().wm_atom(WMAtom::WMTakeFocus);

    if (this->find_atom(take_focus)) {
        XEvent msg;
        memset(&msg, 0, sizeof(msg));
        msg.xclient.type = ClientMessage;
        msg.xclient.message_type = take_focus;
        msg.xclient.window = m_window;
        msg.xclient.format = 32;
        msg.xclient.data.l[0] = take_focus;
        msg.xclient.data.l[1] = CurrentTime;

        CHECK(XSendEvent(dpy, m_window, false, NoEventMask, &msg));
    }

    m_is_focused = true;

	LOG(INFO) << "Window " << m_window << " focused";
}

void Client::unfocus()
{
    XSetInputFocus(WinMan::get().display(), None, RevertToPointerRoot, CurrentTime);
    m_is_focused = false;
	LOG(INFO) << "Window " << m_window << " unfocused";
}

void Client::map()
{
    XMapWindow(WinMan::get().display(), m_window);
}

void Client::unmap()
{
    XUnmapWindow(WinMan::get().display(), m_window);
}

void Client::raise_to_top()
{
    XRaiseWindow(WinMan::get().display(), m_window);
}

void Client::toggle_fullscreen()
{
    //Atom net_wmstate = WinMan::get().net_atom(NetAtom::NetState);
    // Atom net_fullscreen = WinMan::get().net_atom(NetAtom::NetFullscreen);

	// LOG(INFO) << "1 is_fullscreen: " << m_is_fullscreen;
    // Monitor mon = WinMan::get().monitor();
    // if (m_is_fullscreen != true) {
    //     // XChangeProperty(WinMan::get().display(), m_window, net_wmstate, XA_ATOM, 32,
    //     //     PropModeReplace, (unsigned char*)&net_fullscreen, 1);
    //     // border width = old border width
    //     // TODO: position stuff
	// 	this->move(Position<int>(0, 0));
    //     this->resize(mon.size);
    //     this->raise_to_top();

    //     m_is_fullscreen = true;
    // } else if (m_is_fullscreen == true) {
    //     // XChangeProperty(WinMan::get().display(), m_window, net_wmstate, XA_ATOM, 32,
    //     //     PropModeReplace, (unsigned char*)&net_fullscreen, 0);
    //     // this->lock_focus();
    //     //  TODO: old border width = border width
    //     //  TODO: position stuff
    //     this->resize(m_prev_size);

    //     m_is_fullscreen = false;
    // }

	/* FIXME: This code doesn't work for some reason.
	 * I've tried everything, writing it out in the most KISS way possible and nothing
	 * works, it still gets stuck on fullscreen and doesn't go back. Everything else
	 * works as expected, the Client::resize() function works perfectly, I've logged
	 * every value loggable.
	 *
	 * Please help.
	 */

    LOG(INFO) << "1 fullscreen: " << m_is_fullscreen;
	if (m_is_fullscreen == false) {
		m_is_fullscreen = true;
		this->move(Position<int>{0, 0});
		this->resize(WinMan::get().monitor().size);
		this->raise_to_top();
	} else if (m_is_fullscreen == true) {
		m_is_fullscreen = false;
		this->resize(m_prev_size);
	}

    // LOG(INFO) << "[!!!] Window " << m_window << " fullscreen: " << m_is_fullscreen;
    LOG(INFO) << "2 fullscreen: " << m_is_fullscreen;
}

void Client::select_input(long mask = NoEventMask)
{
    XSelectInput(WinMan::get().display(), m_window, mask);
}

/* void Client::toggle_focus_lock()
{
   if (!m_focus_locked) {
        WinMan::get().focus_lock(this, false);
   } else {
        WinMan::get().focus_lock(this, true);
   }
} */

bool Client::find_atom(Atom atom)
{
    Display* dpy = WinMan::get().display();

    Atom* supported_proto;
    int supported_proto_count;
    CHECK(XGetWMProtocols(dpy, this->window(), &supported_proto, &supported_proto_count));

    bool search_result = std::find(supported_proto, supported_proto + supported_proto_count, atom) != supported_proto + supported_proto_count;

    XFree(supported_proto);

    return search_result;
}
