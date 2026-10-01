import 'dart:convert';
import 'dart:typed_data';
import 'dart:ui' as ui;

// DazzUI has its own Menu, MenuItem and ContextMenu (drawn in Flutter); the
// ones this example is about are nativeapi's, so dazzui's are hidden.
import 'package:dazzui_host/dazzui_host.dart' hide Menu, MenuItem, ContextMenu;
import 'package:nativeapi/nativeapi.dart' as na;
import 'package:nativeapi_flutter/nativeapi_flutter.dart';

import 'animated_icon_generator.dart';

// The window is drawn with DazzUI over the shared host (dazzui_host); every
// menu it opens is a native one (NSMenu, Win32 / WinUI 3, GTK).

void main() {
  runApp(const MenuExampleApp());
}

/// The in-app theme choice, applied to the native menus
/// (`Application.setBrightness`) and to this window alike.
enum ThemeChoice { system, light, dark }

class MenuExampleApp extends StatefulWidget {
  const MenuExampleApp({super.key});

  @override
  State<MenuExampleApp> createState() => _MenuExampleAppState();
}

class _MenuExampleAppState extends State<MenuExampleApp> {
  ThemeChoice _theme = ThemeChoice.system;

  @override
  Widget build(BuildContext context) {
    // The host picks Studio Light or Dark from the platform brightness, so a
    // forced choice is handed down as that brightness.
    final media = MediaQuery.of(context);
    final brightness = switch (_theme) {
      ThemeChoice.system => media.platformBrightness,
      ThemeChoice.light => Brightness.light,
      ThemeChoice.dark => Brightness.dark,
    };
    return MediaQuery(
      data: media.copyWith(platformBrightness: brightness),
      child: Host(
        title: 'Menu Example',
        home: MenuExamplePage(
          theme: _theme,
          onThemeChanged: (theme) => setState(() => _theme = theme),
        ),
      ),
    );
  }
}

class MenuExamplePage extends StatefulWidget {
  const MenuExamplePage({
    super.key,
    required this.theme,
    required this.onThemeChanged,
  });

  final ThemeChoice theme;
  final ValueChanged<ThemeChoice> onThemeChanged;

  @override
  State<MenuExamplePage> createState() => _MenuExamplePageState();
}

class _MenuExamplePageState extends State<MenuExamplePage> {
  late final Menu _contextMenu;
  late final Menu _positioningMenu;

  final List<MenuItem> _menuItems = [];
  final List<String> _eventHistory = [];

  bool _checkboxState = false;
  String _radioSelection = 'Option 1';
  String _currentLabel = 'Dynamic Label Item';
  Placement _selectedPlacement = Placement.bottomStart;

  int _menuItemCount = 0;

  // Store references to menu items for state management
  late final MenuItem _checkboxItem;
  late final MenuItem _radio1;
  late final MenuItem _radio2;
  late final MenuItem _radio3;
  late final MenuItem _disabledItem;
  late final MenuItem _disabledCheckbox;
  late final MenuItem _submenuItem;
  late final Menu _submenu;

  // Store icon for demonstration
  na.Image? _testIcon;
  na.Image? _iconFromWidget;

  // Animated icon generator
  AnimatedIconGenerator? _animatedIconGenerator;
  MenuItem? _animatedMenuItem;

  @override
  void initState() {
    super.initState();
    _loadTestIcon();
    _setupContextMenu();
    _setupPositioningMenu();
    if (Menu.isBackendSupported(MenuBackend.winUi3)) {
      _contextMenu.setBackend(MenuBackend.winUi3);
      _positioningMenu.setBackend(MenuBackend.winUi3);
    }
  }

  @override
  void didChangeDependencies() {
    super.didChangeDependencies();
    // Created here rather than in initState: its ink is the theme's accent.
    _animatedIconGenerator ??= AnimatedIconGenerator(
      size: 32, // Higher resolution for better quality
      foregroundColor: context.vars.colorPrimary.shade600,
    );
  }

  void _loadTestIcon() {
    // Try to load a test icon from assets
    _testIcon = ImageAsset.fromAsset('images/flutter_logo.png');
    if (_testIcon != null) {
      _addToHistory('Test icon loaded successfully');
    } else {
      _addToHistory('Test icon not found, icon features will be limited');
    }
  }

  /// Convert a Flutter Icon widget to a base64 image
  Future<na.Image?> _iconToImage(
    IconData iconData, {
    double size = 24.0,
    required Color color,
  }) async {
    try {
      // Create a picture recorder to draw the icon
      final recorder = ui.PictureRecorder();
      final canvas = Canvas(recorder);

      // Create a text painter to render the icon
      final textPainter = TextPainter(textDirection: TextDirection.ltr);

      textPainter.text = TextSpan(
        text: String.fromCharCode(iconData.codePoint),
        style: TextStyle(
          fontSize: size,
          fontFamily: iconData.fontFamily,
          package: iconData.fontPackage,
          color: color,
        ),
      );

      textPainter.layout();
      textPainter.paint(canvas, Offset.zero);

      // Convert to image
      final picture = recorder.endRecording();
      final img = await picture.toImage(size.toInt(), size.toInt());
      final byteData = await img.toByteData(format: ui.ImageByteFormat.png);

      if (byteData == null) {
        return null;
      }

      // Convert to base64
      final Uint8List pngBytes = byteData.buffer.asUint8List();
      final base64String = 'data:image/png;base64,${base64Encode(pngBytes)}';

      // Use base64 to create nativeapi Image
      return na.Image.fromBase64(base64String);
    } catch (e) {
      _addToHistory('Error converting icon to image: $e');
      return null;
    }
  }

