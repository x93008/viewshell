#pragma once

namespace viewshell {

inline constexpr const char* kWindowApiBootstrap = R"JS((function () {
  if (!window.__viewshell) return;
  window.__viewshell.wnd = {
    startDrag: function () {
      return window.__viewshell.invoke('__wnd.startDrag', {});
    },
    setPosition: function (x, y) {
      return window.__viewshell.invoke('__wnd.setPosition', { x: x, y: y });
    },
    setSize: function (width, height) {
      return window.__viewshell.invoke('__wnd.setSize', { width: width, height: height });
    },
    setGeometry: function (x, y, width, height) {
      return window.__viewshell.invoke('__wnd.setGeometry', { x: x, y: y, width: width, height: height });
    },
    getGeometry: function () {
      return window.__viewshell.invoke('__wnd.getGeometry', {});
    },
    close: function () {
      return window.__viewshell.invoke('__wnd.close', {});
    }
  };
})();)JS";

} // namespace viewshell
