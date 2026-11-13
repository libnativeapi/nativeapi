// Read-only compositor observation, scoped to a visible client-area rectangle.
// Usage: frame_probe "exact window title" output.jsonl seconds
// No screenshot files, global input, display-mode changes or injected code.
#include <cstdint>
#include <d3d11.h>
#include <dwmapi.h>
#include <dxgi1_2.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <windows.h>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

static void Check(HRESULT value, const char *operation) {
  if (FAILED(value)) {
    std::cerr << operation << " HRESULT=0x" << std::hex << unsigned(value)
              << '\n';
    throw std::runtime_error(operation);
  }
}
static int64_t Now() {
  LARGE_INTEGER time;
  QueryPerformanceCounter(&time);
  return time.QuadPart;
}

int wmain(int argc, wchar_t **argv) {
  if (argc != 4) {
    std::cerr << "Usage: frame_probe title output.jsonl seconds\n";
    return 2;
  }
  SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
  try {
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    const auto waiting = Now();
    HWND target = nullptr;
    while (!(target = FindWindowW(nullptr, argv[1]))) {
      if (Now() - waiting > frequency.QuadPart * 40)
        throw std::runtime_error("window not found");
      Sleep(20);
    }
    DWORD pid = 0;
    GetWindowThreadProcessId(target, &pid);
    RECT client;
    GetClientRect(target, &client);
    POINT origin{};
    ClientToScreen(target, &origin);
    RECT roi{origin.x + client.right * 30 / 100,
             origin.y + client.bottom * 15 / 100,
             origin.x + client.right * 65 / 100,
             origin.y + client.bottom * 80 / 100};
    const HMONITOR monitor = MonitorFromWindow(target, MONITOR_DEFAULTTONULL);
    ComPtr<IDXGIFactory1> factory;
    Check(CreateDXGIFactory1(IID_PPV_ARGS(&factory)), "CreateDXGIFactory1");
    ComPtr<IDXGIAdapter1> adapter;
    ComPtr<IDXGIOutput1> output;
    DXGI_OUTPUT_DESC description{};
    for (UINT a = 0; !output; ++a) {
      ComPtr<IDXGIAdapter1> next;
      if (factory->EnumAdapters1(a, &next) == DXGI_ERROR_NOT_FOUND)
        break;
      for (UINT o = 0;; ++o) {
        ComPtr<IDXGIOutput> candidate;
        if (next->EnumOutputs(o, &candidate) == DXGI_ERROR_NOT_FOUND)
          break;
        Check(candidate->GetDesc(&description), "GetDesc");
        if (description.Monitor == monitor) {
          adapter = next;
          Check(candidate.As(&output), "IDXGIOutput1");
          break;
        }
      }
    }
    if (!output)
      throw std::runtime_error("target monitor not found");
    if (description.Rotation != DXGI_MODE_ROTATION_IDENTITY)
      throw std::runtime_error("rotated output requires a rotated capture ROI");
    if (roi.left < description.DesktopCoordinates.left ||
        roi.top < description.DesktopCoordinates.top ||
        roi.right > description.DesktopCoordinates.right ||
        roi.bottom > description.DesktopCoordinates.bottom)
      throw std::runtime_error(
          "capture ROI is not entirely on the target monitor");
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    Check(D3D11CreateDevice(adapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr, 0,
                            nullptr, 0, D3D11_SDK_VERSION, &device, nullptr,
                            &context),
          "D3D11CreateDevice");
    ComPtr<IDXGIOutputDuplication> duplication;
    Check(output->DuplicateOutput(device.Get(), &duplication),
          "DuplicateOutput");
    D3D11_TEXTURE2D_DESC stage{};
    stage.Width = roi.right - roi.left;
    stage.Height = roi.bottom - roi.top;
    stage.MipLevels = stage.ArraySize = 1;
    stage.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    stage.SampleDesc.Count = 1;
    stage.Usage = D3D11_USAGE_STAGING;
    stage.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    ComPtr<ID3D11Texture2D> readback;
    Check(device->CreateTexture2D(&stage, nullptr, &readback),
          "CreateTexture2D");
    D3D11_BOX box{UINT(roi.left - description.DesktopCoordinates.left),
                  UINT(roi.top - description.DesktopCoordinates.top),
                  0,
                  UINT(roi.right - description.DesktopCoordinates.left),
                  UINT(roi.bottom - description.DesktopCoordinates.top),
                  1};
    DEVMODEW mode{};
    mode.dmSize = sizeof(mode);
    EnumDisplaySettingsW(description.DeviceName, ENUM_CURRENT_SETTINGS, &mode);
    DWM_TIMING_INFO timing{};
    timing.cbSize = sizeof(timing);
    const auto timingResult = DwmGetCompositionTimingInfo(nullptr, &timing);
    std::ofstream log(argv[2]);
    if (!log)
      throw std::runtime_error("cannot write capture log");
    log << "{\"type\":\"metadata\",\"pid\":" << pid
        << ",\"frequency\":" << frequency.QuadPart
        << ",\"refresh_hz\":" << mode.dmDisplayFrequency
        << ",\"dwm_refresh_numerator\":"
        << (SUCCEEDED(timingResult) ? timing.rateRefresh.uiNumerator : 0)
        << ",\"dwm_refresh_denominator\":"
        << (SUCCEEDED(timingResult) ? timing.rateRefresh.uiDenominator : 0)
        << ",\"roi\":[" << roi.left << ',' << roi.top << ',' << roi.right << ','
        << roi.bottom << "]}\n";
    log.flush();
    const auto start = Now();
    uint64_t previous = 0;
    size_t changes = 0, frames = 0;
    while (Now() - start < std::stod(argv[3]) * frequency.QuadPart &&
           IsWindow(target)) {
      DXGI_OUTDUPL_FRAME_INFO frame{};
      ComPtr<IDXGIResource> resource;
      const auto result = duplication->AcquireNextFrame(100, &frame, &resource);
      if (result == DXGI_ERROR_WAIT_TIMEOUT)
        continue;
      Check(result, "AcquireNextFrame");
      if (!frame.LastPresentTime.QuadPart) {
        duplication->ReleaseFrame();
        continue;
      }
      POINT current{};
      ClientToScreen(target, &current);
      if (!IsWindow(target) || !IsWindowVisible(target) || IsIconic(target)) {
        duplication->ReleaseFrame();
        break;
      }
      if (current.x != origin.x || current.y != origin.y)
        throw std::runtime_error("target moved during capture");
      RECT current_client{};
      GetClientRect(target, &current_client);
      if (!EqualRect(&client, &current_client))
        throw std::runtime_error("target resized during capture");
      if (frame.ProtectedContentMaskedOut)
        throw std::runtime_error("capture contains masked pixels");
      for (POINT point :
           {POINT{roi.left, roi.top}, POINT{roi.right - 1, roi.bottom - 1},
            POINT{(roi.left + roi.right) / 2, (roi.top + roi.bottom) / 2}}) {
        DWORD owner = 0;
        GetWindowThreadProcessId(WindowFromPoint(point), &owner);
        if (owner != pid)
          throw std::runtime_error(
              "capture ROI is occluded by another process");
      }
      ComPtr<ID3D11Texture2D> texture;
      Check(resource.As(&texture), "frame texture");
      context->CopySubresourceRegion(readback.Get(), 0, 0, 0, 0, texture.Get(),
                                     0, &box);
      duplication->ReleaseFrame();
      D3D11_MAPPED_SUBRESOURCE pixels{};
      Check(context->Map(readback.Get(), 0, D3D11_MAP_READ, 0, &pixels), "Map");
      // Sample every fourth column, every row. Ignore alpha and hardware mouse
      // updates; hash only the selected application's composed content pixels.
      uint64_t hash = 14695981039346656037ull;
      for (UINT y = 0; y < stage.Height; ++y) {
        const auto *row = reinterpret_cast<const uint32_t *>(
            static_cast<const char *>(pixels.pData) + y * pixels.RowPitch);
        for (UINT x = 0; x < stage.Width; x += 4) {
          hash ^= row[x] & 0xffffff;
          hash *= 1099511628211ull;
        }
      }
      uint64_t marker = 0;
      if (stage.Width >= 152 && stage.Height >= 16) {
        const auto *row = reinterpret_cast<const uint32_t *>(
            static_cast<const char *>(pixels.pData) + 12 * pixels.RowPitch);
        for (unsigned bit = 0; bit < 48; ++bit) {
          const auto rgb = row[9 + bit * 3];
          if (((rgb >> 16) & 255) + ((rgb >> 8) & 255) + (rgb & 255) > 384)
            marker |= uint64_t(1) << bit;
        }
      }
      const unsigned sequence = unsigned((marker >> 16) & 0xffff);
      const bool marker_valid =
          (marker & 0xffff) == 0xA55A &&
          (sequence ^ unsigned((marker >> 32) & 0xffff)) == 0xffff;
      context->Unmap(readback.Get(), 0);
      ++frames;
      log << "{\"type\":\"frame\",\"present_qpc\":"
          << frame.LastPresentTime.QuadPart << ",\"captured_qpc\":" << Now()
          << ",\"accumulated\":" << frame.AccumulatedFrames
          << ",\"changed\":" << (hash != previous ? "true" : "false")
          << ",\"frame_id\":" << (marker_valid ? sequence : 0) << ",\"hash\":\""
          << std::hex << hash << std::dec << "\"}\n";
      changes += hash != previous;
      previous = hash;
    }
    log << "{\"type\":\"complete\",\"frames\":" << frames
        << ",\"changes\":" << changes << "}\n";
    std::cout << "Captured " << frames << " desktop updates, " << changes
              << " target content changes\n";
    return frames ? 0 : 1;
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return 1;
  }
}
