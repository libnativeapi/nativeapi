#pragma once

namespace nativeapi {
// Shared, lazy XAML environment for all WinUI 3 components on the calling STA.
// Throws on initialization failure; never shuts down a host-owned dispatcher.
void InitializeWinUI3();
}
