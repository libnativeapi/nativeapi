#pragma once

#include <string>
#include <vector>

#include "foundation/event.h"

namespace nativeapi {

/**
 * @brief Application lifecycle events
 */
class ApplicationEvent : public Event {
 public:
  ApplicationEvent() = default;
  virtual ~ApplicationEvent() = default;
};

/**
 * @brief Event emitted when the application starts
 */
class ApplicationStartedEvent : public ApplicationEvent {
 public:
  ApplicationStartedEvent() = default;
  std::string GetTypeName() const override { return "ApplicationStartedEvent"; }
};

/**
 * @brief Event emitted when the application is about to exit
 */
class ApplicationExitingEvent : public ApplicationEvent {
 public:
  ApplicationExitingEvent(int exit_code) : exit_code_(exit_code) {}

  int GetExitCode() const { return exit_code_; }
  std::string GetTypeName() const override { return "ApplicationExitingEvent"; }

 private:
  int exit_code_;
};

/**
 * @brief Event emitted when the application is activated (brought to foreground)
 */
class ApplicationActivatedEvent : public ApplicationEvent {
 public:
  ApplicationActivatedEvent() = default;
  std::string GetTypeName() const override { return "ApplicationActivatedEvent"; }
};

/**
 * @brief Event emitted when the application is deactivated (sent to background)
 */
class ApplicationDeactivatedEvent : public ApplicationEvent {
 public:
  ApplicationDeactivatedEvent() = default;
  std::string GetTypeName() const override { return "ApplicationDeactivatedEvent"; }
};

/**
 * @brief Event emitted when the application receives a quit request
 */
class ApplicationQuitRequestedEvent : public ApplicationEvent {
 public:
  ApplicationQuitRequestedEvent() = default;
  std::string GetTypeName() const override { return "ApplicationQuitRequestedEvent"; }
};

}  // namespace nativeapi
