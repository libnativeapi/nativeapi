#include <iostream>
#include <memory>
#include <chrono>
#include <thread>
#include "nativeapi.h"

using namespace nativeapi;

int main() {
    std::cout << "=== Menu Event System Example ===" << std::endl;
    
    try {
        // Create a menu
        auto menu = Menu::Create();
        std::cout << "Created menu with ID: " << menu->id << std::endl;
        
        // Create menu items with different types
        auto fileItem = MenuItem::Create("New File", MenuItemType::Normal);
        auto separatorItem = MenuItem::CreateSeparator();
        auto checkboxItem = MenuItem::Create("Word Wrap", MenuItemType::Checkbox);
        auto radioItem1 = MenuItem::Create("View Mode 1", MenuItemType::Radio);
        auto radioItem2 = MenuItem::Create("View Mode 2", MenuItemType::Radio);
        auto exitItem = MenuItem::Create("Exit", MenuItemType::Normal);
        
        // Set up radio group
        radioItem1->SetRadioGroup(1);
        radioItem2->SetRadioGroup(1);
        radioItem1->SetChecked(true);
        
        // Set keyboard accelerators
        fileItem->SetAccelerator(KeyboardAccelerator("N", KeyboardAccelerator::Ctrl));
        exitItem->SetAccelerator(KeyboardAccelerator("Q", KeyboardAccelerator::Ctrl));
        
        // Add event listeners using the new event system
        std::cout << "Setting up event listeners..." << std::endl;
        
        // Listen to menu item selection events
        fileItem->AddListener<MenuItemSelectedEvent>([](const MenuItemSelectedEvent& event) {
            std::cout << "[EVENT] Menu item selected: " << event.GetItemText() 
                      << " (ID: " << event.GetItemId() << ")" << std::endl;
        });
        
        checkboxItem->AddListener<MenuItemSelectedEvent>([](const MenuItemSelectedEvent& event) {
            std::cout << "[EVENT] Checkbox item clicked: " << event.GetItemText() 
                      << " (ID: " << event.GetItemId() << ")" << std::endl;
        });
        
        // Listen to state change events for checkable items
        checkboxItem->AddListener<MenuItemStateChangedEvent>([](const MenuItemStateChangedEvent& event) {
            std::cout << "[EVENT] Checkbox state changed: ID " << event.GetItemId() 
                      << ", checked: " << (event.IsChecked() ? "true" : "false") << std::endl;
        });
        
        radioItem1->AddListener<MenuItemStateChangedEvent>([](const MenuItemStateChangedEvent& event) {
            std::cout << "[EVENT] Radio item 1 state changed: ID " << event.GetItemId() 
                      << ", checked: " << (event.IsChecked() ? "true" : "false") << std::endl;
        });
        
        radioItem2->AddListener<MenuItemStateChangedEvent>([](const MenuItemStateChangedEvent& event) {
            std::cout << "[EVENT] Radio item 2 state changed: ID " << event.GetItemId() 
                      << ", checked: " << (event.IsChecked() ? "true" : "false") << std::endl;
        });
        
        exitItem->AddListener<MenuItemSelectedEvent>([](const MenuItemSelectedEvent& event) {
            std::cout << "[EVENT] Exit item selected: " << event.GetItemText() << std::endl;
            std::cout << "Application should exit now..." << std::endl;
        });
        
        // Listen to menu events
        menu->AddListener<MenuOpenedEvent>([](const MenuOpenedEvent& event) {
            std::cout << "[EVENT] Menu opened: ID " << event.GetMenuId() << std::endl;
        });
        
        menu->AddListener<MenuClosedEvent>([](const MenuClosedEvent& event) {
            std::cout << "[EVENT] Menu closed: ID " << event.GetMenuId() << std::endl;
        });
        
        // Add items to menu
        menu->AddItem(fileItem);
        menu->AddItem(separatorItem);
        menu->AddItem(checkboxItem);
        menu->AddSeparator();
        menu->AddItem(radioItem1);
        menu->AddItem(radioItem2);
        menu->AddSeparator();
        menu->AddItem(exitItem);
        
        std::cout << "Menu created with " << menu->GetItemCount() << " items" << std::endl;
        
        // Demonstrate programmatic triggering
        std::cout << "\n=== Testing Programmatic Event Triggering ===" << std::endl;
        
        std::cout << "Triggering file item..." << std::endl;
        fileItem->Trigger();
        
        std::cout << "Triggering checkbox item..." << std::endl;
        checkboxItem->Trigger();
        
        std::cout << "Triggering checkbox item again..." << std::endl;
        checkboxItem->Trigger();
        
        std::cout << "Switching radio button..." << std::endl;
        radioItem2->Trigger();
        
        std::cout << "Triggering exit item..." << std::endl;
        exitItem->Trigger();
        
        // Show menu as context menu (this may not work in console applications)
        std::cout << "\n=== Attempting to Show Context Menu ===" << std::endl;
        std::cout << "Note: Context menu display may not work in console applications" << std::endl;
        
        // Try to show at screen coordinates (100, 100)
        if (menu->ShowAsContextMenu(100, 100)) {
            std::cout << "Context menu shown successfully" << std::endl;
        } else {
            std::cout << "Failed to show context menu (expected in console app)" << std::endl;
        }
        
        // Demonstrate submenu
        std::cout << "\n=== Testing Submenu ===" << std::endl;
        auto submenu = Menu::Create();
        auto submenuItem1 = MenuItem::Create("Submenu Item 1", MenuItemType::Normal);
        auto submenuItem2 = MenuItem::Create("Submenu Item 2", MenuItemType::Normal);
        
        submenuItem1->AddListener<MenuItemSelectedEvent>([](const MenuItemSelectedEvent& event) {
            std::cout << "[EVENT] Submenu item selected: " << event.GetItemText() << std::endl;
        });
        
        submenuItem2->AddListener<MenuItemSelectedEvent>([](const MenuItemSelectedEvent& event) {
            std::cout << "[EVENT] Submenu item selected: " << event.GetItemText() << std::endl;
        });
        
        submenu->AddItem(submenuItem1);
        submenu->AddItem(submenuItem2);
        
        auto submenuParent = MenuItem::Create("Tools", MenuItemType::Submenu);
        submenuParent->SetSubmenu(submenu);
        menu->AddItem(submenuParent);
        
        std::cout << "Added submenu with " << submenu->GetItemCount() << " items" << std::endl;
        
        // Test submenu items
        std::cout << "Triggering submenu items..." << std::endl;
        submenuItem1->Trigger();
        submenuItem2->Trigger();
        
        std::cout << "\n=== Event System Demo Complete ===" << std::endl;
        std::cout << "This example demonstrates:" << std::endl;
        std::cout << "1. Creating menus and menu items with different types" << std::endl;
        std::cout << "2. Using the new event system with AddListener<EventType>()" << std::endl;
        std::cout << "3. Handling MenuItemSelectedEvent and MenuItemStateChangedEvent" << std::endl;
        std::cout << "4. Handling MenuOpenedEvent and MenuClosedEvent" << std::endl;
        std::cout << "5. Programmatic event triggering" << std::endl;
        std::cout << "6. Submenu support with event propagation" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
