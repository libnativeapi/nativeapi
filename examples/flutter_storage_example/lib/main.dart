import 'package:dazzui_host/dazzui_host.dart';
import 'package:nativeapi_flutter/nativeapi_flutter.dart' as na;
import 'package:nativeapi_flutter/nativeapi_flutter.dart'
    show SizeToNative, WindowManager;

// Four stores side by side — Preferences and SecureStorage, each unscoped and
// scoped — in one desktop tool: pick a store in the sidebar, read and edit
// its entries in the table, run the test cases against it, and watch the
// calls in the log at the bottom.

void main() {
  runApp(const StorageExampleApp());
}

class StorageExampleApp extends StatelessWidget {
  const StorageExampleApp({super.key});

  @override
  Widget build(BuildContext context) =>
      const Host(title: 'Storage Example', home: StorageExamplePage());
}

/// The one surface the page needs from a store. [na.Preferences] and
/// [na.SecureStorage] have the same methods but no common type.
class _Store {
  _Store.preferences(na.Preferences this._prefs) : _secure = null;
  _Store.secure(na.SecureStorage this._secure) : _prefs = null;

  final na.Preferences? _prefs;
  final na.SecureStorage? _secure;

  bool get isSecure => _secure != null;

  bool set(String key, String value) =>
      _prefs != null ? _prefs.set(key, value) : _secure!.set(key, value);

  String? get(String key, String defaultValue) => _prefs != null
      ? _prefs.get(key, defaultValue)
      : _secure!.get(key, defaultValue);

  bool remove(String key) =>
      _prefs != null ? _prefs.remove(key) : _secure!.remove(key);

  bool contains(String key) =>
      _prefs != null ? _prefs.contains(key) : _secure!.contains(key);

  bool clear() => _prefs != null ? _prefs.clear() : _secure!.clear();

  List<String> get keys => _prefs != null ? _prefs.keys : _secure!.keys;

  int get size => _prefs != null ? _prefs.size : _secure!.size;

  Map<String, String> get all => _prefs != null ? _prefs.all : _secure!.all;
}

/// A store as the sidebar lists it. [id] is the name the log uses.
typedef _StoreInfo = ({String id, String group, String label, IconData icon});

const List<_StoreInfo> _storeInfos = [
  (
    id: 'preferences',
    group: 'Preferences',
    label: 'Default',
    icon: FluentIcons.settings_20_regular,
  ),
  (
    id: 'scoped_preferences',
    group: 'Preferences',
    label: 'user_settings',
    icon: FluentIcons.folder_20_regular,
  ),
  (
    id: 'secure_storage',
    group: 'SecureStorage',
    label: 'Default',
    icon: FluentIcons.lock_closed_20_regular,
  ),
  (
    id: 'scoped_secure_storage',
    group: 'SecureStorage',
    label: 'api_credentials',
    icon: FluentIcons.key_20_regular,
  ),
];

class StorageExamplePage extends StatefulWidget {
  const StorageExamplePage({super.key});

  @override
  State<StorageExamplePage> createState() => _StorageExamplePageState();
}

class _StorageExamplePageState extends State<StorageExamplePage> {
  // Storage instances (nullable in case initialization fails)
  na.Preferences? _preferences;
  na.Preferences? _scopedPreferences;
  na.SecureStorage? _secureStorage;
  na.SecureStorage? _scopedSecureStorage;

  // Event history, newest first
  final List<String> _eventHistory = [];
  String _lastEvent = 'No events yet';

  // UI state
  final TextEditingController _keyController = TextEditingController();

  String _selectedStorage = 'preferences';
  Map<String, String> _currentData = {};
  int _currentSize = 0;
  Map<String, int> _sizes = {};
  bool _isInitialized = false;
  bool _secureAvailable = false;

  @override
  void initState() {
    super.initState();
    _initializeStorage();
    // A desktop tool's size, whatever the runner's default is.
    WidgetsBinding.instance.addPostFrameCallback((_) {
      WindowManager.instance.getCurrent()?.contentSize = const Size(
        940,
        660,
      ).toNative();
    });
  }

