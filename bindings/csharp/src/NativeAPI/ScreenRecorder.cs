// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.
#nullable enable

using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using CNativeAPI;

namespace NativeAPI;

public enum ScreenRecordingState
{
    Idle = 0,
    Starting = 1,
    Recording = 2,
    Stopping = 3,
    Completed = 4,
    Cancelled = 5,
    Failed = 6,
}

public struct ScreenRecordingOptions
{
    public ScreenCaptureOptions Capture;
    public double FrameRate;
    public ulong VideoBitrate;

    public ScreenRecordingOptions(ScreenCaptureOptions capture, double frameRate, ulong videoBitrate)
    {
        Capture = capture;
        FrameRate = frameRate;
        VideoBitrate = videoBitrate;
    }

    internal static ScreenRecordingOptions FromRaw(in native_screen_recording_options_t raw)
    {
        return new ScreenRecordingOptions(ScreenCaptureOptions.FromRaw(in raw.capture), raw.frame_rate, (ulong)raw.video_bitrate.Value);
    }

    internal native_screen_recording_options_t ToRaw()
    {
        var raw = new native_screen_recording_options_t();
        raw.capture = Capture.ToRaw();
        raw.frame_rate = FrameRate;
        raw.video_bitrate = new CULong(checked((nuint)VideoBitrate));
        return raw;
    }

    internal static void ReleaseRaw(ref native_screen_recording_options_t raw)
    {
    }
}

/// <summary>One ScreenRecordingEvent, in its concrete form.</summary>
public abstract record ScreenRecordingEvent
{
    private ScreenRecordingEvent() { }

    public sealed record Started : ScreenRecordingEvent;
    public sealed record Completed(string? FilePath, double Duration) : ScreenRecordingEvent;
    public sealed record Cancelled : ScreenRecordingEvent;
    public sealed record Failed(string? Message) : ScreenRecordingEvent;

    internal static ScreenRecordingEvent? FromRaw(in native_screen_recording_event_t raw)
    {
        switch (raw.type)
        {
            case 0: return new Started();
            case 1: return new Completed(Marshal.PtrToStringUTF8(raw.data.completed.file_path), raw.data.completed.duration);
            case 2: return new Cancelled();
            case 3: return new Failed(Marshal.PtrToStringUTF8(raw.data.failed.message));
            default: return null;
        }
    }
}

/// <summary>Owned handle to a native ScreenRecorder.</summary>
public sealed partial class ScreenRecorder : IDisposable
{
    public ulong NativeHandle { get; private set; }
    private readonly bool _ownsHandle;

    public ScreenRecorder(ulong nativeHandle, bool ownsHandle = true)
    {
        NativeHandle = nativeHandle;
        _ownsHandle = ownsHandle;
    }

    ~ScreenRecorder() => ReleaseHandle();

    public void Dispose()
    {
        ReleaseHandle();
        GC.SuppressFinalize(this);
    }

    private void ReleaseHandle()
    {
        if (_ownsHandle && NativeHandle != 0)
        {
            Interop.native_screen_recorder_free(NativeHandle);
            NativeHandle = 0;
        }
    }

    /// <summary>Creates a new ScreenRecorder; returns null if the native side failed.</summary>
    public static ScreenRecorder? Create()
    {
        var handle = Interop.native_screen_recorder_create();
        return handle == 0 ? null : new ScreenRecorder(handle);
    }

    public static bool IsSupported()
    {
        var rawResult = Interop.native_screen_recorder_is_supported();
        return rawResult;
    }

    public bool Start(CaptureSource? source, string filePath, ScreenRecordingOptions options)
    {
        var rawOptions = options.ToRaw();
        var rawResult = Interop.native_screen_recorder_start(NativeHandle, source?.NativeHandle ?? 0, filePath, rawOptions);
        ScreenRecordingOptions.ReleaseRaw(ref rawOptions);
        return rawResult;
    }

    public bool Stop()
    {
        var rawResult = Interop.native_screen_recorder_stop(NativeHandle);
        return rawResult;
    }

    public bool Cancel()
    {
        var rawResult = Interop.native_screen_recorder_cancel(NativeHandle);
        return rawResult;
    }

    public ScreenRecordingState State
    {
        get
        {
            var rawResult = Interop.native_screen_recorder_get_state(NativeHandle);
            return (ScreenRecordingState)rawResult;
        }
    }

    public bool IsActive
    {
        get
        {
            var rawResult = Interop.native_screen_recorder_is_active(NativeHandle);
            return rawResult;
        }
    }

    public double Duration
    {
        get
        {
            var rawResult = Interop.native_screen_recorder_get_duration(NativeHandle);
            return rawResult;
        }
    }

    public string? FilePath
    {
        get
        {
            var rawResult = Interop.native_screen_recorder_get_file_path(NativeHandle);
            return Interop.ConsumeString(rawResult);
        }
    }

    /// <summary>Registers <paramref name="callback"/> for every ScreenRecordingEvent this ScreenRecorder emits.</summary>
    /// <remarks>
    /// The delegate is kept alive until the listener is removed or its emitter
    /// destroyed; the core releases it then.
    /// </remarks>
    public ulong AddListener(Action<ScreenRecordingEvent> callback)
    {
        ScreenRecordingEventNativeCallback native = (evt, userData) =>
        {
            if (evt == IntPtr.Zero)
            {
                return;
            }
            var value = ScreenRecordingEvent.FromRaw(Marshal.PtrToStructure<native_screen_recording_event_t>(evt));
            if (value is not null)
            {
                callback(value);
            }
        };
        return Interop.native_screen_recorder_add_listener(NativeHandle, native, CallbackKeeper.Hold(native), CallbackKeeper.Release);
    }

    /// <summary>Unregisters a listener. Returns false if unknown.</summary>
    public bool RemoveListener(ulong listenerId)
    {
        return Interop.native_screen_recorder_remove_listener(NativeHandle, listenerId);
    }

}

