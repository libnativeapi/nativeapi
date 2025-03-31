#include <iostream>
#include <string>
#include "screen_retriever.h"

namespace nativeapi {

void ScreenRetriever::AddEventListener(ScreenEventType event_type,
                                       std::function<void(const void*)> listener) {
  std::cout << "\nAdd!" << std::endl;

  listeners_[event_type].push_back(listener);
}

void ScreenRetriever::RemoveEventListener(ScreenEventType event_type,
                                          std::function<void(const void*)> listener) {
  // Note: This is a simplified implementation that removes all listeners for the event type
  listeners_[event_type].clear();
}


void ScreenRetriever::HandleDisplayChange() {
  auto new_displays = GetAllDisplays();

  // Find added displays
  for (const auto& new_display : new_displays) {
    bool found = false;
    for (const auto& current_display : current_displays_) {
      if (new_display.id == current_display.id) {
        found = true;
        break;
      }
    }
    if (!found) {
      // This is a new display
      for (const auto& listener : listeners_[ScreenEventType::DisplayAdded]) {
        listener(&new_display);
      }
    }
  }

  // Find removed displays
  for (const auto& current_display : current_displays_) {
    bool found = false;
    for (const auto& new_display : new_displays) {
      if (current_display.id == new_display.id) {
        found = true;
        break;
      }
    }
    if (!found) {
      // This display was removed
      for (const auto& listener : listeners_[ScreenEventType::DisplayRemoved]) {
        listener(&current_display);
      }
    }
  }

  // Update current display list
  current_displays_ = std::move(new_displays);
}
}  // namespace nativeapi
