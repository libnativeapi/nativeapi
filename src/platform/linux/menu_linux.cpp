#include <gtk/gtk.h>
#include <algorithm>
#include <cctype>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include "../../foundation/id_allocator.h"
#include "../../image.h"
#include "../../menu.h"
#include "../../menu_event.h"

namespace nativeapi {

// GTK signal handlers → Event emission
static void OnGtkMenuItemActivate(GtkMenuItem* /*item*/, gpointer user_data) {
  MenuItem* menu_item = static_cast<MenuItem*>(user_data);
  if (!menu_item) {
    return;
  }
  menu_item->Emit(MenuItemClickedEvent(menu_item->GetId()));
}

static void OnGtkMenuShow(GtkWidget* /*menu*/, gpointer user_data) {
  Menu* menu_obj = static_cast<Menu*>(user_data);
  if (!menu_obj) {
    return;
  }
  menu_obj->Emit(MenuOpenedEvent(menu_obj->GetId()));
}

static void OnGtkMenuHide(GtkWidget* /*menu*/, gpointer user_data) {
  Menu* menu_obj = static_cast<Menu*>(user_data);
  if (!menu_obj) {
    return;
  }
  menu_obj->Emit(MenuClosedEvent(menu_obj->GetId()));
}

static void OnGtkSubmenuShow(GtkWidget* /*submenu*/, gpointer user_data) {
  MenuItem* menu_item = static_cast<MenuItem*>(user_data);
  if (!menu_item) {
    return;
  }
  // Emit submenu opened on the item
  menu_item->Emit(MenuItemSubmenuOpenedEvent(menu_item->GetId()));
}

static void OnGtkSubmenuHide(GtkWidget* /*submenu*/, gpointer user_data) {
  MenuItem* menu_item = static_cast<MenuItem*>(user_data);
  if (!menu_item) {
    return;
  }
  // Emit submenu closed on the item
  menu_item->Emit(MenuItemSubmenuClosedEvent(menu_item->GetId()));
}

// Private implementation class for MenuItem
class MenuItem::Impl {
 public:
  Impl(MenuItemId id, GtkWidget* menu_item, MenuItemType type)
      : id_(id),
        gtk_menu_item_(menu_item),
        title_(""),
        tooltip_(""),
        type_(type),
        state_(MenuItemState::Unchecked),
        radio_group_(-1),
        accelerator_("", KeyboardAccelerator::None) {}

