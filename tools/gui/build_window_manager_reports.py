#!/usr/bin/env python3
"""Build real window_manager regressions with local or pinned published nativeapi.

Run on the target host through fastforge. Local overrides and copied Dart targets
are restored after the build; complete runnable bundles stay in example/build.
"""
import argparse
import json
from pathlib import Path
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[2]
PACKAGE = ROOT / 'bindings/dart/window_manager'
EXAMPLE = PACKAGE / 'example'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--published', help='Exact nativeapi version for the baseline')
    parser.add_argument('--flutter', default=shutil.which('flutter'))
    parser.add_argument('--label', help='Bundle directory name for SDK comparisons')
    parser.add_argument('--platform', choices=('windows', 'linux'), default='windows')
    args = parser.parse_args()
    assert args.flutter, 'Flutter is not on PATH'
    version = args.published
    assert version is None or all(c.isdigit() or c == '.' for c in version)
    overrides = 'dependency_overrides:\n'
    for name in ('nativeapi', 'nativeapi_flutter', 'cnativeapi'):
        overrides += (f'  {name}: {json.dumps(version)}\n' if version else
                      f'  {name}:\n    path: {json.dumps(str(ROOT / "bindings/dart" / name))}\n')
    target = EXAMPLE / f'lib/nativeapi_{args.platform}_reports.dart'
    runner = EXAMPLE / ('windows/runner/main.cpp' if args.platform == 'windows'
                        else 'linux/runner/my_application.cc')
    paths = [PACKAGE / 'pubspec_overrides.yaml', EXAMPLE / 'pubspec_overrides.yaml', target, runner]
    previous = {p: p.read_bytes() if p.exists() else None for p in paths}
    try:
        for p in paths[:2]:
            p.write_text(overrides)
        target.write_bytes((ROOT / f'tools/gui/window_manager_{args.platform}_reports.dart.in').read_bytes())
        runner_text = runner.read_text()
        if args.platform == 'windows':
            marker = '  FlutterWindow window(project);'
            assert runner_text.count(marker) == 1
            runner_text = '#include <iostream>\n' + runner_text
            runner.write_text(runner_text.replace(marker, '''  wchar_t report_policy[16]{};
  GetEnvironmentVariableW(L"NATIVEAPI_REPORT_THREAD_POLICY", report_policy, 16);
  project.set_ui_thread_policy(std::wstring(report_policy) == L"Merged"
      ? flutter::UIThreadPolicy::RunOnPlatformThread
      : flutter::UIThreadPolicy::RunOnSeparateThread);
  std::cout << "PLATFORM_THREAD " << GetCurrentThreadId() << std::endl;
''' + marker))
        else:
            marker = '  // Use a header bar'
            assert runner_text.count(marker) == 1
            runner.write_text(runner_text.replace(marker, '''  if (g_getenv("NATIVEAPI_REPORT_HIGH_PRIORITY_THEME")) {
    g_autoptr(GtkCssProvider) provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, "window {background:#eff0f1;}", -1, nullptr);
    gtk_style_context_add_provider_for_screen(gtk_widget_get_screen(GTK_WIDGET(window)),
        GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER + 1);
  }

''' + marker))
        for command in ([args.flutter, 'pub', 'get'],
                        [args.flutter, 'analyze', '--no-pub', str(target)],
                        [args.flutter, 'build', args.platform, '--debug', '--target', str(target)]):
            subprocess.run(command, cwd=EXAMPLE, check=True)
        source = EXAMPLE / ('build/windows/x64/runner/Debug' if args.platform == 'windows'
                            else 'build/linux/x64/debug/bundle')
        label = args.label or version or 'local'
        assert all(c.isalnum() or c in '.-_' for c in label)
        destination = EXAMPLE / f'build/{args.platform}-reports' / label
        shutil.copytree(source, destination, dirs_exist_ok=True)
        print('BUNDLE', destination, flush=True)
    finally:
        for p, content in previous.items():
            if content is None:
                p.unlink(missing_ok=True)
            else:
                p.write_bytes(content)


if __name__ == '__main__':
    main()
