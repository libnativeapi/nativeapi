#include "menu.h"
#include <sstream>

namespace nativeapi {

std::string KeyboardAccelerator::ToString() const {
  std::ostringstream oss;
  bool first = true;
  if (modifiers & Ctrl) {
    oss << "Ctrl";
    first = false;
  }
  if (modifiers & Alt) {
    if (!first)
      oss << "+";
    oss << "Alt";
    first = false;
  }
  if (modifiers & Shift) {
    if (!first)
      oss << "+";
    oss << "Shift";
    first = false;
  }
  if (modifiers & Meta) {
    if (!first)
      oss << "+";
    oss << "Meta";
    first = false;
  }
  if (!key.empty()) {
    if (!first)
      oss << "+";
    oss << key;
  }
  return oss.str();
}

}  // namespace nativeapi
