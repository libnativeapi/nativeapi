import 'dart:async';
import 'dart:convert';
import 'dart:io';

import 'package:file_selector/file_selector.dart';
import 'package:flutter/foundation.dart';
import 'package:nativeapi/nativeapi.dart' as na;

class CaptureArtifact {
  const CaptureArtifact(this.path, {this.duration, this.width, this.height});
  final String path;
  final double? duration;
  final int? width;
  final int? height;
  bool get isVideo => duration != null;
  String get name => path.split(Platform.pathSeparator).last;
}

/// Owns native handles explicitly. UI changes never alter an in-flight request.
class CaptureController extends ChangeNotifier {
  final artifacts = <CaptureArtifact>[];
  final log = <String>[];
  List<na.Display> displays = [];
  na.Window? _window;
  na.CaptureSource? _source;
  na.Image? _image;
  na.ScreenRecorder? _recorder;
  int? _recordingListener, _closeListener, _quitListener;
  Timer? _timer;
  final _elapsed = Stopwatch();
  bool _disposed = false;
  bool ready = false;
  bool busy = false;
  bool captureSupported = false;
  bool recordingSupported = false;
  bool pickerSupported = false;
  String sourceLabel = 'No source selected';
  String outputDirectory = '';
  String status = 'Choose a display, this window, or the system picker.';
  String? error;
  Uint8List? preview;
  int imageWidth = 0, imageHeight = 0;
  na.ScreenRecordingState recordingState = na.ScreenRecordingState.idle;

  bool get hasSource => _source != null;
  bool get hasImage => _image != null;
  bool get active => switch (recordingState) {
    na.ScreenRecordingState.starting ||
    na.ScreenRecordingState.recording ||
    na.ScreenRecordingState.stopping => true,
    _ => false,
  };
  bool get locked => busy || active || !ready;
  double get elapsed => _elapsed.elapsedMilliseconds / 1000;

  void _changed() {
    if (!_disposed) notifyListeners();
  }

  void _message(String message, {bool failure = false}) {
    status = message;
    error = failure ? message : null;
    log.insert(0, message);
    if (log.length > 30) log.removeLast();
    debugPrint('[capture] $message');
    _changed();
  }

  Future<void> initialize() async {
    try {
      final directory = await Directory.systemTemp.createTemp(
        'nativeapi-capture-',
      );
      if (_disposed) return;
      outputDirectory = directory.path;
      _window = na.WindowManager.instance.getCurrent();
      _window?.contentSize = const na.Size(width: 1120, height: 800);
      captureSupported = na.ScreenCapturer.isSupported();
      recordingSupported = na.ScreenRecorder.isSupported();
      pickerSupported = na.ScreenCapturer.isSourceSelectionSupported();
      displays = na.DisplayManager.instance.getAll();
      _closeListener = _window?.addListener((event) {
        if (event is na.WindowCloseRequestedEvent && (busy || active)) {
          event.request.cancel();
          _message(
            'Finish the current action, or stop / discard the recording before closing.',
          );
        }
      });
      _quitListener = na.Application.instance.addListener((event) {
        if (event is na.ApplicationQuitRequestedEvent && (busy || active)) {
          event.request.cancel();
          _message('Stop and save, or discard the recording before quitting.');
        }
      });
      ready = true;
      _changed();
    } catch (e) {
      _message('Initialization failed: $e', failure: true);
    }
  }

  void _replaceSource(na.CaptureSource? next) {
    if (next == null) {
      _message(
        'This source is unavailable. Select it again or try the system picker.',
        failure: true,
      );
      return;
    }
    _source?.dispose();
    _source = next;
    final name = next.name;
    sourceLabel = name == null || name.isEmpty
        ? 'Selected ${next.type.name} source'
        : name;
    _message('Source selected: $sourceLabel');
  }

  void selectDisplay(na.Display display) {
    if (locked) return;
    _replaceSource(na.CaptureSource.fromDisplay(display));
  }

  void selectThisWindow() {
    if (locked) return;
    _replaceSource(na.CaptureSource.fromWindow(_window));
  }

