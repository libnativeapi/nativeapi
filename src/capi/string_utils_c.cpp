#include "string_utils_c.h"
#include <cstring>

char* to_c_str(const std::string& str) {
  if (str.empty())
    return nullptr;

  size_t len = str.length() + 1;
  char* result = new (std::nothrow) char[len];
  if (result) {
    std::strcpy(result, str.c_str());
  }
  return result;
}

void free_c_str(char* str) {
  if (str) {
    delete[] str;
  }
}
