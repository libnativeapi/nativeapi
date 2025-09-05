# NativeAPI Design Rules and Conventions

This document outlines the design principles, architectural patterns, and coding conventions for the NativeAPI library. These rules should be followed by all contributors and AI agents working on this codebase.

## Project Overview

NativeAPI is a cross-platform native API library that provides access to system-level functionality (windows, displays, keyboard monitoring, tray icons, etc.) through both C++ and C interfaces.

## Architecture & Design Principles

### 1. Dual API Architecture

The library maintains two parallel APIs:

- **C++ API**: Primary object-oriented interface located in `src/` directory
- **C API**: FFI-compatible wrapper located in `src/capi/` directory for language bindings

**Rules:**
- All public C++ classes must have corresponding C API wrappers
- C API functions must be prefixed with `native_`
- All C API functions must use `FFI_PLUGIN_EXPORT` macro
- C API must handle memory management explicitly with create/destroy pairs

### 2. Cross-Platform Implementation

**Structure:**
```
src/
├── platform/
│   ├── macos/     # macOS-specific implementations (.mm files)
│   ├── windows/   # Windows-specific implementations (.cpp files)
│   └── linux/     # Linux-specific implementations (.cpp files)
├── *.h            # Abstract interfaces and declarations
└── *.cpp          # Platform-agnostic implementations
```

**Rules:**
- Abstract interfaces go in main `src/` directory
- Platform-specific code goes in respective `src/platform/{platform}/` directories
- Use conditional compilation for platform detection
- macOS implementations use Objective-C++ (`.mm` extension)

### 3. Event System Architecture

The library uses a sophisticated event system with the following components:

- `Event`: Base class with timestamp support
- `TypedEvent<T>`: Template for type-safe events
- `EventListener`: Generic observer interface
- `TypedEventListener<T>`: Type-safe observer template
- `CallbackEventListener<T>`: Lambda/function callback support

**Rules:**
- All events must inherit from `Event` or `TypedEvent<T>`
- Event classes must implement `GetTypeName()` and `GetType()` methods
- Use observer pattern for event distribution
- Events must be immutable after creation
- Include timestamp in all events

### 4. Memory Management Strategy

**C++ API:**
- Use RAII principles throughout
- Prefer `std::shared_ptr` for shared ownership
- Use PIMPL idiom for platform-specific implementations
- Automatic cleanup through destructors

**C API:**
- Manual memory management with explicit lifecycle
- All create functions must have corresponding destroy functions
- Null pointer checks required in all C API functions
- Return boolean for success/failure indication

## Coding Conventions

### 5. Naming Conventions

**Namespaces:**
- All C++ code in `nativeapi` namespace
- No nested namespaces

**Classes and Types:**
- PascalCase: `WindowManager`, `EventDispatcher`, `WindowCreatedEvent`
- ID types: `typedef long WindowID` pattern

**Functions and Methods:**
- PascalCase for public methods: `GetInstance()`, `Focus()`, `IsVisible()`
- Private methods may use camelCase or snake_case

**Variables:**
- snake_case with trailing underscore for members: `window_id_`, `new_position_`
- camelCase for local variables and parameters
- UPPER_SNAKE_CASE for constants

**C API Naming:**
- snake_case with `native_` prefix: `native_window_create()`, `native_window_get_title()`
- Type names end with `_t`: `native_window_t`, `native_size_t`

### 6. File Organization

**File Extensions:**
- `.h`: C++ headers with `#pragma once`
- `.cpp`: C++ implementation files
- `.mm`: Objective-C++ for macOS platform code
- `_c.h`: C API headers
- `_c.cpp`: C API implementation

**File Naming:**
- snake_case for all filenames
- Descriptive names matching primary class/functionality
- Event files grouped: `window_event.h`, `keyboard_event.h`, `display_event.h`

### 7. Type System

**Geometry Types:**
```cpp
struct Point { double x, y; };
struct Size { double width, height; };
struct Rectangle { double x, y, width, height; };
```

