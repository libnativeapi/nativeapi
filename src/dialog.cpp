#include "dialog.h"

namespace nativeapi {

Dialog::~Dialog() = default;

DialogModality Dialog::GetModality() const {
  return modality_;
}

void Dialog::SetModality(DialogModality modality) {
  modality_ = modality;
}

bool Dialog::Open() {
  return false;  // Base class implementation - should be overridden
}

bool Dialog::Close() {
  return false;  // Base class implementation - should be overridden
}

}  // namespace nativeapi
