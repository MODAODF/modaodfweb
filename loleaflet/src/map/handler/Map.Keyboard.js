/* -*- js-indent-level: 8 -*- */
/*
 * L.Map.Keyboard is handling keyboard interaction with the map, enabled by default.
 *
 * It handles keyboard interactions which are NOT text input, including those which
 * don't require edit permissions (e.g. page scroll). Text input is handled
 * at TextInput.
 */

/* global app UNOKey UNOModifier */

L.Map.mergeOptions({
	keyboard: true,
	keyboardPanOffset: 20,
	keyboardZoomOffset: 1
});

L.Map.Keyboard = L.Handler.extend({

	keymap: {
		8   : UNOKey.BACKSPACE,
		9   : UNOKey.TAB,
		13  : UNOKey.RETURN,
		16  : null, // shift		: UNKOWN
		17  : null, // ctrl		: UNKOWN
		18  : null, // alt		: UNKOWN
		19  : null, // pause/break	: UNKOWN
		20  : null, // caps lock	: UNKOWN
		27  : UNOKey.ESCAPE,
		32  : UNOKey.SPACE,
		33  : UNOKey.PAGEUP,
		34  : UNOKey.PAGEDOWN,
		35  : UNOKey.END,
		36  : UNOKey.HOME,
		37  : UNOKey.LEFT,
		38  : UNOKey.UP,
		39  : UNOKey.RIGHT,
		40  : UNOKey.DOWN,
		45  : UNOKey.INSERT,
		46  : UNOKey.DELETE,
		48  : UNOKey.NUM0,
		49  : UNOKey.NUM1,
		50  : UNOKey.NUM2,
		51  : UNOKey.NUM3,
		52  : UNOKey.NUM4,
		53  : UNOKey.NUM5,
		54  : UNOKey.NUM6,
		55  : UNOKey.NUM7,
		56  : UNOKey.NUM8,
		57  : UNOKey.NUM9,
		65  : UNOKey.A,
		66  : UNOKey.B,
		67  : UNOKey.C,
		68  : UNOKey.D,
		69  : UNOKey.E,
		70  : UNOKey.F,
		71  : UNOKey.G,
		72  : UNOKey.H,
		73  : UNOKey.I,
		74  : UNOKey.J,
		75  : UNOKey.K,
		76  : UNOKey.L,
		77  : UNOKey.M,
		78  : UNOKey.N,
		79  : UNOKey.O,
		80  : UNOKey.P,
		81  : UNOKey.Q,
		82  : UNOKey.R,
		83  : UNOKey.S,
		84  : UNOKey.T,
		85  : UNOKey.U,
		86  : UNOKey.V,
		87  : UNOKey.W,
		88  : UNOKey.X,
		89  : UNOKey.Y,
		90  : UNOKey.Z,
		91  : null, // left window key	: UNKOWN
		92  : null, // right window key	: UNKOWN
		93  : null, // select key	: UNKOWN
		96  : UNOKey.NUM0,
		97  : UNOKey.NUM1,
		98  : UNOKey.NUM2,
		99  : UNOKey.NUM3,
		100 : UNOKey.NUM4,
		101 : UNOKey.NUM5,
		102 : UNOKey.NUM6,
		103 : UNOKey.NUM7,
		104 : UNOKey.NUM8,
		105 : UNOKey.NUM9,
		106 : UNOKey.MULTIPLY,
		107 : UNOKey.ADD,
		109 : UNOKey.SUBTRACT,
		110 : UNOKey.DECIMAL,
		111 : UNOKey.DIVIDE,
		112 : UNOKey.F1,
		113 : UNOKey.F2,
		114 : UNOKey.F3,
		115 : UNOKey.F4,
		116 : UNOKey.F5,
		117 : UNOKey.F6,
		118 : UNOKey.F7,
		119 : UNOKey.F8,
		120 : UNOKey.F9,
		121 : UNOKey.F10,
		122 : UNOKey.F11,
		144 : UNOKey.NUMLOCK,
		145 : UNOKey.SCROLLLOCK,
		173 : UNOKey.SUBTRACT,
		186 : UNOKey.SEMICOLON,
		187 : UNOKey.EQUAL,
		188 : UNOKey.COMMA,
		189 : UNOKey.SUBTRACT,
		190 : null, // period		: UNKOWN
		191 : null, // forward slash	: UNKOWN
		192 : null, // grave accent	: UNKOWN
		219 : null, // open bracket	: UNKOWN
		220 : null, // back slash	: UNKOWN
		221 : null, // close bracket	: UNKOWN
		222 : null  // single quote	: UNKOWN
	},

	handleOnKeyDownKeys: {
		// these keys need to be handled on keydown in order for them
		// to work on chrome
		// Backspace and Delete are handled at TextInput's 'beforeinput' handler.
		9   : true, // tab
		19  : true, // pause/break
		20  : true, // caps lock
		27  : true, // escape
		33  : true, // page up
		34  : true, // page down
		35  : true, // end
		36  : true, // home
		37  : true, // left arrow
		38  : true, // up arrow
		39  : true, // right arrow
		40  : true, // down arrow
		45  : true, // insert
		113 : true  // f2
	},

	keyCodes: {
		pageUp:   33,
		pageDown: 34,
		enter:    13,
		BACKSPACE:8,
		TAB:      9,
		SPACE :   32,
		SHIFT:    16, // shift		: UNKOWN
		CTRL:     17, // ctrl		: UNKOWN
		ALT:      18, // alt		: UNKOWN
		PAUSE:    19, // pause/break	: UNKOWN
		CAPSLOCK: 20, // caps lock	: UNKOWN,
		END:      35,
		HOME:     36,
		LEFT:     37,
		UP:       38,
		RIGHT:    39,
		DOWN:     40,
		INSERT:   45,
		DELETE:   46,
		NUM0:     [48,96], // two values because of different mapping in mac and windows for same keys
		NUM1:     [49,97],
		NUM2:     [50,98],
		NUM3:     [51,99],
		NUM4:     [52,100],
		NUM5:     [53,101],
		NUM6:     [54,102],
		NUM7:     [55,103],
		NUM8:     [56,103],
		NUM9:     [57,104],
		A:        65,
		B:        66,
		C:        [67,99],
		//c:        99,
		D:        68,
		E:        69,
		//f:        70, no need for separate as for Windows it will remain the same
		F:        70,
		G:        71,
		H:        72,
		I:        73,
		J:        74,
		K:        75,
		L:        76,
		M:        77,
		N:        78,
		O:        79,
		P:        80,
		Q:        81,
		R:        82,
		S:        83,
		T:        84,
		U:        85,
		V:        [86,118],
		//v:        118, // this is for MAC as the value of v changes when we press keydown
		W:        87,
		X:        [88,120],
		//x:        120, // this is for MAC as the value of x changes when we press keydown
		Y:        89,
		Z:        90,
		LEFTWINDOWKEY :    [91,91], // left window key	: UNKOWN  and also for MAC
		RIGHTWINDOWKEY:    [92,93], // right window key	: UNKOWN  and also for MAC
		SELECTKEY:         93, // select key	: UNKOWN
		// NUM0:     96,
		// NUM1:     97,
		// NUM2:     98,
		// NUM3:     99,
		// NUM4:     100,
		// NUM5:     101,
		// NUM6:     102,
		// NUM7:     103,
		// NUM8:     104,
		MULTIPLY:    106,
		ADD:         107,
		//SUBTRACT:    109,
		DECIMAL:     110,
		DIVIDE:      111,
		F1:       112,
		F2:       113,
		F3:       114,
		F4:       115,
		F5:       116,
		F6:       117,
		F7:       118,
		F8:       119,
		F9:       120,
		F10:      121,
		F11:      122,
		NUMLOCK:  144,
		SCROLLLOCK:   145,
		SUBTRACT:     [109,173,189],
		SEMICOLON:    186,
		EQUAL:        187,
		COMMA:        188,
		//SUBTRACT:     189,
		PERIOD:       190, // period		: UNKOWN
		FORWARDSLASH: 191, // forward slash	: UNKOWN
		GRAVEACCENT:  192, // grave accent	: UNKOWN
		OPENBRACKET:  219, // open bracket	: UNKOWN
		BACKSLASH:    220, // back slash	: UNKOWN
		CLOSEBRACKET: 221, // close bracket	: UNKOWN
		SINGLEQUOTE : 222  // single quote	: UNKOWN
	},

	navigationKeyCodes: {
		left:    [37],
		right:   [39],
		down:    [40],
		up:      [38],
		zoomIn:  [187, 107, 61, 171],
		zoomOut: [189, 109, 173]
	},

	initialize: function (map) {
		this._map = map;
		this._setPanOffset(map.options.keyboardPanOffset);
		this._setZoomOffset(map.options.keyboardZoomOffset);
		this.modifier = 0;
	},

	addHooks: function () {
		var container = this._map._container;

		// make the container focusable by tabbing
		if (container.tabIndex === -1) {
			container.tabIndex = '0';
		}

		L.DomEvent.on(this._map.getContainer(), 'keydown keyup keypress', this._onKeyDown, this);
		L.DomEvent.on(window.document, 'keydown', this._globalKeyEvent, this);
	},

	removeHooks: function () {
		L.DomEvent.off(this._map.getContainer(), 'keydown keyup keypress', this._onKeyDown, this);
		L.DomEvent.off(window.document, 'keydown', this._globalKeyEvent, this);
	},

	_ignoreKeyEvent: function(ev) {
		var shift = ev.shiftKey ? UNOModifier.SHIFT : 0;
		if (shift && (ev.keyCode === this.keyCodes.INSERT || ev.keyCode === this.keyCodes.DELETE)) {
			// don't handle shift+insert, shift+delete
			// These are converted to 'cut', 'paste' events which are
			// automatically handled by us, so avoid double-handling
			return true;
		}
	},

	_setPanOffset: function (pan) {
		var keys = this._panKeys = {},
		    codes = this.navigationKeyCodes,
		    i, len;

		for (i = 0, len = codes.left.length; i < len; i++) {
			keys[codes.left[i]] = [-1 * pan, 0];
		}
		for (i = 0, len = codes.right.length; i < len; i++) {
			keys[codes.right[i]] = [pan, 0];
		}
		for (i = 0, len = codes.down.length; i < len; i++) {
			keys[codes.down[i]] = [0, pan];
		}
		for (i = 0, len = codes.up.length; i < len; i++) {
			keys[codes.up[i]] = [0, -1 * pan];
		}
	},

	_setZoomOffset: function (zoom) {
		var keys = this._zoomKeys = {},
		    codes = this.navigationKeyCodes,
		    i, len;

		for (i = 0, len = codes.zoomIn.length; i < len; i++) {
			keys[codes.zoomIn[i]] = zoom;
		}
		for (i = 0, len = codes.zoomOut.length; i < len; i++) {
			keys[codes.zoomOut[i]] = -zoom;
		}
	},

	// Convert javascript key codes to UNO key codes.
	_toUNOKeyCode: function (keyCode) {
		return this.keymap[keyCode] || keyCode;
	},

	// _onKeyDown - called only as a DOM event handler
	// Calls _handleKeyEvent(), but only if the event doesn't have
	// a charCode property (set to something different than 0) - that ignores
	// any 'beforeinput', 'keypress' and 'input' events that would add
	// printable characters. Those are handled by TextInput.js.
	_onKeyDown: function (ev) {
		if (this._map.uiManager.isUIBlocked() || this._map.isPermissionReadOnly())
			return;

		var completeEvent = app.socket.createCompleteTraceEvent('L.Map.Keyboard._onKeyDown', { type: ev.type, charCode: ev.charCode });
		window.app.console.log('keyboard handler:', ev.type, ev.key, ev.charCode, this._expectingInput, ev);

		if (ev.charCode == 0) {
			this._handleKeyEvent(ev);
		}
		if (this._map._docLayer)
			if (ev.shiftKey && ev.type === 'keydown')
				this._map._docLayer.shiftKeyPressed = true;
			else if (ev.keyCode === this.keyCodes.SHIFT && ev.type === 'keyup')
				this._map._docLayer.shiftKeyPressed = false;
		if (completeEvent)
			completeEvent.finish();
	},

	_globalKeyEvent: function(ev) {
		if (this._map.uiManager.isUIBlocked())
			return;

		if (this._map.jsdialog && this._map.jsdialog.hasDialogOpened()
			&& this._map.jsdialog.handleKeyEvent(ev)) {
			ev.preventDefault();
			return;
		}
	},

	// _handleKeyEvent - checks if the given keyboard event shall trigger
	// a message to oxoolwsd, and calls the given keyEventFn(type, charcode, keycode)
	// callback if so.
	// Called from _onKeyDown
	_handleKeyEvent: function (ev, keyEventFn) {
		if (this._map.uiManager.isUIBlocked())
			return;

		this._map.notifyActive();
		if (this._map.slideShow && this._map.slideShow.fullscreen) {
			return;
		}
		var docLayer = this._map._docLayer;
		if (!keyEventFn && docLayer.postKeyboardEvent) {
			// default is to post keyboard events on the document
			keyEventFn = L.bind(docLayer.postKeyboardEvent, docLayer);
		}

		this.modifier = 0;
		var shift = ev.shiftKey ? UNOModifier.SHIFT : 0;
		var ctrl = ev.ctrlKey ? UNOModifier.CTRL : 0;
		var alt = ev.altKey ? UNOModifier.ALT : 0;
		var cmd = ev.metaKey ? UNOModifier.CTRL : 0;
		var location = ev.location;
		this.modifier = shift | ctrl | alt | cmd;

		// On Windows, pressing AltGr = Alt + Ctrl
		// Presence of AltGr is detected if previous Ctrl + Alt 'location' === 2 (i.e right)
		// because Ctrl + Alt + <some char> won't give any 'location' information.
		if (ctrl && alt) {
			if (ev.type === 'keydown' && location === 2) {
				this._prevCtrlAltLocation = location;
				return;
			}
			else if (location === 1) {
				this._prevCtrlAltLocation = undefined;
			}

			if (this._prevCtrlAltLocation === 2 && location === 0) {
				// and we got the final character
				if (ev.type === 'keypress') {
					ctrl = alt = this.modifier = 0;
				}
				else {
					// Don't handle remnant 'keyup'
					return;
				}
			}
		}

		if (this._handleShortcutCommand(ev)) {
			return;
		}

		var charCode = ev.charCode;
		var keyCode = ev.keyCode;

		var DEFAULT = 0;

		if ((this.modifier == UNOModifier.ALT || this.modifier == UNOModifier.SHIFT + UNOModifier.ALT) &&
			keyCode >= this.keyCodes.NUM0[DEFAULT]) {
			// Presumably a Mac or iOS client accessing a "special character". Just ignore the alt modifier.
			// But don't ignore it for Alt + non-printing keys.
			this.modifier -= alt;
			alt = 0;
		}

		var unoKeyCode = this._toUNOKeyCode(keyCode);

		if (this.modifier) {
			unoKeyCode |= this.modifier;
			if (ev.type !== 'keyup' && (this.modifier !== shift || (keyCode === this.keyCodes.SPACE && !this._map._isCursorVisible))) {
				if (keyEventFn) {
					keyEventFn('input', charCode, unoKeyCode);
					ev.preventDefault();
				}

				return;
			}
		}

		if (this._map.isPermissionEdit()) {
			docLayer._resetPreFetching();

			if (this._ignoreKeyEvent(ev)) {
				// key ignored
			}
			else if (ev.type === 'keydown') {
				// window.app.console.log(e);
				if (this.handleOnKeyDownKeys[keyCode] && charCode === 0) {
					if (keyEventFn) {
						keyEventFn('input', charCode, unoKeyCode);
						ev.preventDefault();
					}
				}
			}
			else if ((ev.type === 'keypress') && (!this.handleOnKeyDownKeys[keyCode] || charCode !== 0)) {
				if (charCode === keyCode && charCode !== 13) {
					// Chrome sets keyCode = charCode for printable keys
					// while LO requires it to be 0
					keyCode = 0;
					unoKeyCode = this._toUNOKeyCode(keyCode);
				}
				if (docLayer._debug) {
					// key press times will be paired with the invalidation messages
					docLayer._debugKeypressQueue.push(+new Date());
				}

				if (keyEventFn) {
					keyEventFn('input', charCode, unoKeyCode);
				}
			}
			else if (ev.type === 'keyup') {
				if ((this.handleOnKeyDownKeys[keyCode] && charCode === 0) ||
				    (this.modifier) ||
				    unoKeyCode === UNOKey.RETURN) {
					if (keyEventFn) {
						keyEventFn('up', charCode, unoKeyCode);
					}
				} else {
					// was handled as textinput
				}
			}
			if (keyCode === this.keyCodes.TAB) {
				// tab would change focus to other DOM elements
				ev.preventDefault();
			}
		}
		else if (!this.modifier && (keyCode === this.keyCodes.pageUp || keyCode === this.keyCodes.pageDown) && ev.type === 'keydown') {
			if (this._map._docLayer._docType === 'presentation' || this._map._docLayer._docType === 'drawing') {
				var partToSelect = keyCode === this.keyCodes.pageUp ? 'prev' : 'next';
				this._map._docLayer._preview._scrollViewByDirection(partToSelect);
				if (app.file.fileBasedView)
					this._map._docLayer._checkSelectedPart();
			}
			return;
		}
		else if (!this.modifier && (keyCode === this.keyCodes.END || keyCode === this.keyCodes.HOME) && ev.type === 'keydown') {
			if (this._map._docLayer._docType === 'drawing' && app.file.fileBasedView === true) {
				partToSelect = keyCode === this.keyCodes.HOME ? 0 : this._map._docLayer._parts -1;
				this._map._docLayer._preview._scrollViewToPartPosition(partToSelect);
				this._map._docLayer._checkSelectedPart();
			}
			return;
		}
		else if (ev.type === 'keydown') {
			var key = ev.keyCode;
			var map = this._map;
			if (key in this._panKeys && !ev.shiftKey) {
				if (map._panAnim && map._panAnim._inProgress) {
					return;
				}
				map.fire('scrollby', {x: this._panKeys[key][0], y: this._panKeys[key][1]});
			}
			else if (key in this._panKeys && ev.shiftKey &&
					!docLayer._textCSelections.empty()) {
				// if there is a selection and the user wants to modify it
				if (keyEventFn) {
					keyEventFn('input', charCode, unoKeyCode);
				}
			}
			else if (key in this._zoomKeys) {
				map.setZoom(map.getZoom() + (ev.shiftKey ? 3 : 1) * this._zoomKeys[key], null, true /* animate? */);
			}
		}

		L.DomEvent.stopPropagation(ev);
	},

	_isCtrlKey: function (e) {
		if (window.ThisIsTheiOSApp || navigator.appVersion.indexOf('Mac') != -1 || navigator.userAgent.indexOf('Mac') != -1)
			return e.metaKey;
		else
			return e.ctrlKey;
	},

	/**
	 * 處理鍵盤快捷鍵，以執行相應的指令或訊息
	 * Handle keyboard shortcuts to execute corresponding commands or messages.
	 * @param {object} e - keyboard event
	 * @returns true:已處理(processed), false: 未處理(not processed)
	 */
	_handleShortcutCommand: function(e) {
		if (this._map.uiManager.isUIBlocked()) {
			return;
		}

		// 指按下 Shift / Control / Alt 未配合其他按鍵
		if (e.keyCode === this.keyCodes.SHIFT || e.keyCode === this.keyCodes.CTRL || e.keyCode === this.keyCodes.ALT) {
			return true;
		}

		if (this.modifier === UNOModifier.CTRL && e.type !== 'keydown' && e.key !== 'c' && e.key !== 'v' && e.key !== 'x' &&
		/* Safari */ e.keyCode !== 99 && e.keyCode !== 118 && e.keyCode !== 120) {
			e.preventDefault();
			return true;
		}

		// 組合按鍵易讀名稱
		var hotkey = [];
		if (this.modifier & UNOModifier.CTRL)
			hotkey.push('Ctrl');
		if (this.modifier & UNOModifier.ALT)
			hotkey.push('Alt');
		if (this.modifier &  UNOModifier.SHIFT)
			hotkey.push('Shift');

		hotkey.push(e.key.startsWith('Arrow') ? e.key.substr(5) : e.key);
		var mergeKeys = hotkey.join('+').toLowerCase();

		// 只處理 key down
		if (e.type === 'keydown') {
			if (!window.ThisIsAMobileApp) {
				switch (mergeKeys) {
				case 'ctrl+c': // copy
				case 'ctrl+x': // cut
					// we prepare for a copy or cut event
					this._map.focus();
					this._map._textInput.select();
					return true;
				case 'ctrl+v': // paste
					return true;
				}
			}

			// 焦點在編輯區時，才比對並執行相應的快捷鍵
			if (this._map.editorHasFocus()) {
				switch (mergeKeys) {
				case 'ctrl+f': // 搜尋(search)
					if (!this._map.uiManager.isStatusBarVisible()) {
						this._map.uiManager.showStatusBar();
					}
					this._map.fire('focussearch');
					e.preventDefault();
					return true;
				case 'ctrl+alt+shift+d': // 切換除錯模式
					this._map._docLayer.toggleTileDebugMode();
					break;
				default:
					var command = this._map.getHotkeyCommand(mergeKeys);
					if (command) {
						window.app.console.debug('Found Shortcut command:' + command);
						if (this._map.executeAllowedCommand(command)) {
							e.preventDefault();
							return true;
						}
					}
					break;
				}
			}
		} else if (e.type === 'keypress') {
			if (mergeKeys === 'ctrl+c' || mergeKeys === 'ctrl+v' || mergeKeys === 'ctrl+x') {
				// need to handle this separately for Firefox
				return true;
			}
		}
		return false;
	}
});
