#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include "foundation/event.h"
#include "foundation/event_emitter.h"
#include "foundation/geometry.h"
#include "foundation/native_object_provider.h"
#include "menu_event.h"

namespace nativeapi {

typedef long MenuId;
typedef long MenuItemId;

/**
 * @brief Enumeration of different menu item types.
 *
 * Defines the various types of menu items that can be created,
 * each with different behavior and appearance characteristics.
 */
enum class MenuItemType {
  /**
   * Normal clickable menu item with text and optional icon.
   */
  Normal,

  /**
   * Checkable menu item that can be toggled on/off.
   */
  Checkbox,

  /**
   * Radio button menu item, part of a mutually exclusive group.
   */
  Radio,

  /**
   * Separator line between menu items.
   */
  Separator,

  /**
   * Submenu item that expands to show child items.
   */
  Submenu
};

/**
 * @brief State of a menu item (for checkboxes and radio buttons).
 *
 * Defines the possible states for checkable menu items.
 * Mixed state is typically used for checkboxes to indicate
 * a partially selected or indeterminate state.
 */
enum class MenuItemState {
  /**
   * Item is not checked/selected.
   */
  Unchecked,

  /**
   * Item is checked/selected.
   */
  Checked,

  /**
   * Item is in mixed/indeterminate state (checkboxes only).
   * Typically shown as a dash (-) or special symbol.
   */
  Mixed
};

/**
 * @brief Keyboard accelerator for menu items.
 *
 * Represents a keyboard shortcut that can trigger a menu item.
 * Supports modifier keys and regular keys.
 */
struct KeyboardAccelerator {
  /**
   * Modifier key flags that can be combined.
   */
  enum Modifiers {
    None = 0,
    Ctrl = 1 << 0,
    Alt = 1 << 1,
    Shift = 1 << 2,
    Meta = 1 << 3,  // Windows key on Windows, Cmd key on macOS
  };

  /**
   * Combination of modifier flags.
   */
  int modifiers = None;

  /**
   * The main key code (e.g., 'A', 'F1', etc.).
   */
  std::string key;

  /**
   * Constructor for creating keyboard accelerators.
   *
   * @param key The main key (e.g., "A", "F1", "Enter")
   * @param modifiers Combination of modifier flags
   *
   * @example
   * ```cpp
   * // Ctrl+S
   * KeyboardAccelerator save_accel("S", KeyboardAccelerator::Ctrl);
   *
   * // Ctrl+Shift+N
   * KeyboardAccelerator new_accel("N",
   *     KeyboardAccelerator::Ctrl | KeyboardAccelerator::Shift);
   * ```
   */
  KeyboardAccelerator(const std::string& key, int modifiers = None)
      : key(key), modifiers(modifiers) {}

  /**
   * Get a human-readable string representation of the accelerator.
   *
   * @return String representation like "Ctrl+S" or "Alt+F4"
   */
  std::string ToString() const;
};

// Forward declarations
class Menu;

/**
 * @brief MenuItem represents a single item in a menu.
 *
 * This class provides a cross-platform interface for creating and managing
 * menu items. Menu items can be simple clickable items, checkboxes, radio
 * buttons, separators, or submenu containers.
 *
 * The class supports:
 * - Different item types (normal, checkbox, radio, separator, submenu)
 * - Custom text, icons, and tooltips
 * - Keyboard shortcuts/accelerators
 * - Enable/disable state
 * - Event emission for user interaction
 * - Submenu nesting
 *
 * @note This class uses the PIMPL idiom to hide platform-specific
 * implementation details and ensure binary compatibility across different
 * platforms. It also inherits from EventEmitter to provide event-driven
 * interaction handling.
 *
 * @example
 * ```cpp
 * // Create a normal menu item
 * auto item = std::make_shared<MenuItem>("Open File", MenuItemType::Normal);
 * item->SetIcon("data:image/png;base64,...");
 * item->SetAccelerator(KeyboardAccelerator("O", KeyboardAccelerator::Ctrl));
 * item->AddListener<MenuItemClickedEvent>([](const MenuItemClickedEvent& event)
 * {
 *     // Handle menu item click
 *     std::cout << "Opening file..." << std::endl;
 * });
 *
 * // Create a checkbox item
 * auto checkbox = std::make_shared<MenuItem>("Show Toolbar", MenuItemType::Checkbox);
 * checkbox->SetChecked(true);
 * checkbox->AddListener<MenuItemClickedEvent>([](const MenuItemClickedEvent&
 * event) { std::cout << "Toolbar clicked, handle state change manually" <<
 * std::endl;
 * });
 * ```
 */
class MenuItem : public EventEmitter, public NativeObjectProvider {
 public:
  /**
   * @brief Constructor to create a new menu item.
   *
   * Creates a menu item of the specified type with the given text.
   *
   * @param text The display text for the menu item
   * @param type The type of menu item to create
   *
   * @example
   * ```cpp
   * auto item = std::make_shared<MenuItem>("File", MenuItemType::Normal);
   * auto separator = std::make_shared<MenuItem>("", MenuItemType::Separator);
   * auto checkbox = std::make_shared<MenuItem>("Word Wrap", MenuItemType::Checkbox);
   * ```
   */
  MenuItem(const std::string& text = "", MenuItemType type = MenuItemType::Normal);

