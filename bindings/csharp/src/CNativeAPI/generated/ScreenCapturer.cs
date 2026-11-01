// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.
#nullable enable

using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace CNativeAPI;

[StructLayout(LayoutKind.Sequential)]
public struct native_capture_source_selection_options_t
{
    public byte allow_displays;
    public byte allow_windows;
    public ulong parent_window;
}

[StructLayout(LayoutKind.Sequential)]
public struct native_screen_capture_options_t
{
    public byte include_cursor;
    public byte crop_enabled;
    public native_rectangle_t crop;
}

[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
public delegate void ScreenCapturerCaptureCallbackNativeCallback([MarshalAs(UnmanagedType.I1)] bool arg0, ulong arg1, ulong delivery, IntPtr userData);

[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
public delegate void ScreenCapturerSelectSourceCallbackNativeCallback([MarshalAs(UnmanagedType.I1)] bool arg0, ulong arg1, ulong delivery, IntPtr userData);

public static partial class Interop
{
    [DllImport(Libraries.NativeApi, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.I1)]
    public static extern bool native_screen_capturer_is_direct_capture_supported(int type);

    [DllImport(Libraries.NativeApi, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.I1)]
    public static extern bool native_screen_capturer_is_source_selection_supported();

    [DllImport(Libraries.NativeApi, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.I1)]
    public static extern bool native_screen_capturer_is_supported();

    [DllImport(Libraries.NativeApi, CallingConvention = CallingConvention.Cdecl)]
    public static extern void native_capture_source_selection_options_free(ref native_capture_source_selection_options_t value);

    [DllImport(Libraries.NativeApi, CallingConvention = CallingConvention.Cdecl)]
    public static extern void native_screen_capture_options_free(ref native_screen_capture_options_t value);

    [DllImport(Libraries.NativeApi, CallingConvention = CallingConvention.Cdecl)]
    public static extern void native_screen_capturer_capture(ulong source, native_screen_capture_options_t options, ScreenCapturerCaptureCallbackNativeCallback callback, IntPtr callback_user_data, ReleaseUserDataNativeCallback callback_release_user_data);

    [DllImport(Libraries.NativeApi, CallingConvention = CallingConvention.Cdecl)]
    public static extern void native_screen_capturer_select_source(native_capture_source_selection_options_t options, ScreenCapturerSelectSourceCallbackNativeCallback callback, IntPtr callback_user_data, ReleaseUserDataNativeCallback callback_release_user_data);
}

