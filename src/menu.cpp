#include "menu.h"

namespace nativeapi {
#ifndef _WIN32
bool Menu::SetBackend(MenuBackend backend) {
  return backend == MenuBackend::Native;
}

MenuBackend Menu::GetBackend() const {
  return MenuBackend::Native;
}

bool Menu::IsBackendSupported(MenuBackend backend) {
  return backend == MenuBackend::Native;
}
#endif
}  // namespace nativeapi
