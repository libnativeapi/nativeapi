#include <windows.h>
#undef GetMessage
#undef GetCurrentTime

#include "../../message_dialog.h"
#include "winui3_runtime_windows.h"
#include <winrt/Microsoft.UI.Content.h>
#include <winrt/Microsoft.UI.Interop.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Hosting.h>
#include <winrt/Microsoft.UI.Xaml.Markup.h>
#include <winrt/Windows.Foundation.h>
#include <iostream>
#include <vector>

namespace nativeapi {
namespace X = winrt::Microsoft::UI::Xaml;
namespace C = X::Controls;
namespace H = X::Hosting;

class MessageDialog::Impl {
 public:
  Impl(const std::string& title, const std::string& message) : title_(title), message_(message) {}
  ~Impl() { Reset(); }

  void SetTitle(const std::string& title) {
    title_ = title;
    if (dialog_ && thread_ == GetCurrentThreadId()) {
      dialog_.Title(winrt::box_value(winrt::to_hstring(title)));
      SetWindowTextW(host_, winrt::to_hstring(title).c_str());
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
    Reset();
    thread_ = GetCurrentThreadId();
    try {
      InitializeWinUI3();
      HWND owner = GetActiveWindow();
      if (owner && !IsWindowVisible(owner)) owner = nullptr;
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
      RECT bounds{0, 0, MulDiv(560, dpi, 96), MulDiv(360, dpi, 96)};
      AdjustWindowRectExForDpi(&bounds, GetWindowLongW(host_, GWL_STYLE), FALSE,
                              GetWindowLongW(host_, GWL_EXSTYLE), dpi);
      const int width = bounds.right - bounds.left;
      const int height = bounds.bottom - bounds.top;
      SetWindowPos(host_, nullptr, info.rcWork.left + (info.rcWork.right - info.rcWork.left - width) / 2,
          info.rcWork.top + (info.rcWork.bottom - info.rcWork.top - height) / 2,
          width, height, SWP_NOZORDER | SWP_NOACTIVATE);
      source_ = H::DesktopWindowXamlSource();
      source_.Initialize(winrt::Microsoft::UI::GetWindowIdFromWindow(host_));
      source_.SiteBridge().ResizePolicy(winrt::Microsoft::UI::Content::ContentSizePolicy::ResizeContentToParentWindow);
      Resize();
      source_.SiteBridge().Show();
      root_ = X::Markup::XamlReader::Load(
          LR"(<Grid xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation" Background="{ThemeResource ApplicationPageBackgroundThemeBrush}"/>)").as<C::Grid>();
      text_ = C::TextBlock();
      text_.Text(winrt::to_hstring(message_));
      text_.TextWrapping(X::TextWrapping::Wrap);
      text_.IsTextSelectionEnabled(true);
      C::ScrollViewer scroll;
      scroll.Content(text_);
      scroll.VerticalScrollBarVisibility(C::ScrollBarVisibility::Auto);
      dialog_ = C::ContentDialog();
      dialog_.Title(winrt::box_value(winrt::to_hstring(title_)));
      dialog_.Content(scroll);
      dialog_.CloseButtonText(L"OK");
      dialog_.DefaultButton(C::ContentDialogButton::Close);
      opened_ = dialog_.Opened(winrt::auto_revoke, [this](auto&&, auto&&) { presented_ = true; });
      closed_ = dialog_.Closed(winrt::auto_revoke, [this](auto&&, auto&&) { Finish(); });
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
      if (modality == DialogModality::Application) {
        EnumWindows([](HWND window, LPARAM data) -> BOOL {
          auto self = reinterpret_cast<Impl*>(data);
          DWORD process = 0;
          GetWindowThreadProcessId(window, &process);
          if (process == GetCurrentProcessId() && window != self->host_ && IsWindowVisible(window))
            self->Disable(window);
          return TRUE;
        }, reinterpret_cast<LPARAM>(this));
      } else if (modality == DialogModality::Window && owner) {
        Disable(owner);
      }
      source_.Content(root_);
      SetWindowPos(host_, nullptr, 0, 0, 0, 0,
                   SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
      SetForegroundWindow(host_);
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

  std::string title_;
  std::string message_;

 private:
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
      GetClientRect(host_, &rect);
      source_.SiteBridge().MoveAndResize({0, 0, rect.right, rect.bottom});
    } catch (...) {}
  }
  void Disable(HWND window) {
    if (IsWindowEnabled(window)) {
      disabled_.push_back(window);
      EnableWindow(window, FALSE);
    }
  }
  void Finish() {
    running_ = false;
    for (HWND window : disabled_) if (IsWindow(window)) EnableWindow(window, TRUE);
    disabled_.clear();
    if (host_) ShowWindow(host_, SW_HIDE);
  }
  void Reset() noexcept {
    loaded_.revoke();
    opened_.revoke();
    closed_.revoke();
    try { if (dialog_ && running_) dialog_.Hide(); } catch (...) {}
    Finish();
    operation_ = nullptr;
    dialog_ = nullptr;
    text_ = nullptr;
    root_ = nullptr;
    try { if (source_) source_.Close(); } catch (...) {}
    source_ = nullptr;
    if (host_) DestroyWindow(host_);
    host_ = nullptr;
    presented_ = false;
    failed_ = false;
  }
  DWORD thread_ = 0;
  HWND host_ = nullptr;
  std::vector<HWND> disabled_;
  bool running_ = false;
  bool presented_ = false;
  bool failed_ = false;
  H::DesktopWindowXamlSource source_{nullptr};
  C::Grid root_{nullptr};
  C::TextBlock text_{nullptr};
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
}  // namespace nativeapi
