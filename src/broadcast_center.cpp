#include <iostream>

#include "broadcast_center.h"

namespace nativeapi {

BroadcastCenter::BroadcastCenter() {
  std::cout << "BroadcastCenter::BroadcastCenter()" << std::endl;
};

BroadcastCenter::~BroadcastCenter() {
  std::cout << "BroadcastCenter::~BroadcastCenter()" << std::endl;
};

void BroadcastCenter::NotifyReceivers(
    const std::string& topic,
    std::function<void(BroadcastReceiver*)> callback) {
  for (const auto& receiver : receivers_) {
    callback(receiver);
  }
}

BroadcastEventHandler::BroadcastEventHandler(
    std::function<void(const std::string& message)> onBroadcastReceivedCallback)
    : onBroadcastReceivedCallback_(std::move(onBroadcastReceivedCallback)) {}

void BroadcastEventHandler::OnBroadcastReceived(const std::string& message) {
  if (onBroadcastReceivedCallback_) {
    onBroadcastReceivedCallback_(message);
  }
}

}  // namespace nativeapi