  /**
   * @brief Constructor that wraps an existing platform-specific menu item.
   *
   * This constructor is typically used internally by platform-specific
   * implementations to wrap existing menu items.
   *
   * @param native_item Pointer to the platform-specific menu item object
   */
  explicit MenuItem(void* native_item);

  /**
   * @brief Destructor for MenuItem.
   *
   * Cleans up the menu item and releases any associated platform-specific
   * resources. Also removes any event listeners.
   */
  virtual ~MenuItem();

  /**
   * @brief Unique identifier for this menu item.
   *
   * This ID is assigned when the item is created and can be used to
   * reference the item in event handlers and other operations.
   */
  MenuItemId id;

  /**
   * @brief Get the type of this menu item.
   *
   * @return The MenuItemType of this item
   */
  MenuItemType GetType() const;

  /**
   * @brief Set the display label for the menu item.
   *
   * The label is what appears in the menu. On some platforms,
   * ampersand characters (&) can be used to indicate mnemonics
   * (keyboard navigation keys).
   *
   * @param label The label to display (optional)
   *
   * @example
   * ```cpp
   * item->SetLabel("&File");  // 'F' becomes the mnemonic on Windows/Linux
   * item->SetLabel("Open Recent");
   * item->SetLabel(std::nullopt);  // Clear the label
   * ```
   */
  void SetLabel(const std::optional<std::string>& label);

  /**
   * @brief Get the current display label of the menu item.
   *
   * @return The current label as an optional string
   */
  std::optional<std::string> GetLabel() const;

  /**
   * @brief Set the icon for the menu item.
   *
   * The icon can be specified as either a file path or a base64-encoded
   * image string. Base64 strings should be prefixed with the data URI
   * scheme (e.g., "data:image/png;base64,iVBORw0KGgo...").
   *
   * @param icon File path to an icon image or base64-encoded image data (optional)
   *
   * @note Supported formats depend on the platform:
   *       - macOS: PNG, JPEG, GIF, TIFF, BMP
   *       - Windows: ICO, PNG, BMP
   *       - Linux: PNG, XPM, SVG (depends on desktop environment)
   *
   * @example
   * ```cpp
   * // Using file path
   * item->SetIcon("/path/to/icon.png");
   *
   * // Using base64 data
   * item->SetIcon("data:image/png;base64,iVBORw0KGgo...");
   *
   * // Using system icons (platform-dependent)
   * item->SetIcon("folder");  // May work on some systems
   *
   * // Remove icon
   * item->SetIcon(std::nullopt);
   * ```
   */
  void SetIcon(const std::optional<std::string>& icon);

  /**
   * @brief Get the current icon of the menu item.
   *
   * @return The current icon path or data as an optional string
   */
  std::optional<std::string> GetIcon() const;

  /**
   * @brief Set the tooltip text for the menu item.
   *
   * The tooltip may appear when the user hovers over the menu item,
   * depending on the platform and menu context.
   *
   * @param tooltip The tooltip text to display (optional)
   */
  void SetTooltip(const std::optional<std::string>& tooltip);

  /**
   * @brief Get the current tooltip text of the menu item.
   *
   * @return The current tooltip text as an optional string
   */
  std::optional<std::string> GetTooltip() const;

  /**
   * @brief Set the keyboard accelerator for the menu item.
   *
   * The accelerator allows users to trigger the menu item using
   * keyboard shortcuts. The accelerator is typically displayed
   * next to the menu item text.
   *
   * @param accelerator The keyboard accelerator to set
   *
   * @example
   * ```cpp
   * // Set Ctrl+S as accelerator
   * item->SetAccelerator(KeyboardAccelerator("S", KeyboardAccelerator::Ctrl));
   *
   * // Set F1 as accelerator
   * item->SetAccelerator(KeyboardAccelerator("F1"));
   *
   * // Set Alt+F4 as accelerator
   * item->SetAccelerator(KeyboardAccelerator("F4", KeyboardAccelerator::Alt));
   * ```
   */
  void SetAccelerator(const KeyboardAccelerator& accelerator);

