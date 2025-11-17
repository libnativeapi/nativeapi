#import <Foundation/Foundation.h>
#include <limits.h>
#import <mach-o/dyld.h>
#include <unistd.h>

#include "../../autostart.h"

namespace nativeapi {

namespace {

// Convert std::string <-> NSString helpers
static inline NSString* ToNSString(const std::string& s) {
  return [NSString stringWithUTF8String:s.c_str()];
}

static inline std::string ToStdString(NSString* s) {
  if (!s)
    return std::string();
  const char* cstr = [s UTF8String];
  return cstr ? std::string(cstr) : std::string();
}

// Return the user's Library/LaunchAgents directory path.
static NSString* GetLaunchAgentsDir() {
  @autoreleasepool {
    NSArray<NSString*>* paths =
        NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES);
    NSString* libraryPath = (paths.count > 0) ? paths.firstObject : nil;
    if (!libraryPath || libraryPath.length == 0) {
      // Fallback to ~/Library
      libraryPath = [NSHomeDirectory() stringByAppendingPathComponent:@"Library"];
    }
    return [libraryPath stringByAppendingPathComponent:@"LaunchAgents"];
  }
}

// Replace any '/' in identifier to avoid nested paths in file name.
static NSString* SanitizeIdentifierForFileName(NSString* identifier) {
  @autoreleasepool {
    return [identifier stringByReplacingOccurrencesOfString:@"/" withString:@"_"];
  }
}

// Best-effort default identifier: CFBundleIdentifier or "com.nativeapi.autostart.<process-name>"
static std::string DetectDefaultId() {
  @autoreleasepool {
    NSString* bundleId = [[NSBundle mainBundle] bundleIdentifier];
    if (bundleId.length > 0) {
      return ToStdString(bundleId);
    }
    NSString* processName = [[NSProcessInfo processInfo] processName];
    if (processName.length == 0) {
      processName = @"app";
    }
    NSString* fallback = [NSString stringWithFormat:@"com.nativeapi.autostart.%@", processName];
    return ToStdString(fallback);
  }
}

// Best-effort default display name: CFBundleName or processName
static std::string DetectDefaultDisplayName() {
  @autoreleasepool {
    NSString* name = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleName"];
    if (name.length == 0) {
      name = [[NSProcessInfo processInfo] processName];
    }
    if (name.length == 0) {
      name = @"Application";
    }
    return ToStdString(name);
  }
}

// Resolve current executable path.
// Prefer NSProcessInfo.arguments[0]; fallback to _NSGetExecutablePath.
static std::string DetectDefaultProgramPath() {
  @autoreleasepool {
    NSString* arg0 = [[[NSProcessInfo processInfo] arguments] firstObject];
    if (arg0.length > 0) {
      // If arg0 is relative, attempt to get full path via file system resolution.
      NSString* resolved = [arg0 stringByStandardizingPath];
      if (![resolved isAbsolutePath]) {
        // Try to resolve via current directory (best-effort)
        char cwdBuff[PATH_MAX];
        if (getcwd(cwdBuff, sizeof(cwdBuff))) {
          NSString* cwd = [NSString stringWithUTF8String:cwdBuff];
          resolved = [cwd stringByAppendingPathComponent:arg0];
          resolved = [resolved stringByStandardizingPath];
        }
      }
      return ToStdString(resolved);
    }

    // Fallback to _NSGetExecutablePath
    uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);
    if (size > 0) {
      std::string buffer(size + 1, '\0');
      if (_NSGetExecutablePath(buffer.data(), &size) == 0) {
        return std::string(buffer.c_str());
      }
    }

    return std::string();
  }
}

}  // namespace

class AutoStart::Impl {
 public:
  static bool IsSupported() {
    // On macOS desktop, LaunchAgents are supported.
    return true;
  }

  Impl()
      : id_(DetectDefaultId()),
        display_name_(DetectDefaultDisplayName()),
        program_path_(DetectDefaultProgramPath()) {}

  explicit Impl(const std::string& id)
      : id_(id),
        display_name_(DetectDefaultDisplayName()),
        program_path_(DetectDefaultProgramPath()) {}

  Impl(const std::string& id, const std::string& display_name)
      : id_(id), display_name_(display_name), program_path_(DetectDefaultProgramPath()) {}

  ~Impl() = default;

  std::string GetId() const { return id_; }

  std::string GetDisplayName() const { return display_name_; }

  bool SetDisplayName(const std::string& display_name) {
    display_name_ = display_name;
    return true;
  }

