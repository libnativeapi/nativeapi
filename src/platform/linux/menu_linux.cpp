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
  std::string text = "";
  if (auto label = menu_item->GetLabel(); label.has_value()) {
    text = *label;
  }
  menu_item->EmitSync(MenuItemClickedEvent(menu_item->id, text));
}

static void OnGtkMenuShow(GtkWidget* /*menu*/, gpointer user_data) {
  Menu* menu_obj = static_cast<Menu*>(user_data);
  if (!menu_obj) {
    return;
  }
  menu_obj->EmitSync(MenuOpenedEvent(menu_obj->id));
}

static void OnGtkMenuHide(GtkWidget* /*menu*/, gpointer user_data) {
  Menu* menu_obj = static_cast<Menu*>(user_data);
  if (!menu_obj) {
    return;
  }
  menu_obj->EmitSync(MenuClosedEvent(menu_obj->id));
}

static void OnGtkSubmenuShow(GtkWidget* /*submenu*/, gpointer user_data) {
  MenuItem* menu_item = static_cast<MenuItem*>(user_data);
  if (!menu_item) {
    return;
  }
  // Emit submenu opened on the item
  menu_item->EmitSync(MenuItemSubmenuOpenedEvent(menu_item->id));
}

static void OnGtkSubmenuHide(GtkWidget* /*submenu*/, gpointer user_data) {
  MenuItem* menu_item = static_cast<MenuItem*>(user_data);
  if (!menu_item) {
    return;
  }
  // Emit submenu closed on the item
  menu_item->EmitSync(MenuItemSubmenuClosedEvent(menu_item->id));
}

// Private implementation class for MenuItem
class MenuItem::Impl {
 public:
  Impl(GtkWidget* menu_item, MenuItemType type)
      : gtk_menu_item_(menu_item),
        title_(""),
        tooltip_(""),
        type_(type),
        enabled_(true),
        visible_(true),
        state_(MenuItemState::Unchecked),
        radio_group_(-1),
        accelerator_("", KeyboardAccelerator::None) {}

  GtkWidget* gtk_menu_item_;
  std::optional<std::string> title_;
  std::shared_ptr<Image> image_;
  std::optional<std::string> tooltip_;
  MenuItemType type_;
  bool enabled_;
  bool visible_;
  MenuItemState state_;
  int radio_group_;
  KeyboardAccelerator accelerator_;
  std::shared_ptr<Menu> submenu_;
};

MenuItem::MenuItem(const std::string& text, MenuItemType type)
    : id(IdAllocator::Allocate<MenuItem>()) {
  GtkWidget* gtk_item = nullptr;

  switch (type) {
    case MenuItemType::Separator:
      gtk_item = gtk_separator_menu_item_new();
      break;
    case MenuItemType::Checkbox:
      gtk_item = gtk_check_menu_item_new_with_label(text.c_str());
      break;
    case MenuItemType::Radio:
      gtk_item = gtk_radio_menu_item_new_with_label(nullptr, text.c_str());
      break;
    case MenuItemType::Normal:
    case MenuItemType::Submenu:
    default:
      gtk_item = gtk_menu_item_new_with_label(text.c_str());
      break;
  }

  pimpl_ = std::unique_ptr<Impl>(new Impl(gtk_item, type));

  if (!text.empty()) {
    pimpl_->title_ = text;
  } else {
    pimpl_->title_.reset();
  }

  // Connect activation signal for click events (except separators)
  if (gtk_item && type != MenuItemType::Separator) {
    g_signal_connect(G_OBJECT(gtk_item), "activate",
                     G_CALLBACK(OnGtkMenuItemActivate), this);
  }
}

