// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.
#nullable enable

using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace CNativeAPI;

[StructLayout(LayoutKind.Sequential)]
public struct native_clipboard_data_t
{
    public IntPtr text;
    public IntPtr html;
    public ulong image;
    public native_string_list_t file_paths;
}

[StructLayout(LayoutKind.Sequential)]
public struct native_clipboard_event_t
{
    public int type;
}

[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
public delegate void ClipboardEventNativeCallback(IntPtr evt, IntPtr userData);

[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
public delegate void ClipboardReadCallbackNativeCallback([MarshalAs(UnmanagedType.I1)] bool arg0, IntPtr arg1, ulong delivery, IntPtr userData);

[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
public delegate void ClipboardReadFilePathsCallbackNativeCallback([MarshalAs(UnmanagedType.I1)] bool arg0, IntPtr arg1, ulong delivery, IntPtr userData);

[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
public delegate void ClipboardReadHtmlCallbackNativeCallback([MarshalAs(UnmanagedType.I1)] bool arg0, IntPtr arg1, ulong delivery, IntPtr userData);

[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
public delegate void ClipboardReadImageCallbackNativeCallback([MarshalAs(UnmanagedType.I1)] bool arg0, ulong arg1, ulong delivery, IntPtr userData);

[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
public delegate void ClipboardReadTextCallbackNativeCallback([MarshalAs(UnmanagedType.I1)] bool arg0, IntPtr arg1, ulong delivery, IntPtr userData);

public static partial class Interop
{
    [DllImport(Libraries.NativeApi, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.I1)]
    public static extern bool native_clipboard_clear();

    [DllImport(Libraries.NativeApi, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.I1)]
    public static extern bool native_clipboard_is_change_monitoring_supported();

    [DllImport(Libraries.NativeApi, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.I1)]
    public static extern bool native_clipboard_is_monitoring();

    [DllImport(Libraries.NativeApi, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.I1)]
    public static extern bool native_clipboard_is_supported();

    [DllImport(Libraries.NativeApi, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.I1)]
    public static extern bool native_clipboard_remove_listener(ulong listenerId);

    [DllImport(Libraries.NativeApi, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.I1)]
    public static extern bool native_clipboard_write(native_clipboard_data_t data);

    [DllImport(Libraries.NativeApi, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.I1)]
    public static extern bool native_clipboard_write_file_paths(native_string_list_t filePaths);

    [DllImport(Libraries.NativeApi, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.I1)]
    public static extern bool native_clipboard_write_html([MarshalAs(UnmanagedType.LPUTF8Str)] string? html);

    [DllImport(Libraries.NativeApi, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.I1)]
    public static extern bool native_clipboard_write_image(ulong image);

    [DllImport(Libraries.NativeApi, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.I1)]
    public static extern bool native_clipboard_write_text([MarshalAs(UnmanagedType.LPUTF8Str)] string? text);

    [DllImport(Libraries.NativeApi, CallingConvention = CallingConvention.Cdecl)]
    public static extern ulong native_clipboard_add_listener(ClipboardEventNativeCallback callback, IntPtr userData, ReleaseUserDataNativeCallback releaseUserData);

    [DllImport(Libraries.NativeApi, CallingConvention = CallingConvention.Cdecl)]
    public static extern void native_clipboard_data_free(ref native_clipboard_data_t value);

    [DllImport(Libraries.NativeApi, CallingConvention = CallingConvention.Cdecl)]
    public static extern void native_clipboard_read(ClipboardReadCallbackNativeCallback callback, IntPtr callback_user_data, ReleaseUserDataNativeCallback callback_release_user_data);

    [DllImport(Libraries.NativeApi, CallingConvention = CallingConvention.Cdecl)]
    public static extern void native_clipboard_read_file_paths(ClipboardReadFilePathsCallbackNativeCallback callback, IntPtr callback_user_data, ReleaseUserDataNativeCallback callback_release_user_data);

    [DllImport(Libraries.NativeApi, CallingConvention = CallingConvention.Cdecl)]
    public static extern void native_clipboard_read_html(ClipboardReadHtmlCallbackNativeCallback callback, IntPtr callback_user_data, ReleaseUserDataNativeCallback callback_release_user_data);

    [DllImport(Libraries.NativeApi, CallingConvention = CallingConvention.Cdecl)]
    public static extern void native_clipboard_read_image(ClipboardReadImageCallbackNativeCallback callback, IntPtr callback_user_data, ReleaseUserDataNativeCallback callback_release_user_data);

    [DllImport(Libraries.NativeApi, CallingConvention = CallingConvention.Cdecl)]
    public static extern void native_clipboard_read_text(ClipboardReadTextCallbackNativeCallback callback, IntPtr callback_user_data, ReleaseUserDataNativeCallback callback_release_user_data);
}