  void _setupContextMenu() {
    _contextMenu = Menu.create()!;

    // Listen to menu events
    _contextMenu.addListener((event) {
      if (event is! MenuOpenedEvent) return;
      _addToHistory('Menu opened (ID: ${event.menuId})');
    });
    _contextMenu.addListener((event) {
      if (event is! MenuClosedEvent) return;
      _addToHistory('Menu closed (ID: ${event.menuId})');
    });

    // 1. Normal menu item
    final normalItem = MenuItem.createWithLabelAndType(
      'Normal Menu Item',
      MenuItemType.normal,
    )!;
    normalItem.addListener((event) {
      if (event is! MenuItemClickedEvent) return;
      _addToHistory('Normal item clicked (ID: ${event.itemId})');
    });
    _contextMenu.addItem(normalItem);
    _menuItems.add(normalItem);

    // 2. Separator
    _contextMenu.addSeparator();

    // 3. Checkbox menu item
    _checkboxItem = MenuItem.createWithLabelAndType(
      'Checkbox Item',
      MenuItemType.checkbox,
    )!;
    _checkboxItem.state = MenuItemState.unchecked;
    _checkboxItem.addListener((event) {
      if (event is! MenuItemClickedEvent) return;
      setState(() {
        _checkboxState = !_checkboxState;
        _checkboxItem.state = _checkboxState
            ? MenuItemState.checked
            : MenuItemState.unchecked;
      });
      _addToHistory(
        'Checkbox clicked - State: $_checkboxState (ID: ${event.itemId})',
      );
    });
    _contextMenu.addItem(_checkboxItem);
    _menuItems.add(_checkboxItem);

    // 4. Radio menu items (grouped together)
    _radio1 = MenuItem.createWithLabelAndType(
      'Radio Option 1',
      MenuItemType.radio,
    )!;
    _radio1.radioGroup = 1; // Set radio group ID
    _radio1.state = MenuItemState.checked; // Default selection
    _radio1.addListener((event) {
      if (event is! MenuItemClickedEvent) return;
      setState(() {
        _radioSelection = 'Option 1';
        _radio1.state = MenuItemState.checked;
        _radio2.state = MenuItemState.unchecked;
        _radio3.state = MenuItemState.unchecked;
      });
      _addToHistory('Radio Option 1 selected (ID: ${event.itemId})');
    });
    _contextMenu.addItem(_radio1);
    _menuItems.add(_radio1);

    _radio2 = MenuItem.createWithLabelAndType(
      'Radio Option 2',
      MenuItemType.radio,
    )!;
    _radio2.radioGroup = 1; // Same radio group
    _radio2.state = MenuItemState.unchecked;
    _radio2.addListener((event) {
      if (event is! MenuItemClickedEvent) return;
      setState(() {
        _radioSelection = 'Option 2';
        _radio1.state = MenuItemState.unchecked;
        _radio2.state = MenuItemState.checked;
        _radio3.state = MenuItemState.unchecked;
      });
      _addToHistory('Radio Option 2 selected (ID: ${event.itemId})');
    });
    _contextMenu.addItem(_radio2);
    _menuItems.add(_radio2);

    _radio3 = MenuItem.createWithLabelAndType(
      'Radio Option 3',
      MenuItemType.radio,
    )!;
    _radio3.radioGroup = 1; // Same radio group
    _radio3.state = MenuItemState.unchecked;
    _radio3.addListener((event) {
      if (event is! MenuItemClickedEvent) return;
      setState(() {
        _radioSelection = 'Option 3';
        _radio1.state = MenuItemState.unchecked;
        _radio2.state = MenuItemState.unchecked;
        _radio3.state = MenuItemState.checked;
      });
      _addToHistory('Radio Option 3 selected (ID: ${event.itemId})');
    });
    _contextMenu.addItem(_radio3);
    _menuItems.add(_radio3);

    // 5. Disabled menu items
    _disabledItem = MenuItem.createWithLabelAndType(
      'Disabled Item',
      MenuItemType.normal,
    )!;
    _disabledItem.isEnabled = false;
    _disabledItem.addListener((event) {
      if (event is! MenuItemClickedEvent) return;
      _addToHistory('Disabled item clicked (should not fire!)');
    });
    _contextMenu.addItem(_disabledItem);
    _menuItems.add(_disabledItem);

    _disabledCheckbox = MenuItem.createWithLabelAndType(
      'Disabled Checkbox',
      MenuItemType.checkbox,
    )!;
    _disabledCheckbox.state = MenuItemState.checked;
    _disabledCheckbox.isEnabled = false;
    _disabledCheckbox.addListener((event) {
      if (event is! MenuItemClickedEvent) return;
      _addToHistory('Disabled checkbox clicked (should not fire!)');
    });
    _contextMenu.addItem(_disabledCheckbox);
    _menuItems.add(_disabledCheckbox);

    // 7. Separator
    _contextMenu.addSeparator();

    // 8. Menu item with dynamic label
    final dynamicLabelItem = MenuItem.createWithLabelAndType(
      _currentLabel,
      MenuItemType.normal,
    )!;
    dynamicLabelItem.addListener((event) {
      if (event is! MenuItemClickedEvent) return;
      _addToHistory('Dynamic label item clicked (ID: ${event.itemId})');
    });
    _contextMenu.addItem(dynamicLabelItem);
    _menuItems.add(dynamicLabelItem);

    // 9. Menu item with tooltip
    final tooltipItem = MenuItem.createWithLabelAndType(
      'Item with Tooltip',
      MenuItemType.normal,
    )!;
    tooltipItem.tooltip = 'This is a helpful tooltip message';
    tooltipItem.addListener((event) {
      if (event is! MenuItemClickedEvent) return;
      _addToHistory('Tooltip item clicked (ID: ${event.itemId})');
    });
    _contextMenu.addItem(tooltipItem);
    _menuItems.add(tooltipItem);

    // 10. Separator
    _contextMenu.addSeparator();

    // 11. Submenu
    _submenu = Menu.create()!;
    _submenuItem = MenuItem.createWithLabelAndType(
      'Submenu',
      MenuItemType.submenu,
    )!;

    _submenuItem.addListener((event) {
      if (event is! MenuItemSubmenuOpenedEvent) return;
      _addToHistory('Submenu opened (ID: ${event.itemId})');
    });
    _submenuItem.addListener((event) {
      if (event is! MenuItemSubmenuClosedEvent) return;
      _addToHistory('Submenu closed (ID: ${event.itemId})');
    });

    // Add items to submenu
    final subItem1 = MenuItem.createWithLabelAndType(
      'Submenu Item 1',
      MenuItemType.normal,
    )!;
    subItem1.addListener((event) {
      if (event is! MenuItemClickedEvent) return;
      _addToHistory('Submenu Item 1 clicked (ID: ${event.itemId})');
    });
    _submenu.addItem(subItem1);

    final subItem2 = MenuItem.createWithLabelAndType(
      'Submenu Item 2',
      MenuItemType.normal,
    )!;
    subItem2.addListener((event) {
      if (event is! MenuItemClickedEvent) return;
      _addToHistory('Submenu Item 2 clicked (ID: ${event.itemId})');
    });
    _submenu.addItem(subItem2);

    _submenu.addSeparator();

    final subItem3 = MenuItem.createWithLabelAndType(
      'Submenu Item 3',
      MenuItemType.normal,
    )!;
    subItem3.addListener((event) {
      if (event is! MenuItemClickedEvent) return;
      _addToHistory('Submenu Item 3 clicked (ID: ${event.itemId})');
    });
    _submenu.addItem(subItem3);

    // Associate the submenu with the menu item
    _submenuItem.submenu = _submenu;

    _contextMenu.addItem(_submenuItem);
    _menuItems.add(_submenuItem);

    // 12. Separator
    _contextMenu.addSeparator();

    // 13. Menu items with special characters
    final specialCharsItem = MenuItem.createWithLabelAndType(
      'Special: 中文 日本語 🎉 @#\$%',
      MenuItemType.normal,
    )!;
    specialCharsItem.addListener((event) {
      if (event is! MenuItemClickedEvent) return;
      _addToHistory('Special chars item clicked (ID: ${event.itemId})');
    });
    _contextMenu.addItem(specialCharsItem);
    _menuItems.add(specialCharsItem);

    _updateMenuItemCount();
  }

