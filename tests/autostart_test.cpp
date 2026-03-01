#include <sys/stat.h>
#include <unistd.h>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "../src/autostart.h"

namespace {

// Return a temporary XDG_CONFIG_HOME directory unique to this test run
static std::string MakeTempConfigDir() {
  const char* tmpbase = std::getenv("TMPDIR");
  if (!tmpbase || !*tmpbase)
    tmpbase = "/tmp";
  std::string tmpl = std::string(tmpbase) + "/autostart_test_XXXXXX";
  const char* dir = mkdtemp(tmpl.data());
  if (!dir) {
    std::cerr << "mkdtemp failed" << std::endl;
    return std::string();
  }
  return std::string(dir);
}

// Remove a directory tree (non-recursively safe: only removes known files).
// Failures are intentionally ignored since this is test cleanup only.
static void RemoveDesktopFile(const std::string& config_dir, const std::string& id) {
  std::string path = config_dir + "/autostart/" + id + ".desktop";
  unlink(path.c_str());
  rmdir((config_dir + "/autostart").c_str());
  rmdir(config_dir.c_str());
}

// Read file contents into a string
static std::string ReadFile(const std::string& path) {
  std::ifstream ifs(path);
  if (!ifs.is_open())
    return std::string();
  std::ostringstream oss;
  oss << ifs.rdbuf();
  return oss.str();
}

// Scoped XDG_CONFIG_HOME override to isolate tests
struct ScopedConfigHome {
  std::string dir;
  std::string prev;

  explicit ScopedConfigHome(const std::string& d) : dir(d) {
    const char* p = std::getenv("XDG_CONFIG_HOME");
    prev = p ? std::string(p) : std::string();
    setenv("XDG_CONFIG_HOME", dir.c_str(), 1);
  }

