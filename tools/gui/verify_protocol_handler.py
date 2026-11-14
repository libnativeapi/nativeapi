#!/usr/bin/env python3
"""Build and verify the protocol_handler plugin with real OS URL delivery.

No desktop mouse/keyboard input or native callback injection. Run each platform serially:
  python3 tools/gui/verify_protocol_handler.py macos --build
  python3 tools/gui/verify_protocol_handler.py ios --build --device SIMULATOR_UUID
  python3 tools/gui/verify_protocol_handler.py android --build --device emulator-5554
  python tools/gui/verify_protocol_handler.py windows --build-only
Windows runtime verification must run in the interactive desktop session.
"""
import argparse
import json
import os
from pathlib import Path
import plistlib
import shutil
import subprocess
import time
import urllib.error
import urllib.request

ROOT = Path(__file__).resolve().parents[2]
PACKAGE = ROOT / 'bindings/dart/protocol_handler'
EXAMPLE = PACKAGE / 'example'
PORT = 49227
ANDROID_ID = 'dev.leanflutter.plugins.protocol_handler_example'


def run(command, **kwargs):
    print('RUN', subprocess.list2cmdline([str(x) for x in command]), flush=True)
    return subprocess.run(command, check=True, **kwargs)


def build(platform, check_package=True):
    overrides = 'dependency_overrides:\n' + ''.join(
        f'  {name}:\n    path: {json.dumps(str(ROOT / "bindings/dart" / name))}\n'
        for name in ('nativeapi', 'cnativeapi'))
    paths = [PACKAGE / 'pubspec_overrides.yaml', EXAMPLE / 'pubspec_overrides.yaml',
             EXAMPLE / 'lib/protocol_verification.dart']
    backups = {p: p.read_bytes() if p.exists() else None for p in paths}
    try:
        for path in paths[:2]:
            path.write_text(overrides)
        paths[2].write_bytes((ROOT / 'tools/gui/fixtures/protocol_handler_flutter.dart').read_bytes())
        flutter = shutil.which('flutter')
        if not flutter:
            raise RuntimeError('Flutter is not on PATH')
        run([flutter, 'pub', 'get'], cwd=PACKAGE)
        run([flutter, 'pub', 'get'], cwd=EXAMPLE)
        if check_package:
            run([flutter, 'analyze', '--no-pub'], cwd=PACKAGE)
            run([flutter, 'test', '--no-pub', '--reporter', 'expanded'], cwd=PACKAGE)
        command = [flutter, 'build', 'apk' if platform == 'android' else platform,
                   '--debug', '--no-pub', '-t', 'lib/protocol_verification.dart']
        if platform == 'ios':
            command += ['--simulator']
        if platform == 'android':
            command += ['--target-platform', 'android-arm64']
        run(command, cwd=EXAMPLE)
    finally:
        for path, data in backups.items():
            if data is None:
                path.unlink(missing_ok=True)
            else:
                path.write_bytes(data)


def state(action='state'):
    with urllib.request.urlopen(f'http://127.0.0.1:{PORT}/{action}', timeout=3) as response:
        return json.load(response)


def wait_for(predicate, timeout=30):
    deadline = time.monotonic() + timeout
    last = None
    while time.monotonic() < deadline:
        try:
            last = state()
            if predicate(last):
                return last
        except (OSError, urllib.error.URLError, TimeoutError):
            pass
        time.sleep(.2)
    raise AssertionError(f'Endpoint did not reach expected state: {last}')


def start_ios_prompt_helper(device, output):
    """XCTest handles only the named test app's URL prompt in this simulator."""
    project = output / 'prompt-helper'
    shutil.copytree(ROOT / 'tools/gui/fixtures/protocol_ios_prompt', project, dirs_exist_ok=True)
    run(['xcodegen', 'generate', '--spec', str(project / 'project.yml')])
    base = ['xcodebuild', '-project', str(project / 'ProtocolPrompt.xcodeproj'),
            '-scheme', 'ProtocolPrompt', '-destination', f'platform=iOS Simulator,id={device}',
            '-derivedDataPath', str(project / 'build'), '-parallel-testing-enabled', 'NO']
    with (output / 'prompt-build.log').open('w') as log:
        run(base + ['build-for-testing', '-quiet'], stdout=log, stderr=subprocess.STDOUT)
    with (output / 'prompt-test.log').open('w') as log:
        started = time.time()
        helper = subprocess.Popen(base + ['test-without-building'], stdout=log, stderr=subprocess.STDOUT)
    deadline = time.monotonic() + 90
    while time.monotonic() < deadline and helper.poll() is None:
        container = subprocess.run(['xcrun', 'simctl', 'get_app_container', device,
                                    'dev.libnativeapi.protocolPromptTests.xctrunner', 'data'],
                                   capture_output=True, text=True)
        if container.returncode == 0:
            documents = Path(container.stdout.strip()) / 'Documents'
            ready = documents / 'protocol-ready'
            if ready.exists() and ready.stat().st_mtime >= started:
                return helper, documents
        time.sleep(.5)
    helper.terminate()
    helper.wait(timeout=15)
    raise RuntimeError(f'iOS prompt helper did not start; see {output / "prompt-test.log"}')


