import {
  Accessibility20Regular,
  Desktop20Regular,
  Link20Regular,
  Storage20Regular,
  Window20Regular,
  WindowWrench20Regular,
} from '@fluentui/react-icons'

import type { CheckId, Target } from './types'

export type Glyph = typeof Link20Regular

/** The checks in the order they run: each calls one module, as the Flutter example does. */
export const CHECKS: readonly { id: CheckId; name: string; icon: Glyph; detail: string; calls: string[] }[] = [
  {
    id: 'url',
    name: 'UrlOpener support',
    icon: Link20Regular,
    detail: 'supported: true',
    calls: ['UrlOpener.isSupported() → true'],
  },
  {
    id: 'tray',
    name: 'TrayManager support',
    icon: WindowWrench20Regular,
    detail: 'supported: true',
    calls: ['TrayManager.isSupported() → true'],
  },
  {
    id: 'accessibility',
    name: 'Accessibility state',
    icon: Accessibility20Regular,
    detail: 'enabled: false',
    calls: ['AccessibilityManager.isEnabled() → false'],
  },
  {
    id: 'display',
    name: 'DisplayManager primary display',
    icon: Desktop20Regular,
    detail: 'Built-in Retina Display 1512x982',
    calls: ['DisplayManager.getPrimary() → "Built-in Retina Display" 1512×982'],
  },
  {
    id: 'window',
    name: 'WindowManager current window',
    icon: Window20Regular,
    detail: 'CocoaPods nativeapi smoke test',
    calls: ['WindowManager.getCurrent() → getTitle() "CocoaPods nativeapi smoke test"'],
  },
  {
    id: 'preferences',
    name: 'Preferences read/write',
    icon: Storage20Regular,
    detail: 'wrote: true, matched: true, removed: true',
    calls: [
      'Preferences.createWithScope("cocoapods_example")',
      'set("smoke_test", "2026-10-06T09:41:00") → true',
      'get("smoke_test", "") → "2026-10-06T09:41:00"',
      'remove("smoke_test") → true',
    ],
  },
]

/** What a check throws when the module's symbols did not make it into the CocoaPods build. */
export const FAILURES: Record<CheckId, string> = {
  url: "Failed to lookup symbol 'native_url_opener_is_supported': symbol not found",
  tray: "Failed to lookup symbol 'native_tray_manager_is_supported': symbol not found",
  accessibility: "Failed to lookup symbol 'native_accessibility_manager_is_enabled': symbol not found",
  display: "Invalid argument(s): Failed to lookup symbol 'native_display_manager_get_primary': dlsym(RTLD_DEFAULT, …): symbol not found",
  window: "Failed to lookup symbol 'native_window_manager_get_current': symbol not found",
  preferences: "Failed to lookup symbol 'native_preferences_create_with_scope': symbol not found",
}

export const PUBSPEC = `flutter:
  config:
    enable-swift-package-manager: false`

export const PODFILES: Record<Target, string> = {
  macos: `platform :osx, '12.0'

target 'Runner' do
  use_frameworks!

  flutter_install_all_macos_pods File.dirname(File.realpath(__FILE__))
  target 'RunnerTests' do
    inherit! :search_paths
  end
end`,
  ios: `# platform :ios, '13.0'

target 'Runner' do
  use_frameworks!

  flutter_install_all_ios_pods File.dirname(File.realpath(__FILE__))
  target 'RunnerTests' do
    inherit! :search_paths
  end
end`,
}

export const COMMANDS: Record<Target, string> = {
  macos: 'flutter build macos',
  ios: 'flutter build ios --no-codesign',
}

/** What the build does, in order: the pods are Flutter's own; nativeapi arrives through cnativeapi's build hook. */
export const BUILD_STEPS: Record<Target, readonly { label: string; meta: string }[]> = {
  macos: [
    { label: 'flutter pub get', meta: 'SwiftPM off: plugins go through CocoaPods' },
    { label: 'pod install', meta: 'Installing FlutterMacOS (1.0.0)' },
    { label: 'cnativeapi build hook', meta: 'Compiles core into a code asset, bundled as a framework' },
    { label: 'xcodebuild Runner.xcworkspace', meta: 'use_frameworks!, deployment target 12.0' },
    { label: 'Built cocoapods_example.app', meta: 'build/macos/Build/Products/Release' },
  ],
  ios: [
    { label: 'flutter pub get', meta: 'SwiftPM off: plugins go through CocoaPods' },
    { label: 'pod install', meta: 'Installing Flutter (1.0.0)' },
    { label: 'cnativeapi build hook', meta: 'Compiles core for iphoneos arm64' },
    { label: 'xcodebuild Runner.xcworkspace', meta: 'No code signing' },
    { label: 'Built Runner.app', meta: 'build/ios/iphoneos' },
  ],
}