  MenuItemId id_;
  GtkWidget* gtk_menu_item_;
  std::optional<std::string> title_;
  std::shared_ptr<Image> image_;
  std::optional<std::string> tooltip_;
  MenuItemType type_;
  MenuItemState state_;
  int radio_group_;
  KeyboardAccelerator accelerator_;
  std::shared_ptr<Menu> submenu_;
};

MenuItem::MenuItem(const std::string& label, MenuItemType type) {
  MenuItemId id = IdAllocator::Allocate<MenuItem>();
  GtkWidget* gtk_item = nullptr;

  switch (type) {
    case MenuItemType::Separator:
      gtk_item = gtk_separator_menu_item_new();
      break;
    case MenuItemType::Checkbox:
      gtk_item = gtk_check_menu_item_new_with_label(label.c_str());
      break;
    case MenuItemType::Radio:
      gtk_item = gtk_radio_menu_item_new_with_label(nullptr, label.c_str());
      break;
    case MenuItemType::Normal:
    case MenuItemType::Submenu:
    default:
      gtk_item = gtk_menu_item_new_with_label(label.c_str());
      break;
  }

  pimpl_ = std::unique_ptr<Impl>(new Impl(id, gtk_item, type));

  if (!label.empty()) {
    pimpl_->title_ = label;
  } else {
    pimpl_->title_.reset();
  }

  // Connect activation signal for click events (except separators)
  if (gtk_item && type != MenuItemType::Separator) {
    g_signal_connect(G_OBJECT(gtk_item), "activate", G_CALLBACK(OnGtkMenuItemActivate), this);
  }
}

MenuItem::MenuItem(void* menu_item) {
  MenuItemId id = IdAllocator::Allocate<MenuItem>();
  pimpl_ = std::unique_ptr<Impl>(new Impl(id, (GtkWidget*)menu_item, MenuItemType::Normal));
  if (pimpl_->gtk_menu_item_ && pimpl_->type_ != MenuItemType::Separator) {
    const char* label = gtk_menu_item_get_label(GTK_MENU_ITEM(pimpl_->gtk_menu_item_));
    if (label && label[0] != '\0') {
      pimpl_->title_ = std::string(label);
    } else {
      pimpl_->title_.reset();
    }
  }

  if (pimpl_->gtk_menu_item_) {
    g_signal_connect(G_OBJECT(pimpl_->gtk_menu_item_), "activate",
                     G_CALLBACK(OnGtkMenuItemActivate), this);
  }
}

MenuItem::~MenuItem() {}

MenuItemId MenuItem::GetId() const {
  return pimpl_->id_;
}

MenuItemType MenuItem::GetType() const {
  return pimpl_->type_;
}

void MenuItem::SetLabel(const std::optional<std::string>& label) {
  pimpl_->title_ = label;
  if (pimpl_->gtk_menu_item_ && pimpl_->type_ != MenuItemType::Separator) {
    const char* labelStr = label.has_value() ? label->c_str() : "";
    
    // Check if we have a custom box layout (with icon)
    GtkWidget* child = gtk_bin_get_child(GTK_BIN(pimpl_->gtk_menu_item_));
    if (child && GTK_IS_BOX(child)) {
      // Custom layout with icon - find the label widget and update it
      GList* children = gtk_container_get_children(GTK_CONTAINER(child));
      for (GList* iter = children; iter != nullptr; iter = iter->next) {
        GtkWidget* widget = GTK_WIDGET(iter->data);
        if (GTK_IS_LABEL(widget)) {
          gtk_label_set_text(GTK_LABEL(widget), labelStr);
          break;
        }
      }
      g_list_free(children);
    } else {
      // Simple label-only layout
      gtk_menu_item_set_label(GTK_MENU_ITEM(pimpl_->gtk_menu_item_), labelStr);
    }
  }
}

std::optional<std::string> MenuItem::GetLabel() const {
  return pimpl_->title_;
}

void MenuItem::SetIcon(std::shared_ptr<Image> image) {
  pimpl_->image_ = image;
  
  if (!pimpl_->gtk_menu_item_ || pimpl_->type_ == MenuItemType::Separator) {
    return;
  }

  // Get current label text to preserve it
  const char* label_text = gtk_menu_item_get_label(GTK_MENU_ITEM(pimpl_->gtk_menu_item_));
  std::string current_label = label_text ? label_text : "";

  // Remove existing child widget
  GtkWidget* existing_child = gtk_bin_get_child(GTK_BIN(pimpl_->gtk_menu_item_));
  if (existing_child) {
    gtk_container_remove(GTK_CONTAINER(pimpl_->gtk_menu_item_), existing_child);
  }

  if (image && image->GetNativeObject()) {
    // Create a horizontal box to hold icon and label
    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);  // 6px spacing
    
    // Get the GdkPixbuf from the image
    GdkPixbuf* pixbuf = static_cast<GdkPixbuf*>(image->GetNativeObject());
    
    // Scale the icon to a reasonable menu size (16x16 is standard for menu items)
    const int icon_size = 16;
    GdkPixbuf* scaled_pixbuf = nullptr;
    
    int original_width = gdk_pixbuf_get_width(pixbuf);
    int original_height = gdk_pixbuf_get_height(pixbuf);
    
    if (original_width != icon_size || original_height != icon_size) {
      scaled_pixbuf = gdk_pixbuf_scale_simple(pixbuf, icon_size, icon_size, GDK_INTERP_BILINEAR);
    } else {
      scaled_pixbuf = gdk_pixbuf_copy(pixbuf);
    }
    
    // Create GtkImage from the pixbuf
    GtkWidget* gtk_image = gtk_image_new_from_pixbuf(scaled_pixbuf);
    g_object_unref(scaled_pixbuf);  // GtkImage takes its own reference
    
    // Create label widget
    GtkWidget* label = gtk_label_new(current_label.c_str());
    gtk_label_set_xalign(GTK_LABEL(label), 0.0);  // Left-align the label
    
    // Pack icon and label into box
    gtk_box_pack_start(GTK_BOX(box), gtk_image, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), label, TRUE, TRUE, 0);
    
    // Add box to menu item
    gtk_container_add(GTK_CONTAINER(pimpl_->gtk_menu_item_), box);
    gtk_widget_show_all(box);
  } else {
    // No icon - restore simple label display
    GtkWidget* label = gtk_label_new(current_label.c_str());
    gtk_label_set_xalign(GTK_LABEL(label), 0.0);
    gtk_container_add(GTK_CONTAINER(pimpl_->gtk_menu_item_), label);
    gtk_widget_show(label);
  }
}

std::shared_ptr<Image> MenuItem::GetIcon() const {
  return pimpl_->image_;
}

