#include "window_proc_delegate_manager.h"

namespace nativeapi {

WindowProcDelegateManager& WindowProcDelegateManager::GetInstance() {
  static WindowProcDelegateManager instance;
  return instance;
}

int WindowProcDelegateManager::RegisterDelegate(WindowProcDelegate delegate) {
  if (!delegate) {
    return -1;
  }

  std::lock_guard<std::mutex> lock(mutex_);

  int id = next_id_++;
  delegates_[id] = std::move(delegate);
  return id;
}

bool WindowProcDelegateManager::UnregisterDelegate(int id) {
  if (id < 0) {
    return false;
  }

  std::lock_guard<std::mutex> lock(mutex_);

  auto it = delegates_.find(id);
  if (it != delegates_.end()) {
    delegates_.erase(it);
    return true;
  }

  return false;
}

}  // namespace nativeapi
