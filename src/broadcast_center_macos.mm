#include <map>
#include <memory>
#include <string>
#include <vector>

#include "broadcast_center.h"

// Import Cocoa headers
#import <Cocoa/Cocoa.h>

namespace nativeapi {

// Static map to store BroadcastCenter instances
static std::map<BroadcastCenter*, std::string> g_broadcast_centers;

void BroadcastCenter::RegisterReceiver(const std::string& topic, BroadcastReceiver* receiver) {
  receivers_.push_back(receiver);

  NSDistributedNotificationCenter* center = [NSDistributedNotificationCenter defaultCenter];
  [center addObserverForName:[NSString stringWithUTF8String:topic.c_str()]
                      object:nil
                       queue:[NSOperationQueue mainQueue]
                  usingBlock:^(NSNotification* notification) {
                    std::string message = [notification.userInfo[@"message"] UTF8String];
                    receiver->OnBroadcastReceived(message);
                  }];
}

void BroadcastCenter::UnregisterReceiver(const std::string& topic, BroadcastReceiver* receiver) {}

}  // namespace nativeapi
