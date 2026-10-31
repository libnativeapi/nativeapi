// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.
#nullable enable

using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using CNativeAPI;

namespace NativeAPI;

public struct CaptureSourceSelectionOptions
{
    public bool AllowDisplays;
    public bool AllowWindows;
    public Window? ParentWindow;

    public CaptureSourceSelectionOptions(bool allowDisplays, bool allowWindows, Window? parentWindow)
    {
        AllowDisplays = allowDisplays;
        AllowWindows = allowWindows;
        ParentWindow = parentWindow;
    }

    internal static CaptureSourceSelectionOptions FromRaw(in native_capture_source_selection_options_t raw)
    {
        return new CaptureSourceSelectionOptions(raw.allow_displays != 0, raw.allow_windows != 0, raw.parent_window == 0 ? null : new Window(Interop.native_handle_retain(raw.parent_window)));
    }

    internal native_capture_source_selection_options_t ToRaw()
    {
        var raw = new native_capture_source_selection_options_t();
        raw.allow_displays = (byte)(AllowDisplays ? 1 : 0);
        raw.allow_windows = (byte)(AllowWindows ? 1 : 0);
        raw.parent_window = ParentWindow?.NativeHandle ?? 0;
        return raw;
    }

    internal static void ReleaseRaw(ref native_capture_source_selection_options_t raw)
    {
    }
}

public struct ScreenCaptureOptions
{
    public bool IncludeCursor;
    public bool CropEnabled;
    public Rectangle Crop;

    public ScreenCaptureOptions(bool includeCursor, bool cropEnabled, Rectangle crop)
    {
        IncludeCursor = includeCursor;
        CropEnabled = cropEnabled;
        Crop = crop;
    }

    internal static ScreenCaptureOptions FromRaw(in native_screen_capture_options_t raw)
    {
        return new ScreenCaptureOptions(raw.include_cursor != 0, raw.crop_enabled != 0, Rectangle.FromRaw(in raw.crop));
    }

    internal native_screen_capture_options_t ToRaw()
    {
        var raw = new native_screen_capture_options_t();
        raw.include_cursor = (byte)(IncludeCursor ? 1 : 0);
        raw.crop_enabled = (byte)(CropEnabled ? 1 : 0);
        raw.crop = Crop.ToRaw();
        return raw;
    }

    internal static void ReleaseRaw(ref native_screen_capture_options_t raw)
    {
    }
}

public static partial class ScreenCapturer
{
    public static bool IsSupported()
    {
        var rawResult = Interop.native_screen_capturer_is_supported();
        return rawResult;
    }

    public static bool IsSourceSelectionSupported()
    {
        var rawResult = Interop.native_screen_capturer_is_source_selection_supported();
        return rawResult;
    }

    public static bool IsDirectCaptureSupported(CaptureSourceType type)
    {
        var rawResult = Interop.native_screen_capturer_is_direct_capture_supported((int)type);
        return rawResult;
    }

    public static void SelectSource(CaptureSourceSelectionOptions options, Action<bool, CaptureSource?> callback)
    {
        var rawOptions = options.ToRaw();
        ScreenCapturerSelectSourceCallbackNativeCallback nativeCallback = (arg0, arg1, delivery, userData) => { try { if (!Interop.native_event_delivery_is_active(delivery)) return; callback(arg0, arg1 == 0 ? null : new CaptureSource(Interop.native_handle_retain(arg1))); } catch (Exception) { } finally { Interop.native_event_delivery_complete(delivery, true); } };
        Interop.native_screen_capturer_select_source(rawOptions, nativeCallback, CallbackKeeper.Hold(nativeCallback), CallbackKeeper.Release);
        CaptureSourceSelectionOptions.ReleaseRaw(ref rawOptions);
    }

    public static void Capture(CaptureSource? source, ScreenCaptureOptions options, Action<bool, Image?> callback)
    {
        var rawOptions = options.ToRaw();
        ScreenCapturerCaptureCallbackNativeCallback nativeCallback = (arg0, arg1, delivery, userData) => { try { if (!Interop.native_event_delivery_is_active(delivery)) return; callback(arg0, arg1 == 0 ? null : new Image(Interop.native_handle_retain(arg1))); } catch (Exception) { } finally { Interop.native_event_delivery_complete(delivery, true); } };
        Interop.native_screen_capturer_capture(source?.NativeHandle ?? 0, rawOptions, nativeCallback, CallbackKeeper.Hold(nativeCallback), CallbackKeeper.Release);
        ScreenCaptureOptions.ReleaseRaw(ref rawOptions);
    }

}

