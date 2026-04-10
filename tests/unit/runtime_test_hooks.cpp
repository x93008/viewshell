#include "runtime_test_hooks.h"
#include "../../src/viewshell/runtime_state.h"

namespace viewshell {

void MarkRunStartedForTest(Application& app) {
  app.app_state_->run_started = true;
}

void MarkShutdownStartedForTest(Application& app) {
  app.app_state_->shutdown_started = true;
}

void MarkWindowClosedForTest(WindowHandle& window) {
  window.state_->is_closed = true;
}

void ArmCloseAcknowledgementForTest(WindowHandle& window) {
  window.state_->close_acknowledged = true;
}

void EnterRunLoopForTest(Application& app) {
  app.app_state_->run_started = true;
}

Result<int> FinishRunForTest(Application& app) {
  app.app_state_->shutdown_started = true;
  return app.app_state_->run_exit_code;
}

void PumpPostedTasksForTest(Application& app) {
  std::deque<std::function<void()>> tasks;
  {
    std::lock_guard<std::mutex> lock(app.app_state_->mutex);
    tasks.swap(app.app_state_->posted_tasks);
  }
  for (auto& task : tasks) {
    try {
      task();
    } catch (const std::exception& e) {
      app.app_state_->logs.push_back(std::string("posted task threw: ") + e.what());
    } catch (...) {
      app.app_state_->logs.push_back("posted task threw unknown exception");
    }
  }
}

std::vector<std::string> TakeRuntimeLogsForTest(Application& app) {
  std::vector<std::string> result;
  std::swap(result, app.app_state_->logs);
  return result;
}

}
