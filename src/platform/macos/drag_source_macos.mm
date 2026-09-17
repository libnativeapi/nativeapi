#include "../../drag_source_impl.h"

#include "../../image.h"
#include "coordinate_utils_macos.h"
#include "drag_drop_utils_macos.h"

#import <Cocoa/Cocoa.h>

namespace nativeapi {

// What the delegate reports to; implemented by DragSource::Impl::Platform.
class DragSourceDelegateHandler {
 public:
  virtual ~DragSourceDelegateHandler() = default;
  virtual void Ended(NSPoint screen_point, NSDragOperation operation) = 0;
};

}  // namespace nativeapi

@interface NativeAPIDragSourceDelegate : NSObject <NSDraggingSource>
@property(nonatomic, assign) nativeapi::DragSourceDelegateHandler* source;
@property(nonatomic, assign) NSDragOperation operationMask;
- (void)retainUntilEnded;
@end

namespace nativeapi {

struct DragSource::Impl::Platform : DragSourceDelegateHandler {
  explicit Platform(Impl* impl) : impl(impl) {}

  void Ended(NSPoint screen_point, NSDragOperation operation) override {
    // The delegate still holds itself while it runs the method that called this.
    NativeAPIDragSourceDelegate* ended = delegate;
    delegate = nil;
    ReleaseIfManual(ended);
    CGPoint top_left = NSPointExt::topLeft(screen_point);
    impl->Finish({top_left.x, top_left.y}, FromNSDragOperation(operation));
  }

  Impl* impl;
  NativeAPIDragSourceDelegate* delegate = nil;
};

namespace {

const CGFloat kIconSize = 48;
const CGFloat kStackOffset = 6;

// A small image of the first line of the text, for drags without an image.
NSImage* ImageForText(NSString* text) {
  NSString* line = [[text componentsSeparatedByCharactersInSet:NSCharacterSet.newlineCharacterSet]
      firstObject];
  if (line.length > 40) {
    line = [[line substringToIndex:40] stringByAppendingString:@"…"];
  }
  if (line.length == 0) {
    line = @" ";
  }
  NSDictionary* attributes = @{
    NSFontAttributeName : [NSFont systemFontOfSize:NSFont.systemFontSize],
    NSForegroundColorAttributeName : NSColor.labelColor,
  };
  NSSize text_size = [line sizeWithAttributes:attributes];
  NSSize size = NSMakeSize(ceil(text_size.width) + 12, ceil(text_size.height) + 6);
  return [NSImage imageWithSize:size
                        flipped:NO
                 drawingHandler:^BOOL(NSRect rect) {
                   [[NSColor.controlBackgroundColor colorWithAlphaComponent:0.9] setFill];
                   [[NSBezierPath bezierPathWithRoundedRect:rect xRadius:4 yRadius:4] fill];
                   [line drawAtPoint:NSMakePoint(6, 3) withAttributes:attributes];
                   return YES;
                 }];
}

// The event AppKit starts the session from. The current event is used when it
// is a mouse event of this window; a Flutter handler usually runs later than
// the event that triggered it, so otherwise one is made up at the cursor.
NSEvent* DragEvent(NSWindow* window) {
  NSEvent* current = NSApp.currentEvent;
  if (current.window == window && (current.type == NSEventTypeLeftMouseDown ||
                                   current.type == NSEventTypeLeftMouseDragged)) {
    return current;
  }
  NSPoint location = [window convertPointFromScreen:NSEvent.mouseLocation];
  return [NSEvent mouseEventWithType:NSEventTypeLeftMouseDragged
                            location:location
                       modifierFlags:0
                           timestamp:NSProcessInfo.processInfo.systemUptime
                        windowNumber:window.windowNumber
                             context:nil
                         eventNumber:0
                          clickCount:1
                            pressure:1];
}

// The dragging session swallows the mouse-up that ends it; hand one to the
// window so its content stops treating the button as held.
void SendMouseUp(NSWindow* window, NSPoint screen_point) {
  if (!window) {
    return;
  }
  NSEvent* up = [NSEvent mouseEventWithType:NSEventTypeLeftMouseUp
                                   location:[window convertPointFromScreen:screen_point]
                              modifierFlags:0
                                  timestamp:NSProcessInfo.processInfo.systemUptime
                               windowNumber:window.windowNumber
                                    context:nil
                                eventNumber:0
                                 clickCount:1
                                   pressure:0];
  [window sendEvent:up];
}

}  // namespace
}  // namespace nativeapi

@implementation NativeAPIDragSourceDelegate {
  // Retained until the session ends, for the synthesized mouse-up.
  NSWindow* _window;
  // The delegate keeps itself alive until the session ends: AppKit may call it
  // after the DragSource that started the session is gone.
  id _selfReference;
}

- (void)retainUntilEnded {
  _selfReference = nativeapi::RetainIfManual(self);
}

- (void)setWindow:(NSWindow*)window {
  nativeapi::ReleaseIfManual(_window);
  _window = nativeapi::RetainIfManual(window);
}

- (void)dealloc {
  nativeapi::ReleaseIfManual(_window);
  _window = nil;
#if !__has_feature(objc_arc)
  [super dealloc];
#endif
}

