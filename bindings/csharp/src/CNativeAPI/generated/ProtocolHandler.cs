// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.
#nullable enable

using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace CNativeAPI;

[StructLayout(LayoutKind.Sequential)]
public struct native_protocol_handler_event_t
{
    public int type;
    public DataUnion data;

    [StructLayout(LayoutKind.Explicit)]
    public struct DataUnion
    {
        [FieldOffset(0)] public ProtocolUrlReceivedData protocol_url_received;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct ProtocolUrlReceivedData
    {
        public IntPtr url;
    }
}

[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
public delegate void ProtocolHandlerEventNativeCallback(IntPtr evt, IntPtr userData);

public static partial class Interop
{
    [DllImport(Libraries.NativeApi, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.I1)]
    public static extern bool native_protocol_handler_is_supported();

    [DllImport(Libraries.NativeApi, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.I1)]
    public static extern bool native_protocol_handler_register([MarshalAs(UnmanagedType.LPUTF8Str)] string? scheme);

    [DllImport(Libraries.NativeApi, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.I1)]
    public static extern bool native_protocol_handler_remove_listener(ulong listenerId);

    [DllImport(Libraries.NativeApi, CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr native_protocol_handler_get_initial_url();

    [DllImport(Libraries.NativeApi, CallingConvention = CallingConvention.Cdecl)]
    public static extern ulong native_protocol_handler_add_listener(ProtocolHandlerEventNativeCallback callback, IntPtr userData, ReleaseUserDataNativeCallback releaseUserData);
}

