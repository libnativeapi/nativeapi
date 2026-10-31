// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.
#nullable enable

using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace CNativeAPI;

[StructLayout(LayoutKind.Sequential)]
public struct native_screen_recording_options_t
{
    public native_screen_capture_options_t capture;
    public double frame_rate;
    public CULong video_bitrate;
}

[StructLayout(LayoutKind.Sequential)]
public struct native_screen_recording_event_t
{
    public int type;
    public DataUnion data;

    [StructLayout(LayoutKind.Explicit)]
    public struct DataUnion
    {
        [FieldOffset(0)] public CompletedData completed;
        [FieldOffset(0)] public FailedData failed;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct CompletedData
    {
        public IntPtr file_path;
        public double duration;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct FailedData
    {
        public IntPtr message;
    }
}

[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
public delegate void ScreenRecordingEventNativeCallback(IntPtr evt, IntPtr userData);

public static partial class Interop
{
    [DllImport(Libraries.NativeApi, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.I1)]
    public static extern bool native_screen_recorder_cancel(ulong self);

    [DllImport(Libraries.NativeApi, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.I1)]
    public static extern bool native_screen_recorder_is_active(ulong self);

    [DllImport(Libraries.NativeApi, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.I1)]
    public static extern bool native_screen_recorder_is_supported();

    [DllImport(Libraries.NativeApi, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.I1)]
    public static extern bool native_screen_recorder_remove_listener(ulong self, ulong listenerId);

    [DllImport(Libraries.NativeApi, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.I1)]
    public static extern bool native_screen_recorder_start(ulong self, ulong source, [MarshalAs(UnmanagedType.LPUTF8Str)] string? filePath, native_screen_recording_options_t options);

    [DllImport(Libraries.NativeApi, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.I1)]
    public static extern bool native_screen_recorder_stop(ulong self);

    [DllImport(Libraries.NativeApi, CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr native_screen_recorder_get_file_path(ulong self);

    [DllImport(Libraries.NativeApi, CallingConvention = CallingConvention.Cdecl)]
    public static extern double native_screen_recorder_get_duration(ulong self);

    [DllImport(Libraries.NativeApi, CallingConvention = CallingConvention.Cdecl)]
    public static extern int native_screen_recorder_get_state(ulong self);

    [DllImport(Libraries.NativeApi, CallingConvention = CallingConvention.Cdecl)]
    public static extern ulong native_screen_recorder_add_listener(ulong self, ScreenRecordingEventNativeCallback callback, IntPtr userData, ReleaseUserDataNativeCallback releaseUserData);

    [DllImport(Libraries.NativeApi, CallingConvention = CallingConvention.Cdecl)]
    public static extern ulong native_screen_recorder_create();

    [DllImport(Libraries.NativeApi, CallingConvention = CallingConvention.Cdecl)]
    public static extern void native_screen_recorder_free(ulong handle);

    [DllImport(Libraries.NativeApi, CallingConvention = CallingConvention.Cdecl)]
    public static extern void native_screen_recording_options_free(ref native_screen_recording_options_t value);
}

