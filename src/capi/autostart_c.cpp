#include "autostart_c.h"

#include "../autostart.h"
#include "string_utils_c.h"

#include <new>
#include <string>
#include <vector>

using namespace nativeapi;

native_autostart_t native_autostart_create(void) {
  try {
    auto* instance = new AutoStart();
    return static_cast<void*>(instance);
  } catch (...) {
    return nullptr;
  }
}

native_autostart_t native_autostart_create_with_id(const char* id) {
  if (!id) {
    return nullptr;
  }
  try {
    auto* instance = new AutoStart(std::string(id));
    return static_cast<void*>(instance);
  } catch (...) {
    return nullptr;
  }
}

native_autostart_t native_autostart_create_with_id_and_name(const char* id,
                                                            const char* display_name) {
  if (!id || !display_name) {
    return nullptr;
  }
  try {
    auto* instance = new AutoStart(std::string(id), std::string(display_name));
    return static_cast<void*>(instance);
  } catch (...) {
    return nullptr;
  }
}

void native_autostart_destroy(native_autostart_t autostart) {
  if (autostart) {
    delete static_cast<AutoStart*>(autostart);
  }
}

bool native_autostart_is_supported(void) {
  try {
    return AutoStart::IsSupported();
  } catch (...) {
    return false;
  }
}

char* native_autostart_get_id(native_autostart_t autostart) {
  if (!autostart) {
    return nullptr;
  }
  try {
    auto* as = static_cast<AutoStart*>(autostart);
    return to_c_str(as->GetId());
  } catch (...) {
    return nullptr;
  }
}

char* native_autostart_get_display_name(native_autostart_t autostart) {
  if (!autostart) {
    return nullptr;
  }
  try {
    auto* as = static_cast<AutoStart*>(autostart);
    return to_c_str(as->GetDisplayName());
  } catch (...) {
    return nullptr;
  }
}

bool native_autostart_set_display_name(native_autostart_t autostart, const char* display_name) {
  if (!autostart || !display_name) {
    return false;
  }
  try {
    auto* as = static_cast<AutoStart*>(autostart);
    return as->SetDisplayName(std::string(display_name));
  } catch (...) {
    return false;
  }
}

bool native_autostart_set_program(native_autostart_t autostart,
                                  const char* executable_path,
                                  const char* const* arguments,
                                  size_t argument_count) {
  if (!autostart || !executable_path) {
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
    auto* as = static_cast<AutoStart*>(autostart);
    return as->SetProgram(std::string(executable_path), args);
  } catch (...) {
    return false;
  }
}

char* native_autostart_get_executable_path(native_autostart_t autostart) {
  if (!autostart) {
    return nullptr;
  }
  try {
    auto* as = static_cast<AutoStart*>(autostart);
    return to_c_str(as->GetExecutablePath());
  } catch (...) {
    return nullptr;
  }
}

bool native_autostart_get_arguments(native_autostart_t autostart,
                                    char*** out_arguments,
                                    size_t* out_count) {
  if (!autostart || !out_arguments || !out_count) {
    return false;
  }
  try {
    auto* as = static_cast<AutoStart*>(autostart);
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

bool native_autostart_enable(native_autostart_t autostart) {
  if (!autostart) {
    return false;
  }
  try {
    auto* as = static_cast<AutoStart*>(autostart);
    return as->Enable();
  } catch (...) {
    return false;
  }
}

bool native_autostart_disable(native_autostart_t autostart) {
  if (!autostart) {
    return false;
  }
  try {
    auto* as = static_cast<AutoStart*>(autostart);
    return as->Disable();
  } catch (...) {
    return false;
  }
}

bool native_autostart_is_enabled(native_autostart_t autostart) {
  if (!autostart) {
    return false;
  }
  try {
    auto* as = static_cast<AutoStart*>(autostart);
    return as->IsEnabled();
  } catch (...) {
    return false;
  }
}