  void _initializeStorage() {
    try {
      _preferences = na.Preferences.create()!;
      _scopedPreferences = na.Preferences.createWithScope('user_settings')!;
      _secureStorage = na.SecureStorage.create()!;
      _scopedSecureStorage = na.SecureStorage.createWithScope(
        'api_credentials',
      )!;
      _secureAvailable = na.SecureStorage.isAvailable();

      _isInitialized = true;
      _addToHistory('Storage instances initialized successfully');
      _refreshCurrentData();
    } catch (e) {
      _isInitialized = false;
      _addToHistory('Error initializing storage: $e');
    }
  }

  _Store? _storeFor(String id) {
    if (!_isInitialized) return null;
    return switch (id) {
      'scoped_preferences' => _Store.preferences(_scopedPreferences!),
      'secure_storage' => _Store.secure(_secureStorage!),
      'scoped_secure_storage' => _Store.secure(_scopedSecureStorage!),
      _ => _Store.preferences(_preferences!),
    };
  }

  _Store? _getCurrentStorage() => _storeFor(_selectedStorage);

  _StoreInfo get _currentInfo =>
      _storeInfos.firstWhere((info) => info.id == _selectedStorage);

  void _addToHistory(String message) {
    setState(() {
      final timestamp = DateTime.now().toString().substring(11, 19);
      _lastEvent = message;
      _eventHistory.insert(0, '[$timestamp] $message');
      if (_eventHistory.length > 100) {
        _eventHistory.removeLast();
      }
    });
  }

  void _clearHistory() {
    setState(() {
      _eventHistory.clear();
    });
    _addToHistory('Event history cleared');
  }

  void _toast(String title, {ToastTint tint = ToastTint.success}) {
    Toaster.of(context).add(
      ToastOptions(
        title: title,
        tint: tint,
        // Errors are feedback here, not something to acknowledge.
        timeout: tint == ToastTint.danger ? const Duration(seconds: 5) : null,
      ),
    );
  }

  void _refreshCurrentData() {
    setState(() {
      final storage = _getCurrentStorage();
      if (storage == null) {
        _currentData = {};
        _currentSize = 0;
        _sizes = {};
        return;
      }
      _currentData = storage.all;
      _currentSize = storage.size;
      _sizes = {
        for (final info in _storeInfos) info.id: _storeFor(info.id)!.size,
      };
    });
  }

  void _selectStorage(String value) {
    if (value == _selectedStorage) return;
    setState(() {
      _selectedStorage = value;
    });
    _addToHistory('Switched to: $value');
    _refreshCurrentData();
  }

  // Key-value operations

  bool _setKeyValue(String key, String value) {
    key = key.trim();
    if (key.isEmpty) {
      _addToHistory('Error: Key cannot be empty');
      return false;
    }

    final storage = _getCurrentStorage();
    if (storage == null) {
      _addToHistory('Error: Storage not initialized');
      return false;
    }

    final success = storage.set(key, value);

    if (success) {
      _addToHistory('Set [$_selectedStorage]: "$key" = "$value"');
      _refreshCurrentData();
      _toast('Saved "$key"');
    } else {
      _addToHistory('Error: Failed to set "$key"');
      _toast('Failed to set "$key"', tint: ToastTint.danger);
    }
    return success;
  }

  void _getValue() {
    final key = _keyController.text.trim();

    if (key.isEmpty) {
      _addToHistory('Error: Key cannot be empty');
      _toast('Type a key first', tint: ToastTint.warning);
      return;
    }

    final storage = _getCurrentStorage();
    if (storage == null) {
      _addToHistory('Error: Storage not initialized');
      return;
    }

    final value = storage.get(key, '(not found)');

    _addToHistory('Get [$_selectedStorage]: "$key" = "$value"');
    _toast('"$key" = "$value"', tint: ToastTint.info);
  }

