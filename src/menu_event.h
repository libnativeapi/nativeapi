#pragma once

#include <string>

#include "foundation/event.h"

namespace nativeapi {

// Forward declarations for menu types
typedef long MenuID;
typedef long MenuItemID;

/**
 * @brief Menu opened event.
 *
 * This event is fired when a menu has been displayed.
 */
class MenuOpenedEvent : public Event {
 public:
  MenuOpenedEvent(MenuID menu_id) : menu_id_(menu_id) {}

  MenuID GetMenuId() const { return menu_id_; }

  std::string GetTypeName() const override { return "MenuOpenedEvent"; }

 private:
  MenuID menu_id_;
};

/**
 * @brief Menu closed event.
 *
 * This event is fired when a menu has been hidden or closed.
 */
class MenuClosedEvent : public Event {
 public:
  MenuClosedEvent(MenuID menu_id) : menu_id_(menu_id) {}

  MenuID GetMenuId() const { return menu_id_; }

  std::string GetTypeName() const override { return "MenuClosedEvent"; }

 private:
  MenuID menu_id_;
};

/**
 * @brief Menu item clicked event.
 *
 * This event is fired when a menu item is clicked or activated.
 * Contains information about which menu item was clicked.
 */
class MenuItemClickedEvent : public Event {
 public:
  MenuItemClickedEvent(MenuItemID item_id, const std::string& item_text)
      : item_id_(item_id), item_text_(item_text) {}

  MenuItemID GetItemId() const { return item_id_; }
  const std::string& GetItemText() const { return item_text_; }

  std::string GetTypeName() const override { return "MenuItemClickedEvent"; }

 private:
  MenuItemID item_id_;
  std::string item_text_;
};

/**
 * @brief Menu item submenu opened event.
 *
 * This event is fired when a menu item's submenu has been displayed.
 */
class MenuItemSubmenuOpenedEvent : public Event {
 public:
  MenuItemSubmenuOpenedEvent(MenuItemID item_id) : item_id_(item_id) {}

  MenuItemID GetItemId() const { return item_id_; }

  std::string GetTypeName() const override { return "MenuItemSubmenuOpenedEvent"; }

 private:
  MenuItemID item_id_;
};

/**
 * @brief Menu item submenu closed event.
 *
 * This event is fired when a menu item's submenu has been hidden or closed.
 */
class MenuItemSubmenuClosedEvent : public Event {
 public:
  MenuItemSubmenuClosedEvent(MenuItemID item_id) : item_id_(item_id) {}

  MenuItemID GetItemId() const { return item_id_; }

  std::string GetTypeName() const override { return "MenuItemSubmenuClosedEvent"; }

 private:
  MenuItemID item_id_;
};

}  // namespace nativeapi