  bool SetProgram(const std::string& executable_path, const std::vector<std::string>& arguments) {
    program_path_ = executable_path;
    arguments_ = arguments;
    return true;
  }

  std::string GetExecutablePath() const { return program_path_; }

  std::vector<std::string> GetArguments() const { return arguments_; }

  bool Enable() {
    @autoreleasepool {
      if (!EnsureLaunchAgentsDir()) {
        return false;
      }

      NSString* nsId = ToNSString(id_);
      NSString* nsDisplayName = ToNSString(display_name_);
      NSString* nsProgram = ToNSString(program_path_);

      if (nsProgram.length == 0) {
        // Attempt to detect again if not set
        program_path_ = DetectDefaultProgramPath();
        nsProgram = ToNSString(program_path_);
        if (nsProgram.length == 0) {
          return false;
        }
      }

      NSMutableArray<NSString*>* programArguments = [NSMutableArray array];
      [programArguments addObject:nsProgram];
      for (const auto& arg : arguments_) {
        [programArguments addObject:ToNSString(arg)];
      }

      // Build the plist dictionary in LaunchAgents format
      NSMutableDictionary* plist = [NSMutableDictionary dictionary];
      plist[@"Label"] = nsId;
      plist[@"RunAtLoad"] = @YES;
      plist[@"ProgramArguments"] = programArguments;
      // Optional logging to /tmp
      NSString* outPath =
          [[@"/tmp" stringByAppendingPathComponent:SanitizeIdentifierForFileName(nsId)]
              stringByAppendingString:@".out.log"];
      NSString* errPath =
          [[@"/tmp" stringByAppendingPathComponent:SanitizeIdentifierForFileName(nsId)]
              stringByAppendingString:@".err.log"];
      plist[@"StandardOutPath"] = outPath;
      plist[@"StandardErrorPath"] = errPath;

      // Name can be included for some tools even though not required by launchd
      if (nsDisplayName.length > 0) {
        plist[@"_Comment"] = [NSString stringWithFormat:@"AutoStart for %@", nsDisplayName];
      }

      NSError* error = nil;
      NSData* data = [NSPropertyListSerialization dataWithPropertyList:plist
                                                                format:NSPropertyListXMLFormat_v1_0
                                                               options:0
                                                                 error:&error];
      if (!data || error) {
        return false;
      }

      NSString* plistPath = PlistPath();
      if (![data writeToFile:plistPath atomically:YES]) {
        return false;
      }

      // Set permissions to 0644 (owner read/write, group/others read)
      NSDictionary* attrs = @{NSFilePosixPermissions : @(0644)};
      [[NSFileManager defaultManager] setAttributes:attrs ofItemAtPath:plistPath error:nil];

      // Note: We intentionally do not invoke `launchctl load` here. The agent
      // will take effect on next login. Callers may choose to load/unload manually.
      return true;
    }
  }

  bool Disable() {
    @autoreleasepool {
      NSString* path = PlistPath();
      NSFileManager* fm = [NSFileManager defaultManager];
      if ([fm fileExistsAtPath:path]) {
        NSError* error = nil;
        if (![fm removeItemAtPath:path error:&error]) {
          return false;
        }
      }
      return true;
    }
  }

  bool IsEnabled() const {
    @autoreleasepool {
      NSString* path = PlistPath();
      return [[NSFileManager defaultManager] fileExistsAtPath:path];
    }
  }

 private:
  NSString* PlistPath() const {
    @autoreleasepool {
      NSString* baseDir = GetLaunchAgentsDir();
      NSString* fileName =
          [SanitizeIdentifierForFileName(ToNSString(id_)) stringByAppendingString:@".plist"];
      return [baseDir stringByAppendingPathComponent:fileName];
    }
  }

  bool EnsureLaunchAgentsDir() const {
    @autoreleasepool {
      NSString* dir = GetLaunchAgentsDir();
      NSFileManager* fm = [NSFileManager defaultManager];
      BOOL isDir = NO;
      if ([fm fileExistsAtPath:dir isDirectory:&isDir]) {
        return isDir == YES;
      }
      NSError* error = nil;
      BOOL ok = [fm createDirectoryAtPath:dir
              withIntermediateDirectories:YES
                               attributes:@{
                                 NSFilePosixPermissions : @(0755)
                               }
                                    error:&error];
      return ok && error == nil;
    }
  }

 private:
  std::string id_;
  std::string display_name_;
  std::string program_path_;
  std::vector<std::string> arguments_;
};

// AutoStart public API implementations

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
