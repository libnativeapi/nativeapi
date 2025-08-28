# EventEmitter 使用指南

## 概述

`EventEmitter` 是一个基类，继承它即可为您的类提供完整的事件发射和监听功能。它基于观察者模式实现，支持类型安全的事件处理、同步和异步事件分发。

## 基本用法

### 1. 定义事件类

首先，您需要定义事件类。有几种方式：

#### 方式一：手动定义（推荐）
```cpp
class MyEvent : public TypedEvent<MyEvent> {
public:
    std::string message;
    int value;
    
    MyEvent(std::string msg, int val) 
        : message(std::move(msg)), value(val) {}
};
```

#### 方式二：使用宏定义
```cpp
DEFINE_EVENT_BEGIN(MyEvent)
    std::string message;
    int value;
    
    MyEvent(std::string msg, int val) 
        : message(std::move(msg)), value(val) {}
DEFINE_EVENT_END()
```

#### 方式三：简单事件（仅数据成员）
```cpp
SIMPLE_EVENT(StatusEvent, 
    bool success;
    std::string error_message;
)
```

### 2. 创建继承 EventEmitter 的类

```cpp
class MyClass : public EventEmitter {
public:
    void DoSomething() {
        // 触发同步事件
        EmitSync<MyEvent>("Hello", 42);
        
        // 触发异步事件
        EmitAsync<MyEvent>("Async Hello", 100);
    }
    
    void DoSomethingElse(const std::string& msg, int val) {
        // 直接传递参数构造事件
        EmitSync<MyEvent>(msg, val);
    }
};
```

### 3. 添加事件监听器

#### Lambda 函数监听器（推荐）
```cpp
MyClass obj;

size_t listener_id = obj.AddListener<MyEvent>([](const MyEvent& event) {
    std::cout << "收到事件: " << event.message << ", 值: " << event.value << std::endl;
});
```

#### 类型化监听器
```cpp
class MyListener : public TypedEventListener<MyEvent> {
public:
    void OnTypedEvent(const MyEvent& event) override {
        std::cout << "监听器收到: " << event.message << std::endl;
    }
};

MyListener listener;
size_t id = obj.AddListener<MyEvent>(&listener);
```

### 4. 管理监听器

```cpp
// 移除特定监听器
obj.RemoveListener(listener_id);

// 移除某种事件类型的所有监听器
obj.RemoveAllListeners<MyEvent>();

// 移除所有监听器
obj.RemoveAllListeners();

// 检查监听器数量
size_t count = obj.GetListenerCount<MyEvent>();
size_t total = obj.GetTotalListenerCount();

// 检查是否有监听器
bool has_listeners = obj.HasListeners<MyEvent>();
```

## API 参考

### EventEmitter 公共方法

#### 添加监听器
- `size_t AddListener<EventType>(TypedEventListener<EventType>* listener)`
  - 添加类型化监听器
  - 返回监听器 ID
  
- `size_t AddListener<EventType>(std::function<void(const EventType&)> callback)`
  - 添加 Lambda 函数监听器
  - 返回监听器 ID

#### 移除监听器
- `bool RemoveListener(size_t listener_id)`
  - 根据 ID 移除监听器
  - 返回是否成功

- `void RemoveAllListeners<EventType>()`
  - 移除指定类型的所有监听器

- `void RemoveAllListeners()`
  - 移除所有监听器

#### 查询方法
- `size_t GetListenerCount<EventType>() const`
  - 获取指定事件类型的监听器数量

- `size_t GetTotalListenerCount() const`
  - 获取总监听器数量

- `bool HasListeners<EventType>() const`
  - 检查是否有指定类型的监听器

### EventEmitter 受保护方法（用于子类）

#### 发射事件
- `void EmitSync(const Event& event)`
  - 同步发射事件对象

- `void EmitSync<EventType>(Args&&... args)`
  - 同步发射事件（完美转发参数）

- `void EmitAsync(std::unique_ptr<Event> event)`
  - 异步发射事件对象

- `void EmitAsync<EventType>(Args&&... args)`
  - 异步发射事件（完美转发参数）

## 完整示例

```cpp
#include "event_emitter.h"
#include <iostream>

// 定义事件
class ButtonClickEvent : public TypedEvent<ButtonClickEvent> {
public:
    std::string button_name;
    int x, y;
    
    ButtonClickEvent(std::string name, int x_pos, int y_pos)
        : button_name(std::move(name)), x(x_pos), y(y_pos) {}
};

// 继承 EventEmitter 的类
class Button : public EventEmitter {
public:
    Button(std::string name) : name_(std::move(name)) {}
    
    void Click(int x, int y) {
        std::cout << "按钮 '" << name_ << "' 被点击" << std::endl;
        EmitSync<ButtonClickEvent>(name_, x, y);
    }
    
private:
    std::string name_;
};

int main() {
    Button button("确定");
    
    // 添加监听器
    size_t id = button.AddListener<ButtonClickEvent>([](const ButtonClickEvent& event) {
        std::cout << "监听到点击: " << event.button_name 
                  << " 位置(" << event.x << ", " << event.y << ")" << std::endl;
    });
    
    // 触发事件
    button.Click(100, 200);
    
    // 清理
    button.RemoveListener(id);
    
    return 0;
}
```

## 最佳实践

1. **使用 Lambda 函数监听器**：对于简单的事件处理逻辑，Lambda 函数更加简洁。

2. **管理监听器生命周期**：确保在对象销毁前移除监听器，避免悬空指针。

3. **异常处理**：EventEmitter 会捕获监听器中的异常，但不会传播。确保您的监听器代码健壮。

4. **同步 vs 异步**：
   - 使用 `EmitSync` 进行即时处理
   - 使用 `EmitAsync` 避免阻塞当前线程

5. **事件设计**：保持事件类简单，只包含必要的数据。

6. **性能考虑**：大量监听器可能影响性能，考虑按需添加/移除。

## 线程安全

EventEmitter 是线程安全的，可以在多线程环境中安全使用。异步事件会在后台线程中处理，而同步事件在当前线程中立即处理。