#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/shape.h>
#include <X11/xpm.h>
#include <unistd.h>
#include <iostream>
#include "config_parser.h"

int main() {
    Config cfg = loadConfig();

    Display* display = XOpenDisplay(NULL);
    if (!display) return 1;

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
    XFlush(display);

    XFreePixmap(display, color_pixmap);
    XFreePixmap(display, mask_pixmap);

    while (true) {
        sleep(1);
    }

    XCloseDisplay(display);
    return 0;
}
