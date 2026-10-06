// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

// ignore_for_file: unused_import, unnecessary_import

import 'unsupported.dart';

enum UrlOpenErrorCode {
  none(0),
  invalidUrlEmpty(1),
  invalidUrlMissingScheme(2),
  invalidUrlUnsupportedScheme(3),
  unsupportedPlatform(4),
  invocationFailed(5);

  const UrlOpenErrorCode(this.value);
  final int value;

  static UrlOpenErrorCode fromValue(int value) => switch (value) {
    0 => UrlOpenErrorCode.none,
    1 => UrlOpenErrorCode.invalidUrlEmpty,
    2 => UrlOpenErrorCode.invalidUrlMissingScheme,
    3 => UrlOpenErrorCode.invalidUrlUnsupportedScheme,
    4 => UrlOpenErrorCode.unsupportedPlatform,
    5 => UrlOpenErrorCode.invocationFailed,
    _ => UrlOpenErrorCode.none,
  };
}

class UrlOpenResult {
  const UrlOpenResult({
    required this.success,
    required this.errorCode,
    required this.errorMessage,
  });

  final bool success;
  final UrlOpenErrorCode errorCode;
  final String? errorMessage;

  @override
  bool operator ==(Object other) =>
      identical(this, other) ||
      (other is UrlOpenResult &&
          other.success == success &&
          other.errorCode == errorCode &&
          other.errorMessage == errorMessage);

  @override
  int get hashCode => Object.hash(success, errorCode, errorMessage);

  @override
  String toString() =>
      'UrlOpenResult(success: $success, errorCode: $errorCode, errorMessage: $errorMessage)';
}

class UrlOpener {
  const UrlOpener._();

  static const UrlOpener instance = UrlOpener._();

  bool isSupported() => unsupported();

  bool canOpen(String url) => unsupported();

  UrlOpenResult open(String url) => unsupported();
}
