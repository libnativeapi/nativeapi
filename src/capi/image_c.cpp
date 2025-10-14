#include "image_c.h"

#include <cstring>
#include "../image.h"
#include "string_utils_c.h"

using namespace nativeapi;

// Create an image from a file path
native_image_t native_image_from_file(const char* file_path) {
  if (!file_path) {
    return nullptr;
  }

  try {
    auto image = Image::FromFile(file_path);
    if (image) {
      auto size = image->GetSize();
      if (size.width > 0 && size.height > 0) {
        return new std::shared_ptr<Image>(image);
      }
    }
  } catch (...) {
    // Handle exceptions
  }

  return nullptr;
}

// Create an image from base64-encoded data
native_image_t native_image_from_base64(const char* base64_data) {
  if (!base64_data) {
    return nullptr;
  }

  try {
    auto image = Image::FromBase64(base64_data);
    if (image) {
      auto size = image->GetSize();
      if (size.width > 0 && size.height > 0) {
        return new std::shared_ptr<Image>(image);
      }
    }
  } catch (...) {
    // Handle exceptions
  }

  return nullptr;
}

// Create an image from a platform-specific system icon
native_image_t native_image_from_system_icon(const char* icon_name) {
  if (!icon_name) {
    return nullptr;
  }

  try {
    auto image = Image::FromSystemIcon(icon_name);
    if (image) {
      auto size = image->GetSize();
      if (size.width > 0 && size.height > 0) {
        return new std::shared_ptr<Image>(image);
      }
    }
  } catch (...) {
    // Handle exceptions
  }

  return nullptr;
}

// Destroy an image and release its resources
void native_image_destroy(native_image_t image) {
  if (image) {
    delete static_cast<std::shared_ptr<Image>*>(image);
  }
}

// Get the size of an image in pixels
native_size_t native_image_get_size(native_image_t image) {
  native_size_t result = {0.0, 0.0};

  if (!image) {
    return result;
  }

  try {
    auto img = static_cast<std::shared_ptr<Image>*>(image);
    auto size = (*img)->GetSize();
    result.width = size.width;
    result.height = size.height;
  } catch (...) {
    // Return zero size on error
  }

  return result;
}

// Get the image format string for debugging purposes
char* native_image_get_format(native_image_t image) {
  if (!image) {
    return nullptr;
  }

  try {
    auto img = static_cast<std::shared_ptr<Image>*>(image);
    std::string format = (*img)->GetFormat();

    return to_c_str(format);
  } catch (...) {
    // Handle exceptions
  }

  return nullptr;
}

// Convert an image to base64-encoded PNG data
char* native_image_to_base64(native_image_t image) {
  if (!image) {
    return nullptr;
  }

  try {
    auto img = static_cast<std::shared_ptr<Image>*>(image);
    std::string base64 = (*img)->ToBase64();

    return to_c_str(base64);
  } catch (...) {
    // Handle exceptions
  }

  return nullptr;
}

// Save an image to a file
bool native_image_save_to_file(native_image_t image, const char* file_path) {
  if (!image || !file_path) {
    return false;
  }

  try {
    auto img = static_cast<std::shared_ptr<Image>*>(image);
    return (*img)->SaveToFile(file_path);
  } catch (...) {
    return false;
  }
}
