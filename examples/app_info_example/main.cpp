#include <iostream>

#include "nativeapi.h"

using nativeapi::AppInfo;

int main() {
  auto& info = AppInfo::GetInstance();

  std::cout << "Name:         " << info.GetName() << std::endl;
  std::cout << "Identifier:   " << info.GetIdentifier() << std::endl;
  std::cout << "Version:      " << info.GetVersion() << std::endl;
  std::cout << "Build number: " << info.GetBuildNumber() << std::endl;

  return 0;
}