**ID Types:**
```cpp
typedef long WindowID;
typedef long DisplayID;
```

**C API Types:**
- Opaque handle pattern: `typedef struct native_window_handle* native_window_t;`
- Consistent `native_` prefixing
- Mirror C++ geometry types with `native_` prefix

### 8. Documentation Standards

**Required Documentation:**
- Doxygen-style comments for all public APIs
- Class documentation with purpose and usage
- Method documentation with parameters and return values
- Example usage in complex APIs

**Format:**
```cpp
/**
 * Brief description of the class or method
 *
 * Detailed description if needed, including usage notes
 * or important behavioral information.
 *
 * @param param_name Description of parameter
 * @return Description of return value
 */
```

### 9. Design Pattern Usage

**Singleton Pattern:**
- Use for manager classes: `WindowManager::GetInstance()`
- Thread-safe static initialization
- No public constructors

**PIMPL Pattern:**
- Use for platform-specific implementations
- Private `Impl` class in implementation files
- Forward declare in headers

**Observer Pattern:**
- Event system implementation
- Type-safe event listeners
- Support for both class-based and callback-based observers

**Factory Pattern:**
- Options structs for object creation
- Separate create/destroy functions in C API

### 10. Error Handling

**C++ API:**
- Use exceptions for exceptional conditions
- Return boolean for simple success/failure
- Check return values and handle errors appropriately

**C API:**
- Boolean return values for success/failure
- Null pointer checks at function entry
- Set output parameters only on success
- Provide error information through separate functions if needed

## Implementation Guidelines

### 11. Header Dependencies

- Minimize header dependencies
- Use forward declarations when possible
- Include only what you use
- Platform-specific includes only in implementation files

### 12. Platform-Specific Code

**macOS:**
- Use Objective-C++ (`.mm`) for Cocoa integration
- Bridge between C++ and Objective-C properly
- Handle memory management for both C++ and Objective-C objects

**Windows:**
- Use Windows API directly
- Handle Unicode properly (UTF-8 <-> UTF-16 conversion)
- Proper COM initialization/cleanup

**Linux:**
- Use X11 or Wayland as appropriate
- Handle different desktop environments
- Proper error checking for system calls

### 13. Testing and Examples

- Provide examples for each major feature in `examples/` directory
- Examples should demonstrate both C++ and C API usage
- Include CMakeLists.txt for each example
- Examples should be simple and focused

### 14. Thread Safety

- Document thread safety guarantees for each class
- Use appropriate synchronization primitives
- Event system should be thread-safe
- Manager classes should handle concurrent access

## File Structure Requirements

```
src/
├── *.h                    # Public C++ interfaces
├── *.cpp                  # C++ implementations
├── capi/
│   ├── *_c.h             # C API headers
│   └── *_c.cpp           # C API implementations
└── platform/
    ├── macos/*.mm        # macOS implementations
    ├── windows/*.cpp     # Windows implementations
    └── linux/*.cpp       # Linux implementations

examples/
├── {feature}_example/
│   ├── CMakeLists.txt
│   └── main.cpp          # C++ example
└── {feature}_c_example/
    ├── CMakeLists.txt
    └── main.c            # C example

include/
└── nativeapi.h           # Main public header
```

## Quality Assurance

### 15. Code Review Checklist

- [ ] Follows naming conventions
- [ ] Proper documentation
- [ ] Memory management handled correctly
- [ ] Platform-specific code isolated
- [ ] C API provides equivalent functionality
- [ ] Error handling implemented
- [ ] Thread safety considered
- [ ] Examples updated if needed

### 16. Performance Considerations

- Minimize allocations in hot paths
- Use efficient data structures
- Cache expensive operations
- Profile platform-specific code
- Consider lazy initialization for expensive resources

This document should be referenced by all contributors and AI agents to maintain consistency and quality in the NativeAPI codebase.