  /**
   * @brief Get the current keyboard accelerator of the menu item.
   *
   * @return The current KeyboardAccelerator, or an empty accelerator if none is
   * set
   */
  KeyboardAccelerator GetAccelerator() const;

  /**
   * @brief Remove the keyboard accelerator from the menu item.
   */
  void RemoveAccelerator();

  /**
   * @brief Enable or disable the menu item.
   *
   * Disabled menu items are typically grayed out and cannot be clicked.
   *
   * @param enabled true to enable the item, false to disable it
   */
  void SetEnabled(bool enabled);

  /**
   * @brief Check if the menu item is currently enabled.
   *
   * @return true if the item is enabled, false if disabled
   */
  bool IsEnabled() const;

  /**
   * @brief Show or hide the menu item.
   *
   * Hidden menu items are not displayed in the menu at all.
   *
   * @param visible true to show the item, false to hide it
   */
  void SetVisible(bool visible);

  /**
   * @brief Check if the menu item is currently visible.
   *
   * @return true if the item is visible, false if hidden
   */
  bool IsVisible() const;

  /**
   * @brief Set the state of a checkbox or radio menu item.
   *
   * This method provides more control than SetChecked, allowing
   * you to set mixed/indeterminate state for checkboxes.
   * For radio items, only Unchecked and Checked states are valid.
   *
   * @param state The desired state (Unchecked, Checked, or Mixed)
   */
  void SetState(MenuItemState state);

  /**
   * @brief Get the current state of a checkbox or radio menu item.
   *
   * @return The current state (Unchecked, Checked, or Mixed)
   */
  MenuItemState GetState() const;

  /**
   * @brief Set the radio group ID for radio menu items.
   *
   * Radio items with the same group ID are mutually exclusive -
   * only one item in the group can be checked at a time.
   *
   * @param group_id The radio group identifier
   */
  void SetRadioGroup(int group_id);

  /**
   * @brief Get the radio group ID of this menu item.
   *
   * @return The radio group ID, or -1 if not a radio item or no group set
   */
  int GetRadioGroup() const;

  /**
   * @brief Set the submenu for this menu item.
   *
   * This converts the item into a submenu item that expands to show
   * the provided menu when hovered or clicked.
   *
   * @param submenu Shared pointer to the submenu to attach
   */
  void SetSubmenu(std::shared_ptr<Menu> submenu);

  /**
   * @brief Get the submenu attached to this menu item.
   *
   * @return Shared pointer to the submenu, or nullptr if no submenu is attached
   */
  std::shared_ptr<Menu> GetSubmenu() const;

  /**
   * @brief Remove the submenu from this menu item.
   */
  void RemoveSubmenu();

  /**
   * @brief Programmatically trigger this menu item.
   *
   * Simulates a user click on the menu item, firing the appropriate
   * events and callbacks.
   *
   * @return true if the item was successfully triggered, false otherwise
   */
  bool Trigger();

 protected:
  /**
   * @brief Internal method to get the platform-specific native menu item object.
   *
   * This method must be implemented by platform-specific code to return
   * the underlying native menu item object.
   *
   * @return Pointer to the native menu item object
   */
  void* GetNativeObjectInternal() const override;

 private:
  friend class Menu;

  /**
   * @brief Private implementation class using the PIMPL idiom.
   */
  class Impl;

