#include "winui3_runtime_windows.h"
#include <windows.h>
#undef GetCurrentTime
#include <MddBootstrap.h>
#include <WindowsAppSDK-VersionInfo.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Microsoft.UI.Dispatching.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Hosting.h>
#include <winrt/Microsoft.UI.Xaml.Markup.h>
#include <winrt/Microsoft.UI.Xaml.XamlTypeInfo.h>


namespace nativeapi {
namespace {
namespace X = winrt::Microsoft::UI::Xaml;
namespace C = X::Controls;
namespace D = winrt::Microsoft::UI::Dispatching;
namespace H = X::Hosting;
struct BackendApplication : X::ApplicationT<BackendApplication, X::Markup::IXamlMetadataProvider> {
  X::XamlTypeInfo::XamlControlsXamlMetaDataProvider provider;
  X::Markup::IXamlType GetXamlType(const winrt::Windows::UI::Xaml::Interop::TypeName& type) {
    return provider.GetXamlType(type);
  }
  X::Markup::IXamlType GetXamlType(const winrt::hstring& name) {
    return provider.GetXamlType(name);
  }
  winrt::com_array<X::Markup::XmlnsDefinition> GetXmlnsDefinitions() {
    return provider.GetXmlnsDefinitions();
  }
};

// One XAML environment per calling STA, reused between popup sessions. Never
// shut down a DispatcherQueue owned by an embedding application.
struct XamlThread {
  HMODULE bootstrap = nullptr;
  bool bootstrapped = false;
  bool initialized = false;
  D::DispatcherQueueController controller{nullptr};
  H::WindowsXamlManager manager{nullptr};
  X::Application application{nullptr};

  void Initialize() {
    if (initialized) return;
    APTTYPE apartment;
    APTTYPEQUALIFIER qualifier;
    winrt::check_hresult(CoGetApartmentType(&apartment, &qualifier));
    if (apartment != APTTYPE_STA && apartment != APTTYPE_MAINSTA)
      winrt::throw_hresult(RPC_E_WRONG_THREAD);

    // A WinUI host may already have configured its package graph and queue.
    if (!winrt::try_get_activation_factory<X::Application>()) {
      if (!bootstrap)
        bootstrap = LoadLibraryExW(L"Microsoft.WindowsAppRuntime.Bootstrap.dll", nullptr,
                                  LOAD_LIBRARY_SEARCH_APPLICATION_DIR | LOAD_LIBRARY_SEARCH_SYSTEM32);
      if (!bootstrap) winrt::throw_last_error();
      auto initialize = reinterpret_cast<decltype(&MddBootstrapInitialize2)>(
          GetProcAddress(bootstrap, "MddBootstrapInitialize2"));
      if (!initialize) winrt::throw_hresult(E_NOINTERFACE);
      PACKAGE_VERSION version{};
      version.Version = WINDOWSAPPSDK_RUNTIME_VERSION_UINT64;
      winrt::check_hresult(initialize(WINDOWSAPPSDK_RELEASE_MAJORMINOR,
          WINDOWSAPPSDK_RELEASE_VERSION_TAG_W, version, MddBootstrapInitializeOptions_None));
      bootstrapped = true;
    }
    if (!D::DispatcherQueue::GetForCurrentThread())
      controller = D::DispatcherQueueController::CreateOnCurrentThread();
    if (!X::Application::Current()) application = winrt::make<BackendApplication>();
    manager = H::WindowsXamlManager::InitializeForCurrentThread();
    if (application)
      X::Application::Current().Resources().MergedDictionaries().Append(C::XamlControlsResources());
    initialized = true;
  }

  ~XamlThread() {
    try {
      if (manager) manager.Close();
      manager = nullptr;
      if (controller) {
        auto shutdown = controller.ShutdownQueueAsync();
        bool quit = false;
        int exit_code = 0;
        while (shutdown.Status() == winrt::Windows::Foundation::AsyncStatus::Started) {
          MSG msg{};
          const int result = GetMessageW(&msg, nullptr, 0, 0);
          if (result < 0) break;
          if (!result) { quit = true; exit_code = static_cast<int>(msg.wParam); continue; }
          TranslateMessage(&msg);
          DispatchMessageW(&msg);
        }
        if (quit) PostQuitMessage(exit_code);
      }
    } catch (...) { /* No exceptions from thread teardown. */ }
    application = nullptr;
    controller = nullptr;
    if (bootstrapped) {
      auto shutdown = reinterpret_cast<decltype(&MddBootstrapShutdown)>(
          GetProcAddress(bootstrap, "MddBootstrapShutdown"));
      if (shutdown) shutdown();
    }
    if (bootstrap) FreeLibrary(bootstrap);
  }
};

}
void InitializeWinUI3() {
  thread_local XamlThread runtime;
  runtime.Initialize();
}
}