  void _removeKey(String key, {bool fromLookup = false}) {
    key = key.trim();
    if (key.isEmpty) {
      _addToHistory('Error: Key cannot be empty');
      _toast('Type a key first', tint: ToastTint.warning);
      return;
    }

    final storage = _getCurrentStorage();
    if (storage == null) {
      _addToHistory('Error: Storage not initialized');
      return;
    }

    final previous = _currentData[key];
    final success = storage.remove(key);

    if (success) {
      _addToHistory('Remove [$_selectedStorage]: "$key" removed successfully');
      _refreshCurrentData();
      if (fromLookup) _keyController.clear();
      Toaster.of(context).add(
        ToastOptions(
          title: 'Removed "$key"',
          tint: ToastTint.success,
          action: previous == null
              ? null
              : ToastAction(
                  label: 'Undo',
                  onPressed: () {
                    if (mounted) _setKeyValue(key, previous);
                  },
                ),
        ),
      );
    } else {
      _addToHistory('Error: Failed to remove "$key" (may not exist)');
      _toast('Failed to remove "$key"', tint: ToastTint.danger);
    }
  }

  void _containsKey() {
    final key = _keyController.text.trim();

    if (key.isEmpty) {
      _addToHistory('Error: Key cannot be empty');
      _toast('Type a key first', tint: ToastTint.warning);
      return;
    }

    final storage = _getCurrentStorage();
    if (storage == null) {
      _addToHistory('Error: Storage not initialized');
      return;
    }

    final exists = storage.contains(key);

    _addToHistory('Contains [$_selectedStorage]: "$key" = $exists');
    _toast('Contains "$key": $exists', tint: ToastTint.info);
  }

  // Storage operations

  void _clearStorage() {
    final storage = _getCurrentStorage();
    if (storage == null) {
      _addToHistory('Error: Storage not initialized');
      return;
    }

    final success = storage.clear();

    if (success) {
      _addToHistory('Clear [$_selectedStorage]: All data cleared');
      _refreshCurrentData();
      _toast('Cleared $_selectedStorage');
    } else {
      _addToHistory('Error: Failed to clear storage');
      _toast('Failed to clear storage', tint: ToastTint.danger);
    }
  }

  void _listAllKeys() {
    final storage = _getCurrentStorage();
    if (storage == null) {
      _addToHistory('Error: Storage not initialized');
      return;
    }

    final keys = storage.keys;

    _addToHistory(
      'Keys [$_selectedStorage]: ${keys.length} keys - [${keys.join(', ')}]',
    );
  }

  void _getSize() {
    final storage = _getCurrentStorage();
    if (storage == null) {
      _addToHistory('Error: Storage not initialized');
      return;
    }

    final size = storage.size;

    _addToHistory('Size [$_selectedStorage]: $size items');
  }

  void _getAllData() {
    final storage = _getCurrentStorage();
    if (storage == null) {
      _addToHistory('Error: Storage not initialized');
      return;
    }

    final data = storage.all;

    _addToHistory('GetAll [$_selectedStorage]: ${data.length} items');
    data.forEach((key, value) {
      _addToHistory('  "$key" = "$value"');
    });
  }

  // Test case methods

  void _testBasicOperations() {
    _addToHistory('--- Starting Basic Operations Test ---');

    final storage = _getCurrentStorage();
    if (storage == null) {
      _addToHistory('Error: Storage not initialized');
      return;
    }

    // Set
    storage.set('test_key', 'test_value');
    _addToHistory('✓ Set test_key = test_value');

    // Get
    final value = storage.get('test_key', '');
    _addToHistory('✓ Get test_key = $value');

    // Contains
    final exists = storage.contains('test_key');
    _addToHistory('✓ Contains test_key = $exists');

    // Remove
    storage.remove('test_key');
    _addToHistory('✓ Removed test_key');

    // Contains after remove
    final stillExists = storage.contains('test_key');
    _addToHistory('✓ Contains after remove = $stillExists');

    _addToHistory('--- Basic Operations Test Complete ---');
    _refreshCurrentData();
  }