  /**
   * @brief Pointer to the private implementation instance.
   */
  std::unique_ptr<Impl> pimpl_;
};

/**
 * @brief Menu represents a collection of menu items.
 *
 * This class provides a cross-platform interface for creating and managing
 * menus. Menus can contain various types of items including normal items,
 * checkboxes, radio buttons, separators, and submenus.
 *
 * The class supports:
 * - Adding, removing, and organizing menu items
 * - Displaying as context menus or application menus
 * - Event emission for menu interactions
 * - Hierarchical submenu structures
 * - Dynamic menu modification
 *
 * @note This class uses the PIMPL idiom to hide platform-specific
 * implementation details and ensure binary compatibility across different
 * platforms. It also inherits from EventEmitter to provide event-driven
 * interaction handling.
 *
 * @example
 * ```cpp
 * // Create a file menu
 * auto fileMenu = std::make_shared<Menu>();
 *
 * // Add items to the menu
 * auto newItem = std::make_shared<MenuItem>("New", MenuItemType::Normal);
 * newItem->SetAccelerator(KeyboardAccelerator("N", KeyboardAccelerator::Ctrl));
 * fileMenu->AddItem(newItem);
 *
 * auto openItem = std::make_shared<MenuItem>("Open", MenuItemType::Normal);
 * openItem->SetAccelerator(KeyboardAccelerator("O",
 * KeyboardAccelerator::Ctrl)); fileMenu->AddItem(openItem);
 *
 * fileMenu->AddSeparator();
 *
 * auto exitItem = std::make_shared<MenuItem>("Exit", MenuItemType::Normal);
 * fileMenu->AddItem(exitItem);
 *
 * // Listen to menu events
 * fileMenu->AddListener<MenuOpenedEvent>([](const MenuOpenedEvent& event) {
 *     std::cout << "Menu opened" << std::endl;
 * });
 *
 * // Open as context menu
 * fileMenu->Open(100, 100);
 * ```
 */
class Menu : public EventEmitter, public NativeObjectProvider {
 public:
  /**
   * @brief Constructor to create a new menu.
   *
   * Creates an empty menu that can be populated with menu items.
   */
  Menu();

  /**
   * @brief Constructor that wraps an existing platform-specific menu.
   *
   * This constructor is typically used internally by platform-specific
   * implementations to wrap existing menus.
   *
   * @param native_menu Pointer to the platform-specific menu object
   */
  explicit Menu(void* native_menu);

  /**
   * @brief Destructor for Menu.
   *
   * Cleans up the menu and all its items, releasing any associated
   * platform-specific resources.
   */
  virtual ~Menu();

  /**
   * @brief Unique identifier for this menu.
   *
   * This ID is assigned when the menu is created and can be used to
   * reference the menu in various operations.
   */
  MenuId id;

  /**
   * @brief Add a menu item to the end of the menu.
   *
   * The item is added to the bottom of the menu's item list.
   *
   * @param item Shared pointer to the menu item to add
   *
   * @example
   * ```cpp
   * auto item = std::make_shared<MenuItem>("Save");
   * menu->AddItem(item);
   * ```
   */
  void AddItem(std::shared_ptr<MenuItem> item);

  /**
   * @brief Insert a menu item at a specific position.
   *
   * Inserts the item at the specified index, shifting existing items
   * to make room. If the index is out of bounds, the item is added
   * to the end.
   *
   * @param index The position where to insert the item (0-based)
   * @param item Shared pointer to the menu item to insert
   */
  void InsertItem(size_t index, std::shared_ptr<MenuItem> item);

  /**
   * @brief Remove a menu item from the menu.
   *
   * Removes the specified item from the menu if it exists.
   *
   * @param item Shared pointer to the menu item to remove
   * @return true if the item was found and removed, false otherwise
   */
  bool RemoveItem(std::shared_ptr<MenuItem> item);

  /**
   * @brief Remove a menu item by its ID.
   *
   * Removes the menu item with the specified ID from the menu.
   *
   * @param item_id The ID of the menu item to remove
   * @return true if the item was found and removed, false otherwise
   */
  bool RemoveItemById(MenuItemId item_id);

  /**
   * @brief Remove a menu item at a specific position.
   *
   * Removes the menu item at the specified index.
   *
   * @param index The position of the item to remove (0-based)
   * @return true if the item was removed, false if index was out of bounds
   */
  bool RemoveItemAt(size_t index);

  /**
   * @brief Remove all menu items from the menu.
   *
   * Clears the entire menu, removing all items.
   */
  void Clear();

  /**
   * @brief Add a separator line to the menu.
   *
   * Separators are used to visually group related menu items.
   * This is a convenience method equivalent to adding a separator MenuItem.
   */
  void AddSeparator();

  /**
   * @brief Insert a separator at a specific position.
   *
   * @param index The position where to insert the separator (0-based)
   */
  void InsertSeparator(size_t index);

  /**
   * @brief Get the number of items in the menu.
   *
   * @return The total number of menu items (including separators)
   */
  size_t GetItemCount() const;

  /**
   * @brief Get a menu item by its position.
   *
   * Returns the menu item at the specified index.
   *
   * @param index The position of the item to retrieve (0-based)
   * @return Shared pointer to the menu item, or nullptr if index is out of
   * bounds
   */
  std::shared_ptr<MenuItem> GetItemAt(size_t index) const;

