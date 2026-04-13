#pragma once

#include <window/x11_window_driver.h>
#include <viewshell/window_handle.h>

namespace viewshell {

void TriggerWindowCallbacksForTest(WindowDriver& driver);
void TriggerCloseCallbackForTest(WindowHandle& window);

}
