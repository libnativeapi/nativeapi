// GUI-test fixture for leanflutter/window_manager, built as an alternative
// entry point of its example: the runner copies it to
// example/lib/gui_test_main.dart and builds with `-t lib/gui_test_main.dart`.
//
// It drives window_manager the way 0.5.x apps do (legacy.dart) inside the
// package's own WindowCaption and VirtualWindowFrame, and prints what the
// runner asserts on as texts the UI probe reads:
//
//   closeReports N      onWindowClose calls
//   events a,b,c        the last legacy events, without moves and resizes
//   maximized B / minimized B / fullScreen B
//   hovers N / clicks N on the TARGET area
//   ignoring B          setIgnoreMouseEvents is on
//
// Minimizing restores itself after 2 s, so a minimize can be undone without
// any input. Nothing here takes keyboard input.
// ignore_for_file: deprecated_member_use

import 'dart:async';

import 'package:flutter/material.dart';
import 'package:window_manager/legacy.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  await windowManager.ensureInitialized();
  await windowManager.waitUntilReadyToShow(
    const WindowOptions(
      size: Size(760, 560),
      center: true,
      title: 'wm gui test',
      titleBarStyle: TitleBarStyle.hidden,
    ),
    () async {
      await windowManager.show();
      await windowManager.focus();
    },
  );
  runApp(const MaterialApp(home: Fixture()));
}

class Fixture extends StatefulWidget {
  const Fixture({super.key});

  @override
  State<Fixture> createState() => _FixtureState();
}

class _FixtureState extends State<Fixture> with WindowListener {
  int closeReports = 0;
  int hovers = 0;
  int clicks = 0;
  bool hiddenTitleBar = true;
  bool preventClose = false;
  bool ignoring = false;
  bool maximized = false;
  bool minimized = false;
  bool fullScreen = false;
  final List<String> events = [];

  @override
  void initState() {
    super.initState();
    windowManager.addListener(this);
  }

  @override
  void dispose() {
    windowManager.removeListener(this);
    super.dispose();
  }

  Future<void> _refresh() async {
    final m = await windowManager.isMaximized();
    final n = await windowManager.isMinimized();
    final f = await windowManager.isFullScreen();
    if (!mounted) return;
    setState(() {
      maximized = m;
      minimized = n;
      fullScreen = f;
    });
  }

  @override
  void onWindowEvent(String eventName) {
    // Moves and resizes come in bursts and would push the rest out.
    const bursts = {'move', 'moved', 'resize', 'resized'};
    if (!bursts.contains(eventName)) {
      setState(() {
        events.add(eventName);
        if (events.length > 12) events.removeAt(0);
      });
    }
    _refresh();
    if (eventName == 'minimize') {
      Timer(const Duration(seconds: 2), () => windowManager.restore());
    }
  }

  @override
  void onWindowClose() {
    setState(() => closeReports++);
  }

  Future<void> _setHiddenTitleBar(bool hidden) async {
    await windowManager.setTitleBarStyle(
      hidden ? TitleBarStyle.hidden : TitleBarStyle.normal,
    );
    setState(() => hiddenTitleBar = hidden);
  }

  Future<void> _setPreventClose(bool value) async {
    await windowManager.setPreventClose(value);
    setState(() => preventClose = value);
  }

  void _menuSoon() {
    Timer(const Duration(milliseconds: 1500), windowManager.popUpWindowMenu);
  }

  Future<void> _ignoreForAWhile() async {
    await windowManager.setIgnoreMouseEvents(true, forward: true);
    setState(() => ignoring = true);
    Timer(const Duration(seconds: 8), () async {
      await windowManager.setIgnoreMouseEvents(false);
      if (mounted) setState(() => ignoring = false);
    });
  }

  Widget _chip(String label, VoidCallback onTap) {
    return Padding(
      padding: const EdgeInsets.all(4),
      child: GestureDetector(
        onTap: onTap,
        child: Container(
          padding: const EdgeInsets.symmetric(horizontal: 10, vertical: 6),
          color: const Color(0xFFDDE3F0),
          child: Text(label),
        ),
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    final body = Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        if (hiddenTitleBar)
          const SizedBox(
            height: kWindowCaptionHeight,
            child: WindowCaption(
              brightness: Brightness.light,
              title: Text('WM GUI caption'),
            ),
          ),
        Wrap(
          children: [
            _chip('Prevent close on', () => _setPreventClose(true)),
            _chip('Prevent close off', () => _setPreventClose(false)),
            _chip('Native title bar', () => _setHiddenTitleBar(false)),
            _chip('Hidden title bar', () => _setHiddenTitleBar(true)),
            _chip('Menu in 1.5s', _menuSoon),
            _chip('Ignore mouse 8s', _ignoreForAWhile),
            _chip('destroy()', windowManager.destroy),
          ],
        ),
        Text('closeReports $closeReports'),
        Text('events ${events.join(',')}'),
        Text('maximized $maximized'),
        Text('minimized $minimized'),
        Text('fullScreen $fullScreen'),
        Text('preventClose $preventClose'),
        Text('ignoring $ignoring'),
        Text('hovers $hovers'),
        Text('clicks $clicks'),
        Expanded(
          child: MouseRegion(
            onHover: (_) => setState(() => hovers++),
            child: GestureDetector(
              onTap: () => setState(() => clicks++),
              child: Container(
                margin: const EdgeInsets.all(12),
                color: const Color(0xFFF2D7A6),
                alignment: Alignment.center,
                child: const Text('TARGET'),
              ),
            ),
          ),
        ),
      ],
    );
    return Material(
      color: Colors.white,
      child: VirtualWindowFrame(child: body),
    );
  }
}
