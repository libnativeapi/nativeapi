using NativeAPI;

namespace NativeAPI.Tests;

public class ClipboardTests
{
    [Fact]
    public void InvalidTextFailsBeforeNativeSubmission()
    {
        Assert.False(Clipboard.Shared.WriteText("a\0b"));
        Assert.False(Clipboard.Shared.WriteHtml("\ud800"));
        Assert.False(Clipboard.Shared.Write(new ClipboardData { Text = "\0" }));
        Assert.False(Clipboard.Shared.WriteFilePaths(new[] { "/tmp/a\0b" }));
    }
}
