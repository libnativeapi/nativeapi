#pragma once

#include <memory>
#include <unordered_map>

namespace nativeapi {

// Template function to create and manage global registries for different object
// types Uses heap allocation to avoid destruction order issues at program exit
template <typename T>
static auto& globalRegistry() {
  static auto* instance = new std::unordered_map<void*, std::shared_ptr<T>>();
  return *instance;
}

}  // namespace nativeapi
