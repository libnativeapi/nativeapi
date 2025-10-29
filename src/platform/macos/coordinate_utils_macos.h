#pragma once

// Import Cocoa and Core Graphics headers
#import <Cocoa/Cocoa.h>
#import <CoreGraphics/CoreGraphics.h>

namespace nativeapi {

// NSRect extension-like helper for coordinate system conversion
struct NSRectExt {
  // Convert NSRect with bottom-left origin to topLeft point
  static CGPoint topLeft(NSRect rect) {
    NSRect primaryScreenFrame = [[NSScreen screens][0] frame];
    return CGPointMake(rect.origin.x,
                       primaryScreenFrame.size.height - rect.origin.y - rect.size.height);
  }

  // Convert NSRect with topLeft origin to NSRect with bottom-left origin
  static NSRect bottomLeft(NSRect rect) {
    NSRect primaryScreenFrame = [[NSScreen screens][0] frame];
    // Convert topLeft origin to bottom-left origin
    double bottomY = primaryScreenFrame.size.height - rect.origin.y - rect.size.height;
    return NSMakeRect(rect.origin.x, bottomY, rect.size.width, rect.size.height);
  }
};

// NSPoint extension-like helper for coordinate system conversion
struct NSPointExt {
  static CGPoint topLeft(NSPoint point) {
    NSRect primaryScreenFrame = [[NSScreen screens][0] frame];
    return CGPointMake(point.x, primaryScreenFrame.size.height - point.y);
  }

  // Convert from top-left coordinate system to bottom-left (macOS default)
  static NSPoint bottomLeft(CGPoint topLeftPoint) {
    NSRect primaryScreenFrame = [[NSScreen screens][0] frame];
    return NSMakePoint(topLeftPoint.x, primaryScreenFrame.size.height - topLeftPoint.y);
  }
};

}  // namespace nativeapi