  void _testBulkOperations() {
    _addToHistory('--- Starting Bulk Operations Test ---');

    final storage = _getCurrentStorage();
    if (storage == null) {
      _addToHistory('Error: Storage not initialized');
      return;
    }

    // Add multiple items
    for (int i = 1; i <= 10; i++) {
      storage.set('bulk_key_$i', 'value_$i');
    }
    _addToHistory('✓ Added 10 items');

    // List all keys
    final keys = storage.keys;
    _addToHistory('✓ Total keys: ${keys.length}');

    // Get all data
    final data = storage.all;
    _addToHistory('✓ Retrieved all data: ${data.length} items');

    _addToHistory('--- Bulk Operations Test Complete ---');
    _refreshCurrentData();
  }

  void _testSpecialCharacters() {
    _addToHistory('--- Starting Special Characters Test ---');

    final storage = _getCurrentStorage();
    if (storage == null) {
      _addToHistory('Error: Storage not initialized');
      return;
    }

    final testCases = [
      ('emoji_key', '😀🎉🚀'),
      ('chinese_key', '你好世界'),
      ('special_chars', '@#\$%^&*()'),
      ('unicode_key', 'Héllo Wörld'),
      ('json_like', '{"name":"value","array":[1,2,3]}'),
    ];

    for (var (key, value) in testCases) {
      storage.set(key, value);
      final retrieved = storage.get(key, '');
      final match = retrieved == value ? '✓' : '✗';
      _addToHistory('$match "$key" = "$value" (retrieved: "$retrieved")');
    }

    _addToHistory('--- Special Characters Test Complete ---');
    _refreshCurrentData();
  }

  void _testDefaultValues() {
    _addToHistory('--- Starting Default Values Test ---');

    final storage = _getCurrentStorage();
    if (storage == null) {
      _addToHistory('Error: Storage not initialized');
      return;
    }

    // Test non-existent key with default
    final value1 = storage.get('non_existent_key', 'default_value');
    _addToHistory('✓ Get non-existent with default = "$value1"');

    // Test non-existent key without default
    final value2 = storage.get('another_non_existent_key', '');
    _addToHistory('✓ Get non-existent without default = "$value2"');

    _addToHistory('--- Default Values Test Complete ---');
  }

  void _testOverwriteValues() {
    _addToHistory('--- Starting Overwrite Values Test ---');

    final storage = _getCurrentStorage();
    if (storage == null) {
      _addToHistory('Error: Storage not initialized');
      return;
    }

    storage.set('overwrite_test', 'original_value');
    _addToHistory('✓ Set original value');

    final value1 = storage.get('overwrite_test', '');
    _addToHistory('✓ Retrieved: "$value1"');

    storage.set('overwrite_test', 'updated_value');
    _addToHistory('✓ Overwrote with new value');

    final value2 = storage.get('overwrite_test', '');
    _addToHistory('✓ Retrieved after overwrite: "$value2"');

    storage.remove('overwrite_test');
    _addToHistory('✓ Cleaned up test key');

    _addToHistory('--- Overwrite Values Test Complete ---');
    _refreshCurrentData();
  }

  void _testLargeData() {
    _addToHistory('--- Starting Large Data Test ---');

    final storage = _getCurrentStorage();
    if (storage == null) {
      _addToHistory('Error: Storage not initialized');
      return;
    }

    // Test with increasingly larger strings
    final sizes = [100, 1000, 10000];

    for (var size in sizes) {
      final largeValue = 'x' * size;
      final key = 'large_data_$size';

      final setSuccess = storage.set(key, largeValue);
      final retrieved = storage.get(key, '') ?? '';
      final match = retrieved.length == size;

      _addToHistory(
        '${match ? "✓" : "✗"} $size chars: set=$setSuccess, retrieved=${retrieved.length} chars',
      );

      storage.remove(key);
    }

    _addToHistory('--- Large Data Test Complete ---');
    _refreshCurrentData();
  }