  ~ScopedConfigHome() {
    if (prev.empty()) {
      unsetenv("XDG_CONFIG_HOME");
    } else {
      setenv("XDG_CONFIG_HOME", prev.c_str(), 1);
    }
  }
};

int RunTests() {
  using namespace nativeapi;

  // IsSupported returns true on Linux
  {
    if (!AutoStart::IsSupported()) {
      std::cerr << "AutoStart::IsSupported() should return true on Linux." << std::endl;
      return 1;
    }
  }

  // Constructors and getters
  {
    const std::string tmpDir = MakeTempConfigDir();
    if (tmpDir.empty())
      return 1;
    ScopedConfigHome scope(tmpDir);

    AutoStart as("com.example.testapp", "TestApp");
    if (as.GetId() != "com.example.testapp") {
      std::cerr << "GetId() should return 'com.example.testapp', got '" << as.GetId() << "'"
                << std::endl;
      rmdir(tmpDir.c_str());
      return 1;
    }
    if (as.GetDisplayName() != "TestApp") {
      std::cerr << "GetDisplayName() should return 'TestApp', got '" << as.GetDisplayName() << "'"
                << std::endl;
      rmdir(tmpDir.c_str());
      return 1;
    }
    rmdir(tmpDir.c_str());
  }

  // SetDisplayName
  {
    const std::string tmpDir = MakeTempConfigDir();
    if (tmpDir.empty())
      return 1;
    ScopedConfigHome scope(tmpDir);

    AutoStart as("com.example.testapp", "OriginalName");
    if (!as.SetDisplayName("NewName")) {
      std::cerr << "SetDisplayName() should return true." << std::endl;
      rmdir(tmpDir.c_str());
      return 1;
    }
    if (as.GetDisplayName() != "NewName") {
      std::cerr << "GetDisplayName() after SetDisplayName() should return 'NewName', got '"
                << as.GetDisplayName() << "'" << std::endl;
      rmdir(tmpDir.c_str());
      return 1;
    }
    rmdir(tmpDir.c_str());
  }

  // SetProgram / GetExecutablePath / GetArguments
  {
    const std::string tmpDir = MakeTempConfigDir();
    if (tmpDir.empty())
      return 1;
    ScopedConfigHome scope(tmpDir);

    AutoStart as("com.example.testapp", "TestApp");
    std::vector<std::string> args = {"--minimized", "--tray"};
    if (!as.SetProgram("/usr/bin/testapp", args)) {
      std::cerr << "SetProgram() should return true." << std::endl;
      rmdir(tmpDir.c_str());
      return 1;
    }
    if (as.GetExecutablePath() != "/usr/bin/testapp") {
      std::cerr << "GetExecutablePath() should return '/usr/bin/testapp', got '"
                << as.GetExecutablePath() << "'" << std::endl;
      rmdir(tmpDir.c_str());
      return 1;
    }
    auto gotArgs = as.GetArguments();
    if (gotArgs != args) {
      std::cerr << "GetArguments() returned unexpected values." << std::endl;
      rmdir(tmpDir.c_str());
      return 1;
    }
    rmdir(tmpDir.c_str());
  }

  // Enable creates .desktop file and IsEnabled returns true
  {
    const std::string tmpDir = MakeTempConfigDir();
    if (tmpDir.empty())
      return 1;
    ScopedConfigHome scope(tmpDir);

    const std::string testId = "com.example.autostarttest";
    AutoStart as(testId, "AutoStartTest");
    as.SetProgram("/usr/bin/testapp", {});

    if (!as.Enable()) {
      std::cerr << "Enable() should return true." << std::endl;
      RemoveDesktopFile(tmpDir, testId);
      return 1;
    }

    if (!as.IsEnabled()) {
      std::cerr << "IsEnabled() should return true after Enable()." << std::endl;
      RemoveDesktopFile(tmpDir, testId);
      return 1;
    }

    RemoveDesktopFile(tmpDir, testId);
  }

  // Disable removes .desktop file and IsEnabled returns false
  {
    const std::string tmpDir = MakeTempConfigDir();
    if (tmpDir.empty())
      return 1;
    ScopedConfigHome scope(tmpDir);

    const std::string testId = "com.example.autostarttest";
    AutoStart as(testId, "AutoStartTest");
    as.SetProgram("/usr/bin/testapp", {});

    if (!as.Enable()) {
      std::cerr << "Enable() should return true (precondition for Disable test)." << std::endl;
      RemoveDesktopFile(tmpDir, testId);
      return 1;
    }

    if (!as.Disable()) {
      std::cerr << "Disable() should return true." << std::endl;
      RemoveDesktopFile(tmpDir, testId);
      return 1;
    }

    if (as.IsEnabled()) {
      std::cerr << "IsEnabled() should return false after Disable()." << std::endl;
      RemoveDesktopFile(tmpDir, testId);
      return 1;
    }

    RemoveDesktopFile(tmpDir, testId);
  }

  // Disable is idempotent (no error when file does not exist)
  {
    const std::string tmpDir = MakeTempConfigDir();
    if (tmpDir.empty())
      return 1;
    ScopedConfigHome scope(tmpDir);

    AutoStart as("com.example.autostarttest", "AutoStartTest");
    if (!as.Disable()) {
      std::cerr << "Disable() on non-existent entry should return true (idempotent)." << std::endl;
      rmdir(tmpDir.c_str());
      return 1;
    }
    rmdir(tmpDir.c_str());
  }

  // IsEnabled returns false before Enable() is called
  {
    const std::string tmpDir = MakeTempConfigDir();
    if (tmpDir.empty())
      return 1;
    ScopedConfigHome scope(tmpDir);

    AutoStart as("com.example.autostarttest", "AutoStartTest");
    if (as.IsEnabled()) {
      std::cerr << "IsEnabled() should return false before Enable() is called." << std::endl;
      rmdir(tmpDir.c_str());
      return 1;
    }
    rmdir(tmpDir.c_str());
  }

  // Enable() writes valid .desktop content
  {
    const std::string tmpDir = MakeTempConfigDir();
    if (tmpDir.empty())
      return 1;
    ScopedConfigHome scope(tmpDir);

    const std::string testId = "com.example.contenttest";
    AutoStart as(testId, "ContentTestApp");
    as.SetProgram("/usr/local/bin/myapp", {"--flag", "value with space"});

    if (!as.Enable()) {
      std::cerr << "Enable() should return true." << std::endl;
      RemoveDesktopFile(tmpDir, testId);
      return 1;
    }

    std::string desktopPath = tmpDir + "/autostart/" + testId + ".desktop";
    std::string content = ReadFile(desktopPath);

    if (content.find("[Desktop Entry]") == std::string::npos) {
      std::cerr << "Desktop file missing [Desktop Entry] section." << std::endl;
      RemoveDesktopFile(tmpDir, testId);
      return 1;
    }
    if (content.find("Type=Application") == std::string::npos) {
      std::cerr << "Desktop file missing 'Type=Application'." << std::endl;
      RemoveDesktopFile(tmpDir, testId);
      return 1;
    }
    if (content.find("Name=ContentTestApp") == std::string::npos) {
      std::cerr << "Desktop file missing 'Name=ContentTestApp'." << std::endl;
      RemoveDesktopFile(tmpDir, testId);
      return 1;
    }
    if (content.find("Exec=") == std::string::npos) {
      std::cerr << "Desktop file missing 'Exec=' line." << std::endl;
      RemoveDesktopFile(tmpDir, testId);
      return 1;
    }
    if (content.find("/usr/local/bin/myapp") == std::string::npos) {
      std::cerr << "Desktop file Exec line missing executable path." << std::endl;
      RemoveDesktopFile(tmpDir, testId);
      return 1;
    }
    if (content.find("Hidden=false") == std::string::npos) {
      std::cerr << "Desktop file missing 'Hidden=false'." << std::endl;
      RemoveDesktopFile(tmpDir, testId);
      return 1;
    }
    if (content.find("X-GNOME-Autostart-enabled=true") == std::string::npos) {
      std::cerr << "Desktop file missing 'X-GNOME-Autostart-enabled=true'." << std::endl;
      RemoveDesktopFile(tmpDir, testId);
      return 1;
    }

    RemoveDesktopFile(tmpDir, testId);
  }

  // Enable() then Enable() again updates the entry (re-enables)
  {
    const std::string tmpDir = MakeTempConfigDir();
    if (tmpDir.empty())
      return 1;
    ScopedConfigHome scope(tmpDir);

    const std::string testId = "com.example.autostarttest";
    AutoStart as(testId, "AutoStartTest");
    as.SetProgram("/usr/bin/testapp", {});

    if (!as.Enable()) {
      std::cerr << "Enable() first call should return true." << std::endl;
      RemoveDesktopFile(tmpDir, testId);
      return 1;
    }
    // Update program and enable again
    as.SetProgram("/usr/bin/testapp2", {});
    if (!as.Enable()) {
      std::cerr << "Enable() second call should return true." << std::endl;
      RemoveDesktopFile(tmpDir, testId);
      return 1;
    }
    if (!as.IsEnabled()) {
      std::cerr << "IsEnabled() should return true after second Enable()." << std::endl;
      RemoveDesktopFile(tmpDir, testId);
      return 1;
    }

    std::string desktopPath = tmpDir + "/autostart/" + testId + ".desktop";
    std::string content = ReadFile(desktopPath);
    if (content.find("/usr/bin/testapp2") == std::string::npos) {
      std::cerr << "Desktop file should reference updated executable after second Enable()."
                << std::endl;
      RemoveDesktopFile(tmpDir, testId);
      return 1;
    }

    RemoveDesktopFile(tmpDir, testId);
  }

  return 0;
}

}  // namespace

int main() {
  return RunTests();
}
