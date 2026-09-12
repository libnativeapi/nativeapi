// Included after the platform Impl definition, inside namespace nativeapi.
bool MessageDialog::SetButtons(const std::string& primary, const std::string& secondary,
                               const std::string& close) {
  if (!IsExtendedSupported() || pimpl_->state_.open ||
      (primary.empty() && secondary.empty() && close.empty())) return false;
  pimpl_->state_.primary = primary;
  pimpl_->state_.secondary = secondary;
  pimpl_->state_.close = close;
  return true;
}
bool MessageDialog::SetDefaultButton(MessageDialogResult button) {
  if (!IsExtendedSupported() || pimpl_->state_.open ||
      button < MessageDialogResult::None || button > MessageDialogResult::Close) return false;
  pimpl_->state_.default_button = button;
  return true;
}
bool MessageDialog::SetParentWindow(std::shared_ptr<Window> window) {
  if (!IsExtendedSupported() || pimpl_->state_.open) return false;
  pimpl_->state_.parent = std::move(window);
  return true;
}
MessageDialogResult MessageDialog::GetResult() const { return pimpl_->state_.result; }
bool MessageDialog::IsOpen() const { return pimpl_->state_.open; }
bool MessageDialog::SetInputEnabled(bool enabled) {
  if (!IsExtendedSupported()) return false;
  pimpl_->state_.input_enabled = enabled;
  pimpl_->RefreshExtended();
  return true;
}
bool MessageDialog::SetInputText(const std::string& text) {
  if (!IsExtendedSupported()) return false;
  pimpl_->state_.input = text;
  pimpl_->RefreshExtended();
  return true;
}
std::string MessageDialog::GetInputText() const { return pimpl_->state_.input; }
bool MessageDialog::SetCheckbox(const std::string& label, bool checked) {
  if (!IsExtendedSupported()) return false;
  pimpl_->state_.checkbox = label;
  pimpl_->state_.checked = checked;
  pimpl_->RefreshExtended();
  return true;
}
bool MessageDialog::IsCheckboxChecked() const { return pimpl_->state_.checked; }
bool MessageDialog::SetProgress(double value) {
  if (!IsExtendedSupported() || !(value == -2 || value == -1 || (value >= 0 && value <= 1)))
    return false;
  pimpl_->state_.progress = value;
  pimpl_->RefreshExtended();
  return true;
}
