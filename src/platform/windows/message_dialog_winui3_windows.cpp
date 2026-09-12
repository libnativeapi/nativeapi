#include "../message_dialog_state.h"
#include <windows.h>
#include <commctrl.h>
#undef GetMessage
#undef GetCurrentTime

#include "../../message_dialog.h"
#include "winui3_runtime_windows.h"
#include <winrt/Microsoft.UI.Content.h>
#include <winrt/Microsoft.UI.Interop.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>
#include <winrt/Microsoft.UI.Xaml.Hosting.h>
#include <winrt/Microsoft.UI.Xaml.Markup.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <iostream>
#include <vector>

namespace nativeapi {
namespace X = winrt::Microsoft::UI::Xaml;
namespace C = X::Controls;
namespace H = X::Hosting;

class MessageDialog::Impl {
 public:
  MessageDialogState state_;
  Impl(const std::string& title, const std::string& message) : title_(title), message_(message) {}
  ~Impl() { Reset(); }

  void SetTitle(const std::string& title) {
    title_ = title;
    if (dialog_ && thread_ == GetCurrentThreadId()) {
      dialog_.Title(winrt::box_value(winrt::to_hstring(title)));
      if (host_) SetWindowTextW(host_, winrt::to_hstring(title).c_str());
    }
  }
  void SetMessage(const std::string& message) {
    message_ = message;
    if (text_ && thread_ == GetCurrentThreadId()) text_.Text(winrt::to_hstring(message));
  }

