#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <vector>

#include "display.h"
#include "geometry.h"

namespace nativeapi {

// BroadcastReceiver is an interface that can be implemented by classes that
// want to listen for broadcasts.
class BroadcastReceiver {
 public:
  virtual void OnBroadcastReceived(const std::string& message) = 0;
};

// BroadcastCenter is a singleton that manages all broadcasts on the system.
class BroadcastCenter {
 public:
  BroadcastCenter();
  virtual ~BroadcastCenter();

  // Register a receiver to the broadcast center
  void RegisterReceiver(const std::string& topic, BroadcastReceiver* receiver);

  // Unregister a receiver from the broadcast center
  void UnregisterReceiver(const std::string& topic,
                          BroadcastReceiver* receiver);

  // Notify all receivers of a given topic
  void NotifyReceivers(const std::string& topic,
                       std::function<void(BroadcastReceiver*)> callback);

 private:
  std::vector<BroadcastReceiver*> receivers_;
};

// BroadcastEventHandler is an implementation of BroadcastReceiver that uses
// callbacks to handle broadcast events.
class BroadcastEventHandler : public BroadcastReceiver {
 public:
  // Constructor that takes callbacks for broadcast events
  BroadcastEventHandler(std::function<void(const std::string& message)>
                            onBroadcastReceivedCallback);

  // Implementation of OnBroadcastReceive from BroadcastReceiver interface
  void OnBroadcastReceived(const std::string& message) override;

 private:
  std::function<void(const std::string&)> onBroadcastReceivedCallback_;
};

}  // namespace nativeapi
