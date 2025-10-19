#include "application.h"

namespace nativeapi {

Application& Application::GetInstance() {
  static Application instance;
  return instance;
}

}  // namespace nativeapi
