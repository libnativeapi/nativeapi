#include <iostream>

#include "nativeapi.h"

using nativeapi::DeviceInfo;

int main() {
  auto& info = DeviceInfo::GetInstance();

  std::cout << "Name:           " << info.GetName() << std::endl;
  std::cout << "Model:          " << info.GetModel() << std::endl;
  std::cout << "Manufacturer:   " << info.GetManufacturer() << std::endl;
  std::cout << "OS name:        " << info.GetOsName() << std::endl;
  std::cout << "OS version:     " << info.GetOsVersion() << std::endl;
  std::cout << "Kernel version: " << info.GetKernelVersion() << std::endl;
  std::cout << "Architecture:   " << info.GetArchitecture() << std::endl;

  return 0;
}
