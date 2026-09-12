#include <iostream>
#include <string>
#include "nativeapi.h"

using namespace nativeapi;
int main(int argc, char** argv) {
  auto& app = Application::GetInstance();
  const std::string mode = argc > 1 ? argv[1] : "dialog";
  if (mode == "notify") {
    auto& notifications = NotificationManager::GetInstance();
    notifications.AddListener<NotificationActivatedEvent>([&](const auto& event) {
      std::cout << "Activated: " << event.GetArgument() << std::endl;
      notifications.Remove("demo");
      app.Quit();
    });
    if (!notifications.Initialize() ||
        !notifications.Show("nativeapi", "System notification example", "demo", "Open")) {
      std::cerr << notifications.GetLastError() << std::endl;
      notifications.Shutdown();
      return 1;
    }
    std::cout << "Click the notification or its Open button to finish." << std::endl;
    const int result = app.Run();
    notifications.Shutdown();
    return result;
  }
  auto window = std::make_shared<Window>();
  window->SetTitle("nativeapi desktop features");
  window->SetSize({720, 560}, false);
  window->SetVisualEffect(VisualEffect::Mica);
  window->SetTitleBarColors(Color::FromRGBA(30, 65, 110), Color::White);
  window->Show();
  if (mode == "window") return app.Run();
  if (mode == "dialog") {
    if (!MessageDialog::IsExtendedSupported()) {
      std::cerr << "Build with NATIVEAPI_ENABLE_WINUI3=ON for extended dialogs.\n";
      return 1;
    }
    MessageDialog dialog("Extended MessageDialog", "Enter a name and choose an action.");
    dialog.SetParentWindow(window);
    dialog.SetModality(DialogModality::Window);
    dialog.SetButtons("Save", "Discard", "Cancel");
    dialog.SetDefaultButton(MessageDialogResult::Primary);
    dialog.SetInputEnabled(true);
    dialog.SetInputText("nativeapi");
    dialog.SetCheckbox("Remember my choice", true);
    dialog.SetProgress(0.65);
    if (!dialog.Open()) return 1;
    std::cout << "Result=" << static_cast<int>(dialog.GetResult())
              << " input=" << dialog.GetInputText()
              << " checked=" << dialog.IsCheckboxChecked() << std::endl;
    return 0;
  }
  FileDialogMode picker_mode;
  if (mode == "open") picker_mode = FileDialogMode::OpenFile;
  else if (mode == "multiple") picker_mode = FileDialogMode::OpenFiles;
  else if (mode == "save") picker_mode = FileDialogMode::SaveFile;
  else if (mode == "folder") picker_mode = FileDialogMode::SelectFolder;
  else { std::cerr << "Modes: dialog, window, open, multiple, save, folder, notify\n"; return 1; }
  FileDialog picker(picker_mode);
  picker.SetParentWindow(window);
  if (picker_mode != FileDialogMode::SelectFolder) picker.SetFileTypes({".txt", ".md"});
  picker.SetSuggestedFileName("nativeapi.txt");
  if (!picker.Open()) { std::cerr << picker.GetLastError() << std::endl; return 1; }
  std::cout << "Picker result=" << static_cast<int>(picker.GetResult()) << '\n';
  for (const auto& path : picker.GetPaths()) std::cout << path << '\n';
}
