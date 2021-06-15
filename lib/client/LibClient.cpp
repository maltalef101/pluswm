/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <LibClient.h>
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