  void _setupPositioningMenu() {
    _positioningMenu = Menu.create()!;

    _positioningMenu.addListener((event) {
      if (event is! MenuOpenedEvent) return;
      _addToHistory('Positioning menu opened');
    });
    _positioningMenu.addListener((event) {
      if (event is! MenuClosedEvent) return;
      _addToHistory('Positioning menu closed');
    });

    final item1 = MenuItem.createWithLabelAndType(
      'Positioning Menu Item 1',
      MenuItemType.normal,
    )!;
    item1.addListener((event) {
      if (event is! MenuItemClickedEvent) return;
      _addToHistory('Positioning menu item 1 clicked');
    });
    _positioningMenu.addItem(item1);

    final item2 = MenuItem.createWithLabelAndType(
      'Positioning Menu Item 2',
      MenuItemType.normal,
    )!;
    item2.addListener((event) {
      if (event is! MenuItemClickedEvent) return;
      _addToHistory('Positioning menu item 2 clicked');
    });
    _positioningMenu.addItem(item2);
  }

  void _addToHistory(String message) {
    setState(() {
      final timestamp = DateTime.now().toString().substring(11, 19);
      _eventHistory.insert(0, '[$timestamp] $message');
      if (_eventHistory.length > 50) {
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

  void _updateMenuItemCount() {
    setState(() {
      _menuItemCount = _contextMenu.itemCount;
    });
  }

  void _changeDynamicLabel() {
    setState(() {
      _currentLabel =
          'Updated at ${DateTime.now().toString().substring(11, 19)}';
      // Dynamic label item is now at index 7 due to added disabled items
      if (_menuItems.length > 7) {
        _menuItems[7].label = _currentLabel;
      }
    });
    _addToHistory('Menu item label changed to: $_currentLabel');
  }

  void _setCheckboxMixed() {
    setState(() {
      _checkboxItem.state = MenuItemState.mixed;
    });
    _addToHistory('Checkbox state set to Mixed (indeterminate)');
  }

  void _addSubmenuItem() {
    final newSubItem = MenuItem.createWithLabelAndType(
      'Dynamic Submenu Item ${_submenu.itemCount + 1}',
      MenuItemType.normal,
    )!;
    newSubItem.addListener((event) {
      if (event is! MenuItemClickedEvent) return;
      _addToHistory('Dynamic submenu item clicked (ID: ${event.itemId})');
    });
    _submenu.addItem(newSubItem);
    _addToHistory('Added new item to submenu (Total: ${_submenu.itemCount})');
  }

  void _toggleSubmenu() {
    setState(() {
      if (_submenuItem.submenu != null) {
        _submenuItem.submenu = null;
        _addToHistory('Submenu detached from menu item');
      } else {
        _submenuItem.submenu = _submenu;
        _addToHistory('Submenu attached to menu item');
      }
    });
  }

  void _addNewMenuItem() {
    final newItem = MenuItem.createWithLabelAndType(
      'New Item ${_menuItems.length + 1}',
      MenuItemType.normal,
    )!;
    newItem.addListener((event) {
      if (event is! MenuItemClickedEvent) return;
      _addToHistory('New item ${_menuItems.length} clicked');
    });
    _contextMenu.addItem(newItem);
    _menuItems.add(newItem);
    _updateMenuItemCount();
    _addToHistory('Added new menu item');
  }

  void _insertMenuItemAtPosition() {
    final insertItem = MenuItem.createWithLabelAndType(
      'Inserted Item',
      MenuItemType.normal,
    )!;
    insertItem.addListener((event) {
      if (event is! MenuItemClickedEvent) return;
      _addToHistory('Inserted item clicked');
    });
    _contextMenu.insertItem(2, insertItem);
    _menuItems.insert(2, insertItem);
    _updateMenuItemCount();
    _addToHistory('Inserted menu item at position 2');
  }

  void _insertSeparatorAtPosition() {
    _contextMenu.insertSeparator(3);
    _updateMenuItemCount();
    _addToHistory('Inserted separator at position 3');
  }

  void _setIconOnFirstItem() {
    if (_menuItems.isEmpty) {
      _addToHistory('No menu items available to set icon');
      return;
    }

    if (_testIcon != null) {
      _menuItems[0].icon = _testIcon;
      _addToHistory('Icon set on first menu item');
    } else {
      _addToHistory('No icon available to set');
    }
  }

  void _removeIconFromFirstItem() {
    if (_menuItems.isEmpty) {
      _addToHistory('No menu items available to remove icon from');
      return;
    }

    _menuItems[0].icon = null;
    _addToHistory('Icon removed from first menu item');
  }

  // Animated icon methods
  Future<void> _startSpinnerAnimation() async {
    if (_menuItems.isEmpty) {
      _addToHistory('No menu items available for animation');
      return;
    }

    _animatedMenuItem = _menuItems[0];
    await _animatedIconGenerator?.startSpinner(
      onFrame: (image) async {
        _animatedMenuItem?.icon = image;
      },
    );
    _addToHistory('Started spinner animation');
  }

  Future<void> _startPulseAnimation() async {
    if (_menuItems.isEmpty) {
      _addToHistory('No menu items available for animation');
      return;
    }

    _animatedMenuItem = _menuItems[0];
    await _animatedIconGenerator?.startPulse(
      onFrame: (image) async {
        _animatedMenuItem?.icon = image;
      },
    );
    _addToHistory('Started pulse animation');
  }

  Future<void> _startBlinkAnimation() async {
    if (_menuItems.isEmpty) {
      _addToHistory('No menu items available for animation');
      return;
    }

    _animatedMenuItem = _menuItems[0];
    await _animatedIconGenerator?.startBlink(
      onFrame: (image) async {
        _animatedMenuItem?.icon = image;
      },
    );
    _addToHistory('Started blink animation');
  }

  Future<void> _startProgressAnimation() async {
    if (_menuItems.isEmpty) {
      _addToHistory('No menu items available for animation');
      return;
    }

    _animatedMenuItem = _menuItems[0];
    await _animatedIconGenerator?.startProgress(
      onFrame: (image) async {
        _animatedMenuItem?.icon = image;
      },
    );
    _addToHistory('Started progress animation');
  }

  Future<void> _startWaveAnimation() async {
    if (_menuItems.isEmpty) {
      _addToHistory('No menu items available for animation');
      return;
    }

    _animatedMenuItem = _menuItems[0];
    await _animatedIconGenerator?.startWave(
      onFrame: (image) async {
        _animatedMenuItem?.icon = image;
      },
    );
    _addToHistory('Started wave animation');
  }

  Future<void> _startRotatingSquareAnimation() async {
    if (_menuItems.isEmpty) {
      _addToHistory('No menu items available for animation');
      return;
    }

    _animatedMenuItem = _menuItems[0];
    await _animatedIconGenerator?.startRotatingSquare(
      onFrame: (image) async {
        _animatedMenuItem?.icon = image;
      },
    );
    _addToHistory('Started rotating square animation');
  }

  void _stopAnimation() {
    _animatedIconGenerator?.stop();
    _addToHistory('Stopped animation');
  }

  Future<void> _setIconFromWidget() async {
    if (_menuItems.isEmpty) {
      _addToHistory('No menu items available to set icon');
      return;
    }

    final color = context.vars.colorWarning.shade500;
    _addToHistory('Converting Flutter Icon to image...');

    // Convert a Fluent star glyph (any IconData works) to image
    _iconFromWidget = await _iconToImage(
      FluentIcons.star_16_filled,
      size: 16.0,
      color: color,
    );

    if (_iconFromWidget != null) {
      _menuItems[0].icon = _iconFromWidget;
      _addToHistory('Icon from widget set on first menu item');
    } else {
      _addToHistory('Failed to convert icon from widget');
    }
  }

  void _removeFirstMenuItem() {
    if (_menuItems.isEmpty) {
      _addToHistory('No menu items to remove');
      return;
    }

    final item = _menuItems[0];
    final success = _contextMenu.removeItem(item);

    if (success) {
      _menuItems.removeAt(0);
      _updateMenuItemCount();
      _addToHistory('Removed first menu item');
    } else {
      _addToHistory('Failed to remove first menu item');
    }
  }

  void _removeMenuItemAtPosition() {
    const position = 2;
    if (_menuItems.length <= position) {
      _addToHistory('No menu item at position $position to remove');
      return;
    }

    final success = _contextMenu.removeItemAt(position);

    if (success) {
      _menuItems.removeAt(position);
      _updateMenuItemCount();
      _addToHistory('Removed menu item at position $position');
    } else {
      _addToHistory('Failed to remove menu item at position $position');
    }
  }

  void _removeLastMenuItem() {
    if (_menuItems.isEmpty) {
      _addToHistory('No menu items to remove');
      return;
    }

    final lastIndex = _menuItems.length - 1;
    final success = _contextMenu.removeItemAt(lastIndex);

    if (success) {
      _menuItems.removeLast();
      _updateMenuItemCount();
      _addToHistory('Removed last menu item');
    } else {
      _addToHistory('Failed to remove last menu item');
    }
  }

  void _showMenuAtAbsolutePosition(Offset position) {
    _positioningMenu.open(
      PositioningStrategy.absolute(position.toNative())!,
      Placement.bottomStart,
    );
    _addToHistory('Opened menu at absolute position: $position');
  }

  void _showMenuAtCursorPosition() {
    _positioningMenu.open(
      PositioningStrategy.cursorPosition()!,
      Placement.bottomStart,
    );
    _addToHistory('Opened menu at cursor position');
  }

  /// Reproduce issue #4: checked/disabled menu item states not working on Windows
  void _openBugReproMenu(Offset position) {
    // This directly reproduces the code from the issue:
    // https://github.com/libnativeapi/nativeapi-flutter/issues/4
    final menu = Menu.create()!;
    menu.setBackend(_contextMenu.backend);

    // Checkable item with Checked state
    final checkableItem = MenuItem.createWithLabelAndType(
      'Checkable',
      MenuItemType.checkbox,
    )!;
    checkableItem.state = MenuItemState.checked;
    checkableItem.addListener((event) {
      if (event is! MenuItemClickedEvent) return;
      _addToHistory('Checkable item clicked (ID: ${event.itemId})');
    });
    menu.addItem(checkableItem);

    // Disabled item
    final disabledItem = MenuItem.createWithLabelAndType(
      'Disabled',
      MenuItemType.normal,
    )!;
    disabledItem.isEnabled = false;
    disabledItem.addListener((event) {
      if (event is! MenuItemClickedEvent) return;
      _addToHistory('Disabled item clicked (should not fire!)');
    });
    menu.addItem(disabledItem);

    // Add a few more items for context
    menu.addSeparator();
    final exitItem = MenuItem.createWithLabelAndType(
      'Close Menu',
      MenuItemType.normal,
    )!;
    exitItem.addListener((event) {
      if (event is! MenuItemClickedEvent) return;
      menu.close();
    });
    menu.addItem(exitItem);

    menu.open(
      PositioningStrategy.absolute(position.toNative())!,
      Placement.bottomStart,
    );

    _addToHistory('');
    _addToHistory('=== BUG REPRODUCTION (Issue #4) ===');
    _addToHistory('Menu opened at $position');
    _addToHistory('Checkable item initial state: ${checkableItem.state}');
    _addToHistory('Disabled item enabled: ${disabledItem.isEnabled}');
    _addToHistory('');
    _addToHistory('EXPECTED: Checkbox shows checkmark; Disabled item grayed');
    _addToHistory(
      'BUG (Windows only): Checkmark missing; Disabled item active',
    );
    _addToHistory('');
  }

  // --- The window -----------------------------------------------------------
  //
  // The GUI tests (tools/gui/flutter_menu_test.py, flutter_menu_test.ps1,
  // flutter_menu_theme_test.ps1) find controls by their text: "Right-click
  // here", the Items / Checkbox / Radio read-outs (value right under its
  // label), the placement, backend and theme choices (clicked as "open at the
  // current value, then pick another one lower down", so every group is laid
  // out with its choices on separate lines), the chips, and the history
  // entries "[hh:mm:ss] …" under "Event History (n)", whose right end stays
  // empty: the tests click there to dismiss a menu.

  static const _placements = [
    (Placement.topStart, 'Top Start'),
    (Placement.topEnd, 'Top End'),
    (Placement.bottomStart, 'Bottom Start'),
    (Placement.bottomEnd, 'Bottom End'),
    (Placement.leftStart, 'Left Start'),
    (Placement.leftEnd, 'Left End'),
    (Placement.rightStart, 'Right Start'),
    (Placement.rightEnd, 'Right End'),
  ];

  /// The animation playing on the first item's icon, for the chips.
  String? _animation;

  void _setPlacement(Placement value) {
    if (value == _selectedPlacement) return;
    setState(() => _selectedPlacement = value);
    _addToHistory('Placement changed to: ${value.toString().split('.').last}');
  }

  void _setBackend(MenuBackend? backend) {
    if (backend == null || backend == _contextMenu.backend) return;
    final contextOk = _contextMenu.setBackend(backend);
    final positioningOk = _positioningMenu.setBackend(backend);
    _addToHistory(
      'Backend ${backend.name}: context=$contextOk, positioning=$positioningOk',
    );
    setState(() {});
  }

  void _setTheme(ThemeChoice? mode) {
    if (mode == null || mode == widget.theme) return;
    final brightness = switch (mode) {
      ThemeChoice.system => na.Brightness.system,
      ThemeChoice.light => na.Brightness.light,
      ThemeChoice.dark => na.Brightness.dark,
    };
    final applied = Application.instance.setBrightness(brightness);
    widget.onThemeChanged(mode);
    _addToHistory('Theme: ${mode.name}; native appearance applied: $applied');
  }

  void _animate(String name, Future<void> Function() start) {
    if (_menuItems.isNotEmpty) setState(() => _animation = name);
    start();
  }

  void _stop() {
    _stopAnimation();
    setState(() => _animation = null);
  }

  @override
  Widget build(BuildContext context) {
    final vars = context.vars;
    return Column(
      children: [
        _toolbar(vars),
        const Divider(),
        Expanded(
          child: Row(
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: [
              Expanded(
                child: SingleChildScrollView(
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.stretch,
                    children: [_stage(vars), const Divider(), ..._editRows()],
                  ),
                ),
              ),
              const VerticalDivider(),
              SizedBox(width: 280, child: _sidePanel(vars)),
            ],
          ),
        ),
      ],
    );
  }

  Widget _toolbar(ThemeVariables vars) {
    return Padding(
      padding: EdgeInsets.symmetric(
        horizontal: vars.spacing3,
        vertical: vars.spacing2,
      ),
      child: Row(
        children: [
          Icon(
            FluentIcons.text_bullet_list_square_20_regular,
            size: vars.iconLarge,
            color: vars.colorPrimary.shade600,
          ),
          SizedBox(width: vars.spacing2),
          Text('Native menus', style: vars.titleMedium),
          SizedBox(width: vars.spacing2),
          Expanded(
            child: Text(
              'nativeapi Menu · MenuItem · ContextMenuRegion',
              maxLines: 1,
              overflow: TextOverflow.ellipsis,
              style: vars.muted,
            ),
          ),
          Tooltip(
            label: 'Clear Event History',
            side: PopoverSide.bottom,
            child: IconButton(
              icon: const Icon(FluentIcons.delete_20_regular),
              semanticsLabel: 'Clear Event History',
              onPressed: _clearHistory,
            ),
          ),
        ],
      ),
    );
  }

  /// The right-click target with the placement it opens the menu at, and
  /// what the menu holds right now.
  Widget _stage(ThemeVariables vars) {
    return Padding(
      padding: EdgeInsets.all(vars.spacing3),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          Row(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              Expanded(child: _region(vars)),
              SizedBox(width: vars.spacing3),
              _placementPad(vars),
            ],
          ),
          SizedBox(height: vars.spacing3),
          Row(
            children: [
              _readout(vars, 'Items', '$_menuItemCount'),
              _readout(vars, 'Checkbox', '$_checkboxState'),
              _readout(vars, 'Radio', _radioSelection),
              _readout(
                vars,
                'Submenu',
                _submenuItem.submenu == null
                    ? 'detached'
                    : '${_submenu.itemCount} entries',
              ),
            ],
          ),
        ],
      ),
    );
  }

