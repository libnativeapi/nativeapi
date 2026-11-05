// App-scoped driver for nsh_examples_test_macos.py. Does not post input events
// or traverse another process: all targets come from the supplied app's PID.
#import <AppKit/AppKit.h>
#import <ApplicationServices/ApplicationServices.h>
static id attr(AXUIElementRef e, CFStringRef name) {
  CFTypeRef value = NULL;
  AXUIElementCopyAttributeValue(e, name, &value);
  return CFBridgingRelease(value);
}
static void walk(AXUIElementRef e, int depth) {
  if (depth > 20) return;
  NSMutableDictionary *row = [NSMutableDictionary dictionary];
  for (NSString *key in @[@"AXRole", @"AXSubrole", @"AXTitle", @"AXValue", @"AXDescription"]) {
    id value = attr(e, (__bridge CFStringRef)key);
    if ([value isKindOfClass:NSString.class] && [value length]) row[key] = value;
  }
  NSData *data = [NSJSONSerialization dataWithJSONObject:row options:0 error:nil];
  puts([[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding].UTF8String);
  for (id child in attr(e, kAXChildrenAttribute)) walk((__bridge AXUIElementRef)child, depth + 1);
}
static AXUIElementRef find(AXUIElementRef e, NSString *label, int depth) {
  if (depth > 20) return NULL;
  NSString *role = attr(e, kAXRoleAttribute);
  if ([role isEqual:@"AXButton"] || [role isEqual:@"AXMenuBarItem"]) {
    for (NSString *key in @[@"AXTitle", @"AXValue", @"AXDescription"]) {
      if ([attr(e, (__bridge CFStringRef)key) isEqual:label]) return (AXUIElementRef)CFRetain(e);
    }
  }
  for (id child in attr(e, kAXChildrenAttribute)) {
    AXUIElementRef found = find((__bridge AXUIElementRef)child, label, depth + 1);
    if (found) return found;
  }
  return NULL;
}
int main(int argc, char **argv) {
  @autoreleasepool {
    if (argc < 3) return 2;
    AXUIElementRef app = AXUIElementCreateApplication(atoi(argv[1]));
    NSString *command = @(argv[2]);
    NSArray *windows = attr(app, kAXWindowsAttribute);
    if ([command isEqual:@"dump"]) {
      for (id window in windows) walk((__bridge AXUIElementRef)window, 0);
      id extras = attr(app, CFSTR("AXExtrasMenuBar"));
      if (extras) walk((__bridge AXUIElementRef)extras, 0);
      CFRelease(app);
      return 0;
    }
    AXUIElementRef target = NULL;
    if ([command isEqual:@"press"] && argc == 4) {
      for (id window in windows) {
        target = find((__bridge AXUIElementRef)window, @(argv[3]), 0);
        if (target) break;
      }
      if (!target) {
        id extras = attr(app, CFSTR("AXExtrasMenuBar"));
        if (extras) target = find((__bridge AXUIElementRef)extras, @(argv[3]), 0);
      }
    } else if ([command isEqual:@"close"] && windows.count == 1) {
      id close = attr((__bridge AXUIElementRef)windows[0], kAXCloseButtonAttribute);
      if (close) target = (AXUIElementRef)CFRetain((__bridge CFTypeRef)close);
    } else if ([command isEqual:@"default"]) {
      for (id window in windows) {
        id button = attr((__bridge AXUIElementRef)window, kAXDefaultButtonAttribute);
        if (button) { target = (AXUIElementRef)CFRetain((__bridge CFTypeRef)button); break; }
      }
    }
    if (!target) { CFRelease(app); return 3; }
    AXError result = AXUIElementPerformAction(target, kAXPressAction);
    CFRelease(target);
    CFRelease(app);
    if (result != kAXErrorSuccess) fprintf(stderr, "AXPress failed: %d\n", result);
    return result == kAXErrorSuccess ? 0 : 4;
  }
}
