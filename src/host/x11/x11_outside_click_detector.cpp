#include "x11_outside_click_detector.h"

#include <unistd.h>
#include <sys/select.h>
#include <glib.h>
#include <X11/Xlib.h>
#include <X11/extensions/XInput2.h>

namespace viewshell {

X11GlobalClickListener& X11GlobalClickListener::instance() {
  static X11GlobalClickListener inst;
  return inst;
}

X11GlobalClickListener::~X11GlobalClickListener() {
  std::lock_guard<std::mutex> lock(mutex_);
  stop_locked();
}

X11GlobalClickListener::ListenerId X11GlobalClickListener::add_listener(ClickHandler handler) {
  std::lock_guard<std::mutex> lock(mutex_);
  auto id = next_id_++;
  handlers_.emplace(id, std::move(handler));
  if (handlers_.size() == 1) {
    start_locked();
  }
  return id;
}

void X11GlobalClickListener::remove_listener(ListenerId id) {
  std::lock_guard<std::mutex> lock(mutex_);
  handlers_.erase(id);
  if (handlers_.empty()) {
    stop_locked();
  }
}

void X11GlobalClickListener::start_locked() {
  stop_locked();

  if (::pipe(pipe_fd_) != 0) {
    pipe_fd_[0] = pipe_fd_[1] = -1;
  }

  listening_ = true;

  thread_ = std::make_unique<std::thread>([this]() {
    Display* display = XOpenDisplay(nullptr);
    if (!display) return;

    int xi_opcode, first_event, error;
    if (!XQueryExtension(display, "XInputExtension", &xi_opcode, &first_event, &error)) {
      XCloseDisplay(display);
      return;
    }
    int major = 2, minor = 2;
    if (XIQueryVersion(display, &major, &minor) != Success) {
      XCloseDisplay(display);
      return;
    }

    XIEventMask mask;
    unsigned char mask_bits[XIMaskLen(XI_LASTEVENT)] = {};
    mask.deviceid = XIAllDevices;
    mask.mask_len = sizeof(mask_bits);
    mask.mask = mask_bits;
    XISetMask(mask.mask, XI_RawButtonPress);
    XISelectEvents(display, DefaultRootWindow(display), &mask, 1);
    XSync(display, False);

    int x11_fd = ConnectionNumber(display);
    int read_fd = pipe_fd_[0];

    while (listening_) {
      while (XPending(display) > 0) {
        XEvent xevent;
        XNextEvent(display, &xevent);
        XGenericEventCookie* cookie = &xevent.xcookie;
        if (XGetEventData(display, cookie) && cookie->type == GenericEvent &&
            cookie->evtype == XI_RawButtonPress) {
          XIRawEvent* raw = static_cast<XIRawEvent*>(cookie->data);
          if (raw->detail == 1 || raw->detail == 3) {
            Window root_ret, child_ret;
            int root_x, root_y, win_x, win_y;
            unsigned int btn_mask;
            XQueryPointer(display, DefaultRootWindow(display),
                &root_ret, &child_ret, &root_x, &root_y, &win_x, &win_y, &btn_mask);

            ClickEvent evt{root_x, root_y,
                raw->detail == 1 ? Button::Left : Button::Right};

            // Snapshot current handlers and dispatch on GTK main thread
            std::lock_guard<std::mutex> lock(mutex_);
            for (const auto& [id, handler] : handlers_) {
              auto fn = std::make_shared<ClickHandler>(handler);
              auto ev = evt;
              g_idle_add([](gpointer data) -> gboolean {
                auto* pair = static_cast<std::pair<std::shared_ptr<ClickHandler>, ClickEvent>*>(data);
                if (pair->first && *pair->first) {
                  (*pair->first)(pair->second);
                }
                delete pair;
                return FALSE;
              }, new std::pair<std::shared_ptr<ClickHandler>, ClickEvent>(fn, ev));
            }
          }
        }
        XFreeEventData(display, cookie);
      }

      fd_set fds;
      FD_ZERO(&fds);
      FD_SET(x11_fd, &fds);
      int max_fd = x11_fd;
      if (read_fd >= 0) {
        FD_SET(read_fd, &fds);
        if (read_fd > max_fd) max_fd = read_fd;
      }
      select(max_fd + 1, &fds, nullptr, nullptr, nullptr);
      if (read_fd >= 0 && FD_ISSET(read_fd, &fds)) {
        break;
      }
    }

    XCloseDisplay(display);
  });
}

void X11GlobalClickListener::stop_locked() {
  listening_ = false;
  if (pipe_fd_[1] >= 0) {
    char c = 1;
    (void)::write(pipe_fd_[1], &c, 1);
  }
  if (thread_ && thread_->joinable()) {
    thread_->join();
  }
  thread_.reset();
  for (int i = 0; i < 2; ++i) {
    if (pipe_fd_[i] >= 0) {
      ::close(pipe_fd_[i]);
      pipe_fd_[i] = -1;
    }
  }
}

} // namespace viewshell
