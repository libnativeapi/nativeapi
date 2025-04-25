#include <map>
#include <memory>
#include <string>
#include <vector>

#include "broadcast_center.h"

// Import Cocoa headers
#import <Cocoa/Cocoa.h>

namespace nativeapi {

void BroadcastCenter::SendBroadcast(const std::string& topic, const std::string& message) {
  NSDistributedNotificationCenter* center = [NSDistributedNotificationCenter defaultCenter];
  [center postNotificationName:[NSString stringWithUTF8String:topic.c_str()]
                        object:nil
                      userInfo:@{@"message" : [NSString stringWithUTF8String:message.c_str()]}
            deliverImmediately:YES];
}

void BroadcastCenter::RegisterReceiver(const std::string& topic, BroadcastReceiver* receiver) {
  NSDistributedNotificationCenter* center = [NSDistributedNotificationCenter defaultCenter];
  [center addObserverForName:[NSString stringWithUTF8String:topic.c_str()]
                      object:nil
                       queue:[NSOperationQueue mainQueue]
                  usingBlock:^(NSNotification* notification) {
                    std::string topic = [notification.name UTF8String];
                    std::string message = [notification.userInfo[@"message"] UTF8String];
                    receiver->OnBroadcastReceived(topic, message);
                  }];
}

void BroadcastCenter::UnregisterReceiver(const std::string& topic, BroadcastReceiver* receiver) {
  // TODO: Implement
}

}  // namespace nativeapi
