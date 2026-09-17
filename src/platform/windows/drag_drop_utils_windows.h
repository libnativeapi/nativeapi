#pragma once

// clang-format off
#include <windows.h>
#include <ole2.h>
// clang-format on

#include "../../drag_source.h"

namespace nativeapi {

inline DWORD ToDropEffect(DragOperation operation) {
  switch (operation) {
    case DragOperation::Copy:
      return DROPEFFECT_COPY;
    case DragOperation::Move:
      return DROPEFFECT_MOVE;
    case DragOperation::Link:
      return DROPEFFECT_LINK;
    case DragOperation::None:
      break;
  }
  return DROPEFFECT_NONE;
}

inline DragOperation FromDropEffect(DWORD effect) {
  if (effect & DROPEFFECT_COPY) return DragOperation::Copy;
  if (effect & DROPEFFECT_MOVE) return DragOperation::Move;
  if (effect & DROPEFFECT_LINK) return DragOperation::Link;
  return DragOperation::None;
}

// Balances OleInitialize() for as long as the object lives. OLE drag and drop
// needs it on the calling thread, on top of the COM apartment a Flutter runner
// already sets up.
class ScopedOleInitialize {
 public:
  ScopedOleInitialize() : succeeded_(SUCCEEDED(OleInitialize(nullptr))) {}
  ~ScopedOleInitialize() {
    if (succeeded_) OleUninitialize();
  }
  ScopedOleInitialize(const ScopedOleInitialize&) = delete;
  ScopedOleInitialize& operator=(const ScopedOleInitialize&) = delete;

  bool Succeeded() const { return succeeded_; }

 private:
  bool succeeded_;
};

}  // namespace nativeapi
