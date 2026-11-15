#pragma once

#include <napi/native_api.h>
#include "ohos_runtime.h"

namespace nativeapi::swift_ohos {

// Initializes nativeapi::ohos::Initialize(env) and connects Swift's
// DispatchQueue.main / @MainActor queue to the N-API environment's libuv loop,
// tearing the watcher down automatically when the environment cleans up.
bool Initialize(napi_env env);

// Connects Swift's DispatchQueue.main / @MainActor queue to an already-active
// N-API environment on the calling ArkTS UI thread. Safe to call repeatedly.
bool InstallMainQueue(napi_env env);

}  // namespace nativeapi::swift_ohos
