#include <gtk/gtk.h>
#include <algorithm>
#include <atomic>
#include <cctype>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include "../../menu.h"

namespace nativeapi {

// Global ID generators
static std::atomic<MenuItemID> g_next_menu_item_id{1};
static std::atomic<MenuID> g_next_menu_id{1};

// Global registry to map native objects to C++ objects for event emission
static std::unordered_map<MenuItemID, MenuItem*> g_menu_item_registry;
static std::unordered_map<MenuID, Menu*> g_menu_registry;

// Private implementation class for MenuItem
class MenuItem::Impl {
 public:
  Impl(GtkWidget* menu_item, MenuItemType type)
      : gtk_menu_item_(menu_item),
        title_(""),
        icon_(""),
        tooltip_(""),
        type_(type),
        enabled_(true),
        visible_(true),
        state_(MenuItemState::Unchecked),
        radio_group_(-1),
        accelerator_("", KeyboardAccelerator::None) {}

  GtkWidget* gtk_menu_item_;
  std::optional<std::string> title_;
  std::optional<std::string> icon_;
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
    : id(g_next_menu_item_id++) {
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

  // Register the MenuItem for event emission
  g_menu_item_registry[id] = this;
}

MenuItem::MenuItem(void* menu_item)
    : id(g_next_menu_item_id++),
      pimpl_(new Impl((GtkWidget*)menu_item, MenuItemType::Normal)) {
  if (pimpl_->gtk_menu_item_ && pimpl_->type_ != MenuItemType::Separator) {
    const char* label = gtk_menu_item_get_label(
        GTK_MENU_ITEM(pimpl_->gtk_menu_item_));
    if (label && label[0] != '\0') {
      pimpl_->title_ = std::string(label);
    } else {
      pimpl_->title_.reset();
    }
  }
  g_menu_item_registry[id] = this;
}

MenuItem::~MenuItem() {
  g_menu_item_registry.erase(id);
}

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

void MenuItem::SetIcon(const std::optional<std::string>& icon) {
  pimpl_->icon_ = icon;
  // TODO: Implement icon setting for GTK menu item
}

std::optional<std::string> MenuItem::GetIcon() const {
  return pimpl_->icon_;
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
    : id(g_next_menu_id++),
      pimpl_(std::unique_ptr<Impl>(new Impl(gtk_menu_new()))) {
  g_menu_registry[id] = this;
}

Menu::Menu(void* menu)
    : id(g_next_menu_id++), pimpl_(new Impl((GtkWidget*)menu)) {
  g_menu_registry[id] = this;
}

Menu::~Menu() {
  // Unregister from event registry
  g_menu_registry.erase(id);
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

bool Menu::RemoveItemById(MenuItemID item_id) {
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

std::shared_ptr<MenuItem> Menu::GetItemById(MenuItemID item_id) const {
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
    gtk_menu_popup_at_pointer(GTK_MENU(pimpl_->gtk_menu_), nullptr);
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

std::shared_ptr<MenuItem> Menu::CreateAndAddItem(const std::string& text) {
  auto item = std::make_shared<MenuItem>(text, MenuItemType::Normal);
  AddItem(item);
  return item;
}

std::shared_ptr<MenuItem> Menu::CreateAndAddItem(
    const std::string& text,
    const std::optional<std::string>& icon) {
  auto item = std::make_shared<MenuItem>(text, MenuItemType::Normal);
  if (icon.has_value()) {
    item->SetIcon(icon);
  }
  AddItem(item);
  return item;
}

std::shared_ptr<MenuItem> Menu::CreateAndAddSubmenu(
    const std::string& text,
    std::shared_ptr<Menu> submenu) {
  auto item = std::make_shared<MenuItem>(text, MenuItemType::Submenu);
  item->SetSubmenu(submenu);
  AddItem(item);
  return item;
}

void* Menu::GetNativeObjectInternal() const {
  return (void*)pimpl_->gtk_menu_;
}

void Menu::EmitOpenedEvent() {
  // TODO: Implement menu opened event emission for GTK
}

void Menu::EmitClosedEvent() {
  // TODO: Implement menu closed event emission for GTK
}

}  // namespace nativeapi