// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.
#nullable enable

using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using CNativeAPI;

namespace NativeAPI;

/// <summary>Owned handle to a native EventDecision.</summary>
public sealed partial class EventDecision : IDisposable
{
    public ulong NativeHandle { get; private set; }
    private readonly bool _ownsHandle;

    public EventDecision(ulong nativeHandle, bool ownsHandle = true)
    {
        NativeHandle = nativeHandle;
        _ownsHandle = ownsHandle;
    }

    ~EventDecision() => ReleaseHandle();

    public void Dispose()
    {
        ReleaseHandle();
        GC.SuppressFinalize(this);
    }

    private void ReleaseHandle()
    {
        if (_ownsHandle && NativeHandle != 0)
        {
            Interop.native_event_decision_free(NativeHandle);
            NativeHandle = 0;
        }
    }

    public bool Accept()
    {
        var rawResult = Interop.native_event_decision_accept(NativeHandle);
        return rawResult;
    }

    public bool Cancel()
    {
        var rawResult = Interop.native_event_decision_cancel(NativeHandle);
        return rawResult;
    }

    public bool IsPending
    {
        get
        {
            var rawResult = Interop.native_event_decision_is_pending(NativeHandle);
            return rawResult;
        }
    }

}

/// <summary>Owned handle to a native EventRequest.</summary>
public sealed partial class EventRequest : IDisposable
{
    public ulong NativeHandle { get; private set; }
    private readonly bool _ownsHandle;

    public EventRequest(ulong nativeHandle, bool ownsHandle = true)
    {
        NativeHandle = nativeHandle;
        _ownsHandle = ownsHandle;
    }

    ~EventRequest() => ReleaseHandle();

    public void Dispose()
    {
        ReleaseHandle();
        GC.SuppressFinalize(this);
    }

    private void ReleaseHandle()
    {
        if (_ownsHandle && NativeHandle != 0)
        {
            Interop.native_event_request_free(NativeHandle);
            NativeHandle = 0;
        }
    }

    public bool IsCancelable
    {
        get
        {
            var rawResult = Interop.native_event_request_is_cancelable(NativeHandle);
            return rawResult;
        }
    }

    public bool IsCancelled
    {
        get
        {
            var rawResult = Interop.native_event_request_is_cancelled(NativeHandle);
            return rawResult;
        }
    }

    public bool IsPending
    {
        get
        {
            var rawResult = Interop.native_event_request_is_pending(NativeHandle);
            return rawResult;
        }
    }

    public bool Cancel()
    {
        var rawResult = Interop.native_event_request_cancel(NativeHandle);
        return rawResult;
    }

    public EventDecision? Defer()
    {
        var rawResult = Interop.native_event_request_defer(NativeHandle);
        return rawResult == 0 ? null : new EventDecision(rawResult);
    }

}