MenuItem::MenuItem(void* menu_item)
    : id(IdAllocator::Allocate<MenuItem>()),
      pimpl_(new Impl((GtkWidget*)menu_item, MenuItemType::Normal)) {
  if (pimpl_->gtk_menu_item_ && pimpl_->type_ != MenuItemType::Separator) {
    const char* label =
        gtk_menu_item_get_label(GTK_MENU_ITEM(pimpl_->gtk_menu_item_));
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

MenuItemType MenuItem::GetType() const {
  return pimpl_->type_;
}

void MenuItem::SetLabel(const std::optional<std::string>& label) {
  pimpl_->title_ = label;
  if (pimpl_->gtk_menu_item_ && pimpl_->type_ != MenuItemType::Separator) {
    const char* labelStr = label.has_value() ? label->c_str() : "";
    gtk_menu_item_set_label(GTK_MENU_ITEM(pimpl_->gtk_menu_item_), labelStr);
  }
}

std::optional<std::string> MenuItem::GetLabel() const {
  return pimpl_->title_;
}

void MenuItem::SetIcon(std::shared_ptr<Image> image) {
  pimpl_->image_ = image;
  // TODO: Implement icon setting for GTK menu item
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

void MenuItem::SetAccelerator(const KeyboardAccelerator& accelerator) {
  pimpl_->accelerator_ = accelerator;
  // TODO: Implement GTK accelerator setting
}

KeyboardAccelerator MenuItem::GetAccelerator() const {
  return pimpl_->accelerator_;
}

void MenuItem::RemoveAccelerator() {
  pimpl_->accelerator_ = KeyboardAccelerator("", KeyboardAccelerator::None);
  // TODO: Implement GTK accelerator removal
}

void MenuItem::SetEnabled(bool enabled) {
  pimpl_->enabled_ = enabled;
  if (pimpl_->gtk_menu_item_) {
    gtk_widget_set_sensitive(pimpl_->gtk_menu_item_, enabled ? TRUE : FALSE);
  }
}

bool MenuItem::IsEnabled() const {
  return pimpl_->enabled_;
}

void MenuItem::SetVisible(bool visible) {
  pimpl_->visible_ = visible;
  if (pimpl_->gtk_menu_item_) {
    gtk_widget_set_visible(pimpl_->gtk_menu_item_, visible ? TRUE : FALSE);
  }
}

bool MenuItem::IsVisible() const {
  return pimpl_->visible_;
}

void MenuItem::SetState(MenuItemState state) {
  pimpl_->state_ = state;
  if (pimpl_->gtk_menu_item_) {
    if (pimpl_->type_ == MenuItemType::Checkbox) {
      gboolean active = (state == MenuItemState::Checked) ? TRUE : FALSE;
      gtk_check_menu_item_set_active(
          GTK_CHECK_MENU_ITEM(pimpl_->gtk_menu_item_), active);
    } else if (pimpl_->type_ == MenuItemType::Radio) {
      gboolean active = (state == MenuItemState::Checked) ? TRUE : FALSE;
      gtk_check_menu_item_set_active(
          GTK_CHECK_MENU_ITEM(pimpl_->gtk_menu_item_), active);
    }
  }
}

MenuItemState MenuItem::GetState() const {
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
      g_signal_connect(G_OBJECT(submenu_widget), "show",
                       G_CALLBACK(OnGtkSubmenuShow), this);
      g_signal_connect(G_OBJECT(submenu_widget), "hide",
                       G_CALLBACK(OnGtkSubmenuHide), this);
    }
  }
}

std::shared_ptr<Menu> MenuItem::GetSubmenu() const {
  return pimpl_->submenu_;
}

void MenuItem::RemoveSubmenu() {
  pimpl_->submenu_.reset();
  if (pimpl_->gtk_menu_item_) {
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(pimpl_->gtk_menu_item_), nullptr);
  }
}

bool MenuItem::Trigger() {
  if (!pimpl_->enabled_)
    return false;
  if (pimpl_->gtk_menu_item_) {
    g_signal_emit_by_name(pimpl_->gtk_menu_item_, "activate");
    return true;
  }
  return false;
}

void* MenuItem::GetNativeObjectInternal() const {
  return (void*)pimpl_->gtk_menu_item_;
}

// Private implementation class for Menu
class Menu::Impl {
 public:
  Impl(GtkWidget* menu) : gtk_menu_(menu), enabled_(true), visible_(false) {}

  GtkWidget* gtk_menu_;
  std::vector<std::shared_ptr<MenuItem>> items_;
  bool enabled_;
  bool visible_;
};

Menu::Menu()
    : id(IdAllocator::Allocate<Menu>()),
      pimpl_(std::unique_ptr<Impl>(new Impl(gtk_menu_new()))) {
  // Connect menu show/hide to emit open/close events
  if (pimpl_->gtk_menu_) {
    g_signal_connect(G_OBJECT(pimpl_->gtk_menu_), "show",
                     G_CALLBACK(OnGtkMenuShow), this);
    g_signal_connect(G_OBJECT(pimpl_->gtk_menu_), "hide",
                     G_CALLBACK(OnGtkMenuHide), this);
  }
}

Menu::Menu(void* menu)
    : id(IdAllocator::Allocate<Menu>()), pimpl_(new Impl((GtkWidget*)menu)) {
  if (pimpl_->gtk_menu_) {
    g_signal_connect(G_OBJECT(pimpl_->gtk_menu_), "show",
                     G_CALLBACK(OnGtkMenuShow), this);
    g_signal_connect(G_OBJECT(pimpl_->gtk_menu_), "hide",
                     G_CALLBACK(OnGtkMenuHide), this);
  }
}

