// clang-format off
#include <windows.h>
#include <ole2.h>
#include <shellapi.h>
#include <shlobj.h>
// clang-format on

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "../../drop_target_impl.h"
#include "dpi_utils_windows.h"
#include "drag_drop_utils_windows.h"
#include "string_utils_windows.h"

namespace nativeapi {
namespace {

FORMATETC HGlobalFormat(CLIPFORMAT format) {
  return {format, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
}

bool HasFormat(IDataObject* data, CLIPFORMAT format) {
  FORMATETC etc = HGlobalFormat(format);
  return data && data->QueryGetData(&etc) == S_OK;
}

std::vector<std::string> ReadFilePaths(IDataObject* data) {
  std::vector<std::string> paths;
  FORMATETC etc = HGlobalFormat(CF_HDROP);
  STGMEDIUM medium = {};
  if (!data || FAILED(data->GetData(&etc, &medium))) {
    return paths;
  }
  if (HDROP drop = static_cast<HDROP>(GlobalLock(medium.hGlobal))) {
    const UINT count = DragQueryFileW(drop, 0xFFFFFFFF, nullptr, 0);
    for (UINT i = 0; i < count; ++i) {
      const UINT length = DragQueryFileW(drop, i, nullptr, 0);
      std::wstring path(length + 1, L'\0');
      DragQueryFileW(drop, i, &path[0], length + 1);
      path.resize(length);
      paths.push_back(WStringToString(path));
    }
    GlobalUnlock(medium.hGlobal);
  }
  ReleaseStgMedium(&medium);
  return paths;
}

std::string ReadText(IDataObject* data) {
  std::string text;
  FORMATETC etc = HGlobalFormat(CF_UNICODETEXT);
  STGMEDIUM medium = {};
  if (!data || FAILED(data->GetData(&etc, &medium))) {
    return text;
  }
  if (auto* chars = static_cast<const wchar_t*>(GlobalLock(medium.hGlobal))) {
    // The buffer is not guaranteed to be terminated within its size.
    const size_t capacity = GlobalSize(medium.hGlobal) / sizeof(wchar_t);
    size_t length = 0;
    while (length < capacity && chars[length] != L'\0') ++length;
    text = WStringToString(std::wstring(chars, length));
    GlobalUnlock(medium.hGlobal);
  }
  ReleaseStgMedium(&medium);
  return text;
}

// What the COM object reports to; implemented by DropTarget::Impl::Platform.
class DropTargetComHandler {
 public:
  virtual ~DropTargetComHandler() = default;
  virtual DWORD Entered(IDataObject* data, POINTL point, DWORD allowed) = 0;
  virtual DWORD Moved(POINTL point, DWORD allowed) = 0;
  virtual void Exited() = 0;
  virtual DWORD Dropped(IDataObject* data, POINTL point, DWORD allowed) = 0;
};

class DropTargetCom : public IDropTarget {
 public:
  explicit DropTargetCom(HWND hwnd, DropTargetComHandler* handler)
      : hwnd_(hwnd), handler_(handler) {
    // Draws the shell's drag image (Explorer's file thumbnails) over the window.
    CoCreateInstance(CLSID_DragDropHelper, nullptr, CLSCTX_INPROC_SERVER,
                     IID_PPV_ARGS(&helper_));
  }

  void Detach() { handler_ = nullptr; }

  // IUnknown
  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** object) override {
    if (!object) return E_POINTER;
    if (riid == IID_IUnknown || riid == IID_IDropTarget) {
      *object = static_cast<IDropTarget*>(this);
      AddRef();
      return S_OK;
    }
    *object = nullptr;
    return E_NOINTERFACE;
  }
  ULONG STDMETHODCALLTYPE AddRef() override { return InterlockedIncrement(&references_); }
  ULONG STDMETHODCALLTYPE Release() override {
    const LONG references = InterlockedDecrement(&references_);
    if (references == 0) delete this;
    return references;
  }

  // IDropTarget
  HRESULT STDMETHODCALLTYPE DragEnter(IDataObject* data,
                                      DWORD /*key_state*/,
                                      POINTL point,
                                      DWORD* effect) override {
    const DWORD allowed = *effect;
    *effect = handler_ ? handler_->Entered(data, point, allowed) : DROPEFFECT_NONE;
    if (helper_) {
      POINT pt = {point.x, point.y};
      helper_->DragEnter(hwnd_, data, &pt, *effect);
    }
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE DragOver(DWORD /*key_state*/, POINTL point, DWORD* effect) override {
    const DWORD allowed = *effect;
    *effect = handler_ ? handler_->Moved(point, allowed) : DROPEFFECT_NONE;
    if (helper_) {
      POINT pt = {point.x, point.y};
      helper_->DragOver(&pt, *effect);
    }
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE DragLeave() override {
    if (helper_) helper_->DragLeave();
    if (handler_) handler_->Exited();
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE Drop(IDataObject* data,
                                 DWORD /*key_state*/,
                                 POINTL point,
                                 DWORD* effect) override {
    const DWORD allowed = *effect;
    if (helper_) {
      POINT pt = {point.x, point.y};
      helper_->Drop(data, &pt, *effect);
    }
    *effect = handler_ ? handler_->Dropped(data, point, allowed) : DROPEFFECT_NONE;
    return S_OK;
  }

 private:
  ~DropTargetCom() {
    if (helper_) helper_->Release();
  }

  LONG references_ = 1;
  HWND hwnd_;
  DropTargetComHandler* handler_;
  IDropTargetHelper* helper_ = nullptr;
};

}  // namespace

struct DropTarget::Impl::Platform : DropTargetComHandler {
  explicit Platform(Impl* impl) : impl(impl) {}

  static unsigned SourceOperations(DWORD allowed) {
    unsigned result = 0;
    for (DragOperation operation :
         {DragOperation::Copy, DragOperation::Move, DragOperation::Link}) {
      if (allowed & ToDropEffect(operation)) {
        result |= OperationBit(operation);
      }
    }
    return result;
  }

  // Screen pixels -> logical pixels relative to the client area, which is the
  // content area of a top-level window.
  Point ToContent(POINTL point) const {
    POINT client = {point.x, point.y};
    ScreenToClient(hwnd, &client);
    double scale = GetScaleFactorForWindow(hwnd);
    if (scale <= 0.0) scale = 1.0;
    return {client.x / scale, client.y / scale};
  }

  DWORD Entered(IDataObject* data, POINTL point, DWORD allowed) override {
    const bool has_data = HasFormat(data, CF_HDROP) || HasFormat(data, CF_UNICODETEXT);
    last_position = ToContent(point);
    return ToDropEffect(impl->Entered(last_position, SourceOperations(allowed), has_data));
  }

  DWORD Moved(POINTL point, DWORD allowed) override {
    last_position = ToContent(point);
    return ToDropEffect(impl->Moved(last_position, SourceOperations(allowed)));
  }

  void Exited() override {
    // DragLeave carries no position; report where the drag was last seen.
    impl->Exited(last_position);
  }

  DWORD Dropped(IDataObject* data, POINTL point, DWORD allowed) override {
    last_position = ToContent(point);
    // A source that changed its offer since the last DragOver refuses here too.
    if (impl->Moved(last_position, SourceOperations(allowed)) == DragOperation::None) {
      impl->Exited(last_position);
      return DROPEFFECT_NONE;
    }
    const DWORD effect = ToDropEffect(impl->operation);
    impl->Dropped(last_position, ReadFilePaths(data), ReadText(data));
    return effect;
  }

  Impl* impl;
  HWND hwnd = nullptr;
  DropTargetCom* com = nullptr;
  std::optional<ScopedOleInitialize> ole;
  Point last_position{0, 0};
};

bool DropTarget::IsSupported() {
  return true;
}

DropTarget::Impl::Impl(DropTarget* owner, std::shared_ptr<Window> window)
    : owner(owner),
      window(std::move(window)),
      window_id(this->window ? this->window->GetId() : 0),
      platform(std::make_unique<Platform>(this)) {}

DropTarget::Impl::~Impl() = default;

bool DropTarget::Impl::Register() {
  HWND hwnd = static_cast<HWND>(window->GetNativeObject());
  if (!hwnd || !IsWindow(hwnd)) {
    return false;
  }
  platform->ole.emplace();
  if (!platform->ole->Succeeded()) {
    platform->ole.reset();
    return false;
  }
  auto* com = new DropTargetCom(hwnd, platform.get());
  // OLE finds the target by walking up from the window under the cursor, so
  // registering the top-level window also covers child windows (a Flutter view).
  if (FAILED(RegisterDragDrop(hwnd, com))) {
    com->Detach();
    com->Release();
    platform->ole.reset();
    return false;
  }
  platform->hwnd = hwnd;
  platform->com = com;
  return true;
}

void DropTarget::Impl::Unregister() {
  if (platform->hwnd && IsWindow(platform->hwnd)) {
    RevokeDragDrop(platform->hwnd);
  }
  if (platform->com) {
    platform->com->Detach();
    platform->com->Release();
  }
  platform->com = nullptr;
  platform->hwnd = nullptr;
  platform->ole.reset();
}

}  // namespace nativeapi