  void _testEmptyValues() {
    _addToHistory('--- Starting Empty Values Test ---');

    final storage = _getCurrentStorage();
    if (storage == null) {
      _addToHistory('Error: Storage not initialized');
      return;
    }

    // Test empty string value
    storage.set('empty_value', '');
    final retrieved = storage.get('empty_value', 'default') ?? '';
    _addToHistory(
      '✓ Empty value retrieved: "$retrieved" (length: ${retrieved.length})',
    );

    // Test if key exists
    final exists = storage.contains('empty_value');
    _addToHistory('✓ Empty value key exists: $exists');

    storage.remove('empty_value');
    _addToHistory('✓ Cleaned up empty value test');

    _addToHistory('--- Empty Values Test Complete ---');
    _refreshCurrentData();
  }

  void _testScopedStorage() {
    _addToHistory('--- Starting Scoped Storage Test ---');

    if (!_isInitialized || _preferences == null || _scopedPreferences == null) {
      _addToHistory('Error: Storage not initialized');
      return;
    }

    // Test both regular and scoped preferences
    _preferences!.set('shared_key', 'regular_value');
    _scopedPreferences!.set('shared_key', 'scoped_value');

    final regularValue = _preferences!.get('shared_key', '');
    final scopedValue = _scopedPreferences!.get('shared_key', '');

    _addToHistory('✓ Regular preferences: "$regularValue"');
    _addToHistory('✓ Scoped preferences: "$scopedValue"');
    _addToHistory('✓ Values are isolated: ${regularValue != scopedValue}');

    _preferences!.remove('shared_key');
    _scopedPreferences!.remove('shared_key');

    _addToHistory('--- Scoped Storage Test Complete ---');
    _refreshCurrentData();
  }

  void _compareStorageTypes() {
    _addToHistory('--- Starting Storage Types Comparison ---');

    if (!_isInitialized ||
        _preferences == null ||
        _scopedPreferences == null ||
        _secureStorage == null ||
        _scopedSecureStorage == null) {
      _addToHistory('Error: Storage not initialized');
      return;
    }

    const testKey = 'comparison_test';
    const testValue = 'test_value_123';

    // Test in all storage types
    _preferences!.set(testKey, testValue);
    _scopedPreferences!.set(testKey, testValue);
    _secureStorage!.set(testKey, testValue);
    _scopedSecureStorage!.set(testKey, testValue);

    _addToHistory('✓ Set value in all storage types');

    _addToHistory('Preferences size: ${_preferences!.size}');
    _addToHistory('Scoped Preferences size: ${_scopedPreferences!.size}');
    _addToHistory('SecureStorage size: ${_secureStorage!.size}');
    _addToHistory('Scoped SecureStorage size: ${_scopedSecureStorage!.size}');

    // Clean up
    _preferences!.remove(testKey);
    _scopedPreferences!.remove(testKey);
    _secureStorage!.remove(testKey);
    _scopedSecureStorage!.remove(testKey);

    _addToHistory('--- Storage Types Comparison Complete ---');
    _refreshCurrentData();
  }

  // Dialogs

  Future<void> _editEntry({String? key, String? value}) async {
    final result = await showDialog<(String, String)>(
      context: context,
      builder: (_) => _EntryDialog(
        storeName: '${_currentInfo.group} · ${_currentInfo.label}',
        initialKey: key,
        initialValue: value ?? '',
      ),
    );
    if (result == null || !mounted) return;
    _setKeyValue(result.$1, result.$2);
  }

  Future<void> _confirmClear() async {
    final confirmed = await showDialog<bool>(
      context: context,
      builder: (context) => Dialog(
        tone: DialogTone.danger,
        children: [
          DialogHeader(
            title: 'Clear $_selectedStorage?',
            subtitle: 'Removes all $_currentSize entries from this store.',
          ),
          DialogFooter(
            children: [
              const Spacer(),
              Button(
                variant: ButtonVariant.normal,
                tint: ButtonTint.neutral,
                onPressed: () => Navigator.of(context).pop(false),
                child: const Text('Cancel'),
              ),
              Button(
                variant: ButtonVariant.filled,
                tint: ButtonTint.danger,
                onPressed: () => Navigator.of(context).pop(true),
                child: const Text('Clear All'),
              ),
            ],
          ),
        ],
      ),
    );
    if (confirmed == true && mounted) _clearStorage();
  }

