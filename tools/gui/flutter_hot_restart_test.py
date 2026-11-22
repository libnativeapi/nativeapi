#!/usr/bin/env python3
"""Exercise Windows GetCurrent across actual Flutter hot restarts, then close.

Run locally with --host windows-laptop (fastforge forwards the VM service), or
on Windows without --host. No synthetic mouse or keyboard input is sent.
The app uses flutter_window_example's unmodified runner and local bindings.
"""
import argparse
import json
from pathlib import Path
import queue
import re
import subprocess
import threading
import time
import urllib.parse
import urllib.request
import urllib.error

ROOT = Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--host')
    parser.add_argument('--restarts', type=int, default=3)
    parser.add_argument('--timeout', type=int, default=900)
    args = parser.parse_args()
    command = (['fastforge', '--no-version-check', 'run', '--host', args.host, '-p', 'windows']
               if args.host else ['flutter', '--no-version-check', 'run', '-d', 'windows'])
    command += ['-t', '../../tools/gui/window_hot_restart_flutter.dart']
    process = subprocess.Popen(command, cwd=ROOT / 'examples/flutter_window_example',
                               stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                               stderr=subprocess.STDOUT, text=True, bufsize=1)
    lines = queue.Queue()

    def read_output():
        for line in process.stdout:
            lines.put(line)
        lines.put(None)

    threading.Thread(target=read_output, daemon=True).start()
    reports = []
    service = None
    restart_ready = False
    close_sent = False
    close_observed = False

    def rpc(method, **parameters):
        # fastforge prints a URL just before its SSH port forward starts.
        for attempt in range(50):
            try:
                with urllib.request.urlopen(service + 'getVM', timeout=10) as response:
                    vm = json.load(response)
                break
            except urllib.error.URLError:
                if attempt == 49:
                    raise
                time.sleep(.2)
        isolate = next(i['id'] for i in vm['result']['isolates'] if i['name'] == 'main')
        query = urllib.parse.urlencode({'isolateId': isolate, **parameters})
        with urllib.request.urlopen(service + method + '?' + query, timeout=10) as response:
            result = json.load(response)
        assert 'error' not in result, result
        return result['result']

    deadline = time.monotonic() + args.timeout
    try:
        while time.monotonic() < deadline:
            try:
                line = lines.get(timeout=1)
            except queue.Empty:
                line = ''
            if line is None:
                break
            print(line, end='', flush=True)
            match = re.search(r'(http://(?:127\.0\.0\.1|localhost):\d+/[^\s]*)', line)
            if match and ('VM Service' in line or 'VM service' in line):
                service = match[1].rstrip('/') + '/'
            # Do not send the next command before flutter run finishes attaching
            # or restarting, even if the new isolate has already printed main().
            if 'key commands' in line or 'Restarted application in' in line:
                restart_ready = True
            if service and restart_ready:
                report = rpc('ext.nativeapi.hotRestartState')
                assert report['id'] and report['hwnd'], f'GetCurrent returned null: {report}'
                if reports:
                    assert all(report[k] == reports[0][k] for k in ('pid', 'id', 'hwnd')), report
                    assert report['generation'] != reports[-1]['generation'], 'main() did not restart'
                reports.append(report)
                print('CHECK', json.dumps(report), flush=True)
                restart_ready = False
                if len(reports) <= args.restarts:
                    process.stdin.write('R')
                    process.stdin.flush()
                else:
                    assert rpc('ext.nativeapi.closeForTest')['requested']
                    close_sent = True
                    deadline = min(deadline, time.monotonic() + 30)
            if close_sent and not close_observed:
                state = rpc('ext.nativeapi.hotRestartState')
                if state['closeCount']:
                    assert state['closeCount'] == 1, state
                    close_observed = True
                    assert rpc('ext.nativeapi.closeForTest', allow='true')['requested']
        assert len(reports) == args.restarts + 1, f'Expected startup and {args.restarts} restarts: {reports}'
        assert close_sent and close_observed, 'The restarted isolate did not receive and approve native close'
        code = process.wait(timeout=15)
        assert code == 0, f'Flutter run exited with {code}'
        print(f'PASS startup, {args.restarts} hot restarts with stable HWND/id, vetoed close and clean exit')
    finally:
        if process.poll() is None:
            try:
                process.stdin.write('q')
                process.stdin.flush()
                process.wait(timeout=15)
            except (BrokenPipeError, subprocess.TimeoutExpired):
                process.terminate()
                process.wait(timeout=10)


if __name__ == '__main__':
    main()