  void releaseSource() {
    if (locked) return;
    _source?.dispose();
    _source = null;
    sourceLabel = 'No source selected';
    _message('Source released. Any retained sharing session can now close.');
  }

  Future<void> selectSource() async {
    if (locked || !pickerSupported) return;
    busy = true;
    _message('Choose one screen or window in the system picker.');
    try {
      final result = Completer<(bool, na.CaptureSource?)>();
      na.ScreenCapturer.selectSource(
        na.CaptureSourceSelectionOptions(
          allowDisplays: true,
          allowWindows: true,
          parentWindow: _window,
        ),
        (success, source) => result.complete((success, source)),
      );
      final (success, source) = await result.future;
      if (_disposed) {
        source?.dispose();
        return;
      }
      if (!success) {
        source?.dispose();
        _message(
          'Source selection failed. Check system permission and try again.',
          failure: true,
        );
      } else if (source == null) {
        _message('Selection cancelled. The previous source is unchanged.');
      } else {
        _replaceSource(source);
      }
    } catch (e) {
      _message('Source selection failed: $e', failure: true);
    } finally {
      busy = false;
      _changed();
    }
  }

  Future<void> capture(na.ScreenCaptureOptions options) async {
    if (locked || _source == null) return;
    busy = true;
    _message('Capturing a new frame…');
    try {
      final result = Completer<(bool, na.Image?)>();
      na.ScreenCapturer.capture(
        _source,
        options,
        (success, image) => result.complete((success, image)),
      );
      final (success, image) = await result.future;
      if (_disposed) {
        image?.dispose();
        return;
      }
      if (!success || image == null) {
        image?.dispose();
        throw StateError(
          'No frame received. Check screen-recording permission, source availability and crop bounds.',
        );
      }
      try {
        final encoded = image.toBase64();
        if (encoded == null) {
          throw StateError('Cannot convert the captured image for preview.');
        }
        // Image.ToBase64 returns a data URI, including its MIME type.
        final bytes = encoded.startsWith('data:')
            ? UriData.parse(encoded).contentAsBytes()
            : base64Decode(encoded);
        imageWidth = image.size.width.toInt();
        imageHeight = image.size.height.toInt();
        _image?.dispose();
        _image = image;
        preview = bytes;
      } catch (_) {
        image.dispose();
        rethrow;
      }
      _message('Screenshot ready · $imageWidth × $imageHeight px');
    } catch (e) {
      _message('Screenshot failed: $e', failure: true);
    } finally {
      busy = false;
      _changed();
    }
  }

  Future<String> _newPath(String extension) async {
    if (outputDirectory.isEmpty || !await Directory(outputDirectory).exists()) {
      throw StateError('Choose an existing output folder.');
    }
    final path =
        '$outputDirectory${Platform.pathSeparator}capture-${DateTime.now().microsecondsSinceEpoch}.$extension';
    if (await FileSystemEntity.type(path) != FileSystemEntityType.notFound) {
      throw StateError(
        'The output already exists. Try again with a new filename.',
      );
    }
    return path;
  }

  Future<void> saveScreenshot() async {
    if (locked || _image == null) return;
    busy = true;
    _changed();
    try {
      final path = await _newPath('png');
      if (_disposed) return;
      if (!_image!.saveToFile(path)) {
        throw StateError('PNG write failed. Check folder permissions.');
      }
      artifacts.insert(
        0,
        CaptureArtifact(path, width: imageWidth, height: imageHeight),
      );
      _message('PNG saved: $path');
    } catch (e) {
      _message('Save failed: $e', failure: true);
    } finally {
      busy = false;
      _changed();
    }
  }

  Future<void> chooseOutputDirectory() async {
    if (locked) return;
    busy = true;
    _changed();
    try {
      final path = await getDirectoryPath(confirmButtonText: 'Use this folder');
      if (_disposed) return;
      if (path != null) {
        outputDirectory = path;
        _message('Output folder: $path');
      }
    } catch (e) {
      _message('Cannot select a folder: $e', failure: true);
    } finally {
      busy = false;
      _changed();
    }
  }