  // Layout

  @override
  Widget build(BuildContext context) {
    return Row(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        _sidebar(context.vars),
        Expanded(
          child: Column(
            children: [
              _toolbar(context.vars),
              const Divider(),
              OptionRow(
                label: 'Lookup',
                children: [
                  SizedBox(
                    width: 220,
                    child: TextField(
                      controller: _keyController,
                      size: WidgetSize.small,
                      mono: true,
                      placeholder: 'Key',
                      onSubmitted: (_) => _getValue(),
                    ),
                  ),
                  ActionChip(label: 'Get', onTap: _getValue),
                  ActionChip(label: 'Contains', onTap: _containsKey),
                  ActionChip(
                    label: 'Remove',
                    onTap: () =>
                        _removeKey(_keyController.text, fromLookup: true),
                  ),
                ],
              ),
              OptionRow(
                label: 'Inspect',
                children: [
                  ActionChip(label: 'List Keys', onTap: _listAllKeys),
                  ActionChip(label: 'Get Size', onTap: _getSize),
                  ActionChip(label: 'Get All', onTap: _getAllData),
                ],
              ),
              OptionRow(
                label: 'Tests',
                children: [
                  ActionChip(
                    label: 'Basic Operations',
                    onTap: _testBasicOperations,
                  ),
                  ActionChip(
                    label: 'Bulk Operations',
                    onTap: _testBulkOperations,
                  ),
                  ActionChip(
                    label: 'Special Chars',
                    onTap: _testSpecialCharacters,
                  ),
                  ActionChip(
                    label: 'Default Values',
                    onTap: _testDefaultValues,
                  ),
                  ActionChip(label: 'Overwrite', onTap: _testOverwriteValues),
                  ActionChip(label: 'Large Data', onTap: _testLargeData),
                  ActionChip(label: 'Empty Values', onTap: _testEmptyValues),
                  ActionChip(
                    label: 'Scoped Storage',
                    onTap: _testScopedStorage,
                  ),
                  ActionChip(
                    label: 'Compare Types',
                    onTap: _compareStorageTypes,
                  ),
                ],
              ),
              Expanded(child: _entries(context.vars)),
              EventFooter(
                headline: _lastEvent,
                lines: _eventHistory,
                onClear: _clearHistory,
                visibleLines: 5,
                height: 132,
              ),
            ],
          ),
        ),
      ],
    );
  }

  Widget _sidebar(ThemeVariables vars) {
    NavItem item(_StoreInfo info) {
      final current = info.id == _selectedStorage;
      final count = _sizes[info.id];
      return NavItem(
        label: info.label,
        icon: info.icon,
        current: current,
        trailing: count == null
            ? null
            : Text(
                '$count',
                style: vars.labelSmall.copyWith(
                  color: current ? vars.colorOnAccent : vars.colorContentMuted,
                ),
              ),
        onPressed: () => _selectStorage(info.id),
      );
    }

    return Sidebar(
      width: 200,
      header: Row(
        spacing: vars.spacing2,
        children: [
          Icon(
            FluentIcons.database_20_regular,
            size: vars.iconLarge,
            color: vars.colorContentSecondary,
          ),
          Text('Storage', style: vars.titleSmall),
        ],
      ),
      children: [
        for (final group in ['Preferences', 'SecureStorage']) ...[
          if (group != 'Preferences') SizedBox(height: vars.spacing2),
          SidebarGroup(
            label: group,
            children: [
              for (final info in _storeInfos)
                if (info.group == group) item(info),
            ],
          ),
        ],
        SizedBox(height: vars.spacing2),
        SidebarCard(
          label: 'SecureStorage',
          children: [
            Badge(
              size: WidgetSize.small,
              variant: BadgeVariant.tinted,
              tint: _secureAvailable ? BadgeTint.success : BadgeTint.warning,
              child: Text(_secureAvailable ? 'Available' : 'Unavailable'),
            ),
            Text(
              'Keychain on macOS, Credential Manager on Windows, '
              'libsecret on Linux.',
              style: vars.captionSmall.copyWith(color: vars.colorContentMuted),
            ),
          ],
        ),
      ],
    );
  }

  Widget _toolbar(ThemeVariables vars) {
    final info = _currentInfo;
    final secure = _getCurrentStorage()?.isSecure ?? false;
    final scoped = info.id.startsWith('scoped_');
    return Container(
      height: vars.frameTitlebarSize,
      padding: EdgeInsets.symmetric(horizontal: vars.spacing4),
      child: Row(
        spacing: vars.spacing2,
        children: [
          Flexible(
            child: Text(
              info.group,
              maxLines: 1,
              overflow: TextOverflow.ellipsis,
              style: vars.titleMedium,
            ),
          ),
          Badge(
            size: WidgetSize.small,
            variant: BadgeVariant.outlined,
            tint: BadgeTint.neutral,
            child: Text(scoped ? 'scope: ${info.label}' : 'no scope'),
          ),
          if (secure)
            Badge(
              size: WidgetSize.small,
              variant: BadgeVariant.tinted,
              tint: BadgeTint.success,
              child: const Text('encrypted'),
            ),
          Text('$_currentSize items', style: vars.muted),
          const Spacer(),
          Tooltip(
            label: 'Refresh View',
            child: IconButton(
              icon: const Icon(FluentIcons.arrow_clockwise_20_regular),
              semanticsLabel: 'Refresh View',
              onPressed: _refreshCurrentData,
            ),
          ),
          Button(
            variant: ButtonVariant.outlined,
            tint: ButtonTint.danger,
            onPressed: _currentSize == 0 ? null : _confirmClear,
            child: const Text('Clear All'),
          ),
          Button(
            variant: ButtonVariant.filled,
            onPressed: _isInitialized ? () => _editEntry() : null,
            child: Row(
              mainAxisSize: MainAxisSize.min,
              spacing: vars.spacing1,
              children: const [
                Icon(FluentIcons.add_20_regular),
                Text('Add entry'),
              ],
            ),
          ),
        ],
      ),
    );
  }

  Widget _entries(ThemeVariables vars) {
    if (!_isInitialized) {
      return Padding(
        padding: EdgeInsets.all(vars.spacing4),
        child: Align(
          alignment: Alignment.topCenter,
          child: Callout(
            tint: CalloutTint.danger,
            icon: const Icon(FluentIcons.error_circle_20_regular),
            title: const Text('Storage not initialized'),
            message: Text(_lastEvent),
          ),
        ),
      );
    }

    if (_currentData.isEmpty) {
      return Center(
        child: EmptyState(
          title: 'No data in storage',
          actions: [
            Button(
              variant: ButtonVariant.normal,
              tint: ButtonTint.neutral,
              onPressed: () => _editEntry(),
              child: const Text('Add entry'),
            ),
            Button(
              variant: ButtonVariant.normal,
              tint: ButtonTint.neutral,
              onPressed: _testBulkOperations,
              child: const Text('Add 10 samples'),
            ),
          ],
        ),
      );
    }

    final entries = _currentData.entries.toList();
    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        TableHead(
          children: [
            const TableCell(head: true, width: 36, child: Text('#')),
            const TableCell(head: true, flex: 2, child: Text('Key')),
            const TableCell(head: true, flex: 3, child: Text('Value')),
            const TableCell(
              head: true,
              width: 72,
              align: TableCellAlign.end,
              child: Text('Actions'),
            ),
          ],
        ),
        Expanded(
          child: SingleChildScrollView(
            child: Table(
              children: [
                for (final (index, entry) in entries.indexed)
                  TableRow(
                    onPressed: () =>
                        _editEntry(key: entry.key, value: entry.value),
                    children: [
                      TableCell(
                        width: 36,
                        child: Text('${index + 1}', style: vars.mono),
                      ),
                      TableCell(
                        flex: 2,
                        child: Text(
                          entry.key,
                          maxLines: 1,
                          overflow: TextOverflow.ellipsis,
                          style: vars.labelLarge.copyWith(
                            color: vars.colorContent,
                          ),
                        ),
                      ),
                      TableCell(
                        flex: 3,
                        child: Text(
                          entry.value.isEmpty ? '(empty)' : entry.value,
                          maxLines: 1,
                          overflow: TextOverflow.ellipsis,
                          style: vars.mono.copyWith(
                            color: entry.value.isEmpty
                                ? vars.colorContentFaint
                                : vars.colorContentSecondary,
                          ),
                        ),
                      ),
                      TableCell(
                        width: 72,
                        align: TableCellAlign.end,
                        child: Row(
                          mainAxisSize: MainAxisSize.min,
                          spacing: vars.spacing05,
                          children: [
                            Tooltip(
                              label: 'Edit',
                              child: IconButton(
                                icon: const Icon(FluentIcons.edit_20_regular),
                                semanticsLabel: 'Edit ${entry.key}',
                                onPressed: () => _editEntry(
                                  key: entry.key,
                                  value: entry.value,
                                ),
                              ),
                            ),
                            Tooltip(
                              label: 'Remove',
                              child: IconButton(
                                icon: const Icon(FluentIcons.delete_20_regular),
                                tint: IconButtonTint.danger,
                                semanticsLabel: 'Remove ${entry.key}',
                                onPressed: () => _removeKey(entry.key),
                              ),
                            ),
                          ],
                        ),
                      ),
                    ],
                  ),
              ],
            ),
          ),
        ),
      ],
    );
  }

  @override
  void dispose() {
    _keyController.dispose();

    _preferences?.dispose();
    _scopedPreferences?.dispose();
    _secureStorage?.dispose();
    _scopedSecureStorage?.dispose();

    super.dispose();
  }
}