void MenuItem::SetTooltip(const std::optional<std::string>& tooltip) {
  pimpl_->tooltip_ = tooltip;
  if (pimpl_->gtk_menu_item_) {
    if (tooltip.has_value()) {
      gtk_widget_set_tooltip_text(pimpl_->gtk_menu_item_, tooltip->c_str());
    } else {
      gtk_widget_set_tooltip_text(pimpl_->gtk_menu_item_, nullptr);
    }
  }
}

std::optional<std::string> MenuItem::GetTooltip() const {
  return pimpl_->tooltip_;
}

void MenuItem::SetAccelerator(const std::optional<KeyboardAccelerator>& accelerator) {
  if (accelerator.has_value()) {
    pimpl_->accelerator_ = *accelerator;
  } else {
    pimpl_->accelerator_ = KeyboardAccelerator("", KeyboardAccelerator::None);
  }
  // TODO: Implement GTK accelerator setting
}

KeyboardAccelerator MenuItem::GetAccelerator() const {
  return pimpl_->accelerator_;
}

void MenuItem::SetEnabled(bool enabled) {
  if (pimpl_->gtk_menu_item_) {
    gtk_widget_set_sensitive(pimpl_->gtk_menu_item_, enabled ? TRUE : FALSE);
  }
}

bool MenuItem::IsEnabled() const {
  if (pimpl_->gtk_menu_item_) {
    return gtk_widget_get_sensitive(pimpl_->gtk_menu_item_) == TRUE;
  }
  return true;
}

void MenuItem::SetState(MenuItemState state) {
  pimpl_->state_ = state;
  if (pimpl_->gtk_menu_item_) {
    if (pimpl_->type_ == MenuItemType::Checkbox) {
      gboolean active = (state == MenuItemState::Checked) ? TRUE : FALSE;
      // Block the "activate" signal to prevent recursive triggering
      g_signal_handlers_block_by_func(G_OBJECT(pimpl_->gtk_menu_item_), 
                                      (gpointer)OnGtkMenuItemActivate, this);
      gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(pimpl_->gtk_menu_item_), active);
      g_signal_handlers_unblock_by_func(G_OBJECT(pimpl_->gtk_menu_item_), 
                                        (gpointer)OnGtkMenuItemActivate, this);
    } else if (pimpl_->type_ == MenuItemType::Radio) {
      gboolean active = (state == MenuItemState::Checked) ? TRUE : FALSE;
      // Block the "activate" signal to prevent recursive triggering
      g_signal_handlers_block_by_func(G_OBJECT(pimpl_->gtk_menu_item_), 
                                      (gpointer)OnGtkMenuItemActivate, this);
      gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(pimpl_->gtk_menu_item_), active);
      g_signal_handlers_unblock_by_func(G_OBJECT(pimpl_->gtk_menu_item_), 
                                        (gpointer)OnGtkMenuItemActivate, this);
    }
  }
}

MenuItemState MenuItem::GetState() const {
  // For checkbox and radio items, get the actual state from GTK widget
  if (pimpl_->gtk_menu_item_ && 
      (pimpl_->type_ == MenuItemType::Checkbox || pimpl_->type_ == MenuItemType::Radio)) {
    gboolean active = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(pimpl_->gtk_menu_item_));
    return active ? MenuItemState::Checked : MenuItemState::Unchecked;
  }
  return pimpl_->state_;
}

void MenuItem::SetRadioGroup(int group_id) {
  pimpl_->radio_group_ = group_id;
  // TODO: Implement proper radio grouping for GTK
}

int MenuItem::GetRadioGroup() const {
  return pimpl_->radio_group_;
}

void MenuItem::SetSubmenu(std::shared_ptr<Menu> submenu) {
  pimpl_->submenu_ = submenu;
  if (pimpl_->gtk_menu_item_ && submenu) {
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(pimpl_->gtk_menu_item_),
                              (GtkWidget*)submenu->GetNativeObject());

    // Emit submenu open/close events on the parent item when submenu
    // shows/hides
    GtkWidget* submenu_widget = (GtkWidget*)submenu->GetNativeObject();
    if (submenu_widget) {
      g_signal_connect(G_OBJECT(submenu_widget), "show", G_CALLBACK(OnGtkSubmenuShow), this);
      g_signal_connect(G_OBJECT(submenu_widget), "hide", G_CALLBACK(OnGtkSubmenuHide), this);
    }
  }
}

std::shared_ptr<Menu> MenuItem::GetSubmenu() const {
  return pimpl_->submenu_;
}

void* MenuItem::GetNativeObjectInternal() const {
  return (void*)pimpl_->gtk_menu_item_;
}

