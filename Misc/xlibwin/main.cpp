#include <X11/Xlib.h>

#include <string.h>
#include <iostream>

using namespace std;

int main()
{
  Display *display = XOpenDisplay(nullptr);
  int screen = DefaultScreen(display);
  Window window = XCreateSimpleWindow(display, RootWindow(display, screen), 0, 0, 400, 400, 0, BlackPixel(display, screen), WhitePixel(display, screen));
  XStoreName(display, window, "xlibwin");

  XSelectInput(display, window, ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask);
  Atom wmDeleteMessage = XInternAtom(display, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(display, window, &wmDeleteMessage, 1);

  XMapWindow(display, window);

  char const *msg = "Message from the dead";

  bool running = true;
  while (running) {
    XEvent event;
    while (XPending(display)) {
      XNextEvent(display, &event);
      switch (event.type) {
        case Expose:
          XFillRectangle(display, window, DefaultGC(display, screen), 20, 20, 10, 10);
          XDrawString(display, window, DefaultGC(display, screen), 10, 50, msg, strlen(msg));
          break;
        case KeyPress:
//          running = false;
          break;
        case ClientMessage:
          if ((Atom) event.xclient.data.l[0] == wmDeleteMessage)
            running = false;
          break;
      }
    }
  }

  XCloseDisplay(display);

  cout << "Bye world!" << endl;
  return 0;
}
