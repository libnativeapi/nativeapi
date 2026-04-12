#include <nativeapi.h>
#include <stdio.h>

int main(void) {
  printf("URL Opener C API Example\n");
  printf("========================\n\n");

  native_url_opener_t opener = native_url_opener_create();
  if (!opener) {
    fprintf(stderr, "Failed to create URL opener instance.\n");
    return 1;
  }

  if (!native_url_opener_is_supported(opener)) {
    printf("URL opening is not supported on this platform.\n");
    native_url_opener_destroy(opener);
    return 0;
  }

  printf("URL opening is supported.\n");
  printf("Opening https://example.com ...\n");

  native_url_open_result_t result = native_url_opener_open(opener, "https://example.com");
  if (result.success) {
    printf("URL opened successfully.\n");
  } else {
    fprintf(stderr, "Failed to open URL.\n");
    fprintf(stderr, "Error code: %d\n", (int)result.error_code);
    fprintf(stderr, "Message: %s\n", result.error_message ? result.error_message : "(none)");
  }

  native_url_open_result_free(&result);
  native_url_opener_destroy(opener);
  return result.success ? 0 : 1;
}