// Private implementation class for Menu
class Menu::Impl {
 public:
  Impl(MenuId id, GtkWidget* menu) : id_(id), gtk_menu_(menu) {}

  MenuId id_;
  GtkWidget* gtk_menu_;
  std::vector<std::shared_ptr<MenuItem>> items_;
};

Menu::Menu() {
  MenuId id = IdAllocator::Allocate<Menu>();
  pimpl_ = std::unique_ptr<Impl>(new Impl(id, gtk_menu_new()));
  // Connect menu show/hide to emit open/close events
  if (pimpl_->gtk_menu_) {
    g_signal_connect(G_OBJECT(pimpl_->gtk_menu_), "show", G_CALLBACK(OnGtkMenuShow), this);
    g_signal_connect(G_OBJECT(pimpl_->gtk_menu_), "hide", G_CALLBACK(OnGtkMenuHide), this);
  }
}

Menu::Menu(void* menu) {
  MenuId id = IdAllocator::Allocate<Menu>();
  pimpl_ = std::unique_ptr<Impl>(new Impl(id, (GtkWidget*)menu));
  if (pimpl_->gtk_menu_) {
    g_signal_connect(G_OBJECT(pimpl_->gtk_menu_), "show", G_CALLBACK(OnGtkMenuShow), this);
    g_signal_connect(G_OBJECT(pimpl_->gtk_menu_), "hide", G_CALLBACK(OnGtkMenuHide), this);
  }
}

Menu::~Menu() {
  if (pimpl_->gtk_menu_) {
    g_object_unref(pimpl_->gtk_menu_);
  }
}

MenuId Menu::GetId() const {
  return pimpl_->id_;
}

void Menu::AddItem(std::shared_ptr<MenuItem> item) {
  if (pimpl_->gtk_menu_ && item && item->GetNativeObject()) {
    pimpl_->items_.push_back(item);
    gtk_menu_shell_append(GTK_MENU_SHELL(pimpl_->gtk_menu_), (GtkWidget*)item->GetNativeObject());
  }
}

void Menu::InsertItem(size_t index, std::shared_ptr<MenuItem> item) {
  if (!item)
    return;

  if (index >= pimpl_->items_.size()) {
    AddItem(item);
    return;
  }

  pimpl_->items_.insert(pimpl_->items_.begin() + index, item);
  if (pimpl_->gtk_menu_ && item->GetNativeObject()) {
    gtk_menu_shell_insert(GTK_MENU_SHELL(pimpl_->gtk_menu_), (GtkWidget*)item->GetNativeObject(),
                          index);
  }
}

bool Menu::RemoveItem(std::shared_ptr<MenuItem> item) {
  if (pimpl_->gtk_menu_ && item && item->GetNativeObject()) {
    auto it = std::find(pimpl_->items_.begin(), pimpl_->items_.end(), item);
    if (it != pimpl_->items_.end()) {
      pimpl_->items_.erase(it);
      gtk_container_remove(GTK_CONTAINER(pimpl_->gtk_menu_), (GtkWidget*)item->GetNativeObject());
      return true;
    }
  }
  return false;
}

bool Menu::RemoveItemById(MenuItemId item_id) {
  for (auto& item : pimpl_->items_) {
    if (item->GetId() == item_id) {
      return RemoveItem(item);
    }
  }
  return false;
}

bool Menu::RemoveItemAt(size_t index) {
  if (index < pimpl_->items_.size()) {
    auto item = pimpl_->items_[index];
    return RemoveItem(item);
  }
  return false;
}

void Menu::Clear() {
  while (!pimpl_->items_.empty()) {
    RemoveItem(pimpl_->items_.back());
  }
}

void Menu::AddSeparator() {
  auto separator = std::make_shared<MenuItem>("", MenuItemType::Separator);
  AddItem(separator);
}

void Menu::InsertSeparator(size_t index) {
  auto separator = std::make_shared<MenuItem>("", MenuItemType::Separator);
  InsertItem(index, separator);
}

size_t Menu::GetItemCount() const {
  return pimpl_->items_.size();
}

std::shared_ptr<MenuItem> Menu::GetItemAt(size_t index) const {
  if (index < pimpl_->items_.size()) {
    return pimpl_->items_[index];
  }
  return nullptr;
}

std::shared_ptr<MenuItem> Menu::GetItemById(MenuItemId item_id) const {
  for (const auto& item : pimpl_->items_) {
    if (item->GetId() == item_id) {
      return item;
    }
  }
  return nullptr;
}

std::vector<std::shared_ptr<MenuItem>> Menu::GetAllItems() const {
  return pimpl_->items_;
}