Menu::~Menu() {
  if (pimpl_->gtk_menu_) {
    g_object_unref(pimpl_->gtk_menu_);
  }
}

void Menu::AddItem(std::shared_ptr<MenuItem> item) {
  if (pimpl_->gtk_menu_ && item && item->GetNativeObject()) {
    pimpl_->items_.push_back(item);
    gtk_menu_shell_append(GTK_MENU_SHELL(pimpl_->gtk_menu_),
                          (GtkWidget*)item->GetNativeObject());
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
    gtk_menu_shell_insert(GTK_MENU_SHELL(pimpl_->gtk_menu_),
                          (GtkWidget*)item->GetNativeObject(), index);
  }
}

bool Menu::RemoveItem(std::shared_ptr<MenuItem> item) {
  if (pimpl_->gtk_menu_ && item && item->GetNativeObject()) {
    auto it = std::find(pimpl_->items_.begin(), pimpl_->items_.end(), item);
    if (it != pimpl_->items_.end()) {
      pimpl_->items_.erase(it);
      gtk_container_remove(GTK_CONTAINER(pimpl_->gtk_menu_),
                           (GtkWidget*)item->GetNativeObject());
      return true;
    }
  }
  return false;
}

bool Menu::RemoveItemById(MenuItemId item_id) {
  for (auto& item : pimpl_->items_) {
    if (item->id == item_id) {
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
    if (item->id == item_id) {
      return item;
    }
  }
  return nullptr;
}

std::vector<std::shared_ptr<MenuItem>> Menu::GetAllItems() const {
  return pimpl_->items_;
}

std::shared_ptr<MenuItem> Menu::FindItemByText(const std::string& text,
                                               bool case_sensitive) const {
  for (const auto& item : pimpl_->items_) {
    auto itemTextOpt = item->GetLabel();
    if (!itemTextOpt.has_value()) {
      continue;
    }

    const std::string& item_text = itemTextOpt.value();
    if (case_sensitive) {
      if (item_text == text) {
        return item;
      }
    } else {
      std::string lower_item_text = item_text;
      std::string lower_search_text = text;
      std::transform(lower_item_text.begin(), lower_item_text.end(),
                     lower_item_text.begin(), ::tolower);
      std::transform(lower_search_text.begin(), lower_search_text.end(),
                     lower_search_text.begin(), ::tolower);
      if (lower_item_text == lower_search_text) {
        return item;
      }
    }
  }
  return nullptr;
}

bool Menu::Open(double x, double y) {
  if (pimpl_->gtk_menu_) {
    pimpl_->visible_ = true;
    gtk_widget_show_all(pimpl_->gtk_menu_);

    // Try to position at explicit coordinates if available
    GdkWindow* root_window = gdk_get_default_root_window();
    if (root_window) {
      GdkRectangle rect;
      rect.x = static_cast<int>(x);
      rect.y = static_cast<int>(y);
      rect.width = 1;
      rect.height = 1;
      gtk_menu_popup_at_rect(GTK_MENU(pimpl_->gtk_menu_), root_window, &rect,
                             GDK_GRAVITY_NORTH_WEST, GDK_GRAVITY_NORTH_WEST,
                             nullptr);
    } else {
      // Fallback to pointer if root window not available
      gtk_menu_popup_at_pointer(GTK_MENU(pimpl_->gtk_menu_), nullptr);
    }
    return true;
  }
  return false;
}

bool Menu::Open() {
  return Open(0, 0);  // GTK will position at pointer
}

bool Menu::Close() {
  if (pimpl_->gtk_menu_ && pimpl_->visible_) {
    gtk_menu_popdown(GTK_MENU(pimpl_->gtk_menu_));
    pimpl_->visible_ = false;
    return true;
  }
  return false;
}

bool Menu::IsVisible() const {
  return pimpl_->visible_;
}

void Menu::SetEnabled(bool enabled) {
  pimpl_->enabled_ = enabled;
  if (pimpl_->gtk_menu_) {
    gtk_widget_set_sensitive(pimpl_->gtk_menu_, enabled ? TRUE : FALSE);
  }
}

bool Menu::IsEnabled() const {
  return pimpl_->enabled_;
}

void* Menu::GetNativeObjectInternal() const {
  return (void*)pimpl_->gtk_menu_;
}

}  // namespace nativeapi
