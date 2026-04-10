#pragma once

#include <platform/linux_x11/window_driver.h>
#include <viewshell/window_handle.h>

namespace viewshell {

void TriggerWindowCallbacksForTest(WindowDriver& driver);
void TriggerCloseCallbackForTest(WindowHandle& window);

}