bool Menu::Open(const PositioningStrategy& strategy, Placement placement) {
  if (pimpl_->gtk_menu_) {
    gtk_widget_show_all(pimpl_->gtk_menu_);

    double x = 0, y = 0;
    bool use_explicit_position = false;

    // Determine position based on strategy type
    switch (strategy.GetType()) {
      case PositioningStrategy::Type::Absolute:
        x = strategy.GetAbsolutePosition().x;
        y = strategy.GetAbsolutePosition().y;
        use_explicit_position = true;
        break;

      case PositioningStrategy::Type::CursorPosition:
        // Will use gtk_menu_popup_at_pointer
        use_explicit_position = false;
        break;

      case PositioningStrategy::Type::Relative: {
        Rectangle rect = strategy.GetRelativeRectangle();
        Point offset = strategy.GetRelativeOffset();
        // Position at top-left corner of rectangle plus offset
        x = rect.x + offset.x;
        y = rect.y + offset.y;
        use_explicit_position = true;
        break;
      }
    }

    // Map placement to GDK gravity values
    GdkGravity anchor_gravity = GDK_GRAVITY_NORTH_WEST;
    GdkGravity menu_gravity = GDK_GRAVITY_NORTH_WEST;

    switch (placement) {
      case Placement::Top:
        anchor_gravity = GDK_GRAVITY_SOUTH;
        menu_gravity = GDK_GRAVITY_NORTH;
        break;
      case Placement::TopStart:
        anchor_gravity = GDK_GRAVITY_SOUTH_WEST;
        menu_gravity = GDK_GRAVITY_NORTH_WEST;
        break;
      case Placement::TopEnd:
        anchor_gravity = GDK_GRAVITY_SOUTH_EAST;
        menu_gravity = GDK_GRAVITY_NORTH_EAST;
        break;
      case Placement::Right:
        anchor_gravity = GDK_GRAVITY_WEST;
        menu_gravity = GDK_GRAVITY_EAST;
        break;
      case Placement::RightStart:
        anchor_gravity = GDK_GRAVITY_NORTH_WEST;
        menu_gravity = GDK_GRAVITY_NORTH_EAST;
        break;
      case Placement::RightEnd:
        anchor_gravity = GDK_GRAVITY_SOUTH_WEST;
        menu_gravity = GDK_GRAVITY_SOUTH_EAST;
        break;
      case Placement::Bottom:
        anchor_gravity = GDK_GRAVITY_NORTH;
        menu_gravity = GDK_GRAVITY_SOUTH;
        break;
      case Placement::BottomStart:
        anchor_gravity = GDK_GRAVITY_NORTH_WEST;
        menu_gravity = GDK_GRAVITY_SOUTH_WEST;
        break;
      case Placement::BottomEnd:
        anchor_gravity = GDK_GRAVITY_NORTH_EAST;
        menu_gravity = GDK_GRAVITY_SOUTH_EAST;
        break;
      case Placement::Left:
        anchor_gravity = GDK_GRAVITY_EAST;
        menu_gravity = GDK_GRAVITY_WEST;
        break;
      case Placement::LeftStart:
        anchor_gravity = GDK_GRAVITY_NORTH_EAST;
        menu_gravity = GDK_GRAVITY_NORTH_WEST;
        break;
      case Placement::LeftEnd:
        anchor_gravity = GDK_GRAVITY_SOUTH_EAST;
        menu_gravity = GDK_GRAVITY_SOUTH_WEST;
        break;
    }

    // Try to position at explicit coordinates if available
    if (use_explicit_position) {
      GdkWindow* root_window = gdk_get_default_root_window();
      if (root_window) {
        GdkRectangle rect;
        rect.x = static_cast<int>(x);
        rect.y = static_cast<int>(y);
        rect.width = 1;
        rect.height = 1;
        gtk_menu_popup_at_rect(GTK_MENU(pimpl_->gtk_menu_), root_window, &rect,
                               anchor_gravity, menu_gravity, nullptr);
      } else {
        // Fallback to pointer if root window not available
        gtk_menu_popup_at_pointer(GTK_MENU(pimpl_->gtk_menu_), nullptr);
      }
    } else {
      // Use pointer position
      gtk_menu_popup_at_pointer(GTK_MENU(pimpl_->gtk_menu_), nullptr);
    }
    return true;
  }
  return false;
}

bool Menu::Close() {
  if (pimpl_->gtk_menu_) {
    gtk_menu_popdown(GTK_MENU(pimpl_->gtk_menu_));
    return true;
  }
  return false;
}

void* Menu::GetNativeObjectInternal() const {
  return (void*)pimpl_->gtk_menu_;
}

}  // namespace nativeapi
