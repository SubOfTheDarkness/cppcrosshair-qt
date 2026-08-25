#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/shape.h>
#include <X11/xpm.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <signal.h>
#include <sys/stat.h>
#include "config_parser.h"

const std::string pid_path = "/tmp/crosshair_overlay.pid";
Display* global_display = nullptr;

void handleSystemSignal(int signum) {
    (void)signum;
    unlink(pid_path.c_str());
    if (global_display) {
        XCloseDisplay(global_display);
    }
    _exit(0);
}

int main() {
    struct sigaction action;
    action.sa_handler = handleSystemSignal;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGTERM, &action, nullptr);
    sigaction(SIGINT, &action, nullptr);

    struct sigaction sa_pipe;
    sa_pipe.sa_handler = SIG_IGN;
    sigemptyset(&sa_pipe.sa_mask);
    sa_pipe.sa_flags = 0;
    sigaction(SIGPIPE, &sa_pipe, nullptr);

    std::ifstream check_file(pid_path);
    if (check_file.is_open()) {
        pid_t old_pid;
        if (check_file >> old_pid) {
            if (kill(old_pid, SIGTERM) == 0) {
                usleep(50000); 
            }
        }
        check_file.close();
    }

    std::ofstream pid_file(pid_path, std::ios::trunc);
    if (pid_file.is_open()) {
        pid_file << getpid();
        pid_file.close();
    }

    Config cfg = loadConfig();

    Display* display = XOpenDisplay(NULL);
    if (!display) return 1;
    global_display = display;

    int screen = DefaultScreen(display);
    Window root = RootWindow(display, screen);

    Pixmap color_pixmap;
    Pixmap mask_pixmap;
    XpmAttributes xpm_attrs;
    xpm_attrs.valuemask = 0;

    int result = XpmReadFileToPixmap(display, root, cfg.xpm_path.c_str(), &color_pixmap, &mask_pixmap, &xpm_attrs);
    if (result != XpmSuccess) {
        std::cerr << "[!] Error: Cannot load XPM file: " << cfg.xpm_path << std::endl;
        XCloseDisplay(display);
        unlink(pid_path.c_str());
        return 1;
    }

    Window dummy_win;
    int dummy_x, dummy_y;
    unsigned int width, height, dummy_border, dummy_depth;
    XGetGeometry(display, color_pixmap, &dummy_win, &dummy_x, &dummy_y, &width, &height, &dummy_border, &dummy_depth);

    int screen_w = DisplayWidth(display, screen);
    int screen_h = DisplayHeight(display, screen);
    int x = ((screen_w - width) / 2) + cfg.offset_x;
    int y = ((screen_h - height) / 2) + cfg.offset_y;

    XSetWindowAttributes attrs;
    attrs.override_redirect = True;

    Window win = XCreateWindow(
        display, root, 
        x, y, width, height, 0, 
        CopyFromParent, InputOutput, CopyFromParent, 
        CWOverrideRedirect, &attrs
    );

    XSetWindowBackgroundPixmap(display, win, color_pixmap);
    XShapeCombineMask(display, win, ShapeBounding, 0, 0, mask_pixmap, ShapeSet);

    Region input_region = XCreateRegion();
    XShapeCombineRegion(display, win, ShapeInput, 0, 0, input_region, ShapeSet);
    XDestroyRegion(input_region);

    XMapWindow(display, win);
    
    bool has_hotkey = !cfg.hotkey.empty();
    KeyCode target_keycode = 0;
    unsigned int modifiers = 0;

    if (has_hotkey) {
        std::string hk_str = cfg.hotkey;
        std::string key_char = hk_str;
        size_t last_plus = hk_str.find_last_of('+');
        if (last_plus != std::string::npos) {
            key_char = hk_str.substr(last_plus + 1);
        }

        if (hk_str.find("Ctrl") != std::string::npos)    modifiers |= ControlMask;
        if (hk_str.find("Alt") != std::string::npos)     modifiers |= Mod1Mask;
        if (hk_str.find("Shift") != std::string::npos)   modifiers |= ShiftMask;
        if (hk_str.find("Meta") != std::string::npos)    modifiers |= Mod4Mask;

        target_keycode = XKeysymToKeycode(display, XStringToKeysym(key_char.c_str()));
        
        if (target_keycode != 0) {
            XGrabKey(display, target_keycode, modifiers, root, False, GrabModeAsync, GrabModeAsync);
            std::cout << "[*] Hotkey registered actively: " << hk_str << std::endl;
        } else {
            has_hotkey = false;
            std::cout << "[!] Warning: Config contained bad hotkey string. Keyhook disabled." << std::endl;
        }
    } else {
        std::cout << "[*] Running in pure standalone mode (No global hotkey assigned)." << std::endl;
    }
    
    XFlush(display);

    XFreePixmap(display, color_pixmap);
    XFreePixmap(display, mask_pixmap);

    XEvent ev;
    bool is_visible = true;

    while (true) {
        while (XPending(display) > 0) {
            XNextEvent(display, &ev);
            
            if (ev.type == KeyPress) {
                if (ev.xkey.keycode == target_keycode && (ev.xkey.state & modifiers)) {
                    if (is_visible) {
                        XUnmapWindow(display, win);
                        std::cout << "[*] Overlay hidden via hotkey." << std::endl;
                    } else {
                        XMapWindow(display, win);
                        std::cout << "[*] Overlay shown via hotkey." << std::endl;
                    }
                    is_visible = !is_visible;
                    XFlush(display);
                }
            }
        }

        usleep(20000); 
    }

    unlink(pid_path.c_str());
    XCloseDisplay(display);
    return 0;
}