  Future<void> start(na.ScreenRecordingOptions options) async {
    if (locked || _source == null || !recordingSupported) return;
    busy = true;
    _changed();
    try {
      final path = await _newPath('mp4');
      if (_disposed) return;
      final recorder = na.ScreenRecorder.create();
      if (recorder == null) throw StateError('Cannot create a recorder.');
      _recorder = recorder;
      _recordingListener = recorder.addListener((event) {
        if (_disposed || !identical(_recorder, recorder)) return;
        if (event is na.ScreenRecordingStartedEvent) {
          // Queued Started must not turn an already-cancelled session back on.
          recordingState = recorder.state;
          if (recordingState == na.ScreenRecordingState.recording) {
            _elapsed.start();
            _timer = Timer.periodic(
              const Duration(milliseconds: 200),
              (_) => _changed(),
            );
            _message('Recording · MP4 / H.264 · no audio');
          }
        } else if (event is na.ScreenRecordingCompletedEvent) {
          recordingState = na.ScreenRecordingState.completed;
          final saved = event.filePath;
          if (saved != null) {
            artifacts.insert(
              0,
              CaptureArtifact(saved, duration: event.duration),
            );
          }
          _message(
            'Recording saved · ${event.duration.toStringAsFixed(1)} s · $saved',
          );
          _releaseRecorder();
        } else if (event is na.ScreenRecordingCancelledEvent) {
          recordingState = na.ScreenRecordingState.cancelled;
          _message('Recording discarded. No video file was saved.');
          _releaseRecorder();
        } else if (event is na.ScreenRecordingFailedEvent) {
          recordingState = na.ScreenRecordingState.failed;
          _message(
            'Recording failed: ${event.message ?? 'Unknown native failure'}',
            failure: true,
          );
          _releaseRecorder();
        }
        _changed();
      });
      if (_recordingListener == 0) {
        throw StateError('Cannot subscribe to recording events.');
      }
      _elapsed.reset();
      recordingState = na.ScreenRecordingState.starting;
      if (!recorder.start(_source, path, options)) {
        recordingState = na.ScreenRecordingState.idle;
        throw StateError(
          'Start rejected. Check the source, options and output path.',
        );
      }
      _message('Starting… waiting for permission and the first encoded frame.');
    } catch (e) {
      _releaseRecorder();
      recordingState = na.ScreenRecordingState.failed;
      _message('Cannot start recording: $e', failure: true);
    } finally {
      busy = false;
      _changed();
    }
  }

  void stop() {
    if (_recorder?.stop() != true) return;
    recordingState = na.ScreenRecordingState.stopping;
    _elapsed.stop();
    _message('Saving… waiting for the MP4 to be finalized.');
  }

  void cancel() {
    if (_recorder?.cancel() != true) return;
    recordingState = na.ScreenRecordingState.stopping;
    _elapsed.stop();
    _message('Discarding… waiting for temporary output cleanup.');
  }

  void openPath(String path, {bool directory = false}) {
    final uri = directory ? Uri.directory(path) : Uri.file(path);
    final result = na.UrlOpener.instance.open(uri.toString());
    if (!result.success) {
      _message('Cannot open $path: ${result.errorMessage}', failure: true);
    }
  }

  void reportInvalidOptions(String message) => _message(message, failure: true);

  void _releaseRecorder() {
    _timer?.cancel();
    _timer = null;
    _elapsed.stop();
    if (_recordingListener != null) {
      _recorder?.removeListener(_recordingListener!);
    }
    _recordingListener = null;
    _recorder?.dispose();
    _recorder = null;
  }

  @override
  void dispose() {
    _disposed = true;
    // Ordinary close is vetoed while active; forced teardown discards output.
    _releaseRecorder();
    if (_closeListener != null) _window?.removeListener(_closeListener!);
    if (_quitListener != null) {
      na.Application.instance.removeListener(_quitListener!);
    }
    _image?.dispose();
    _source?.dispose();
    for (final display in displays) {
      display.dispose();
    }
    _window?.dispose();
    super.dispose();
  }
}
