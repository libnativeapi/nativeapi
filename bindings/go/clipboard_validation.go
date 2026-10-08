package nativeapi

import (
	"strings"
	"unicode/utf8"
)

func validClipboardText(value string) bool {
	return utf8.ValidString(value) && !strings.ContainsRune(value, 0)
}
func validClipboardOptionalText(value *string) bool {
	return value == nil || validClipboardText(*value)
}
func validClipboardPaths(values []string) bool {
	for _, value := range values {
		if !validClipboardText(value) {
			return false
		}
	}
	return true
}
