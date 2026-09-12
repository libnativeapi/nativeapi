#pragma once
#include <memory>
#include <string>
#include "foundation/event_emitter.h"

namespace nativeapi {
class NotificationEvent : public Event {
 public:
  std::string GetTypeName() const override { return "NotificationEvent"; }
};
class NotificationActivatedEvent : public NotificationEvent {
 public:
  explicit NotificationActivatedEvent(const std::string& argument) : argument_(argument) {}
  const std::string& GetArgument() const { return argument_; }
  std::string GetTypeName() const override { return "NotificationActivatedEvent"; }
 private:
  std::string argument_;
};

/** System notifications. Windows App SDK backend requires NATIVEAPI_ENABLE_WINUI3.
 * Subscribe before Initialize(), including on notification-triggered app launches.
 * Methods run on the main STA; activation events are delivered on the main dispatcher.
 * IsSupported reports build support, not runtime installation or notification settings.
 */
class NotificationManager : public EventEmitter<NotificationEvent> {
 public:
  static NotificationManager& GetInstance();
  ~NotificationManager();
  NotificationManager(const NotificationManager&) = delete;
  NotificationManager& operator=(const NotificationManager&) = delete;
  NotificationManager(NotificationManager&&) = delete;
  NotificationManager& operator=(NotificationManager&&) = delete;
  static bool IsSupported();
  /** Registers this process with Windows. Call once at application startup. */
  bool Initialize();
  /** Release registration before stopping the UI loop. Delivered notifications remain. */
  void Shutdown();
  /** Uses tag to replace an existing notification. Optional button emits action=button.
   * True means accepted by the OS; notification settings may suppress the banner.
   */
  bool Show(const std::string& title, const std::string& message,
            const std::string& tag, const std::string& button_label);
  bool Remove(const std::string& tag);
  std::string GetLastError() const;
 private:
  NotificationManager();
  class Impl;
  std::unique_ptr<Impl> pimpl_;
};
}
