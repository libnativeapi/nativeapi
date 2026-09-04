#ifndef NATIVEAPI_PLATFORM_MACOS_STRING_UTILS_H_
#define NATIVEAPI_PLATFORM_MACOS_STRING_UTILS_H_

#import <Foundation/Foundation.h>
#include <string>

namespace nativeapi {
namespace {  // Anonymous namespace, visible only within the translation unit including this header

// Convert NSString to std::string (UTF-8); nil becomes an empty string
inline std::string ToStdString(NSString* value) {
  if (!value) {
    return std::string();
  }
  const char* utf8 = [value UTF8String];
  return utf8 ? std::string(utf8) : std::string();
}

// Convert std::string (UTF-8) to NSString
inline NSString* ToNSString(const std::string& value) {
  return [NSString stringWithUTF8String:value.c_str()];
}

}  // anonymous namespace
}  // namespace nativeapi

#endif  // NATIVEAPI_PLATFORM_MACOS_STRING_UTILS_H_
