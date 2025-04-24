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
  virtual void OnBroadcastReceived(const std::string& topic,
                                   const std::string& message) = 0;
};

// BroadcastCenter is a singleton that manages all broadcasts on the system.
class BroadcastCenter {
 public:
  BroadcastCenter();
  virtual ~BroadcastCenter();

  // Send a broadcast message to all receivers of a given topic
  void SendBroadcast(const std::string& topic, const std::string& message);

  // Register a receiver to the broadcast center
  void RegisterReceiver(const std::string& topic, BroadcastReceiver* receiver);

  // Unregister a receiver from the broadcast center
  void UnregisterReceiver(const std::string& topic,
                          BroadcastReceiver* receiver);

 private:
  std::vector<BroadcastReceiver*> receivers_;
};

// BroadcastEventHandler is an implementation of BroadcastReceiver that uses
// callbacks to handle broadcast events.
class BroadcastEventHandler : public BroadcastReceiver {
 public:
  // Constructor that takes callbacks for broadcast events
  BroadcastEventHandler(
      std::function<void(const std::string& topic, const std::string& message)>
          onBroadcastReceivedCallback);

  // Implementation of OnBroadcastReceive from BroadcastReceiver interface
  void OnBroadcastReceived(const std::string& topic,
                           const std::string& message) override;

 private:
  std::function<void(const std::string& topic, const std::string& message)>
      onBroadcastReceivedCallback_;
};

}  // namespace nativeapi