- (NSDragOperation)draggingSession:(NSDraggingSession*)session
    sourceOperationMaskForDraggingContext:(NSDraggingContext)context {
  return self.operationMask;
}

- (BOOL)ignoreModifierKeysForDraggingSession:(NSDraggingSession*)session {
  // Only one operation is offered; modifiers must not narrow it to nothing.
  return YES;
}

- (void)draggingSession:(NSDraggingSession*)session
           endedAtPoint:(NSPoint)screenPoint
              operation:(NSDragOperation)operation {
  if (_window.isVisible) {
    nativeapi::SendMouseUp(_window, screenPoint);
  }
  [self setWindow:nil];
  nativeapi::DragSourceDelegateHandler* source = self.source;
  self.source = nullptr;
  if (source) {
    source->Ended(screenPoint, operation);
  }
  // Released on a later turn of the run loop, not while this method runs.
  id reference = _selfReference;
  _selfReference = nil;
  dispatch_async(dispatch_get_main_queue(), ^{
    nativeapi::ReleaseIfManual(reference);
  });
}

@end

namespace nativeapi {

bool DragSource::IsSupported() {
  return true;
}

DragSource::Impl::Impl(DragSource* owner)
    : owner(owner), platform(std::make_unique<Platform>(this)) {}

DragSource::Impl::~Impl() {
  // The session may outlive the source; it must not call back into it.
  NativeAPIDragSourceDelegate* delegate = platform->delegate;
  platform->delegate = nil;
  if (delegate) {
    delegate.source = nullptr;
    ReleaseIfManual(delegate);
  }
}

bool DragSource::Impl::Start() {
  NSWindow* ns_window = (__bridge NSWindow*)window->GetNativeObject();
  NSView* view = ns_window.contentView;
  if (!view || ([NSEvent pressedMouseButtons] & 1) == 0) {
    return false;
  }

  NSPoint cursor = [view convertPoint:[ns_window convertPointFromScreen:NSEvent.mouseLocation]
                             fromView:nil];
  NSImage* custom_image =
      image ? (__bridge NSImage*)image->GetNativeObject() : nil;

  NSMutableArray<NSDraggingItem*>* items = [NSMutableArray array];
  auto add_item = [&](NSPasteboardItem* pasteboard_item, NSImage* contents, NSUInteger index) {
    NSSize size = contents ? contents.size : NSMakeSize(kIconSize, kIconSize);
    if (custom_image) {
      index = 0;  // One picture for the whole drag.
    }
    CGFloat offset = kStackOffset * index;
    // Centered on the cursor, stacked down and to the right.
    CGFloat y_offset = view.isFlipped ? offset : -offset;
    NSRect frame = NSMakeRect(cursor.x - size.width / 2 + offset,
                              cursor.y - size.height / 2 + y_offset, size.width, size.height);
    NSDraggingItem* item = [[NSDraggingItem alloc] initWithPasteboardWriter:pasteboard_item];
    [item setDraggingFrame:frame contents:contents];
    [items addObject:item];
    ReleaseIfManual(item);
  };

  NSString* text = nil;
  if (this->text.has_value()) {
    text = [NSString stringWithUTF8String:this->text->c_str()] ?: @"";
  }

  NSUInteger index = 0;
  for (const auto& path : file_paths) {
    NSString* ns_path = [NSString stringWithUTF8String:path.c_str()];
    if (!ns_path) {
      continue;
    }
    NSURL* url = [NSURL fileURLWithPath:ns_path];
    NSPasteboardItem* pasteboard_item = [[NSPasteboardItem alloc] init];
    [pasteboard_item setString:url.absoluteString forType:NSPasteboardTypeFileURL];
    // Text travels on the first item, so targets see a single drop.
    if (text && index == 0) {
      [pasteboard_item setString:text forType:NSPasteboardTypeString];
    }
    NSImage* contents = custom_image;
    if (!contents) {
      contents = [[NSWorkspace sharedWorkspace] iconForFile:ns_path];
      contents.size = NSMakeSize(kIconSize, kIconSize);
    }
    add_item(pasteboard_item, contents, index++);
    ReleaseIfManual(pasteboard_item);
  }
  if (items.count == 0) {
    if (!text) {
      return false;
    }
    NSPasteboardItem* pasteboard_item = [[NSPasteboardItem alloc] init];
    [pasteboard_item setString:text forType:NSPasteboardTypeString];
    add_item(pasteboard_item, custom_image ?: ImageForText(text), 0);
    ReleaseIfManual(pasteboard_item);
  }

  NativeAPIDragSourceDelegate* delegate = [[NativeAPIDragSourceDelegate alloc] init];
  delegate.source = platform.get();
  delegate.operationMask = ToNSDragOperation(operation);
  [delegate setWindow:ns_window];

  NSDraggingSession* session = [view beginDraggingSessionWithItems:items
                                                             event:DragEvent(ns_window)
                                                            source:delegate];
  if (!session) {
    ReleaseIfManual(delegate);
    return false;
  }
  [delegate retainUntilEnded];
  session.animatesToStartingPositionsOnCancelOrFail = YES;
  session.draggingFormation = NSDraggingFormationNone;
  // The source's reference (the +1 from alloc under manual reference counting),
  // dropped when the session ends or the source is destroyed.
  platform->delegate = delegate;
  return true;
}

}  // namespace nativeapi
