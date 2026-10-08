package nativeapi

import "testing"

func TestClipboardInputValidation(t *testing.T) {
	if Clipboard.WriteText("a\x00b") == nil {
		t.Fatal("NUL text accepted")
	}
	if Clipboard.WriteHTML("\xc0\xaf") == nil {
		t.Fatal("invalid UTF-8 accepted")
	}
	if Clipboard.Write(ClipboardData{Text: ptrClipboardTest("\x00")}) == nil {
		t.Fatal("invalid data accepted")
	}
}
func ptrClipboardTest(value string) *string { return &value }
