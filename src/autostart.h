#pragma once

#include <memory>
#include <string>
#include <vector>

namespace nativeapi {

/**
 * @brief Manage application auto-start behavior on user login (cross-platform).
 *
 * AutoStart provides a unified API to enable or disable starting your application
 * automatically when the user logs in. The actual mechanism is platform-specific,
 * but this class abstracts away those differences.
 *
 * Platform implementations:
 * - Windows: HKCU\Software\Microsoft\Windows\CurrentVersion\Run registry key
 * - macOS: Launch Agents (~/Library/LaunchAgents/[app_id].plist) with ProgramArguments
 * - Linux (XDG): ~/.config/autostart/[app_id].desktop
 * - Android/iOS/OHOS: Not supported (returns false from IsSupported/operations)
 *
 * Notes:
 * - This API is intended for desktop environments. On mobile platforms this API is
 *   typically unsupported by design. Methods will fail gracefully.
 * - You can let the implementation determine the current executable path, or call
 *   SetProgram() to customize the target binary and arguments recorded in the OS.
 * - Some platforms may require application-specific permissions or entitlements
 *   (e.g., sandbox restrictions on macOS). In such cases, operations may fail.
 *
 * Typical usage:
 * @code
 * using namespace nativeapi;
 *
 * if (AutoStart::IsSupported()) {
 *   AutoStart autostart("com.example.myapp", "MyApp");
 *   // Optionally override the program and arguments (defaults to current executable):
 *   autostart.SetProgram("/usr/local/bin/myapp", {"--minimized"});
 *
 *   autostart.Enable();
 *   bool enabled = autostart.IsEnabled(); // should be true
 * }
 * @endcode
 */
class AutoStart {
 public:
  /**
   * @brief Check whether auto-start is supported on this platform.
   *
   * @return true if supported; false for unsupported platforms (e.g., mobile).
   */
  static bool IsSupported();

  /**
   * @brief Construct an AutoStart manager with default identifier and display name.
   *
   * The default identifier and display name are implementation-defined. Typically,
   * the identifier is derived from the current process/bundle information, and the
   * display name is derived from the application or executable name.
   */
  AutoStart();

  /**
   * @brief Construct an AutoStart manager with a custom identifier.
   *
   * @param id A stable, unique identifier for your app.
   *           Examples:
   *           - Windows: used as the registry value name, e.g., "MyApp"
   *           - macOS: used as LaunchAgent Label, e.g., "com.example.myapp"
   *           - Linux: used as the .desktop file name (without extension), e.g., "myapp"
   *
   * Recommendation: Use a reverse-DNS identifier when possible, e.g., "com.example.myapp".
   */
  explicit AutoStart(const std::string& id);

  /**
   * @brief Construct an AutoStart manager with a custom identifier and display name.
   *
   * @param id           Stable, unique identifier (see above).
   * @param display_name Human-readable name shown in OS surfaces where applicable.
   */
  AutoStart(const std::string& id, const std::string& display_name);

  virtual ~AutoStart();

  /**
   * @brief Get the unique identifier associated with this AutoStart instance.
   *
   * @return The identifier string.
   */
  std::string GetId() const;

  /**
   * @brief Get the human-readable display name used where applicable.
   *
   * @return The display name string.
   */
  std::string GetDisplayName() const;

  /**
   * @brief Set a human-readable display name used where applicable.
   *
   * Some platforms surface a name in their UI (e.g., Linux .desktop Name, macOS LaunchAgent Label
   * often mirrors the identifier but may display the name in some tools).
   *
   * @param display_name The human-readable name.
   * @return true if the value was stored locally; does not change OS registration until Enable().
   */
  bool SetDisplayName(const std::string& display_name);

  /**
   * @brief Set the program (executable) path and optional arguments used for auto-start.
   *
   * If not set, implementations will try to use the current process executable path.
   * On platforms that require a single string (e.g., Windows registry), arguments will
   * be encoded appropriately (quoted when needed). On macOS/Linux, arguments are stored
   * as vector items (.plist ProgramArguments / .desktop Exec respectively).
   *
   * @param executable_path Absolute path to the executable to run on login.
   * @param arguments       Optional arguments; order is preserved.
   * @return true if stored locally; does not change OS registration until Enable().
   */
  bool SetProgram(const std::string& executable_path,
                  const std::vector<std::string>& arguments = {});

  /**
   * @brief Get the currently configured executable path used for auto-start.
   *
   * This returns the locally configured value (not necessarily what is stored in the OS).
   * If never set explicitly and cannot be resolved from the current process, it may be empty.
   *
   * @return Executable path string (may be empty).
   */
  std::string GetExecutablePath() const;

  /**
   * @brief Get the currently configured arguments used for auto-start.
   *
   * This returns the locally configured value (not necessarily what is stored in the OS).
   *
   * @return Vector of arguments (may be empty).
   */
  std::vector<std::string> GetArguments() const;

  /**
   * @brief Enable auto-start at user login for the configured program and arguments.
   *
   * If no program was explicitly set via SetProgram(), the implementation will attempt
   * to resolve the current executable path and use that as the program to start.
   *
   * @return true on success; false on error or when unsupported.
   */
  bool Enable();

  /**
   * @brief Disable auto-start at user login.
   *
   * @return true on success; false on error or when unsupported.
   */
  bool Disable();

  /**
   * @brief Query whether auto-start is currently enabled for this manager's identifier.
   *
   * This checks the platform-specific mechanism to determine whether the app (program path
   * and arguments currently configured) is registered to start at user login.
   *
   * @return true if currently enabled; false otherwise.
   */
  bool IsEnabled() const;

  // Prevent copying and moving
  AutoStart(const AutoStart&) = delete;
  AutoStart& operator=(const AutoStart&) = delete;
  AutoStart(AutoStart&&) = delete;
  AutoStart& operator=(AutoStart&&) = delete;

 private:
  class Impl;
  std::unique_ptr<Impl> pimpl_;
};

}  // namespace nativeapi