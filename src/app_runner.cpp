#include "app_runner.h"

namespace nativeapi {

AppRunner& AppRunner::GetInstance() {
  static AppRunner instance;
  return instance;
}

int RunApp(std::shared_ptr<Window> window) {
  return AppRunner::GetInstance().Run(window);
}

}  // namespace nativeapi
