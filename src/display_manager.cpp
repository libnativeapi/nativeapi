#include "display_manager.h"

namespace nativeapi {

DisplayManager& DisplayManager::GetInstance() {
  static DisplayManager instance;
  return instance;
}

}  // namespace nativeapi
