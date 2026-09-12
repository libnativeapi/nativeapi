#pragma once
#include "../message_dialog.h"
#include "../window.h"
namespace nativeapi {
struct MessageDialogState {
  std::string primary;
  std::string secondary;
  std::string close = "OK";
  MessageDialogResult default_button = MessageDialogResult::Close;
  MessageDialogResult result = MessageDialogResult::None;
  std::shared_ptr<Window> parent;
  bool open = false;
  bool input_enabled = false;
  std::string input;
  std::string checkbox;
  bool checked = false;
  double progress = -2;
};
}
