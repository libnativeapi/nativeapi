// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.
#nullable enable

using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using CNativeAPI;

namespace NativeAPI;

public struct ClipboardData
{
    public string? Text;
    public string? Html;
    public Image? Image;
    public string[] FilePaths;

    public ClipboardData(string? text, string? html, Image? image, string[] filePaths)
    {
        Text = text;
        Html = html;
        Image = image;
        FilePaths = filePaths;
    }

    internal static ClipboardData FromRaw(in native_clipboard_data_t raw)
    {
        return new ClipboardData(Marshal.PtrToStringUTF8(raw.text), Marshal.PtrToStringUTF8(raw.html), raw.image == 0 ? null : new Image(Interop.native_handle_retain(raw.image)), Interop.ReadStringList(in raw.file_paths));
    }

    internal native_clipboard_data_t ToRaw()
    {
        var raw = new native_clipboard_data_t();
        raw.text = Marshal.StringToCoTaskMemUTF8(Text);
        raw.html = Marshal.StringToCoTaskMemUTF8(Html);
        raw.image = Image?.NativeHandle ?? 0;
        var file_pathsValues = FilePaths ?? Array.Empty<string>();
        raw.file_paths.count = new CLong(file_pathsValues.Length);
        raw.file_paths.items = Marshal.AllocCoTaskMem(IntPtr.Size * file_pathsValues.Length);
        for (var i = 0; i < file_pathsValues.Length; ++i) Marshal.WriteIntPtr(raw.file_paths.items, i * IntPtr.Size, Marshal.StringToCoTaskMemUTF8(file_pathsValues[i]));
        return raw;
    }

    internal static void ReleaseRaw(ref native_clipboard_data_t raw)
    {
        Marshal.FreeCoTaskMem(raw.text);
        raw.text = IntPtr.Zero;
        Marshal.FreeCoTaskMem(raw.html);
        raw.html = IntPtr.Zero;
        for (var i = 0; i < (int)raw.file_paths.count.Value; ++i) Marshal.FreeCoTaskMem(Marshal.ReadIntPtr(raw.file_paths.items, i * IntPtr.Size));
        Marshal.FreeCoTaskMem(raw.file_paths.items);
        raw.file_paths = default;
    }
}

/// <summary>One ClipboardEvent, in its concrete form.</summary>
public abstract record ClipboardEvent
{
    private ClipboardEvent() { }

    public sealed record Changed : ClipboardEvent;

    internal static ClipboardEvent? FromRaw(in native_clipboard_event_t raw)
    {
        switch (raw.type)
        {
            case 0: return new Changed();
            default: return null;
        }
    }
}

public sealed partial class Clipboard
{
    /// <summary>The shared instance backed by the native singleton.</summary>
    public static Clipboard Shared { get; } = new Clipboard();

    private Clipboard() { }

    public bool IsSupported()
    {
        var rawResult = Interop.native_clipboard_is_supported();
        return rawResult;
    }

    public bool IsChangeMonitoringSupported()
    {
        var rawResult = Interop.native_clipboard_is_change_monitoring_supported();
        return rawResult;
    }

    public void Read(Action<bool, ClipboardData> callback)
    {
        ClipboardReadCallbackNativeCallback nativeCallback = (arg0, arg1, delivery, userData) => { try { if (!Interop.native_event_delivery_is_active(delivery)) return; var raw1 = Marshal.PtrToStructure<native_clipboard_data_t>(arg1); callback(arg0, ClipboardData.FromRaw(in raw1)); } catch (Exception) { } finally { Interop.native_event_delivery_complete(delivery, true); } };
        Interop.native_clipboard_read(nativeCallback, CallbackKeeper.Hold(nativeCallback), CallbackKeeper.Release);
    }

    public void ReadText(Action<bool, string?> callback)
    {
        ClipboardReadTextCallbackNativeCallback nativeCallback = (arg0, arg1, delivery, userData) => { try { if (!Interop.native_event_delivery_is_active(delivery)) return; callback(arg0, Marshal.PtrToStringUTF8(arg1)); } catch (Exception) { } finally { Interop.native_event_delivery_complete(delivery, true); } };
        Interop.native_clipboard_read_text(nativeCallback, CallbackKeeper.Hold(nativeCallback), CallbackKeeper.Release);
    }

    public void ReadHtml(Action<bool, string?> callback)
    {
        ClipboardReadHtmlCallbackNativeCallback nativeCallback = (arg0, arg1, delivery, userData) => { try { if (!Interop.native_event_delivery_is_active(delivery)) return; callback(arg0, Marshal.PtrToStringUTF8(arg1)); } catch (Exception) { } finally { Interop.native_event_delivery_complete(delivery, true); } };
        Interop.native_clipboard_read_html(nativeCallback, CallbackKeeper.Hold(nativeCallback), CallbackKeeper.Release);
    }

    public void ReadImage(Action<bool, Image?> callback)
    {
        ClipboardReadImageCallbackNativeCallback nativeCallback = (arg0, arg1, delivery, userData) => { try { if (!Interop.native_event_delivery_is_active(delivery)) return; callback(arg0, arg1 == 0 ? null : new Image(Interop.native_handle_retain(arg1))); } catch (Exception) { } finally { Interop.native_event_delivery_complete(delivery, true); } };
        Interop.native_clipboard_read_image(nativeCallback, CallbackKeeper.Hold(nativeCallback), CallbackKeeper.Release);
    }

