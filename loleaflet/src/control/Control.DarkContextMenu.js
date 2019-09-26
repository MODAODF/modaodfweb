/* -*- js-indent-level: 8 -*- */
/*
* Control.ContextMenu
*/

/* global $ _UNO*/
L.Control.DarkContextMenu = L.Control.extend({
	options: {
		SEPARATOR: '---------'
	},

	onAdd: function(map) {
		this._prevMousePos = null;
		map.on('updatepermission', this._onUpdatePermission, this); // Check Permission
	},

	// Add by Firefly <firefly@ossii.com.tw>
	// Right-click menu only in edit mode.
	_onUpdatePermission: function(e) {
		if (e.perm === 'edit') {
			this._map.on('locontextmenu', this._onContextMenu, this);
			this._map.on('mousedown', this._onMouseDown, this);
			this._map.on('keydown', this._onKeyDown, this);
		}
	},

	_onMouseDown: function(e) {
		this._prevMousePos = {x: e.originalEvent.pageX, y: e.originalEvent.pageY};

		$.contextMenu('destroy', '.leaflet-layer');
	},

	_onKeyDown: function(e) {
		if (e.originalEvent.keyCode === 27 /* ESC */) {
			$.contextMenu('destroy', '.leaflet-layer');
		}
	},

	_onContextMenu: function(obj) {
		var map = this._map;
		if (map._permission !== 'edit' && map._permission !== 'view') {
			return;
		}

		var contextMenu = this._createContextMenuStructure(obj);
		L.installContextMenu({
			selector: '.leaflet-layer',
			className: 'loleaflet-font',
			trigger: 'none',
			build: function() {
				return {
					callback: function(key) {
						// Modify by Firefly <firefly@ossii.com.tw>
						// 把插入註解的行為統一化
						if (key === '.uno:InsertAnnotation')
							map.insertComment();
						else {
							map.sendUnoCommand(key);
							// Give the stolen focus back to map
							map.focus();
						}
					},
					items: contextMenu
				};
			}

		});

		$('.leaflet-layer').contextMenu(this._prevMousePos);
	},

	_createContextMenuStructure: function(obj) {
		var docType = this._map.getDocType();
		var contextMenu = {};
		var sepIdx = 1, itemName;
		var isLastItemText = false;
	
		for (var idx in obj.menu) {
			var item = obj.menu[idx];
			
			if (item.enabled === 'false') { continue; }

			if (item.type !== 'separator') {
				// 取得 uno command 翻譯
				itemName = _UNO(item.command, docType, true);
				// 沒有翻譯的話，用 item.text 當選項標題
				if (item.command === '.uno:' + itemName)
				{
					itemName = item.text;
				}
			}

			if (item.type === 'separator') {
				if (isLastItemText) {
					contextMenu['sep' + sepIdx++] = '---------';
				}
				isLastItemText = false;
			}
			else if (item.type === 'command') {
				contextMenu[item.command] = {
					name: itemName
				};

				if (item.checktype === 'checkmark') {
					if (item.checked === 'true') {
						contextMenu[item.command]['icon'] = 'lo-checkmark';
					}
				} else if (item.checktype === 'radio') {
					if (item.checked === 'true') {
						contextMenu[item.command]['icon'] = 'radio';
					}
				}
				isLastItemText = true;
			} else if (item.type === 'menu') {
				var submenu = this._createContextMenuStructure(item);
				// ignore submenus with all items disabled
				if (Object.keys(submenu).length === 0) {
					continue;
				}

				contextMenu[item.command] = {
					name: itemName,
					items: submenu
				};
				isLastItemText = true;
			}
		}

		// Remove separator, if present, at the end
		var lastItem = Object.keys(contextMenu)[Object.keys(contextMenu).length - 1];
		if (lastItem !== undefined && lastItem.startsWith('sep')) {
			delete contextMenu[lastItem];
		}

		return contextMenu;
	}
});

L.control.darkContextMenu = function (options) {
	return new L.Control.DarkContextMenu(options);
};

// Using 'click' and <a href='#' is vital for copy/paste security context.
L.installContextMenu = function(options) {
	options.itemClickEvent = 'click';
	var rewrite = function(items) {
		if (items === undefined)
			return;
		var keys = Object.keys(items);
		for (var i = 0; i < keys.length; i++) {
			var key = keys[i];
			if (items[key] === undefined)
				continue;
			if (!items[key].isHtmlName) {
				console.log('re-write name ' + items[key].name);
				items[key].name = '<a href="#" class="context-menu-link">' + items[key].name + '</a';
				items[key].isHtmlName = true;
			}
			rewrite(items[key].items);
		}
	}
	rewrite(options.items);
	$.contextMenu(options);
};