  /**
   * @brief Get a menu item by its ID.
   *
   * Searches for and returns the menu item with the specified ID.
   *
   * @param item_id The ID of the menu item to find
   * @return Shared pointer to the menu item, or nullptr if not found
   */
  std::shared_ptr<MenuItem> GetItemById(MenuItemId item_id) const;

  /**
   * @brief Get all menu items in the menu.
   *
   * Returns a vector containing all menu items in order.
   *
   * @return Vector of shared pointers to all menu items
   */
  std::vector<std::shared_ptr<MenuItem>> GetAllItems() const;

  /**
   * @brief Find a menu item by its text.
   *
   * Searches for the first menu item with matching text.
   *
   * @param text The text to search for
   * @param case_sensitive Whether the search should be case-sensitive
   * @return Shared pointer to the menu item, or nullptr if not found
   */
  std::shared_ptr<MenuItem> FindItemByText(const std::string& text,
                                           bool case_sensitive = true) const;

  /**
   * @brief Display the menu as a context menu at the specified screen
   * coordinates.
   *
   * Shows the menu at the given position and waits for user interaction.
   * The menu will close when the user clicks outside of it or selects an item.
   *
   * @param x The x-coordinate in screen coordinates where to open the menu
   * @param y The y-coordinate in screen coordinates where to open the menu
   * @return true if the menu was successfully opened, false otherwise
   *
   * @example
   * ```cpp
   * // Open context menu at cursor position
   * menu->Open(mouse_x, mouse_y);
   * ```
   */
  bool Open(double x, double y);

  /**
   * @brief Display the menu as a context menu at the current cursor position.
   *
   * Shows the menu at the current mouse cursor location.
   *
   * @return true if the menu was successfully opened, false otherwise
   */
  bool Open();

  /**
   * @brief Programmatically close the menu if it's currently showing.
   *
   * @return true if the menu was successfully closed, false otherwise
   */
  bool Close();

  /**
   * @brief Check if the menu is currently visible.
   *
   * @return true if the menu is currently showing, false otherwise
   */
  bool IsVisible() const;

  /**
   * @brief Enable or disable the entire menu.
   *
   * When disabled, all menu items become non-interactive.
   *
   * @param enabled true to enable the menu, false to disable it
   */
  void SetEnabled(bool enabled);

  /**
   * @brief Check if the menu is currently enabled.
   *
   * @return true if the menu is enabled, false if disabled
   */
  bool IsEnabled() const;

  /**
   * @brief Create a standard menu item and add it to the menu.
   *
   * Convenience method that creates a normal menu item with the given
   * text and adds it to the menu in one operation.
   *
   * @param text The text for the menu item
   * @return Shared pointer to the created and added menu item
   */
  std::shared_ptr<MenuItem> CreateAndAddItem(const std::string& text);

  /**
   * @brief Create a menu item with icon and add it to the menu.
   *
   * Convenience method that creates a normal menu item with text and icon
   * and adds it to the menu in one operation.
   *
   * @param text The text for the menu item
   * @param icon The icon for the menu item (file path or base64 data, optional)
   * @return Shared pointer to the created and added menu item
   */
  std::shared_ptr<MenuItem> CreateAndAddItem(const std::string& text,
                                             const std::optional<std::string>& icon);

  /**
   * @brief Create a submenu item and add it to the menu.
   *
   * Convenience method that creates a submenu item and adds it to the menu.
   *
   * @param text The text for the submenu item
   * @param submenu The submenu to attach
   * @return Shared pointer to the created and added submenu item
   */
  std::shared_ptr<MenuItem> CreateAndAddSubmenu(const std::string& text,
                                                std::shared_ptr<Menu> submenu);

  /**
   * @brief Emit a menu opened event.
   *
   * This is primarily used by platform implementations to
   * emit events when a menu has been opened.
   */
  void EmitOpenedEvent();

  /**
   * @brief Emit a menu closed event.
   *
   * This is primarily used by platform implementations to
   * emit events when a menu has been closed.
   */
  void EmitClosedEvent();

 protected:
  /**
   * @brief Internal method to get the platform-specific native menu object.
   *
   * This method must be implemented by platform-specific code to return
   * the underlying native menu object.
   *
   * @return Pointer to the native menu object
   */
  void* GetNativeObjectInternal() const override;

 private:
  /**
   * @brief Private implementation class using the PIMPL idiom.
   */
  class Impl;

  /**
   * @brief Pointer to the private implementation instance.
   */
  std::unique_ptr<Impl> pimpl_;
};

}  // namespace nativeapi