    public void ReadFilePaths(Action<bool, string[]> callback)
    {
        ClipboardReadFilePathsCallbackNativeCallback nativeCallback = (arg0, arg1, delivery, userData) => { try { if (!Interop.native_event_delivery_is_active(delivery)) return; var raw1 = Marshal.PtrToStructure<native_string_list_t>(arg1); callback(arg0, Interop.ReadStringList(in raw1)); } catch (Exception) { } finally { Interop.native_event_delivery_complete(delivery, true); } };
        Interop.native_clipboard_read_file_paths(nativeCallback, CallbackKeeper.Hold(nativeCallback), CallbackKeeper.Release);
    }

    public bool Write(ClipboardData data)
    {
        if (!ValidText(data.Text) || !ValidText(data.Html) || Array.Exists(data.FilePaths ?? Array.Empty<string>(), path => !ValidText(path))) return false;
        var rawData = data.ToRaw();
        var rawResult = Interop.native_clipboard_write(rawData);
        ClipboardData.ReleaseRaw(ref rawData);
        return rawResult;
    }

    public bool WriteText(string text)
    {
        if (!ValidText(text)) return false;
        var rawResult = Interop.native_clipboard_write_text(text);
        return rawResult;
    }

    public bool WriteHtml(string html)
    {
        if (!ValidText(html)) return false;
        var rawResult = Interop.native_clipboard_write_html(html);
        return rawResult;
    }

    public bool WriteImage(Image? image)
    {
        var rawResult = Interop.native_clipboard_write_image(image?.NativeHandle ?? 0);
        return rawResult;
    }

    public bool WriteFilePaths(IReadOnlyList<string> filePaths)
    {
        if (System.Linq.Enumerable.Any(filePaths, path => !ValidText(path))) return false;
        var itemsFilePaths = Interop.AllocUtf8Array(filePaths);
        var blockFilePaths = Interop.AllocPointerArray(itemsFilePaths);
        var listFilePaths = new native_string_list_t { items = blockFilePaths, count = new CLong(itemsFilePaths.Length) };
        var rawResult = Interop.native_clipboard_write_file_paths(listFilePaths);
        Interop.FreeUtf8Array(itemsFilePaths);
        Marshal.FreeHGlobal(blockFilePaths);
        return rawResult;
    }

    public bool Clear()
    {
        var rawResult = Interop.native_clipboard_clear();
        return rawResult;
    }

    public bool IsMonitoring()
    {
        var rawResult = Interop.native_clipboard_is_monitoring();
        return rawResult;
    }

    private static bool ValidText(string? value) {
        if (value is null) return true;
        if (value.IndexOf((char)0) >= 0) return false;
        try { new System.Text.UTF8Encoding(false, true).GetByteCount(value); return true; }
        catch (System.Text.EncoderFallbackException) { return false; }
    }
    public System.Threading.Tasks.Task<ClipboardData> ReadAsync() {
        var completion = new System.Threading.Tasks.TaskCompletionSource<ClipboardData>(System.Threading.Tasks.TaskCreationOptions.RunContinuationsAsynchronously);
        Read((success, value) => { if (success) completion.TrySetResult(value); else completion.TrySetException(new InvalidOperationException("Clipboard operation failed")); });
        return completion.Task;
    }
    public System.Threading.Tasks.Task<string?> ReadTextAsync() {
        var completion = new System.Threading.Tasks.TaskCompletionSource<string?>(System.Threading.Tasks.TaskCreationOptions.RunContinuationsAsynchronously);
        ReadText((success, value) => { if (success) completion.TrySetResult(value); else completion.TrySetException(new InvalidOperationException("Clipboard operation failed")); });
        return completion.Task;
    }
    public System.Threading.Tasks.Task<string?> ReadHtmlAsync() {
        var completion = new System.Threading.Tasks.TaskCompletionSource<string?>(System.Threading.Tasks.TaskCreationOptions.RunContinuationsAsynchronously);
        ReadHtml((success, value) => { if (success) completion.TrySetResult(value); else completion.TrySetException(new InvalidOperationException("Clipboard operation failed")); });
        return completion.Task;
    }
    public System.Threading.Tasks.Task<Image?> ReadImageAsync() {
        var completion = new System.Threading.Tasks.TaskCompletionSource<Image?>(System.Threading.Tasks.TaskCreationOptions.RunContinuationsAsynchronously);
        ReadImage((success, value) => { if (success) completion.TrySetResult(value); else completion.TrySetException(new InvalidOperationException("Clipboard operation failed")); });
        return completion.Task;
    }
    public System.Threading.Tasks.Task<string[]> ReadFilePathsAsync() {
        var completion = new System.Threading.Tasks.TaskCompletionSource<string[]>(System.Threading.Tasks.TaskCreationOptions.RunContinuationsAsynchronously);
        ReadFilePaths((success, value) => { if (success) completion.TrySetResult(value); else completion.TrySetException(new InvalidOperationException("Clipboard operation failed")); });
        return completion.Task;
    }
    /// <summary>Registers <paramref name="callback"/> for every ClipboardEvent this Clipboard emits.</summary>
    /// <remarks>
    /// The delegate is kept alive until the listener is removed or its emitter
    /// destroyed; the core releases it then.
    /// </remarks>
    public ulong AddListener(Action<ClipboardEvent> callback)
    {
        ClipboardEventNativeCallback native = (evt, userData) =>
        {
            if (evt == IntPtr.Zero)
            {
                return;
            }
            var value = ClipboardEvent.FromRaw(Marshal.PtrToStructure<native_clipboard_event_t>(evt));
            if (value is not null)
            {
                callback(value);
            }
        };
        return Interop.native_clipboard_add_listener(native, CallbackKeeper.Hold(native), CallbackKeeper.Release);
    }

    /// <summary>Unregisters a listener. Returns false if unknown.</summary>
    public bool RemoveListener(ulong listenerId)
    {
        return Interop.native_clipboard_remove_listener(listenerId);
    }

}

