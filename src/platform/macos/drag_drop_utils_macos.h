#pragma once

#import <Cocoa/Cocoa.h>

#include "../../drag_source.h"

namespace nativeapi {

// The library is built both with ARC (CocoaPods) and without it (CMake).
// Objects stored in C++ structs are retained and released explicitly under
// manual reference counting, and left to ARC otherwise.
inline id RetainIfManual(id object) {
#if __has_feature(objc_arc)
  return object;
#else
  return [object retain];
#endif
}

inline void ReleaseIfManual(id object) {
#if !__has_feature(objc_arc)
  [object release];
#else
  (void)object;
#endif
}

inline NSDragOperation ToNSDragOperation(DragOperation operation) {
  switch (operation) {
    case DragOperation::Copy:
      return NSDragOperationCopy;
    case DragOperation::Move:
      return NSDragOperationMove;
    case DragOperation::Link:
      return NSDragOperationLink;
    case DragOperation::None:
      break;
  }
  return NSDragOperationNone;
}

inline DragOperation FromNSDragOperation(NSDragOperation operation) {
  if (operation & NSDragOperationCopy) return DragOperation::Copy;
  if (operation & NSDragOperationMove) return DragOperation::Move;
  if (operation & NSDragOperationLink) return DragOperation::Link;
  return DragOperation::None;
}

}  // namespace nativeapi
