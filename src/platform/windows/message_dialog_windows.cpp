#include "../../message_dialog.h"
#include "../../dialog.h"

namespace nativeapi {

// Private implementation class for MessageDialog (Windows stub)
class MessageDialog::Impl {
 public:
  Impl(const std::string& title, const std::string& message)
      : title_(title), message_(message) {
    // TODO: Implement Windows MessageBox or Task Dialog
  }

  ~Impl() {
    // TODO: Cleanup if needed
  }

  void SetTitle(const std::string& title) {
    title_ = title;
  }

  std::string GetTitle() const { 
    return title_; 
  }

  void SetMessage(const std::string& message) {
    message_ = message;
  }

  std::string GetMessage() const { 
    return message_; 
  }

  bool Open(DialogModality modality) {
    // TODO: Implement using MessageBox or TaskDialogIndirect
    // For now, return false (not implemented)
    return false;
  }

  bool Close() {
    // TODO: Implement closing logic
    return false;
  }

 private:
  std::string title_;
  std::string message_;
};

// MessageDialog implementation
MessageDialog::MessageDialog(const std::string& title, const std::string& message)
    : pimpl_(std::make_unique<Impl>(title, message)) {
  // Set default modality to None (non-modal)
  SetModality(DialogModality::None);
}

MessageDialog::~MessageDialog() = default;

void MessageDialog::SetTitle(const std::string& title) {
  pimpl_->SetTitle(title);
}

std::string MessageDialog::GetTitle() const {
  return pimpl_->GetTitle();
}

void MessageDialog::SetMessage(const std::string& message) {
  pimpl_->SetMessage(message);
}

std::string MessageDialog::GetMessage() const {
  return pimpl_->GetMessage();
}

bool MessageDialog::Open() {
  DialogModality modality = GetModality();
  return pimpl_->Open(modality);
}

bool MessageDialog::Close() {
  return pimpl_->Close();
}

}  // namespace nativeapi

