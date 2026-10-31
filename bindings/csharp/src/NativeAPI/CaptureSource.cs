// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.
#nullable enable

using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using CNativeAPI;

namespace NativeAPI;

public enum CaptureSourceType
{
    None = 0,
    Display = 1,
    Window = 2,
    Unknown = 3,
}

/// <summary>Owned handle to a native CaptureSource.</summary>
public sealed partial class CaptureSource : IDisposable
{
    public ulong NativeHandle { get; private set; }
    private readonly bool _ownsHandle;

    public CaptureSource(ulong nativeHandle, bool ownsHandle = true)
    {
        NativeHandle = nativeHandle;
        _ownsHandle = ownsHandle;
    }

    ~CaptureSource() => ReleaseHandle();

    public void Dispose()
    {
        ReleaseHandle();
        GC.SuppressFinalize(this);
    }

    private void ReleaseHandle()
    {
        if (_ownsHandle && NativeHandle != 0)
        {
            Interop.native_capture_source_free(NativeHandle);
            NativeHandle = 0;
        }
    }

    public static CaptureSource? FromDisplay(Display? display)
    {
        var rawResult = Interop.native_capture_source_from_display(display?.NativeHandle ?? 0);
        return rawResult == 0 ? null : new CaptureSource(rawResult);
    }

    public static CaptureSource? FromWindow(Window? window)
    {
        var rawResult = Interop.native_capture_source_from_window(window?.NativeHandle ?? 0);
        return rawResult == 0 ? null : new CaptureSource(rawResult);
    }

    public CaptureSourceType Type
    {
        get
        {
            var rawResult = Interop.native_capture_source_get_type(NativeHandle);
            return (CaptureSourceType)rawResult;
        }
    }

    public string? Name
    {
        get
        {
            var rawResult = Interop.native_capture_source_get_name(NativeHandle);
            return Interop.ConsumeString(rawResult);
        }
    }

}