  Widget _region(ThemeVariables vars) {
    return ContextMenuRegion(
      menu: _contextMenu,
      placement: _selectedPlacement,
      child: Container(
        height: 136,
        decoration: BoxDecoration(
          color: vars.colorSurfaceSunken,
          border: Border.all(color: vars.colorBorder),
          borderRadius: BorderRadius.circular(vars.radiusMedium),
        ),
        child: Center(
          child: Column(
            mainAxisSize: MainAxisSize.min,
            children: [
              Icon(
                FluentIcons.cursor_click_24_regular,
                size: vars.iconLarge * 1.5,
                color: vars.colorPrimary.shade600,
              ),
              SizedBox(height: vars.spacing2),
              Text('Right-click here', style: vars.titleSmall),
              SizedBox(height: vars.spacing05),
              Text('opens the native context menu', style: vars.muted),
            ],
          ),
        ),
      ),
    );
  }

  /// Where the menu goes relative to the click point: a two-by-four pad, so
  /// each choice sits on a line of its own side.
  Widget _placementPad(ThemeVariables vars) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        const SectionLabel('Placement'),
        SizedBox(height: vars.spacing1),
        for (var i = 0; i < _placements.length; i += 2)
          Padding(
            padding: EdgeInsets.only(bottom: vars.spacing1),
            child: Row(
              children: [
                for (final (value, label) in _placements.sublist(i, i + 2))
                  Padding(
                    padding: EdgeInsets.only(right: vars.spacing1),
                    child: SizedBox(
                      width: 100,
                      child: OptionChip(
                        label: label,
                        selected: value == _selectedPlacement,
                        onTap: () => _setPlacement(value),
                      ),
                    ),
                  ),
              ],
            ),
          ),
      ],
    );
  }

  /// A label with its value right under it, as the tests read them.
  Widget _readout(ThemeVariables vars, String label, String value) {
    return Expanded(
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text(label, style: vars.muted),
          Text(
            value,
            maxLines: 1,
            overflow: TextOverflow.ellipsis,
            style: vars.titleSmall,
          ),
        ],
      ),
    );
  }

  List<Widget> _editRows() {
    return [
      OptionRow(
        label: 'Add',
        children: [
          ActionChip(label: 'Add Item', onTap: _addNewMenuItem),
          ActionChip(
            label: 'Insert at Pos 2',
            onTap: _insertMenuItemAtPosition,
          ),
          ActionChip(
            label: 'Insert Separator',
            onTap: _insertSeparatorAtPosition,
          ),
        ],
      ),
      OptionRow(
        label: 'Remove',
        children: [
          ActionChip(label: 'Remove First', onTap: _removeFirstMenuItem),
          ActionChip(
            label: 'Remove at Pos 2',
            onTap: _removeMenuItemAtPosition,
          ),
          ActionChip(label: 'Remove Last', onTap: _removeLastMenuItem),
        ],
      ),
      OptionRow(
        label: 'Properties',
        children: [
          ActionChip(label: 'Update Label', onTap: _changeDynamicLabel),
          ActionChip(label: 'Checkbox Mixed', onTap: _setCheckboxMixed),
          const Hint('label and checkbox state'),
        ],
      ),
      OptionRow(
        label: 'Submenu',
        children: [
          ActionChip(label: 'Add Submenu Item', onTap: _addSubmenuItem),
          // Pressed again, it attaches the submenu back.
          ActionChip(label: 'Detach Submenu', onTap: _toggleSubmenu),
        ],
      ),
      OptionRow(
        label: 'Icon',
        children: [
          ActionChip(label: 'Set Asset Icon', onTap: _setIconOnFirstItem),
          ActionChip(label: 'Set Widget Icon', onTap: _setIconFromWidget),
          ActionChip(label: 'Remove Icon', onTap: _removeIconFromFirstItem),
          const Hint('on the first item'),
        ],
      ),
      OptionRow(
        label: 'Animate',
        children: [
          for (final (name, start) in [
            ('Spinner', _startSpinnerAnimation),
            ('Pulse', _startPulseAnimation),
            ('Blink', _startBlinkAnimation),
            ('Progress', _startProgressAnimation),
            ('Wave', _startWaveAnimation),
            ('Rotate', _startRotatingSquareAnimation),
          ])
            OptionChip(
              label: name,
              selected: _animation == name,
              onTap: () => _animate(name, start),
            ),
          ActionChip(label: 'Stop', onTap: _animation == null ? null : _stop),
        ],
      ),
      OptionRow(
        label: 'Open at',
        children: [
          ActionChip(
            label: 'Pos (100,100)',
            onTap: () => _showMenuAtAbsolutePosition(const Offset(100, 100)),
          ),
          ActionChip(
            label: 'Pos (300,200)',
            onTap: () => _showMenuAtAbsolutePosition(const Offset(300, 200)),
          ),
          ActionChip(label: 'At Cursor', onTap: _showMenuAtCursorPosition),
          const Hint('the positioning menu'),
        ],
      ),
      OptionRow(
        label: 'Edges',
        children: [
          ActionChip(
            label: 'Top-Left Edge',
            onTap: () {
              _showMenuAtAbsolutePosition(const Offset(10, 10));
              _addToHistory('Testing menu near screen edge (top-left)');
            },
          ),
          ActionChip(
            label: 'Bottom-Right Edge',
            onTap: () {
              _showMenuAtAbsolutePosition(const Offset(1500, 900));
              _addToHistory('Testing menu near screen edge (bottom-right)');
            },
          ),
          const Hint('kept on screen'),
        ],
      ),
      OptionRow(
        label: 'Stress',
        children: [
          ActionChip(
            label: 'Add 10 Items',
            onTap: () {
              for (int i = 0; i < 10; i++) {
                _addNewMenuItem();
              }
            },
          ),
          ActionChip(
            label: 'Rapid Open/Close',
            onTap: () {
              for (int i = 0; i < 5; i++) {
                _showMenuAtAbsolutePosition(
                  Offset(100.0 + i * 50, 100.0 + i * 50),
                );
                Future.delayed(
                  const Duration(milliseconds: 100),
                  () => _contextMenu.close(),
                );
              }
            },
          ),
          // Issue #4: checked / disabled states on Windows.
          ActionChip(
            label: 'Bug #4 (Win)',
            onTap: () => _openBugReproMenu(const Offset(200, 200)),
          ),
        ],
      ),
    ];
  }

  /// Backend and theme on top, the event history under them.
  Widget _sidePanel(ThemeVariables vars) {
    return ColoredBox(
      color: vars.colorSurfaceSunken,
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          Padding(
            padding: EdgeInsets.all(vars.spacing3),
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Row(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    Expanded(
                      child: _choice(
                        vars,
                        'Backend',
                        RadioGroup<MenuBackend>(
                          size: WidgetSize.small,
                          spacing: vars.spacing15,
                          options: [
                            for (final backend in MenuBackend.values)
                              RadioItem(
                                value: backend,
                                enabled: Menu.isBackendSupported(backend),
                                label: Text(
                                  backend == MenuBackend.winUi3
                                      ? 'WinUI 3'
                                      : 'Native',
                                ),
                              ),
                          ],
                          value: _contextMenu.backend,
                          onChanged: _setBackend,
                        ),
                      ),
                    ),
                    Expanded(
                      child: _choice(
                        vars,
                        'Theme',
                        RadioGroup<ThemeChoice>(
                          size: WidgetSize.small,
                          spacing: vars.spacing15,
                          options: const [
                            RadioItem(
                              value: ThemeChoice.system,
                              label: Text('System'),
                            ),
                            RadioItem(
                              value: ThemeChoice.light,
                              label: Text('Light'),
                            ),
                            RadioItem(
                              value: ThemeChoice.dark,
                              label: Text('Dark'),
                            ),
                          ],
                          value: widget.theme,
                          onChanged: _setTheme,
                        ),
                      ),
                    ),
                  ],
                ),
                SizedBox(height: vars.spacing2),
                const Hint('Open a menu to preview the selected theme.'),
              ],
            ),
          ),
          const Divider(),
          Padding(
            padding: EdgeInsets.symmetric(
              horizontal: vars.spacing3,
              vertical: vars.spacing2,
            ),
            child: Row(
              children: [
                Icon(
                  FluentIcons.history_20_regular,
                  size: vars.iconMedium,
                  color: vars.colorContentMuted,
                ),
                SizedBox(width: vars.spacing2),
                Text(
                  'Event History (${_eventHistory.length})',
                  style: vars.titleSmall,
                ),
              ],
            ),
          ),
          const Divider(),
          Expanded(
            child: _eventHistory.isEmpty
                ? Center(
                    child: Text(
                      'No events yet\nInteract with menus to see events',
                      textAlign: TextAlign.center,
                      style: vars.muted,
                    ),
                  )
                : ListView.builder(
                    itemCount: _eventHistory.length,
                    padding: EdgeInsets.all(vars.spacing3),
                    itemBuilder: (context, index) => Padding(
                      padding: EdgeInsets.only(bottom: vars.spacing1),
                      child: Text(
                        _eventHistory[index],
                        style: index == 0
                            ? vars.mono.copyWith(color: vars.colorContent)
                            : vars.mono,
                      ),
                    ),
                  ),
          ),
        ],
      ),
    );
  }

  Widget _choice(ThemeVariables vars, String label, Widget group) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        SectionLabel(label),
        SizedBox(height: vars.spacing15),
        group,
      ],
    );
  }

  @override
  void dispose() {
    _animatedIconGenerator?.dispose();
    _contextMenu.dispose();
    _positioningMenu.dispose();
    for (var item in _menuItems) {
      item.dispose();
    }
    super.dispose();
  }
}
