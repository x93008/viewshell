#pragma once

#include <viewshell/application.h>
#include <viewshell/window_handle.h>
#include <viewshell/bridge_handle.h>

namespace viewshell {

void MarkRunStartedForTest(Application& app);
void MarkShutdownStartedForTest(Application& app);
void MarkWindowClosedForTest(WindowHandle& window);
void ArmCloseAcknowledgementForTest(WindowHandle& window);
void EnterRunLoopForTest(Application& app);
Result<int> FinishRunForTest(Application& app);
void PumpPostedTasksForTest(Application& app);
std::vector<std::string> TakeRuntimeLogsForTest(Application& app);

}
