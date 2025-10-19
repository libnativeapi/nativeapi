#pragma once

#include <functional>
#include <memory>
#include <string>

#include "foundation/event_emitter.h"
#include "keyboard_event.h"

namespace nativeapi {

enum class ModifierKey : uint32_t {
  None = 0,
  Shift = 1 << 0,
  Ctrl = 1 << 1,
  Alt = 1 << 2,
  Meta = 1 << 3,  // Windows key or Cmd key
  Fn = 1 << 4,
  CapsLock = 1 << 5,
  NumLock = 1 << 6,
  ScrollLock = 1 << 7
};

// Bitwise operators for ModifierKey enum
inline ModifierKey operator|(ModifierKey a, ModifierKey b) {
  return static_cast<ModifierKey>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline ModifierKey operator&(ModifierKey a, ModifierKey b) {
  return static_cast<ModifierKey>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

class KeyboardMonitor : public EventEmitter<KeyboardEvent> {
 public:
  KeyboardMonitor();
  virtual ~KeyboardMonitor();

  // Start the keyboard monitor
  void Start();

  // Stop the keyboard monitor
  void Stop();

  // Check if the keyboard monitor is monitoring
  bool IsMonitoring() const;

  // Get access to the event emitter for internal use
  EventEmitter& GetInternalEventEmitter();

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;

  friend class Impl;
};

}  // namespace nativeapi
