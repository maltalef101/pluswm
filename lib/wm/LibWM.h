#pragma once

#include <X11/Xlib.h>
#include <memory>
#include <unordered_map>

enum wmatom { WMProtocols, WMDelete, WMState, WMTakeFocus, WMLast };

class WindowManager {
public:
    WindowManager(const WindowManager&) = delete;
    WindowManager operator= (const WindowManager&) = delete;

    static std::unique_ptr<WindowManager> get(Display*);

    void run();

    ~WindowManager();
private:
    WindowManager(Display*);

    static int on_wm_detected(Display*, XErrorEvent*);
    static int on_x_error(Display*, XErrorEvent*);

    Display* m_display;
    const Window m_root_window;
    std::unordered_map<Window, Window> m_clients;

    int m_screen;
    int m_screen_width;
    int m_screen_height;

    inline static bool m_wm_detected = false;

    Atom m_wmatom[sizeof(wmatom)];
};
