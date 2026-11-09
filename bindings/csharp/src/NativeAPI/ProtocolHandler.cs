// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.
#nullable enable

using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using CNativeAPI;

namespace NativeAPI;

/// <summary>One ProtocolHandlerEvent, in its concrete form.</summary>
public abstract record ProtocolHandlerEvent
{
    private ProtocolHandlerEvent() { }

    public sealed record ProtocolUrlReceived(string? Url) : ProtocolHandlerEvent;

    internal static ProtocolHandlerEvent? FromRaw(in native_protocol_handler_event_t raw)
    {
        switch (raw.type)
        {
            case 0: return new ProtocolUrlReceived(Marshal.PtrToStringUTF8(raw.data.protocol_url_received.url));
            default: return null;
        }
    }
}

public sealed partial class ProtocolHandler
{
    /// <summary>The shared instance backed by the native singleton.</summary>
    public static ProtocolHandler Shared { get; } = new ProtocolHandler();

    private ProtocolHandler() { }

    public bool IsSupported()
    {
        var rawResult = Interop.native_protocol_handler_is_supported();
        return rawResult;
    }

    public bool Register(string scheme)
    {
        var rawResult = Interop.native_protocol_handler_register(scheme);
        return rawResult;
    }

    public string? GetInitialUrl()
    {
        var rawResult = Interop.native_protocol_handler_get_initial_url();
        return Interop.ConsumeString(rawResult);
    }

    /// <summary>Registers <paramref name="callback"/> for every ProtocolHandlerEvent this ProtocolHandler emits.</summary>
    /// <remarks>
    /// The delegate is kept alive until the listener is removed or its emitter
    /// destroyed; the core releases it then.
    /// </remarks>
    public ulong AddListener(Action<ProtocolHandlerEvent> callback)
    {
        ProtocolHandlerEventNativeCallback native = (evt, userData) =>
        {
            if (evt == IntPtr.Zero)
            {
                return;
            }
            var value = ProtocolHandlerEvent.FromRaw(Marshal.PtrToStructure<native_protocol_handler_event_t>(evt));
            if (value is not null)
            {
                callback(value);
            }
        };
        return Interop.native_protocol_handler_add_listener(native, CallbackKeeper.Hold(native), CallbackKeeper.Release);
    }

    /// <summary>Unregisters a listener. Returns false if unknown.</summary>
    public bool RemoveListener(ulong listenerId)
    {
        return Interop.native_protocol_handler_remove_listener(listenerId);
    }

}

