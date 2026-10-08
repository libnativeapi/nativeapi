using NativeAPI;

static void Check(bool value) { if (!value) throw new Exception("clipboard assertion failed"); }
var board = Clipboard.Shared;
Check(!board.WriteText("a\0b"));
Check(!board.WriteHtml("\ud800"));
using var image = Image.FromBase64("iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAAC0lEQVR42mNgAAIAAAUAAen63NgAAAAASUVORK5CYII=");
Check(image is not null);
Check(board.Write(new ClipboardData("剪贴板 😀", "<b>中文</b>", image, new[] { "/tmp/file", "/tmp/directory" })));
var data = board.ReadAsync();
var text = board.ReadTextAsync();
var html = board.ReadHtmlAsync();
var paths = board.ReadFilePathsAsync();
var returnedImage = board.ReadImageAsync();
var complete = Task.WhenAll(data, text, html, paths, returnedImage).ContinueWith(_ => Application.Shared.Quit(0));
Application.Shared.Run();
complete.GetAwaiter().GetResult();
Check(data.Result.Text == text.Result && text.Result == "剪贴板 😀");
Check(data.Result.Html == html.Result && html.Result == "<b>中文</b>");
Check(data.Result.FilePaths.SequenceEqual(paths.Result) && paths.Result.Length == 2);
Check(board.Clear());
Check(data.Result.Image!.Size.Width == 1 && returnedImage.Result!.Size.Width == 1);
data.Result.Image!.Dispose(); returnedImage.Result!.Dispose();
Console.WriteLine("C# clipboard roundtrip passed");
