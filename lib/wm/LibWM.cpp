#include <glog/logging.h>
#include <X11/Xutil.h>

#include <LibUtil.h>
#include <LibWM.h>

std::unique_ptr<WindowManager> WindowManager::get(Display* display) {
    return std::unique_ptr<WindowManager>(new WindowManager(display));
}

WindowManager::WindowManager(Display* display)
    : m_display(CHECK_NOTNULL(display)),
      m_root_window(DefaultRootWindow(m_display)),
      m_screen(DefaultScreen(m_display)),
      m_screen_width(ScreenOfDisplay(m_display, m_screen)->width),
      m_screen_height(ScreenOfDisplay(m_display, m_screen)->height)
{
    // init atoms
    m_wmatom[WMProtocols] = XInternAtom(m_display, "WM_PROTOCOLS", false);
    m_wmatom[WMDelete] = XInternAtom(m_display, "WM_DELETE_WINDOW", false);
    m_wmatom[WMState] = XInternAtom(m_display, "WM_STATE", false);
    m_wmatom[WMTakeFocus] = XInternAtom(m_display, "WM_TAKE_FOCUS", false);
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

        LOG(INFO) << "Recieved event: "  << Util::x_event_code_to_string(e);

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
        case ConfigureRequest:
            on_ConfigureRequest(e.xconfigurerequest);
            break;
        case ConfigureNotify:
            on_ConfigureNotify(e.xconfigure);
            break;
        }
    }
}

void WindowManager::on_CreateNotify(const XCreateWindowEvent& e)
{
    LOG(INFO) << "Created window " << e.window;
    // insert the window into the client list
    m_clients[e.window] = e.window;
}

void WindowManager::on_DestroyNotify(const XDestroyWindowEvent& e)
{
    LOG(INFO) << "Destoryed window " << e.window;
}

void WindowManager::on_MapRequest(const XMapRequestEvent &e)
{
    XMapWindow(m_display, e.window);
}

void WindowManager::on_MapNotify(const XMapEvent& e)
{
}

void WindowManager::on_ConfigureRequest(const XConfigureRequestEvent& e)
{
    // Get the request data and store it.
    XWindowChanges changes;
    changes.x = e.x;
    changes.y = e.y;
    changes.width = e.width;
    changes.height = e.height;
    changes.border_width = e.border_width;
    changes.sibling = e.above;
    changes.stack_mode = e.detail;

    // Grant the request.
    XConfigureWindow(m_display, e.window, e.value_mask, &changes);

    LOG(INFO) << "Resize " << e.window << " to " << Util::Size<int>(e.width, e.height);
}

void WindowManager::on_ConfigureNotify(const XConfigureEvent& e)
{
    /* XWindowChanges changes;
    changes.x = e.x;
    changes.y = e.y;
    changes.width = m_screen_width;
    changes.height = m_screen_height;

    unsigned int values_changed = CWWidth | CWHeight;

    XConfigureWindow(m_display, e.window, values_changed, &changes);

    LOG(INFO) << "Resize " << e.window << " to " << Util::Size<int>(e.width, e.height);*/
}
