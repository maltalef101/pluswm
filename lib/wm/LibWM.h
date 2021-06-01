#pragma once

#include <X11/Xlib.h>
#include <memory>
#include <unordered_map>
#include <vector>

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

    void on_CreateNotify(const XCreateWindowEvent&);
    void on_DestroyNotify(const XDestroyWindowEvent&);

    void on_MapRequest(const XMapRequestEvent&);
    void on_MapNotify(const XMapEvent&);

    void on_ConfigureRequest(const XConfigureRequestEvent&);
    void on_ConfigureNotify(const XConfigureEvent&);

    Display* m_display;
    const Window m_root_window;

    int m_screen;
    int m_screen_width;
    int m_screen_height;

    std::unordered_map<Window, Window> m_clients;
    std::vector<Window> m_stack;

    inline static bool m_wm_detected = false;

    Atom m_wmatom[sizeof(wmatom)];
};
