#include "../../notification_manager.h"
#ifdef NATIVEAPI_ENABLE_WINUI3
#include <windows.h>
#undef GetCurrentTime
#include "winui3_runtime_windows.h"
#include "winrt_wait_windows.h"
#include <winrt/Microsoft.Windows.AppNotifications.h>
#include <winrt/Microsoft.Windows.AppNotifications.Builder.h>
#include <mutex>
#endif

namespace nativeapi {
class NotificationManager::Impl {
 public:
  std::string error;
#ifdef NATIVEAPI_ENABLE_WINUI3
  struct CallbackState {
    std::mutex mutex;
    NotificationManager* owner = nullptr;
  };
  std::shared_ptr<CallbackState> state = std::make_shared<CallbackState>();
  winrt::Microsoft::Windows::AppNotifications::AppNotificationManager manager{nullptr};
  winrt::event_token token{};
  bool registered = false;
  DWORD thread = 0;
#endif
};
NotificationManager::NotificationManager() : pimpl_(std::make_unique<Impl>()) {}
NotificationManager& NotificationManager::GetInstance() {
  static NotificationManager instance;
  return instance;
}
NotificationManager::~NotificationManager() {
  ShutdownEmitter();
  Shutdown();
}
bool NotificationManager::IsSupported() {
#ifdef NATIVEAPI_ENABLE_WINUI3
  return true;
#else
  return false;
#endif
}
bool NotificationManager::Initialize() {
#ifdef NATIVEAPI_ENABLE_WINUI3
  if (pimpl_->registered) return pimpl_->thread == GetCurrentThreadId();
  pimpl_->error.clear();
  try {
    InitializeWinUI3();
    pimpl_->thread = GetCurrentThreadId();
    pimpl_->manager = winrt::Microsoft::Windows::AppNotifications::AppNotificationManager::Default();
    {
      std::lock_guard<std::mutex> lock(pimpl_->state->mutex);
      pimpl_->state->owner = this;
    }
    pimpl_->token = pimpl_->manager.NotificationInvoked([state = pimpl_->state](auto&&, auto&& args) {
      std::lock_guard<std::mutex> lock(state->mutex);
      if (state->owner)
        state->owner->EmitAsync<NotificationActivatedEvent>(winrt::to_string(args.Argument()));
    });
    pimpl_->manager.Register();
    pimpl_->registered = true;
    return true;
  } catch (const winrt::hresult_error& e) {
    pimpl_->error = winrt::to_string(e.message());
    Shutdown();
    return false;
  }
#else
  pimpl_->error = "System notifications require the Windows WinUI3 build";
  return false;
#endif
}
void NotificationManager::Shutdown() {
#ifdef NATIVEAPI_ENABLE_WINUI3
  {
    std::lock_guard<std::mutex> lock(pimpl_->state->mutex);
    pimpl_->state->owner = nullptr;
  }
  try {
    if (pimpl_->manager) {
      if (pimpl_->token.value) pimpl_->manager.NotificationInvoked(pimpl_->token);
      if (pimpl_->registered) pimpl_->manager.Unregister();
    }
  } catch (const winrt::hresult_error& e) { pimpl_->error = winrt::to_string(e.message()); }
  pimpl_->manager = nullptr;
  pimpl_->token = {};
  pimpl_->registered = false;
#endif
}
bool NotificationManager::Show(const std::string& title, const std::string& message,
                               const std::string& tag, const std::string& button_label) {
#ifdef NATIVEAPI_ENABLE_WINUI3
  if (!pimpl_->registered || pimpl_->thread != GetCurrentThreadId()) {
    pimpl_->error = "Initialize notifications on the calling UI thread first";
    return false;
  }
  if (tag.empty() || tag.size() > 16 || tag.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-") != std::string::npos) {
    pimpl_->error = "Notification tag must contain 1-16 ASCII letters, digits, underscores or hyphens";
    return false;
  }
  pimpl_->error.clear();
  try {
    namespace B = winrt::Microsoft::Windows::AppNotifications::Builder;
    B::AppNotificationBuilder builder;
    builder.AddText(winrt::to_hstring(title));
    builder.AddText(winrt::to_hstring(message));
    builder.AddArgument(L"tag", winrt::to_hstring(tag));
    builder.AddArgument(L"action", L"open");
    if (!button_label.empty()) {
      B::AppNotificationButton button(winrt::to_hstring(button_label));
      button.AddArgument(L"tag", winrt::to_hstring(tag));
      button.AddArgument(L"action", L"button");
      builder.AddButton(button);
    }
    auto notification = builder.BuildNotification();
    notification.Tag(winrt::to_hstring(tag));
    notification.Group(L"nativeapi");
    pimpl_->manager.Show(notification);
    if (!notification.Id()) { pimpl_->error = "Windows did not assign a notification ID"; return false; }
    return true;
  } catch (const winrt::hresult_error& e) { pimpl_->error = winrt::to_string(e.message()); }
#else
  pimpl_->error = "System notifications require the Windows WinUI3 build";
#endif
  return false;
}
bool NotificationManager::Remove(const std::string& tag) {
#ifdef NATIVEAPI_ENABLE_WINUI3
  if (!pimpl_->registered || pimpl_->thread != GetCurrentThreadId()) {
    pimpl_->error = "Initialize notifications on the calling UI thread first";
    return false;
  }
  try {
    auto operation = pimpl_->manager.RemoveByTagAndGroupAsync(winrt::to_hstring(tag), L"nativeapi");
    WaitForWinRT(operation);
    operation.GetResults();
    pimpl_->error.clear();
    return true;
  } catch (const winrt::hresult_error& e) { pimpl_->error = winrt::to_string(e.message()); }
#else
  pimpl_->error = "System notifications require the Windows WinUI3 build";
#endif
  return false;
}
std::string NotificationManager::GetLastError() const { return pimpl_->error; }
}