def verify(args):
    platform = args.platform
    output = ROOT / 'tools/gui/output' / f'protocol-handler-{platform}'
    output.mkdir(parents=True, exist_ok=True)
    checks = []
    def check(name, condition, evidence=None):
        checks.append({'check': name, 'passed': bool(condition), 'evidence': evidence})
        print(('PASS ' if condition else 'FAIL ') + name, flush=True)
        if not condition:
            raise AssertionError(f'{name}: {evidence}')

    process = None
    app_pid = None
    registry_backup = None
    registry_existed = False
    bundle_id = None
    completed = False
    prompt_helper = None
    prompt_documents = None
    adb = ['adb', '-s', args.device] if args.device else ['adb']
    executable = None
    stop = None
    try:
        try:
            state()
        except (OSError, urllib.error.URLError):
            pass
        else:
            raise RuntimeError(f'Port {PORT} is already occupied; do not reuse another run')
        if platform == 'macos':
            bundle = EXAMPLE / 'build/macos/Build/Products/Debug/protocol_handler_example.app'
            executable = bundle / 'Contents/MacOS/protocol_handler_example'
            def launch(url=None):
                run(['open', '-g', '-a', str(bundle)] + ([url] if url else []))
            def stop():
                if app_pid:
                    os.kill(app_pid, 15)
        elif platform == 'ios':
            if not args.device:
                raise ValueError('--device must name a booted disposable iOS simulator')
            bundle = EXAMPLE / 'build/ios/iphonesimulator/Runner.app'
            bundle_id = plistlib.loads((bundle / 'Info.plist').read_bytes())['CFBundleIdentifier']
            run(['xcrun', 'simctl', 'install', args.device, str(bundle)])
            if args.confirm_ios_prompts:
                prompt_helper, prompt_documents = start_ios_prompt_helper(args.device, output)
            def launch(url=None):
                run(['xcrun', 'simctl', 'openurl', args.device, url] if url else
                    ['xcrun', 'simctl', 'launch', args.device, bundle_id])
            def stop():
                subprocess.run(['xcrun', 'simctl', 'terminate', args.device, bundle_id], check=False)
        elif platform == 'android':
            if not args.device:
                raise ValueError('--device must name a disposable Android emulator')
            run(adb + ['install', '-r', str(EXAMPLE / 'build/app/outputs/flutter-apk/app-debug.apk')])
            run(adb + ['forward', f'tcp:{PORT}', f'tcp:{PORT}'])
            def launch(url=None):
                if url:
                    # Separate adb shell arguments are re-parsed by Android's shell.
                    import shlex
                    run(adb + ['shell', 'am start -W -a android.intent.action.VIEW '
                               '-c android.intent.category.BROWSABLE -d ' + shlex.quote(url) +
                               ' ' + ANDROID_ID])
                else:
                    run(adb + ['shell', 'am', 'start', '-W', '-n', ANDROID_ID + '/.MainActivity'])
            def stop():
                run(adb + ['shell', 'am', 'force-stop', ANDROID_ID])
        else:
            if os.name != 'nt':
                raise ValueError('Windows must run on the Windows host')
            executable = EXAMPLE / 'build/windows/x64/runner/Debug/protocol_handler_example.exe'
            import winreg
            try:
                with winreg.OpenKey(winreg.HKEY_CURRENT_USER, r'Software\Classes\myprotocol'):
                    registry_existed = True
            except FileNotFoundError:
                pass
            backup = output / 'myprotocol-before.reg'
            if registry_existed:
                run(['reg', 'export', r'HKCU\Software\Classes\myprotocol', str(backup), '/y'])
            registry_backup = backup
            def launch(url=None):
                nonlocal process
                if url:
                    os.startfile(url)
                else:
                    process = subprocess.Popen([str(executable)])
            def stop():
                if app_pid:
                    run(['taskkill', '/PID', str(app_pid), '/F'])

        # First start registers Windows and checks absence of a launch URL.
        launch()
        s = wait_for(lambda s: s['listeners'] == 1, 60)
        app_pid = s['pid']
        check('normal launch has no initial URL', s['initial'] is None, s)
        check('declared scheme registration succeeds', s['registered'] is True)
        check('invalid scheme is rejected', s['invalid'] is False)
        stop()
        app_pid = None
        time.sleep(2)

        cold = 'myprotocol://verification/cold?value=hello%20world&emoji=%F0%9F%98%82'
        launch(cold)
        s = wait_for(lambda s: s['initial'] == cold, 60)
        app_pid = s['pid']
        check('OS cold launch preserves complete URL', s['initial'] == cold, s)
        cold_pid = app_pid
        # Launch delivery may precede the first read or arrive late as an event.
        if s['initialAtRead'] is None:
            s = wait_for(lambda s: cold in s['listener'] and cold in s['stream'])
            check('late launch URL reaches listener and stream', True, s)
        else:
            check('early launch URL is available on first read', s['initialAtRead'] == cold, s)
        time.sleep(.5)
        s = state()
        check('launch URL is not duplicated', s['listener'].count(cold) <= 1 and s['stream'].count(cold) <= 1)
        s = state('snapshot')
        check('listeners returns an independent snapshot', s['listeners'] == 1 and s['hasListeners'])

        warm = 'myprotocol://verification/warm?value=%E4%B8%AD%E6%96%87&n=2'
        launch(warm)
        s = wait_for(lambda s: warm in s['listener'] and warm in s['stream'])
        time.sleep(.5)
        s = state()
        check('running app receives URL once in listener and stream',
              s['listener'].count(warm) == 1 and s['stream'].count(warm) == 1, s)
        check('warm delivery keeps original process and initial URL', s['pid'] == cold_pid and s['initial'] == cold)
        launch(warm)
        s = wait_for(lambda s: s['listener'].count(warm) == 2 and s['stream'].count(warm) == 2)
        check('opening the same URL again is a new activation', True, s)

        state('remove-listener')
        only_stream = 'myprotocol://verification/stream-only'
        launch(only_stream)
        s = wait_for(lambda s: only_stream in s['stream'])
        check('removed listener is silent while stream remains active',
              only_stream not in s['listener'] and not s['hasListeners'] and s['listeners'] == 0, s)
        state('stop')
        before = state()
        queued = ['myprotocol://verification/queued-1', 'myprotocol://verification/queued-2']
        for url in queued:
            launch(url)
            time.sleep(.5)
        s = state()
        check('all subscriptions cancelled: no Dart delivery',
              s['listener'] == before['listener'] and s['stream'] == before['stream'], s)
        state('listen')
        s = wait_for(lambda s: all(u in s['listener'] and u in s['stream'] for u in queued))
        check('resubscription replays queued URLs once and in order',
              s['listener'][-2:] == queued and s['stream'][-2:] == queued and
              all(s['listener'].count(u) == s['stream'].count(u) == 1 for u in queued), s)
        check('initial URL remains stable after resubscription', s['initial'] == cold)
        state('stop')
        completed = True
    finally:
        if stop and (app_pid or platform in ('ios', 'android')):
            stop()
        if prompt_helper:
            (prompt_documents / 'protocol-stop').touch()
            try:
                if prompt_helper.wait(timeout=30) != 0:
                    raise RuntimeError(f'iOS prompt helper failed; see {output / "prompt-test.log"}')
            except subprocess.TimeoutExpired:
                prompt_helper.terminate()
                prompt_helper.wait(timeout=15)
                raise
        if platform == 'windows' and registry_backup is not None:
            subprocess.run(['reg', 'delete', r'HKCU\Software\Classes\myprotocol', '/f'], capture_output=True)
            if registry_existed:
                run(['reg', 'import', str(registry_backup)])
        if platform == 'android' and args.device:
            subprocess.run(adb + ['forward', '--remove', f'tcp:{PORT}'], check=False)
        (output / 'results.json').write_text(json.dumps({
            'platform': platform, 'completed': completed, 'checks': checks,
        }, indent=2) + '\n')
    print(f'PASS {platform}: {len(checks)} checks; {output / "results.json"}', flush=True)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('platform', choices=['macos', 'ios', 'android', 'windows'])
    parser.add_argument('--build', action='store_true')
    parser.add_argument('--build-only', action='store_true')
    parser.add_argument('--skip-package-checks', action='store_true',
                        help='Reuse successful package analysis/tests when rebuilding a platform fixture')
    parser.add_argument('--device')
    parser.add_argument('--confirm-ios-prompts', action='store_true',
                        help='Use XCTest to accept only test-app URL prompts in the dedicated simulator (requires XcodeGen)')
    args = parser.parse_args()
    if args.build or args.build_only:
        build(args.platform, check_package=not args.skip_package_checks)
    if not args.build_only:
        verify(args)
