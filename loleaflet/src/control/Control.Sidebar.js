/* -*- js-indent-level: 8 -*- */
/*
 * L.Control.Sidebar
 */

/* global $ */
L.Control.Sidebar = L.Control.extend({

	options: {
		animSpeed: 1000 /* Default speed: to be used on load */
	},

	container: null,
	builder: null,

	onAdd: function (map) {
		this.map = map;

		this.builder = new L.control.jsDialogBuilder({mobileWizard: this, map: map, cssClass: 'jsdialog sidebar'});
		this.container = L.DomUtil.create('div', 'sidebar-container', $('#sidebar-panel').get(0));

		this.map.on('sidebar', this.onSidebar, this);
		this.map.on('jsdialogupdate', this.onJSUpdate, this);
		this.map.on('jsdialogaction', this.onJSAction, this);
	},

	onRemove: function() {
		this.map.off('sidebar');
		this.map.off('jsdialogupdate', this.onJSUpdate, this);
		this.map.off('jsdialogaction', this.onJSAction, this);
	},

	closeSidebar: function() {
		$('#sidebar-dock-wrapper').hide();
		this.map._onResize();
		this.map.dialog._resizeCalcInputBar();

		if (!this.map.editorHasFocus()) {
			this.map.fire('editorgotfocus');
			this.map.focus();
		}

		if (window.initSidebarState) {
			this.map.uiManager.setSavedState('ShowSidebar', false);
			window.initSidebarState = false;
		}
	},

	onJSUpdate: function (e) {
		var data = e.data;

		if (data.jsontype !== 'sidebar')
			return;

		if (!this.container)
			return;

		var control = this.container.querySelector('[id=\'' + data.control.id + '\']');
		if (!control) {
			window.app.console.warn('jsdialogupdate: not found control with id: "' + data.control.id + '"');
			return;
		}

		var parent = control.parentNode;
		if (!parent)
			return;

		if (!this.builder)
			return;

		var scrollTop = control.scrollTop;
		var focusedElement = document.activeElement;
		var focusedElementInDialog = focusedElement ? this.container.querySelector('[id=\'' + focusedElement.id + '\']') : null;
		var focusedId = focusedElementInDialog ? focusedElementInDialog.id : null;
		control.style.visibility = 'hidden';

		var temporaryParent = L.DomUtil.create('div');
		this.builder.build(temporaryParent, [data.control], false);
		parent.insertBefore(temporaryParent.firstChild, control.nextSibling);
		L.DomUtil.remove(control);

		var newControl = this.container.querySelector('[id=\'' + data.control.id + '\']');
		if (newControl)
			newControl.scrollTop = scrollTop;

		if (focusedId)
			this.container.querySelector('[id=\'' + focusedId + '\']').focus();
	},

	onJSAction: function (e) {
		var data = e.data;

		if (data.jsontype !== 'sidebar')
			return;

		if (!this.builder)
			return;

		if (!this.container)
			return;

		// Panels share the same name for main containers, do not execute actions for them
		// if panel has to be shown or hidden, full update will appear
		if (data.data && (data.data.control_id === 'contents' ||
			data.data.control_id === 'Panel' ||
			data.data.control_id === 'titlebar')) {
			window.app.console.log('Ignored action: ' + data.data.action_type + ' for control: ' + data.data.control_id);
			return;
		}

		this.builder.executeAction(this.container, data.data);
	},

	onResize: function() {
		var wrapper = document.getElementById('sidebar-dock-wrapper');
		wrapper.style.maxHeight = document.getElementById('document-container').getBoundingClientRect().height + 'px';
	},

	onSidebar: function(data) {
		var sidebarData = data.data;
		this.builder.setWindowId(sidebarData.id);
		$(this.container).empty();

		if (window.app.dontUseSidebar || sidebarData.action === 'close' || window.app.file.fileBasedView || this.map.isPermissionReadOnly()) {
			this.closeSidebar();
		} else if (sidebarData.children) {
			for (var i = sidebarData.children.length - 1; i >= 0; i--) {
				if (sidebarData.children[i].type !== 'deck' || sidebarData.children[i].visible === false)
					sidebarData.children.splice(i, 1);
			}

			if (sidebarData.children.length) {
				var wrapper = document.getElementById('sidebar-dock-wrapper');

				this.onResize();
				this.map.dialog._resizeCalcInputBar();

				this.builder.build(this.container, [sidebarData]);
				if (wrapper.style.display === 'none')
					$('#sidebar-dock-wrapper').show(this.options.animSpeed);

				if (window.initSidebarState) {
					this.map.uiManager.setSavedState('ShowSidebar', true);
					window.initSidebarState = false;
				}
			} else {
				this.closeSidebar();
			}
		}
	},

	setTabs: function(/*tabs*/) {
	},

	selectedTab: function() {
		// implement in child classes
	},
});

L.control.sidebar = function (options) {
	return new L.Control.Sidebar(options);
};
