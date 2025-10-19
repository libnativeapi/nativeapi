#pragma once

#include <string>

#include "foundation/event.h"
#include "foundation/id_allocator.h"

namespace nativeapi {

// Forward declarations for menu types
typedef IdAllocator::IdType MenuId;
typedef IdAllocator::IdType MenuItemId;

/**
 * @brief Base class for all menu-related events.
 *
 * This class provides common functionality for menu events.
 */
class MenuEvent : public Event {
 public:
  virtual ~MenuEvent() = default;

  std::string GetTypeName() const override { return "MenuEvent"; }
};

/**
 * @brief Menu opened event.
 *
 * This event is fired when a menu has been displayed.
 */
class MenuOpenedEvent : public MenuEvent {
 public:
  MenuOpenedEvent(MenuId menu_id) : menu_id_(menu_id) {}

  MenuId GetMenuId() const { return menu_id_; }

  std::string GetTypeName() const override { return "MenuOpenedEvent"; }

 private:
  MenuId menu_id_;
};

/**
 * @brief Menu closed event.
 *
 * This event is fired when a menu has been hidden or closed.
 */
class MenuClosedEvent : public MenuEvent {
 public:
  MenuClosedEvent(MenuId menu_id) : menu_id_(menu_id) {}

  MenuId GetMenuId() const { return menu_id_; }

  std::string GetTypeName() const override { return "MenuClosedEvent"; }

 private:
  MenuId menu_id_;
};

/**
 * @brief Menu item clicked event.
 *
 * This event is fired when a menu item is clicked or activated.
 * Contains information about which menu item was clicked.
 */
class MenuItemClickedEvent : public MenuEvent {
 public:
  MenuItemClickedEvent(MenuItemId item_id, const std::string& item_text)
      : item_id_(item_id), item_text_(item_text) {}

  MenuItemId GetItemId() const { return item_id_; }
  const std::string& GetItemText() const { return item_text_; }

  std::string GetTypeName() const override { return "MenuItemClickedEvent"; }

 private:
  MenuItemId item_id_;
  std::string item_text_;
};

/**
 * @brief Menu item submenu opened event.
 *
 * This event is fired when a menu item's submenu has been displayed.
 */
class MenuItemSubmenuOpenedEvent : public MenuEvent {
 public:
  MenuItemSubmenuOpenedEvent(MenuItemId item_id) : item_id_(item_id) {}

  MenuItemId GetItemId() const { return item_id_; }

  std::string GetTypeName() const override { return "MenuItemSubmenuOpenedEvent"; }

 private:
  MenuItemId item_id_;
};

/**
 * @brief Menu item submenu closed event.
 *
 * This event is fired when a menu item's submenu has been hidden or closed.
 */
class MenuItemSubmenuClosedEvent : public MenuEvent {
 public:
  MenuItemSubmenuClosedEvent(MenuItemId item_id) : item_id_(item_id) {}

  MenuItemId GetItemId() const { return item_id_; }

  std::string GetTypeName() const override { return "MenuItemSubmenuClosedEvent"; }

 private:
  MenuItemId item_id_;
};

}  // namespace nativeapi