/// Add or edit one entry. Pops with the key and the value; the key is fixed
/// when an existing entry is edited.
class _EntryDialog extends StatefulWidget {
  const _EntryDialog({
    required this.storeName,
    this.initialKey,
    required this.initialValue,
  });

  final String storeName;
  final String? initialKey;
  final String initialValue;

  @override
  State<_EntryDialog> createState() => _EntryDialogState();
}

class _EntryDialogState extends State<_EntryDialog> {
  late final TextEditingController _key = TextEditingController(
    text: widget.initialKey ?? '',
  );
  late final TextEditingController _value = TextEditingController(
    text: widget.initialValue,
  );
  bool _keyMissing = false;

  bool get _editing => widget.initialKey != null;

  @override
  void dispose() {
    _key.dispose();
    _value.dispose();
    super.dispose();
  }

  void _save() {
    if (_key.text.trim().isEmpty) {
      setState(() => _keyMissing = true);
      return;
    }
    Navigator.of(context).pop((_key.text, _value.text));
  }

  @override
  Widget build(BuildContext context) {
    return Dialog(
      children: [
        DialogHeader(
          title: _editing ? 'Edit entry' : 'Add entry',
          subtitle: widget.storeName,
        ),
        DialogBody(
          children: [
            FormField(
              label: 'Key',
              invalid: _keyMissing,
              hint: _keyMissing ? 'Key cannot be empty' : null,
              child: TextField(
                controller: _key,
                mono: true,
                enabled: !_editing,
                autofocus: !_editing,
                placeholder: 'e.g. theme',
                onChanged: (_) {
                  if (_keyMissing) setState(() => _keyMissing = false);
                },
              ),
            ),
            FormField(
              label: 'Value',
              child: TextField(
                controller: _value,
                mono: true,
                autofocus: _editing,
                minLines: 3,
                maxLines: 6,
                placeholder: 'Any string',
              ),
            ),
          ],
        ),
        DialogFooter(
          children: [
            const Spacer(),
            Button(
              variant: ButtonVariant.normal,
              tint: ButtonTint.neutral,
              onPressed: () => Navigator.of(context).pop(),
              child: const Text('Cancel'),
            ),
            Button(
              variant: ButtonVariant.filled,
              onPressed: _save,
              child: const Text('Set'),
            ),
          ],
        ),
      ],
    );
  }
}
