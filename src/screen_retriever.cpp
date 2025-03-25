#include "screen_retriever.h"

// Forward declarations for platform-specific implementations
#ifdef __APPLE__
#include "screen_retriever_macos.h"
#elif defined(_WIN32)
#include "screen_retriever_windows.h"
#else
#include "screen_retriever_linux.h"
#endif

std::unique_ptr<ScreenRetriever> ScreenRetriever::Create() {
#ifdef __APPLE__
  return std::unique_ptr<ScreenRetriever>(new ScreenRetrieverMacOS());
#elif defined(_WIN32)
  return std::unique_ptr<ScreenRetriever>(new ScreenRetrieverWindows());
#else
  return std::unique_ptr<ScreenRetriever>(new ScreenRetrieverLinux());
#endif
}

// This file is left intentionally empty as the implementations
// have been moved to platform-specific files:
// - screen_retriever_linux.cpp
// - screen_retriever_macos.mm
// - screen_retriever_windows.cpp
