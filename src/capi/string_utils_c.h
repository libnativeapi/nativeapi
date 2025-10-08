#pragma once

#ifdef __cplusplus
#include <string>
#endif

#if _WIN32
#define FFI_PLUGIN_EXPORT __declspec(dllexport)
#else
#define FFI_PLUGIN_EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * String utilities for C API interoperability
 */

/**
 * Convert a C++ string to a C string with memory allocation
 * @param str The C++ string to convert
 * @return Allocated C string copy, or nullptr if str is empty or allocation
 * failed. Caller must free the returned string with free_c_str().
 */
#ifdef __cplusplus
char* to_c_str(const std::string& str);
#endif

/**
 * Free a C string allocated by to_c_str
 * @param str The string to free (can be nullptr)
 */
FFI_PLUGIN_EXPORT
void free_c_str(char* str);

#ifdef __cplusplus
}
#endif
