#include <glog/logging.h>

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

    XSetErrorHandler(&WindowManager::on_x_error);
}
