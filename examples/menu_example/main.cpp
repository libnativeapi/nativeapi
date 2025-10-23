#include <chrono>
#include <iostream>
#include <memory>
#include <thread>
#include "nativeapi.h"

using namespace nativeapi;

int main() {
  std::cout << "=== Menu Event System Example ===" << std::endl;

  try {
    // Create a menu
    auto menu = std::make_shared<Menu>();
    std::cout << "Created menu with ID: " << menu->GetId() << std::endl;

    // Create menu items with different types
    auto file_item = std::make_shared<MenuItem>("New File", MenuItemType::Normal);
    auto separator_item = std::make_shared<MenuItem>("", MenuItemType::Separator);
    auto checkbox_item = std::make_shared<MenuItem>("Word Wrap", MenuItemType::Checkbox);
    auto radio_item1 = std::make_shared<MenuItem>("View Mode 1", MenuItemType::Radio);
    auto radio_item2 = std::make_shared<MenuItem>("View Mode 2", MenuItemType::Radio);
    auto exit_item = std::make_shared<MenuItem>("Exit", MenuItemType::Normal);

    // Set up radio group
    radio_item1->SetRadioGroup(1);
    radio_item2->SetRadioGroup(1);
    radio_item1->SetState(MenuItemState::Checked);

    // Set keyboard accelerators
    file_item->SetAccelerator(KeyboardAccelerator("N", KeyboardAccelerator::Ctrl));
    exit_item->SetAccelerator(KeyboardAccelerator("Q", KeyboardAccelerator::Ctrl));

    // Add event listeners using the new event system
    std::cout << "Setting up event listeners..." << std::endl;

    // Listen to menu item selection events
    file_item->AddListener<MenuItemClickedEvent>([](const MenuItemClickedEvent& event) {
      std::cout << "[EVENT] Menu item clicked: New File"
                << " (ID: " << event.GetItemId() << ")" << std::endl;
    });

    checkbox_item->AddListener<MenuItemClickedEvent>([](const MenuItemClickedEvent& event) {
      std::cout << "[EVENT] Checkbox item clicked: Word Wrap"
                << " (ID: " << event.GetItemId() << ") - Handle state manually" << std::endl;
    });

    // Note: State management is now handled by the application

    radio_item1->AddListener<MenuItemClickedEvent>([](const MenuItemClickedEvent& event) {
      std::cout << "[EVENT] Radio item 1 clicked: ID " << event.GetItemId()
                << " - Handle state manually" << std::endl;
    });

    radio_item2->AddListener<MenuItemClickedEvent>([](const MenuItemClickedEvent& event) {
      std::cout << "[EVENT] Radio item 2 clicked: ID " << event.GetItemId()
                << " - Handle state manually" << std::endl;
    });

    exit_item->AddListener<MenuItemClickedEvent>([](const MenuItemClickedEvent& event) {
      std::cout << "[EVENT] Exit item clicked: Exit" << std::endl;
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
    menu->AddItem(file_item);
    menu->AddItem(separator_item);
    menu->AddItem(checkbox_item);
    menu->AddSeparator();
    menu->AddItem(radio_item1);
    menu->AddItem(radio_item2);
    menu->AddSeparator();
    menu->AddItem(exit_item);

    std::cout << "Menu created with " << menu->GetItemCount() << " items" << std::endl;

    // Demonstrate programmatic triggering
    std::cout << "\n=== Testing Programmatic Event Triggering ===" << std::endl;

    std::cout << "Triggering file item..." << std::endl;
    file_item->Trigger();

    std::cout << "Triggering checkbox item..." << std::endl;
    checkbox_item->Trigger();

    std::cout << "Triggering checkbox item again..." << std::endl;
    checkbox_item->Trigger();

    std::cout << "Switching radio button..." << std::endl;
    radio_item2->Trigger();

    std::cout << "Triggering exit item..." << std::endl;
    exit_item->Trigger();

    // Open menu as context menu (this may not work in console applications)
    std::cout << "\n=== Attempting to Open Context Menu ===" << std::endl;
    std::cout << "Note: Context menu display may not work in console applications" << std::endl;

    // Try to show at screen coordinates (100, 100)
    if (menu->Open(100, 100)) {
      std::cout << "Context menu opened successfully" << std::endl;
    } else {
      std::cout << "Failed to open context menu (expected in console app)" << std::endl;
    }

    // Demonstrate submenu
    std::cout << "\n=== Testing Submenu ===" << std::endl;
    auto submenu = std::make_shared<Menu>();
    auto submenu_item1 = std::make_shared<MenuItem>("Submenu Item 1", MenuItemType::Normal);
    auto submenu_item2 = std::make_shared<MenuItem>("Submenu Item 2", MenuItemType::Normal);

    submenu_item1->AddListener<MenuItemClickedEvent>([](const MenuItemClickedEvent& event) {
      std::cout << "[EVENT] Submenu item clicked: Submenu Item 1" << std::endl;
    });

    submenu_item2->AddListener<MenuItemClickedEvent>([](const MenuItemClickedEvent& event) {
      std::cout << "[EVENT] Submenu item clicked: Submenu Item 2" << std::endl;
    });

    submenu->AddItem(submenu_item1);
    submenu->AddItem(submenu_item2);

    auto submenu_parent = std::make_shared<MenuItem>("Tools", MenuItemType::Submenu);
    submenu_parent->SetSubmenu(submenu);

    // Add submenu event listeners
    submenu_parent->AddListener<MenuItemSubmenuOpenedEvent>(
        [](const MenuItemSubmenuOpenedEvent& event) {
          std::cout << "[EVENT] Submenu opened: ID " << event.GetItemId() << std::endl;
        });

    submenu_parent->AddListener<MenuItemSubmenuClosedEvent>(
        [](const MenuItemSubmenuClosedEvent& event) {
          std::cout << "[EVENT] Submenu closed: ID " << event.GetItemId() << std::endl;
        });

    menu->AddItem(submenu_parent);

    std::cout << "Added submenu with " << submenu->GetItemCount() << " items" << std::endl;

    // Test submenu items
    std::cout << "Triggering submenu items..." << std::endl;
    submenu_item1->Trigger();
    submenu_item2->Trigger();

    std::cout << "\n=== Event System Demo Complete ===" << std::endl;
    std::cout << "This example demonstrates:" << std::endl;
    std::cout << "1. Creating menus and menu items with different types" << std::endl;
    std::cout << "2. Using the new event system with AddListener<EventType>()" << std::endl;
    std::cout << "3. Handling MenuItemClickedEvent (state managed by application)" << std::endl;
    std::cout << "4. Handling MenuOpenedEvent and MenuClosedEvent" << std::endl;
    std::cout << "5. Handling MenuItemSubmenuOpenedEvent and "
                 "MenuItemSubmenuClosedEvent"
              << std::endl;
    std::cout << "6. Programmatic event triggering" << std::endl;
    std::cout << "7. Submenu support with event propagation" << std::endl;

  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
