#include "message_dialog_c.h"
#include <cstring>
#include "../dialog.h"
#include "../message_dialog.h"
#include "string_utils_c.h"

using namespace nativeapi;

// Helper function to convert C dialog modality to C++ DialogModality
static DialogModality convert_dialog_modality(native_dialog_modality_t modality) {
  switch (modality) {
    case NATIVE_DIALOG_MODALITY_NONE:
      return DialogModality::None;
    case NATIVE_DIALOG_MODALITY_APPLICATION:
      return DialogModality::Application;
    case NATIVE_DIALOG_MODALITY_WINDOW:
      return DialogModality::Window;
    default:
      return DialogModality::None;
  }
}

// Helper function to convert C++ DialogModality to C dialog modality
static native_dialog_modality_t convert_dialog_modality(DialogModality modality) {
  switch (modality) {
    case DialogModality::None:
      return NATIVE_DIALOG_MODALITY_NONE;
    case DialogModality::Application:
      return NATIVE_DIALOG_MODALITY_APPLICATION;
    case DialogModality::Window:
      return NATIVE_DIALOG_MODALITY_WINDOW;
    default:
      return NATIVE_DIALOG_MODALITY_NONE;
  }
}

// MessageDialog C API Implementation

native_message_dialog_t native_message_dialog_create(const char* title, const char* message) {
  if (!title || !message)
    return nullptr;

  try {
    auto dialog = new MessageDialog(std::string(title), std::string(message));
    return static_cast<native_message_dialog_t>(dialog);
  } catch (...) {
    return nullptr;
  }
}

void native_message_dialog_destroy(native_message_dialog_t dialog) {
  if (!dialog)
    return;

  // Delete MessageDialog instance
  auto dialog_ptr = static_cast<MessageDialog*>(dialog);
  delete dialog_ptr;
}

void native_message_dialog_set_title(native_message_dialog_t dialog, const char* title) {
  if (!dialog)
    return;

  try {
    auto dialog_ptr = static_cast<MessageDialog*>(dialog);
    if (!dialog_ptr)
      return;
    if (title) {
      dialog_ptr->SetTitle(std::string(title));
    }
  } catch (...) {
    // Ignore exceptions
  }
}

char* native_message_dialog_get_title(native_message_dialog_t dialog) {
  if (!dialog)
    return nullptr;

  try {
    auto dialog_ptr = static_cast<MessageDialog*>(dialog);
    if (!dialog_ptr)
      return nullptr;

    std::string title = dialog_ptr->GetTitle();
    return to_c_str(title);
  } catch (...) {
    return nullptr;
  }
}

void native_message_dialog_set_message(native_message_dialog_t dialog, const char* message) {
  if (!dialog)
    return;

  try {
    auto dialog_ptr = static_cast<MessageDialog*>(dialog);
    if (!dialog_ptr)
      return;
    if (message) {
      dialog_ptr->SetMessage(std::string(message));
    }
  } catch (...) {
    // Ignore exceptions
  }
}

char* native_message_dialog_get_message(native_message_dialog_t dialog) {
  if (!dialog)
    return nullptr;

  try {
    auto dialog_ptr = static_cast<MessageDialog*>(dialog);
    if (!dialog_ptr)
      return nullptr;

    std::string message = dialog_ptr->GetMessage();
    return to_c_str(message);
  } catch (...) {
    return nullptr;
  }
}

void native_message_dialog_set_modality(native_message_dialog_t dialog,
                                        native_dialog_modality_t modality) {
  if (!dialog)
    return;

  try {
    auto dialog_ptr = static_cast<MessageDialog*>(dialog);
    if (!dialog_ptr)
      return;
    dialog_ptr->SetModality(convert_dialog_modality(modality));
  } catch (...) {
    // Ignore exceptions
  }
}

native_dialog_modality_t native_message_dialog_get_modality(native_message_dialog_t dialog) {
  if (!dialog)
    return NATIVE_DIALOG_MODALITY_NONE;

  try {
    auto dialog_ptr = static_cast<MessageDialog*>(dialog);
    if (!dialog_ptr)
      return NATIVE_DIALOG_MODALITY_NONE;

    return convert_dialog_modality(dialog_ptr->GetModality());
  } catch (...) {
    return NATIVE_DIALOG_MODALITY_NONE;
  }
}

bool native_message_dialog_open(native_message_dialog_t dialog) {
  if (!dialog)
    return false;

  try {
    auto dialog_ptr = static_cast<MessageDialog*>(dialog);
    if (!dialog_ptr)
      return false;

    return dialog_ptr->Open();
  } catch (...) {
    return false;
  }
}

bool native_message_dialog_close(native_message_dialog_t dialog) {
  if (!dialog)
    return false;

  try {
    auto dialog_ptr = static_cast<MessageDialog*>(dialog);
    if (!dialog_ptr)
      return false;

    return dialog_ptr->Close();
  } catch (...) {
    return false;
  }
}
