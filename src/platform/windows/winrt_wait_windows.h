#pragma once
#include <windows.h>
#include <winrt/Windows.Foundation.h>

namespace nativeapi {
// Wait on the calling STA without blocking dispatcher work or consuming WM_QUIT.
template <typename Operation>
void WaitForWinRT(const Operation& operation) {
  while (operation.Status() == winrt::Windows::Foundation::AsyncStatus::Started) {
    MSG message{};
    if (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
      if (message.message == WM_QUIT) {
        operation.Cancel();
        PostQuitMessage(static_cast<int>(message.wParam));
        winrt::throw_hresult(HRESULT_FROM_WIN32(ERROR_CANCELLED));
      }
      TranslateMessage(&message);
      DispatchMessageW(&message);
    } else {
      MsgWaitForMultipleObjectsEx(0, nullptr, 10, QS_ALLINPUT, MWMO_INPUTAVAILABLE);
    }
  }
  if (operation.Status() == winrt::Windows::Foundation::AsyncStatus::Canceled)
    winrt::throw_hresult(HRESULT_FROM_WIN32(ERROR_CANCELLED));
  if (operation.Status() == winrt::Windows::Foundation::AsyncStatus::Error)
    winrt::throw_hresult(operation.ErrorCode());
}
}
