#pragma once
#include <functional>
#include "file_dialog.h"
#include "window.h"
namespace nativeapi {
class FileDialog::Impl {
 public:
  explicit Impl(FileDialogMode value) : mode(value) {
    if (mode == FileDialogMode::SaveFile) extensions = {".txt"};
  }
  ~Impl();
  bool Open();
  bool Close();
  FileDialogMode mode;
  DialogModality modality = DialogModality::Window;
  FileDialogResult result = FileDialogResult::None;
  std::shared_ptr<Window> parent;
  std::vector<std::string> extensions;
  std::string suggested_name;
  std::vector<std::string> paths;
  std::string error;
  bool open = false;
  // Platform cancellation is installed only for the duration of Open().
  std::function<bool()> cancel;
};
}
