#pragma once

#include <string>
#include "event.h"

namespace nativeapi {

// Forward declarations for menu types
typedef long MenuID;
typedef long MenuItemID;

/**
 * @brief Menu item selection event.
 *
 * This event is fired when a menu item is clicked or activated.
 * Contains information about which menu item was selected.
 */
class MenuItemSelectedEvent : public TypedEvent<MenuItemSelectedEvent> {
 public:
  MenuItemSelectedEvent(MenuItemID item_id, const std::string& item_text)
      : item_id_(item_id), item_text_(item_text) {}

  MenuItemID GetItemId() const { return item_id_; }
  const std::string& GetItemText() const { return item_text_; }

 private:
  MenuItemID item_id_;
  std::string item_text_;
};

/**
 * @brief Menu item state change event.
 *
 * This event is fired when a checkable menu item's state changes.
 */
class MenuItemStateChangedEvent : public TypedEvent<MenuItemStateChangedEvent> {
 public:
  MenuItemStateChangedEvent(MenuItemID item_id, bool checked)
      : item_id_(item_id), checked_(checked) {}

  MenuItemID GetItemId() const { return item_id_; }
  bool IsChecked() const { return checked_; }

 private:
  MenuItemID item_id_;
  bool checked_;
};

/**
 * @brief Menu will open event.
 *
 * This event is fired when a menu is about to be displayed.
 */
class MenuWillOpenEvent : public TypedEvent<MenuWillOpenEvent> {
 public:
  MenuWillOpenEvent(MenuID menu_id) : menu_id_(menu_id) {}

  MenuID GetMenuId() const { return menu_id_; }

 private:
  MenuID menu_id_;
};

/**
 * @brief Menu will close event.
 *
 * This event is fired when a menu is about to be hidden or closed.
 */
class MenuWillCloseEvent : public TypedEvent<MenuWillCloseEvent> {
 public:
  MenuWillCloseEvent(MenuID menu_id) : menu_id_(menu_id) {}

  MenuID GetMenuId() const { return menu_id_; }

 private:
  MenuID menu_id_;
};

}  // namespace nativeapi
