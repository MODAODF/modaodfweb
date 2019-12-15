/* -*- js-indent-level: 8 -*- */
/*
 * L.Control.Tabs is used to switch sheets in Calc
 */

/* global $ vex _ _UNO */
L.Control.Tabs = L.Control.extend({
	onAdd: function(map) {
		map.on('updatepermission', this._onUpdatePermission, this);
		this._initialized = false;
	},

	_onUpdatePermission: function(e) {
		if (this._map.getDocType() !== 'spreadsheet') {
			return;
		}

		if (!this._initialized) {
			this._initialize();
		}
		setTimeout(function() {
			$('.spreadsheet-tab').contextMenu(e.perm === 'edit');
		}, 100);

		if (window.mode.isMobile() == true) {
			if (e.perm === 'edit') {
				$('.spreadsheet-tabs-container')
					.removeClass('mobile-view')
					.addClass('mobile-edit');
			} else {
				$('.spreadsheet-tabs-container')
					.addClass('mobile-view')
					.addClass('mobile-edit');
			}
		}
	},

	_initialize: function () {
		this._initialized = true;
		this._tabsInitialized = false;
		this._spreadsheetTabs = {};
		this._tabForContextMenu = 0;
		var map = this._map;
		var that = this;
		var docContainer = map.options.documentContainer;
		this._tabsCont = L.DomUtil.create('div', 'spreadsheet-tabs-container', docContainer.parentElement);
		L.DomEvent.on(this._tabsCont, 'touchstart',
			function (e) {
				if (e && e.touches.length > 1) {
					L.DomEvent.preventDefault(e);
				}
			},
			this);

		L.installContextMenu({
			selector: '.spreadsheet-tab',
			className: 'loleaflet-font',
			callback: (function(key) {
				if (key === 'insertsheetbefore') {
					map.insertPage(this._tabForContextMenu);
				}
				if (key === 'insertsheetafter') {
					map.insertPage(this._tabForContextMenu + 1);
				}
			}).bind(this),
			items: {
				'insertsheetbefore': {name: _('Insert sheet before this')},
				'insertsheetafter': {name: _('Insert sheet after this')},
				'movesheets': {
					name: _UNO('.uno:Move', 'spreadsheet', true),
					callback: (function() {
						that._movePart();
					}).bind(this)
				},
				'deletesheet': {name: _UNO('.uno:Remove', 'spreadsheet', true),
						callback: (function(key, options) {
							var nPos = this._tabForContextMenu;
							vex.dialog.confirm({
								message: _('Are you sure you want to delete sheet, %sheet% ?').replace('%sheet%', options.$trigger.text()),
								callback: function(data) {
									if (data) {
										map.deletePage(nPos);
									}
								}
							});
						}).bind(this)
				 },
				'renamesheet': {name: _UNO('.uno:RenameTable', 'spreadsheet', true),
							callback: (function(key, options) {
								var nPos = this._tabForContextMenu;
								vex.dialog.open({
									message: _('Enter new sheet name'),
									input: '<input name="sheetname" type="text" value="' + options.$trigger.text() + '" required />',
									callback: function(data) {
										if (!data)
											return;

										if (map.isSheetnameValid(data.sheetname, nPos)) {
											map.renamePage(data.sheetname, nPos);
										} else {
											var msg = _('Invalid sheet name.\nThe sheet name must not be empty or a duplicate of \nan existing name and may not contain the characters [ ] * ? : / \\ \nor the character \' (apostrophe) as first or last character.');
											vex.dialog.alert(msg.replace(/\n/g, '<br>'));
										}
									}
								});
							}).bind(this)
				} ,
				'showsheets': {
					name: _UNO('.uno:Show', 'spreadsheet', true),
					callback: (function() {
						that._showPage();
					}).bind(this)
				},
				'hiddensheets': {
					name: _UNO('.uno:Hide', 'spreadsheet', true),
					callback: (function() {
						map.hidePage();
					}).bind(this)
				}
			},
			zIndex: 1000
		});

		map.on('updateparts', this._updateDisabled, this);
	},

	_showPage: function () {
		var map = this._map;
		if (!map.hasAnyHiddenPart()) {
			return;
		}
		var hiddenNames = map.getHiddenPartNames().split(',');
		var sels = '<div style="height:200px; overflow-y:auto; overflow-y:none; padding: 5px; border:1px #bbbbbb solid">';
		for (var i=0 ; i < hiddenNames.length ; i++) {
			sels += '<input type="checkbox" name="' + hiddenNames[i] + '" ' +
					'value="' + hiddenNames[i] + '">' + hiddenNames[i] + '<br>';
		}
		vex.dialog.open({
			message: _('Hidden Sheets'),
			input: sels,
			buttons: [
				$.extend({}, vex.dialog.buttons.YES, { text: _('OK') }),
				$.extend({}, vex.dialog.buttons.NO, { text: _('Cancel') })
			],
			callback: function(data) {
				for (var sheet in data) {
					map.showPage(sheet);
				}
			}
		});
	},

	_movePart: function () {
		var map = this._map;
		var partNames = map._docLayer._partNames;
		var currPart = map.getCurrentPartNumber();
		var options = '';
		for (var i = 0 ; i < partNames.length ; i++) {
			if (!map.isHiddenPart(i)) {
				options += '<option value="' + (i+1) + '">' + partNames[i] + '</option>';
			}
		}
		options += '<option value="32767">' + _('- move to end position -') + '</option>';
		vex.dialog.open({
			message: _UNO('.uno:Move', 'spreadsheet', true),
			input: [
				'<div><input type="radio" id="movepart" name="copypart" value="false" checked><label for="movepart"> ' + _('Move') + '</label>&emsp;&emsp;&emsp;&emsp;' +
				'<input type="radio" id="copypart" name="copypart" value="true"><label for="copypart"> ' + _('Copy') + '</label></div>',
				'<div><b>' + _('Insert before') + '</b></div>',
				'<select name="moveTo" size="10" style="font-size:16px; width: 100%;">' + options + '</select>'
			],
			callback: function (data) {
				if (data !== undefined) {
					// 移動或複製
					var copy = (data.copypart === 'true');
					var pos = data.moveTo;
					if (pos === undefined) {
						pos = currPart + 2;
						if (pos > partNames.length)
							pos = 32767; // 最後
					}
					var params = {
						'DocName': {
							'type': 'string',
							'value': map.getDocName()
						},
						'Index': {
							'type': 'long',
							'value': pos
						},
						'Copy': {
							'type': 'boolean',
							'value': copy
						}
					}
					map.sendUnoCommand('.uno:Move', params);
				}
			}
		});
	},

	_updateDisabled: function (e) {
		var parts = e.parts;
		var selectedPart = e.selectedPart;
		var docType = e.docType;
		if (docType === 'text') {
			return;
		}
		if (docType === 'spreadsheet') {
			if (!this._tabsInitialized) {
				// make room for the preview
				var docContainer = this._map.options.documentContainer;
				L.DomUtil.addClass(docContainer, 'spreadsheet-document');
				setTimeout(L.bind(function () {
					this._map.invalidateSize();
					$('.scroll-container').mCustomScrollbar('update');
					$('.scroll-container').mCustomScrollbar('scrollTo', [0, 0]);
				}, this), 100);
				this._tabsInitialized = true;
			}
			if ('partNames' in e) {
				while (this._tabsCont.firstChild) {
					this._tabsCont.removeChild(this._tabsCont.firstChild);
				}
				var ssTabScroll = L.DomUtil.create('div', 'spreadsheet-tab-scroll', this._tabsCont);
				ssTabScroll.id = 'spreadsheet-tab-scroll';

				for (var i = 0; i < parts; i++) {
					if (e.hiddenParts.indexOf(i) !== -1)
						continue;
					var id = 'spreadsheet-tab' + i;
					var tab = L.DomUtil.create('button', 'spreadsheet-tab', ssTabScroll);
					L.DomEvent.enableLongTap(tab, 15, 1000);
					
					L.DomEvent.on(tab, 'contextmenu', function(j) {
						return function() {
							this._tabForContextMenu = j;
							$('#spreadsheet-tab' + j).contextMenu();
						}
					}(i).bind(this));
					
					tab.textContent = e.partNames[i];
					tab.id = id;

					L.DomEvent
						.on(tab, 'click', L.DomEvent.stopPropagation)
						.on(tab, 'click', L.DomEvent.stop)
						.on(tab, 'mousedown', this._setPart, this)
						.on(tab, 'mousedown', this._map.focus, this._map);
					this._spreadsheetTabs[id] = tab;
				}
			}
			for (var key in this._spreadsheetTabs) {
				var part =  parseInt(key.match(/\d+/g)[0]);
				L.DomUtil.removeClass(this._spreadsheetTabs[key], 'spreadsheet-tab-selected');
				if (part === selectedPart) {
					L.DomUtil.addClass(this._spreadsheetTabs[key], 'spreadsheet-tab-selected');
				}
			}
		}
	},

	_setPart: function (e) {
		var part =  e.target.id.match(/\d+/g)[0];
		if (part !== null) {
			this._map.setPart(parseInt(part), /*external:*/ false, /*calledFromSetPartHandler:*/ true);
		}
	}
});

L.control.tabs = function (options) {
	return new L.Control.Tabs(options);
};
