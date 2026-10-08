#pragma once
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
#if _WIN32
#define NATIVEAPI_GO_EXPORT __declspec(dllexport)
#else
#define NATIVEAPI_GO_EXPORT
#endif
typedef void (*nativeapi_go_callback_t)(void*);
NATIVEAPI_GO_EXPORT void nativeapi_go_init(void);
NATIVEAPI_GO_EXPORT bool nativeapi_go_poll_events(int timeout_ms);
NATIVEAPI_GO_EXPORT bool nativeapi_go_dispatch(void (*callback)(void*), void* data,
                                             void (*release)(void*));
#ifdef __cplusplus
}
#endif
