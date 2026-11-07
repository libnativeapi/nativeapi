// Targets only the fixture's PID and executable. No mouse or keyboard input.
#import <Cocoa/Cocoa.h>
#import <ApplicationServices/ApplicationServices.h>

static id Attribute(AXUIElementRef element, CFStringRef name) {
  CFTypeRef value = NULL;
  AXUIElementCopyAttributeValue(element, name, &value);
  return CFBridgingRelease(value);
}

static AXError PressQuit(AXUIElementRef element, int depth) {
  if (depth > 5) return kAXErrorNoValue;
  NSString* command = Attribute(element, kAXMenuItemCmdCharAttribute);
  NSNumber* modifiers = Attribute(element, kAXMenuItemCmdModifiersAttribute);
  if ([command.lowercaseString isEqualToString:@"q"] && modifiers.intValue == 0) {
    printf("MENU_QUIT %s\n", [Attribute(element, kAXTitleAttribute) UTF8String]);
    return AXUIElementPerformAction(element, kAXPressAction);
  }
  for (id child in Attribute(element, kAXChildrenAttribute)) {
    AXError error = PressQuit((__bridge AXUIElementRef)child, depth + 1);
    if (error != kAXErrorNoValue) return error;
  }
  return kAXErrorNoValue;
}

int main(int argc, const char** argv) {
  @autoreleasepool {
    if (argc != 4) return 2;
    pid_t pid = atoi(argv[1]);
    NSRunningApplication* app = [NSRunningApplication runningApplicationWithProcessIdentifier:pid];
    NSString* expected = [NSString stringWithUTF8String:argv[2]].stringByStandardizingPath;
    if (pid <= 0 || !app || ![app.executableURL.path.stringByStandardizingPath isEqual:expected]) {
      fprintf(stderr, "refusing to target a different process or executable\n");
      return 3;
    }
    if (strcmp(argv[3], "system") == 0) return [app terminate] ? 0 : 4;
    if (strcmp(argv[3], "menu") != 0) return 2;
    AXUIElementRef application = AXUIElementCreateApplication(pid);
    id bar = Attribute(application, kAXMenuBarAttribute);
    NSArray* items = bar ? Attribute((__bridge AXUIElementRef)bar, kAXChildrenAttribute) : nil;
    if (items.count < 2) { CFRelease(application); return 5; }
    // The first item is the system Apple menu; the second belongs to this app.
    AXUIElementRef menu = (__bridge AXUIElementRef)items[1];
    AXError error = AXUIElementPerformAction(menu, kAXPressAction);
    if (error == kAXErrorSuccess) error = PressQuit(menu, 0);
    CFRelease(application);
    if (error != kAXErrorSuccess) fprintf(stderr, "quit menu AX error %d\n", error);
    return error == kAXErrorSuccess ? 0 : 6;
  }
}
