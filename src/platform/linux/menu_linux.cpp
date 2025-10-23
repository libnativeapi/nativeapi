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
      gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(pimpl_->gtk_menu_item_), active);
    } else if (pimpl_->type_ == MenuItemType::Radio) {
      gboolean active = (state == MenuItemState::Checked) ? TRUE : FALSE;
      gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(pimpl_->gtk_menu_item_), active);
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
      g_signal_connect(G_OBJECT(submenu_widget), "show", G_CALLBACK(OnGtkSubmenuShow), this);
      g_signal_connect(G_OBJECT(submenu_widget), "hide", G_CALLBACK(OnGtkSubmenuHide), this);
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
  if (!IsEnabled())
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

bool Menu::Open(double x, double y) {
  if (pimpl_->gtk_menu_) {
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
                             GDK_GRAVITY_NORTH_WEST, GDK_GRAVITY_NORTH_WEST, nullptr);
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
