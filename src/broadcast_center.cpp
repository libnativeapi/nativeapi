#include <iostream>

#include "broadcast_center.h"

namespace nativeapi {

BroadcastCenter::BroadcastCenter() {
  std::cout << "BroadcastCenter::BroadcastCenter()" << std::endl;
};

BroadcastCenter::~BroadcastCenter() {
  std::cout << "BroadcastCenter::~BroadcastCenter()" << std::endl;
};

BroadcastEventHandler::BroadcastEventHandler(
    std::function<void(const std::string& topic, const std::string& message)>
        onBroadcastReceivedCallback)
    : onBroadcastReceivedCallback_(std::move(onBroadcastReceivedCallback)) {}

void BroadcastEventHandler::OnBroadcastReceived(const std::string& topic,
                                                const std::string& message) {
  if (onBroadcastReceivedCallback_) {
    onBroadcastReceivedCallback_(topic, message);
  }
}

}  // namespace nativeapi
