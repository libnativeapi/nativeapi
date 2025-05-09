#include <iostream>

#include "broadcast_center.h"

namespace nativeapi {

BroadcastCenter::BroadcastCenter() {}

BroadcastCenter::~BroadcastCenter() {}

BroadcastEventHandler::BroadcastEventHandler(
    std::function<void(const std::string& topic, const std::string& message)>
        onBroadcastReceivedCallback)
    : onBroadcastReceivedCallback_(std::move(onBroadcastReceivedCallback)) {}

void BroadcastEventHandler::OnBroadcastReceived(const std::string& topic,
                                                const std::string& message) {
  if (onBroadcastReceivedCallback_) {
    std::cout << "BroadcastEventHandler::OnBroadcastReceived() " << topic << " "
              << message << std::endl;
    onBroadcastReceivedCallback_(topic, message);
  }
}

}  // namespace nativeapi
