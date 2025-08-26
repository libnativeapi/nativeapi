#include "event_system.h"

namespace nativeapi {

EventDispatcher& GetGlobalEventDispatcher() {
  static EventDispatcher global_dispatcher;
  return global_dispatcher;
}

}  // namespace nativeapi