  bool Open(DialogModality modality) {
    if (running_ || (thread_ && thread_ != GetCurrentThreadId())) return false;
    if (modality != DialogModality::None && modality != DialogModality::Application &&
        modality != DialogModality::Window) return false;
    // Closing a previous island can change activation. Resolve the caller's
    // current parent before teardown, rather than accidentally adopting it.
    HWND owner = state_.parent ? static_cast<HWND>(state_.parent->GetNativeObject()) : GetActiveWindow();
    Reset();
    state_.result = MessageDialogResult::None;
    thread_ = GetCurrentThreadId();
    try {
      InitializeWinUI3();
      if (state_.parent && (!owner || !IsWindow(owner))) return false;
      if (owner && !IsWindowVisible(owner)) owner = nullptr;
      if (owner) {
        // XAML Islands and window subclassing must stay on the owner's UI thread.
        if (GetWindowThreadProcessId(owner, nullptr) != thread_ ||
            !IsWindowEnabled(owner) || GetPropW(owner, kDialogProperty)) return false;
        owner_ = owner;
        if (!SetPropW(owner_, kDialogProperty, this)) winrt::throw_last_error();
        if (!SetWindowSubclass(owner_, OwnerProc, reinterpret_cast<UINT_PTR>(this),
                               reinterpret_cast<DWORD_PTR>(this))) winrt::throw_last_error();
      }
      focus_ = GetFocus();
      // A parentless/tray-only dialog still needs a standalone XAML host.
      if (!owner_) {
        WNDCLASSW wc{};
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.lpszClassName = L"nativeapi.WinUI3.MessageDialog";
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        if (!RegisterClassW(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
          winrt::throw_last_error();
        POINT cursor{};
        GetCursorPos(&cursor);
        HMONITOR monitor = owner ? MonitorFromWindow(owner, MONITOR_DEFAULTTONEAREST)
                                : MonitorFromPoint(cursor, MONITOR_DEFAULTTONEAREST);
        MONITORINFO info{sizeof(info)};
        GetMonitorInfoW(monitor, &info);
        host_ = CreateWindowExW(WS_EX_DLGMODALFRAME | WS_EX_NOREDIRECTIONBITMAP,
            wc.lpszClassName, winrt::to_hstring(title_).c_str(), WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
            info.rcWork.left, info.rcWork.top, 560, 360, owner, nullptr, wc.hInstance, this);
        if (!host_) winrt::throw_last_error();
        const UINT dpi = GetDpiForWindow(host_);
        RECT bounds{0, 0, MulDiv(560, dpi, 96), MulDiv(480, dpi, 96)};
        AdjustWindowRectExForDpi(&bounds, GetWindowLongW(host_, GWL_STYLE), FALSE,
                                GetWindowLongW(host_, GWL_EXSTYLE), dpi);
        const int width = bounds.right - bounds.left;
        const int height = bounds.bottom - bounds.top;
        SetWindowPos(host_, nullptr, info.rcWork.left + (info.rcWork.right - info.rcWork.left - width) / 2,
            info.rcWork.top + (info.rcWork.bottom - info.rcWork.top - height) / 2,
            width, height, SWP_NOZORDER | SWP_NOACTIVATE);
      }
      source_ = H::DesktopWindowXamlSource();
      source_.Initialize(winrt::Microsoft::UI::GetWindowIdFromWindow(owner_ ? owner_ : host_));
      island_ = winrt::Microsoft::UI::GetWindowFromWindowId(source_.SiteBridge().WindowId());
      source_.SiteBridge().ResizePolicy(winrt::Microsoft::UI::Content::ContentSizePolicy::ResizeContentToParentWindow);
      Resize();
      source_.SiteBridge().Show();
      root_ = X::Markup::XamlReader::Load(
          LR"(<Grid xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation" Background="Transparent"/>)").as<C::Grid>();
      text_ = C::TextBlock();
      text_.Text(winrt::to_hstring(message_));
      text_.TextWrapping(X::TextWrapping::Wrap);
      text_.IsTextSelectionEnabled(true);
      C::ScrollViewer scroll;
      C::StackPanel content;
      content.Spacing(12);
      content.Children().Append(text_);
      input_ = C::TextBox();
      input_.TextChanged([this](auto&&, auto&&) { state_.input = winrt::to_string(input_.Text()); });
      checkbox_ = C::CheckBox();
      checkbox_.Checked([this](auto&&, auto&&) { state_.checked = true; });
      checkbox_.Unchecked([this](auto&&, auto&&) { state_.checked = false; });
      progress_ = C::ProgressBar();
      progress_.Minimum(0);
      progress_.Maximum(1);
      content.Children().Append(input_);
      content.Children().Append(checkbox_);
      content.Children().Append(progress_);
      RefreshExtended();
      scroll.Content(content);
      scroll.VerticalScrollBarVisibility(C::ScrollBarVisibility::Auto);
      dialog_ = C::ContentDialog();
      dialog_.Title(winrt::box_value(winrt::to_hstring(title_)));
      dialog_.Content(scroll);
      dialog_.PrimaryButtonText(winrt::to_hstring(state_.primary));
      dialog_.SecondaryButtonText(winrt::to_hstring(state_.secondary));
      dialog_.CloseButtonText(winrt::to_hstring(state_.close));
      auto default_button = C::ContentDialogButton::None;
      if (state_.default_button == MessageDialogResult::Primary && !state_.primary.empty()) default_button = C::ContentDialogButton::Primary;
      if (state_.default_button == MessageDialogResult::Secondary && !state_.secondary.empty()) default_button = C::ContentDialogButton::Secondary;
      if (state_.default_button == MessageDialogResult::Close && !state_.close.empty()) default_button = C::ContentDialogButton::Close;
      dialog_.DefaultButton(default_button);
      opened_ = dialog_.Opened(winrt::auto_revoke, [this](auto&&, auto&&) {
        presented_ = true;
        source_.NavigateFocus(H::XamlSourceFocusNavigationRequest(H::XamlSourceFocusNavigationReason::First));
      });
      closed_ = dialog_.Closed(winrt::auto_revoke, [this](auto&&, const C::ContentDialogClosedEventArgs& args) {
        state_.result = args.Result() == C::ContentDialogResult::Primary ? MessageDialogResult::Primary :
            args.Result() == C::ContentDialogResult::Secondary ? MessageDialogResult::Secondary : MessageDialogResult::Close;
        Finish();
      });
      loaded_ = root_.Loaded(winrt::auto_revoke, [this](auto&&, auto&&) {
        if (!running_ || operation_) return;
        try {
          dialog_.XamlRoot(root_.XamlRoot());
          operation_ = dialog_.ShowAsync();
        } catch (const winrt::hresult_error& e) {
          std::cerr << "WinUI3 dialog: " << winrt::to_string(e.message()) << '\n';
          failed_ = true;
          Finish();
        }
      });
      running_ = true;
      state_.open = true;
      if (owner_) {
        // Keep the top-level HWND enabled: disabling it would disable the island
        // too. Cover its client area and disable only the pre-existing controls.
        for (HWND child = GetWindow(owner_, GW_CHILD); child; child = GetWindow(child, GW_HWNDNEXT)) {
          if (child != island_) Disable(child);
        }
      }
      if (modality == DialogModality::Application) {
        EnumWindows([](HWND window, LPARAM data) -> BOOL {
          auto self = reinterpret_cast<Impl*>(data);
          DWORD process = 0;
          GetWindowThreadProcessId(window, &process);
          if (process == GetCurrentProcessId() && window != self->host_ &&
              window != self->owner_ && IsWindowVisible(window))
            self->Disable(window);
          return TRUE;
        }, reinterpret_cast<LPARAM>(this));
      }
      source_.Content(root_);
      if (host_) {
        SetWindowPos(host_, nullptr, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
      }
      source_.SiteBridge().MoveInZOrderAtTop();
      SetForegroundWindow(owner_ ? owner_ : host_);
      // Wait for actual presentation so initialization/layout errors reach Open.
      // A modeless call then returns; the caller's UI loop continues dispatching.
      while (running_ && (modality != DialogModality::None || !presented_)) {
        MSG msg{};
        const int result = GetMessageW(&msg, nullptr, 0, 0);
        if (result <= 0) {
          if (!result) PostQuitMessage(static_cast<int>(msg.wParam));
          failed_ = true;
          Reset();
          return false;
        }
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
      }
      return presented_ && !failed_;
    } catch (const winrt::hresult_error& e) {
      std::cerr << "WinUI3 dialog: " << winrt::to_string(e.message()) << '\n';
      Reset();
      return false;
    } catch (const std::exception& e) {
      std::cerr << "WinUI3 dialog: " << e.what() << '\n';
      Reset();
      return false;
    }
  }

  bool Close() {
    if (!running_ || thread_ != GetCurrentThreadId()) return false;
    try {
      if (operation_) dialog_.Hide();
      else Finish();
      return true;
    } catch (const winrt::hresult_error& e) {
      std::cerr << "WinUI3 dialog close: " << winrt::to_string(e.message()) << '\n';
      return false;
    }
  }

  void RefreshExtended() {
    if (!input_ || thread_ != GetCurrentThreadId()) return;
    input_.Visibility(state_.input_enabled ? X::Visibility::Visible : X::Visibility::Collapsed);
    if (winrt::to_string(input_.Text()) != state_.input) input_.Text(winrt::to_hstring(state_.input));
    checkbox_.Visibility(state_.checkbox.empty() ? X::Visibility::Collapsed : X::Visibility::Visible);
    checkbox_.Content(winrt::box_value(winrt::to_hstring(state_.checkbox)));
    checkbox_.IsChecked(state_.checked);
    progress_.Visibility(state_.progress == -2 ? X::Visibility::Collapsed : X::Visibility::Visible);
    progress_.IsIndeterminate(state_.progress == -1);
    if (state_.progress >= 0) progress_.Value(state_.progress);
  }
  std::string title_;
  std::string message_;

 private:
  static constexpr const wchar_t* kDialogProperty = L"nativeapi.WinUI3.ActiveMessageDialog";
  static LRESULT CALLBACK OwnerProc(HWND window, UINT message, WPARAM wp, LPARAM lp,
                                    UINT_PTR id, DWORD_PTR data) {
    auto self = reinterpret_cast<Impl*>(data);
    if (message == WM_DESTROY) {
      // Tear down XAML before Windows destroys the borrowed parent/its children.
      self->focus_ = nullptr;
      self->Reset();
      return DefSubclassProc(window, message, wp, lp);
    }
    if (self->running_) {
      if (message == WM_COMMAND || message == WM_NOTIFY ||
          (message == WM_SYSCOMMAND && (wp & 0xfff0) == SC_KEYMENU)) return 0;
      if (message == WM_SETFOCUS) {
        if (self->island_) SetFocus(self->island_);
        return 0;
      }
    }
    const LRESULT result = DefSubclassProc(window, message, wp, lp);
    // Let the embedding framework finish laying out its own child HWND first.
    if ((message == WM_SIZE || message == WM_DPICHANGED) && self->running_) self->Resize();
    return result;
  }
  static LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wp, LPARAM lp) {
    auto self = reinterpret_cast<Impl*>(GetWindowLongPtrW(window, GWLP_USERDATA));
    if (message == WM_NCCREATE) {
      self = static_cast<Impl*>(reinterpret_cast<CREATESTRUCTW*>(lp)->lpCreateParams);
      SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    }
    if (self) {
      if (message == WM_CLOSE) { self->Close(); return 0; }
      if (message == WM_SIZE) { self->Resize(); return 0; }
      if (message == WM_DPICHANGED) {
        const auto rect = reinterpret_cast<RECT*>(lp);
        SetWindowPos(window, nullptr, rect->left, rect->top, rect->right - rect->left,
                     rect->bottom - rect->top, SWP_NOZORDER | SWP_NOACTIVATE);
        return 0;
      }
    }
    return DefWindowProcW(window, message, wp, lp);
  }
  void Resize() noexcept {
    try {
      if (!source_) return;
      RECT rect{};
      GetClientRect(owner_ ? owner_ : host_, &rect);
      source_.SiteBridge().MoveAndResize({0, 0, rect.right, rect.bottom});
      source_.SiteBridge().MoveInZOrderAtTop();
    } catch (...) {}
  }
  void Disable(HWND window) {
    if (IsWindowEnabled(window)) {
      disabled_.push_back(window);
      EnableWindow(window, FALSE);
    }
  }
  void Finish() {
    const bool restore_focus = running_;
    running_ = false;
    state_.open = false;
    try { if (source_) source_.SiteBridge().Hide(); } catch (...) {}
    for (HWND window : disabled_) if (IsWindow(window)) EnableWindow(window, TRUE);
    disabled_.clear();
    if (host_) ShowWindow(host_, SW_HIDE);
    if (owner_ && GetPropW(owner_, kDialogProperty) == this) RemovePropW(owner_, kDialogProperty);
    if (restore_focus && focus_ && IsWindow(focus_) && IsWindowEnabled(focus_)) SetFocus(focus_);
    focus_ = nullptr;
  }
  void Reset() noexcept {
    if (resetting_) return;
    resetting_ = true;
    if (owner_) RemoveWindowSubclass(owner_, OwnerProc, reinterpret_cast<UINT_PTR>(this));
    loaded_.revoke();
    opened_.revoke();
    closed_.revoke();
    try { if (dialog_ && running_) dialog_.Hide(); } catch (...) {}
    Finish();
    operation_ = nullptr;
    dialog_ = nullptr;
    text_ = nullptr;
    input_ = nullptr;
    checkbox_ = nullptr;
    progress_ = nullptr;
    root_ = nullptr;
    try { if (source_) source_.Close(); } catch (...) {}
    source_ = nullptr;
    island_ = nullptr;
    owner_ = nullptr;
    if (host_) DestroyWindow(host_);
    host_ = nullptr;
    presented_ = false;
    failed_ = false;
    resetting_ = false;
  }
  DWORD thread_ = 0;
  HWND host_ = nullptr;
  HWND owner_ = nullptr;  // Borrowed, never hidden, disabled or destroyed by us.
  HWND island_ = nullptr; // Owned by DesktopWindowXamlSource.
  HWND focus_ = nullptr;
  std::vector<HWND> disabled_;
  bool running_ = false;
  bool presented_ = false;
  bool failed_ = false;
  bool resetting_ = false;
  H::DesktopWindowXamlSource source_{nullptr};
  C::Grid root_{nullptr};
  C::TextBlock text_{nullptr};
  C::TextBox input_{nullptr};
  C::CheckBox checkbox_{nullptr};
  C::ProgressBar progress_{nullptr};
  C::ContentDialog dialog_{nullptr};
  winrt::Windows::Foundation::IAsyncOperation<C::ContentDialogResult> operation_{nullptr};
  X::FrameworkElement::Loaded_revoker loaded_;
  C::ContentDialog::Opened_revoker opened_;
  C::ContentDialog::Closed_revoker closed_;
};

MessageDialog::MessageDialog(const std::string& title, const std::string& message)
    : pimpl_(std::make_unique<Impl>(title, message)) {}
MessageDialog::~MessageDialog() = default;
void MessageDialog::SetTitle(const std::string& title) { pimpl_->SetTitle(title); }
std::string MessageDialog::GetTitle() const { return pimpl_->title_; }
void MessageDialog::SetMessage(const std::string& message) { pimpl_->SetMessage(message); }
std::string MessageDialog::GetMessage() const { return pimpl_->message_; }
DialogModality MessageDialog::GetModality() const { return modality_; }
void MessageDialog::SetModality(DialogModality modality) { modality_ = modality; }
bool MessageDialog::Open() { return pimpl_->Open(modality_); }
bool MessageDialog::Close() { return pimpl_->Close(); }
bool MessageDialog::IsExtendedSupported() { return true; }
#include "../message_dialog_extensions.inc"

}  // namespace nativeapi
