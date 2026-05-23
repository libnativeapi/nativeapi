#include "launch_at_login_c.h"

#include "../launch_at_login.h"
#include "string_utils_c.h"

#include <new>
#include <string>
#include <vector>

using namespace nativeapi;

native_launch_at_login_t native_launch_at_login_create(void) {
  try {
    auto* instance = new LaunchAtLogin();
    return static_cast<void*>(instance);
  } catch (...) {
    return nullptr;
  }
}

native_launch_at_login_t native_launch_at_login_create_with_id(const char* id) {
  if (!id) {
    return nullptr;
  }
  try {
    auto* instance = new LaunchAtLogin(std::string(id));
    return static_cast<void*>(instance);
  } catch (...) {
    return nullptr;
  }
}

native_launch_at_login_t native_launch_at_login_create_with_id_and_name(const char* id,
                                                                        const char* display_name) {
  if (!id || !display_name) {
    return nullptr;
  }
  try {
    auto* instance = new LaunchAtLogin(std::string(id), std::string(display_name));
    return static_cast<void*>(instance);
  } catch (...) {
    return nullptr;
  }
}

void native_launch_at_login_destroy(native_launch_at_login_t launch_at_login) {
  if (launch_at_login) {
    delete static_cast<LaunchAtLogin*>(launch_at_login);
  }
}

bool native_launch_at_login_is_supported(void) {
  try {
    return LaunchAtLogin::IsSupported();
  } catch (...) {
    return false;
  }
}

char* native_launch_at_login_get_id(native_launch_at_login_t launch_at_login) {
  if (!launch_at_login) {
    return nullptr;
  }
  try {
    auto* as = static_cast<LaunchAtLogin*>(launch_at_login);
    return to_c_str(as->GetId());
  } catch (...) {
    return nullptr;
  }
}

char* native_launch_at_login_get_display_name(native_launch_at_login_t launch_at_login) {
  if (!launch_at_login) {
    return nullptr;
  }
  try {
    auto* as = static_cast<LaunchAtLogin*>(launch_at_login);
    return to_c_str(as->GetDisplayName());
  } catch (...) {
    return nullptr;
  }
}

bool native_launch_at_login_set_display_name(native_launch_at_login_t launch_at_login,
                                             const char* display_name) {
  if (!launch_at_login || !display_name) {
    return false;
  }
  try {
    auto* as = static_cast<LaunchAtLogin*>(launch_at_login);
    return as->SetDisplayName(std::string(display_name));
  } catch (...) {
    return false;
  }
}

bool native_launch_at_login_set_program(native_launch_at_login_t launch_at_login,
                                        const char* executable_path,
                                        const char* const* arguments,
                                        size_t argument_count) {
  if (!launch_at_login || !executable_path) {
    return false;
  }
  try {
    std::vector<std::string> args;
    args.reserve(argument_count);
    if (arguments && argument_count > 0) {
      for (size_t i = 0; i < argument_count; ++i) {
        const char* arg = arguments[i];
        args.emplace_back(arg ? arg : "");
      }
    }
    auto* as = static_cast<LaunchAtLogin*>(launch_at_login);
    return as->SetProgram(std::string(executable_path), args);
  } catch (...) {
    return false;
  }
}

char* native_launch_at_login_get_executable_path(native_launch_at_login_t launch_at_login) {
  if (!launch_at_login) {
    return nullptr;
  }
  try {
    auto* as = static_cast<LaunchAtLogin*>(launch_at_login);
    return to_c_str(as->GetExecutablePath());
  } catch (...) {
    return nullptr;
  }
}

bool native_launch_at_login_get_arguments(native_launch_at_login_t launch_at_login,
                                          char*** out_arguments,
                                          size_t* out_count) {
  if (!launch_at_login || !out_arguments || !out_count) {
    return false;
  }
  try {
    auto* as = static_cast<LaunchAtLogin*>(launch_at_login);
    auto args = as->GetArguments();
    *out_count = args.size();
    *out_arguments = nullptr;

    if (args.empty()) {
      return true;
    }

    char** arr = new (std::nothrow) char*[*out_count];
    if (!arr) {
      *out_count = 0;
      return false;
    }

    for (size_t i = 0; i < *out_count; ++i) {
      arr[i] = to_c_str(args[i]);
      if (!arr[i]) {
        // Cleanup already allocated strings
        for (size_t j = 0; j < i; ++j) {
          free_c_str(arr[j]);
        }
        delete[] arr;
        *out_count = 0;
        return false;
      }
    }

    *out_arguments = arr;
    return true;
  } catch (...) {
    *out_arguments = nullptr;
    *out_count = 0;
    return false;
  }
}

bool native_launch_at_login_enable(native_launch_at_login_t launch_at_login) {
  if (!launch_at_login) {
    return false;
  }
  try {
    auto* as = static_cast<LaunchAtLogin*>(launch_at_login);
    return as->Enable();
  } catch (...) {
    return false;
  }
}

bool native_launch_at_login_disable(native_launch_at_login_t launch_at_login) {
  if (!launch_at_login) {
    return false;
  }
  try {
    auto* as = static_cast<LaunchAtLogin*>(launch_at_login);
    return as->Disable();
  } catch (...) {
    return false;
  }
}

bool native_launch_at_login_is_enabled(native_launch_at_login_t launch_at_login) {
  if (!launch_at_login) {
    return false;
  }
  try {
    auto* as = static_cast<LaunchAtLogin*>(launch_at_login);
    return as->IsEnabled();
  } catch (...) {
    return false;
  }
}