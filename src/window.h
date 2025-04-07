#pragma once
#include <string>

namespace nativeapi {

class Window {
 public:
  Window();
  virtual ~Window();

  std::string id;
  std::string name;
};

}  // namespace nativeapi