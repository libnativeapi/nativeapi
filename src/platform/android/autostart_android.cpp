#include "../../autostart.h"

namespace nativeapi {

/**
 * Android stub implementation for AutoStart.
 *
 * Auto-start at user login is not supported on Android. All operations that would
 * enable/disable or configure auto-start return false. Getters return the locally
 * stored values (typically empty), while setters return false and do not modify state.
 */
class AutoStart::Impl {
 public:
  // Unsupported platform semantics
  static bool IsSupported() { return false; }

  Impl() = default;

  explicit Impl(const std::string& id) : id_(id) {}

  Impl(const std::string& id, const std::string& display_name)
      : id_(id), display_name_(display_name) {}

  ~Impl() = default;

  // Getters return whatever is locally available (likely empty)
  std::string GetId() const { return id_; }
  std::string GetDisplayName() const { return display_name_; }

  // No-op setter; returns false to indicate unsupported
  bool SetDisplayName(const std::string& /*display_name*/) { return false; }

  bool SetProgram(const std::string& /*executable_path*/,
                  const std::vector<std::string>& /*arguments*/) {
    return false;
  }

  std::string GetExecutablePath() const { return program_path_; }
  std::vector<std::string> GetArguments() const { return arguments_; }

  bool Enable() { return false; }
  bool Disable() { return false; }
  bool IsEnabled() const { return false; }

 private:
  std::string id_;
  std::string display_name_;
  std::string program_path_;
  std::vector<std::string> arguments_;
};

// AutoStart public API forwarding to Impl

AutoStart::AutoStart() : pimpl_(std::make_unique<Impl>()) {}

AutoStart::AutoStart(const std::string& id) : pimpl_(std::make_unique<Impl>(id)) {}

AutoStart::AutoStart(const std::string& id, const std::string& display_name)
    : pimpl_(std::make_unique<Impl>(id, display_name)) {}

AutoStart::~AutoStart() = default;

bool AutoStart::IsSupported() {
  return Impl::IsSupported();
}

std::string AutoStart::GetId() const {
  return pimpl_->GetId();
}

std::string AutoStart::GetDisplayName() const {
  return pimpl_->GetDisplayName();
}

bool AutoStart::SetDisplayName(const std::string& display_name) {
  return pimpl_->SetDisplayName(display_name);
}

bool AutoStart::SetProgram(const std::string& executable_path,
                           const std::vector<std::string>& arguments) {
  return pimpl_->SetProgram(executable_path, arguments);
}

std::string AutoStart::GetExecutablePath() const {
  return pimpl_->GetExecutablePath();
}

std::vector<std::string> AutoStart::GetArguments() const {
  return pimpl_->GetArguments();
}

bool AutoStart::Enable() {
  return pimpl_->Enable();
}

bool AutoStart::Disable() {
  return pimpl_->Disable();
}

bool AutoStart::IsEnabled() const {
  return pimpl_->IsEnabled();
}

}  // namespace nativeapi