#pragma once
#include <memory>
#include <string>
#include <vector>
#include "dialog.h"
namespace nativeapi {
class Window;
enum class FileDialogMode { OpenFile, OpenFiles, SaveFile, SelectFolder };
enum class FileDialogResult { None, Accepted, Cancelled, Failed };
/** System file picker. UI-thread only; Open pumps messages until dismissed.
 * Windows: WinRT picker with WinUI3 enabled, IFileDialog otherwise.
 * Other platforms currently return IsSupported() == false.
 */
class FileDialog : public Dialog {
 public:
  explicit FileDialog(FileDialogMode mode);
  ~FileDialog() override;
  FileDialog(const FileDialog&) = delete;
  FileDialog& operator=(const FileDialog&) = delete;
  FileDialog(FileDialog&&) = delete;
  FileDialog& operator=(FileDialog&&) = delete;
  static bool IsSupported();
  /** Configuration setters reject changes while open. */
  bool SetParentWindow(std::shared_ptr<Window> window);
  /** Extensions such as .txt or .png; * allowed for Open, not Save. */
  bool SetFileTypes(const std::vector<std::string>& extensions);
  bool SetSuggestedFileName(const std::string& name);
  DialogModality GetModality() const override;
  /** Pickers are window-modal. None/Application are rejected by Open. */
  void SetModality(DialogModality modality) override;
  /** True on acceptance or cancellation; false on failure. Inspect GetResult().
   * WinRT SaveFile may create an empty selected file as part of the system picker.
   */
  bool Open() override;
  bool Close() override;
  FileDialogResult GetResult() const;
  std::vector<std::string> GetPaths() const;
  std::string GetLastError() const;
 private:
  class Impl;
  std::unique_ptr<Impl> pimpl_;
};
}
