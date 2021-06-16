/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <LibClient.h>
#include <LibWM.h>
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

Window Client::window() const { return m_window; }

Position<int> Client::position() const { return m_position; }

Size<unsigned int> Client::size() const { return m_size; }

bool Client::focused() const { return m_is_focused; }

void Client::kill()
{
    Display* dpy = WindowManager::get().display();

    Atom* supp_proto;
    int supp_proto_count;

    Atom delete_window = WindowManager::get().atom(wmatom::WMDelete);
    Atom wm_protocols = WindowManager::get().atom(wmatom::WMProtocols);

    if (XGetWMProtocols(dpy, m_window, &supp_proto, &supp_proto_count)
        && (std::find(supp_proto, supp_proto + supp_proto_count, delete_window)
            != supp_proto + supp_proto_count)) {
        LOG(INFO) << "Gracefully closing window " << m_window;
        // 1. Construct message.
        XEvent msg;
        memset(&msg, 0, sizeof(msg));
        msg.xclient.type = ClientMessage;
        msg.xclient.message_type = wm_protocols;
        msg.xclient.window = m_window;
        msg.xclient.format = 32;
        msg.xclient.data.l[0] = delete_window;

        CHECK(XSendEvent(dpy, m_window, false, 0, &msg));
    } else {
        LOG(INFO) << "Forcefully killing window " << m_window;
        XGrabServer(dpy);
        XKillClient(dpy, m_window);
        XUngrabServer(dpy);
    }
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

void Client::focus()
{
    XSetInputFocus(WindowManager::get().display(), this->window(), RevertToPointerRoot, CurrentTime);
    m_is_focused = true;
}

void Client::unfocus()
{
    XSetInputFocus(WindowManager::get().display(), None, RevertToPointerRoot, CurrentTime);
    m_is_focused = false;
}
