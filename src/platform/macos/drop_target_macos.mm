#include "../../drop_target_impl.h"

#include "drag_drop_utils_macos.h"

#import <Cocoa/Cocoa.h>

namespace nativeapi {
namespace {

NSDictionary* FileURLOptions() {
  return @{NSPasteboardURLReadingFileURLsOnlyKey : @YES};
}

}  // namespace

// What the view reports to; implemented by DropTarget::Impl::Platform.
class DropTargetViewHandler {
 public:
  virtual ~DropTargetViewHandler() = default;
  virtual NSDragOperation Entered(Point position, NSDragOperation source_mask, bool has_data) = 0;
  virtual NSDragOperation Moved(Point position, NSDragOperation source_mask) = 0;
  virtual void Exited(Point position) = 0;
  virtual bool IsAccepted() const = 0;
  virtual void Dropped(Point position, std::vector<std::string> file_paths, std::string text) = 0;
};

}  // namespace nativeapi

// Laid over the window's content view so that the content (a Flutter view, a
// web view) needs no changes. It returns nil from -hitTest:, so mouse events go
// to the views below; AppKit finds drag destinations by their registered types
// instead.
@interface NativeAPIDropTargetView : NSView
@property(nonatomic, assign) nativeapi::DropTargetViewHandler* target;
@end

@implementation NativeAPIDropTargetView

- (NSView*)hitTest:(NSPoint)point {
  return nil;
}

- (BOOL)wantsPeriodicDraggingUpdates {
  return NO;
}

- (nativeapi::Point)positionOf:(id<NSDraggingInfo>)info {
  NSView* content = self.window.contentView ?: self;
  NSPoint point = [content convertPoint:info.draggingLocation fromView:nil];
  if (!content.isFlipped) {
    point.y = NSHeight(content.bounds) - point.y;
  }
  return {point.x, point.y};
}

- (BOOL)hasData:(id<NSDraggingInfo>)info {
  NSPasteboard* pasteboard = info.draggingPasteboard;
  return [pasteboard canReadObjectForClasses:@[ [NSURL class] ]
                                     options:nativeapi::FileURLOptions()] ||
         [pasteboard availableTypeFromArray:@[ NSPasteboardTypeString ]] != nil;
}

- (NSDragOperation)draggingEntered:(id<NSDraggingInfo>)info {
  if (!self.target) {
    return NSDragOperationNone;
  }
  return self.target->Entered([self positionOf:info], info.draggingSourceOperationMask,
                              [self hasData:info]);
}

- (NSDragOperation)draggingUpdated:(id<NSDraggingInfo>)info {
  if (!self.target) {
    return NSDragOperationNone;
  }
  return self.target->Moved([self positionOf:info], info.draggingSourceOperationMask);
}

- (void)draggingExited:(id<NSDraggingInfo>)info {
  if (self.target) {
    self.target->Exited([self positionOf:info]);
  }
}

// Also sent when a drop was refused after -draggingEntered: accepted it.
- (void)draggingEnded:(id<NSDraggingInfo>)info {
  if (self.target) {
    self.target->Exited([self positionOf:info]);
  }
}

- (BOOL)prepareForDragOperation:(id<NSDraggingInfo>)info {
  return self.target != nullptr && self.target->IsAccepted();
}

- (BOOL)performDragOperation:(id<NSDraggingInfo>)info {
  if (!self.target || !self.target->IsAccepted()) {
    return NO;
  }
  NSPasteboard* pasteboard = info.draggingPasteboard;
  std::vector<std::string> file_paths;
  NSArray<NSURL*>* urls = [pasteboard readObjectsForClasses:@[ [NSURL class] ]
                                                    options:nativeapi::FileURLOptions()];
  for (NSURL* url in urls) {
    const char* path = url.path.UTF8String;
    if (path) {
      file_paths.emplace_back(path);
    }
  }
  std::string text;
  NSString* string = [pasteboard stringForType:NSPasteboardTypeString];
  if (string.UTF8String) {
    text = string.UTF8String;
  }
  self.target->Dropped([self positionOf:info], std::move(file_paths), std::move(text));
  return YES;
}

@end

namespace nativeapi {

struct DropTarget::Impl::Platform : DropTargetViewHandler {
  explicit Platform(Impl* impl) : impl(impl) {}

  static unsigned SourceOperations(NSDragOperation mask) {
    unsigned result = 0;
    for (DragOperation operation :
         {DragOperation::Copy, DragOperation::Move, DragOperation::Link}) {
      if (mask & ToNSDragOperation(operation)) {
        result |= OperationBit(operation);
      }
    }
    return result;
  }

  NSDragOperation Entered(Point position, NSDragOperation source_mask, bool has_data) override {
    return ToNSDragOperation(impl->Entered(position, SourceOperations(source_mask), has_data));
  }
  NSDragOperation Moved(Point position, NSDragOperation source_mask) override {
    return ToNSDragOperation(impl->Moved(position, SourceOperations(source_mask)));
  }
  void Exited(Point position) override { impl->Exited(position); }
  bool IsAccepted() const override { return impl->accepted; }
  void Dropped(Point position, std::vector<std::string> file_paths, std::string text) override {
    impl->Dropped(position, std::move(file_paths), std::move(text));
  }

  Impl* impl;
  NativeAPIDropTargetView* view = nil;
};

bool DropTarget::IsSupported() {
  return true;
}

DropTarget::Impl::Impl(DropTarget* owner, std::shared_ptr<Window> window)
    : owner(owner),
      window(std::move(window)),
      window_id(this->window ? this->window->GetId() : 0),
      platform(std::make_unique<Platform>(this)) {}

DropTarget::Impl::~Impl() = default;

bool DropTarget::Impl::Register() {
  NSWindow* ns_window = (__bridge NSWindow*)window->GetNativeObject();
  NSView* content = ns_window.contentView;
  if (!content) {
    return false;
  }
  NativeAPIDropTargetView* view = [[NativeAPIDropTargetView alloc] initWithFrame:content.bounds];
  view.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
  view.target = platform.get();
  [view registerForDraggedTypes:@[ NSPasteboardTypeFileURL, NSPasteboardTypeString ]];
  [content addSubview:view positioned:NSWindowAbove relativeTo:nil];
  // Kept (the +1 from alloc under manual reference counting) until
  // Unregister(), even if the content view goes away first.
  platform->view = view;
  return true;
}

void DropTarget::Impl::Unregister() {
  NativeAPIDropTargetView* view = platform->view;
  platform->view = nil;
  if (!view) {
    return;
  }
  view.target = nullptr;
  [view unregisterDraggedTypes];
  [view removeFromSuperview];
  ReleaseIfManual(view);
}

}  // namespace nativeapi
