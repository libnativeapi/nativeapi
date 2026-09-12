#include "file_dialog_impl.h"
namespace nativeapi {
FileDialog::FileDialog(FileDialogMode mode) : pimpl_(std::make_unique<Impl>(mode)) {}
FileDialog::~FileDialog() = default;
bool FileDialog::SetParentWindow(std::shared_ptr<Window> window) {
  if (pimpl_->open) return false;
  pimpl_->parent = std::move(window);
  return true;
}
bool FileDialog::SetFileTypes(const std::vector<std::string>& extensions) {
  if (pimpl_->open) return false;
  for (const auto& ext : extensions) {
    if (ext == "*" && pimpl_->mode != FileDialogMode::SaveFile) continue;
    if (ext.size() < 2 || ext[0] != '.' || ext.find_first_of("*/\\:;?\"<>|") != std::string::npos)
      return false;
  }
  pimpl_->extensions = extensions;
  return true;
}
bool FileDialog::SetSuggestedFileName(const std::string& name) {
  if (pimpl_->open || name.find_first_of("/\\:;?*\"<>|") != std::string::npos) return false;
  pimpl_->suggested_name = name;
  return true;
}
DialogModality FileDialog::GetModality() const { return pimpl_->modality; }
void FileDialog::SetModality(DialogModality modality) { if (!pimpl_->open) pimpl_->modality = modality; }
bool FileDialog::Open() {
  if (pimpl_->open) return false;
  pimpl_->result = FileDialogResult::None;
  pimpl_->paths.clear();
  pimpl_->error.clear();
  if (pimpl_->modality != DialogModality::Window ||
      pimpl_->mode < FileDialogMode::OpenFile || pimpl_->mode > FileDialogMode::SelectFolder) {
    pimpl_->result = FileDialogResult::Failed;
    pimpl_->error = "Invalid picker mode or modality";
    return false;
  }
  return pimpl_->Open();
}
bool FileDialog::Close() { return pimpl_->Close(); }
FileDialogResult FileDialog::GetResult() const { return pimpl_->result; }
std::vector<std::string> FileDialog::GetPaths() const { return pimpl_->paths; }
std::string FileDialog::GetLastError() const { return pimpl_->error; }
}
