#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <chrono>

#include "../src/event_emitter.h"

namespace nativeapi {

// Define some example events using the improved macros
DEFINE_EVENT_BEGIN(ButtonClickEvent)
    std::string button_name;
    int x, y;
    
    ButtonClickEvent(std::string name, int x_pos, int y_pos) 
        : button_name(std::move(name)), x(x_pos), y(y_pos) {}
DEFINE_EVENT_END()

DEFINE_EVENT_BEGIN(DataReceivedEvent)
    std::string data;
    size_t size;
    
    DataReceivedEvent(std::string d, size_t s) 
        : data(std::move(d)), size(s) {}
DEFINE_EVENT_END()

DEFINE_EVENT_BEGIN(ConnectionStatusEvent)
    bool connected;
    std::string message;
    
    ConnectionStatusEvent(bool conn, std::string msg) 
        : connected(conn), message(std::move(msg)) {}
DEFINE_EVENT_END()

// Example class that inherits from EventEmitter
class NetworkManager : public EventEmitter {
public:
    void Connect(const std::string& address) {
        std::cout << "Connecting to " << address << "..." << std::endl;
        
        // Simulate connection process
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Emit connection event
        EmitSync<ConnectionStatusEvent>(true, "Connected to " + address);
        
        // Start receiving data (simulate)
        StartReceiving();
    }
    
    void Disconnect() {
        std::cout << "Disconnecting..." << std::endl;
        EmitSync<ConnectionStatusEvent>(false, "Disconnected");
    }
    
private:
    void StartReceiving() {
        // Simulate receiving some data asynchronously
        std::thread([this]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            EmitAsync<DataReceivedEvent>("Hello World", 11);
            
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            EmitAsync<DataReceivedEvent>("How are you?", 12);
        }).detach();
    }
};

// Example class that uses button events
class Button : public EventEmitter {
public:
    Button(std::string name) : name_(std::move(name)) {}
    
    void Click(int x, int y) {
        std::cout << "Button '" << name_ << "' clicked at (" << x << ", " << y << ")" << std::endl;
        EmitSync<ButtonClickEvent>(name_, x, y);
    }
    
private:
    std::string name_;
};

// Example typed event listener class
class NetworkListener : public TypedEventListener<ConnectionStatusEvent> {
public:
    void OnTypedEvent(const ConnectionStatusEvent& event) override {
        std::cout << "[NetworkListener] Connection status: " 
                  << (event.connected ? "CONNECTED" : "DISCONNECTED")
                  << " - " << event.message << std::endl;
    }
};

}  // namespace nativeapi

int main() {
    using namespace nativeapi;
    
    std::cout << "=== EventEmitter Example ===" << std::endl;
    
    // Create objects that emit events
    NetworkManager network;
    Button button1("OK");
    Button button2("Cancel");
    
    // Create a typed listener
    NetworkListener network_listener;
    
    std::cout << "\n1. Adding listeners..." << std::endl;
    
    // Add typed listener
    size_t listener_id1 = network.AddListener<ConnectionStatusEvent>(&network_listener);
    
    // Add lambda listeners
    size_t listener_id2 = network.AddListener<DataReceivedEvent>([](const DataReceivedEvent& event) {
        std::cout << "[Lambda] Received data: \"" << event.data 
                  << "\" (size: " << event.size << " bytes)" << std::endl;
    });
    
    size_t listener_id3 = button1.AddListener<ButtonClickEvent>([](const ButtonClickEvent& event) {
        std::cout << "[Lambda] Button '" << event.button_name 
                  << "' clicked at position (" << event.x << ", " << event.y << ")" << std::endl;
    });
    
    size_t listener_id4 = button2.AddListener<ButtonClickEvent>([](const ButtonClickEvent& event) {
        std::cout << "[Lambda] Cancel button clicked!" << std::endl;
    });
    
    std::cout << "Network manager has " << network.GetTotalListenerCount() << " listeners" << std::endl;
    std::cout << "Button1 has " << button1.GetTotalListenerCount() << " listeners" << std::endl;
    std::cout << "Button2 has " << button2.GetTotalListenerCount() << " listeners" << std::endl;
    
    std::cout << "\n2. Triggering events..." << std::endl;
    
    // Trigger some events
    network.Connect("192.168.1.100");
    button1.Click(100, 200);
    button2.Click(150, 250);
    
    // Wait a bit for async events to be processed
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    std::cout << "\n3. Removing some listeners..." << std::endl;
    
    // Remove one listener
    bool removed = network.RemoveListener(listener_id2);
    std::cout << "Removed data listener: " << (removed ? "success" : "failed") << std::endl;
    
    // Check listener count
    std::cout << "Network manager now has " << network.GetListenerCount<DataReceivedEvent>() 
              << " DataReceivedEvent listeners" << std::endl;
    
    std::cout << "\n4. Testing after listener removal..." << std::endl;
    network.Disconnect();
    
    // Wait a bit more
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::cout << "\n5. Remove all listeners..." << std::endl;
    button1.RemoveAllListeners();
    button2.RemoveAllListeners<ButtonClickEvent>();
    
    std::cout << "Button1 has " << button1.GetTotalListenerCount() << " listeners" << std::endl;
    std::cout << "Button2 has " << button2.GetTotalListenerCount() << " listeners" << std::endl;
    
    // These clicks should not trigger any output from listeners
    std::cout << "\n6. Testing after removing all listeners..." << std::endl;
    button1.Click(300, 400);
    button2.Click(350, 450);
    
    std::cout << "\n=== Example completed ===" << std::endl;
    
    return 0;
}