/* -*- js-indent-level: 8 -*- */
/*
	View logs in the admin console.
*/
/* global $ AdminSocketBase Admin */
var AdminSocketTemplaterepo = AdminSocketBase.extend({
	constructor: function(host) {
		this.base(host);
	},

	_intervalId: 0,

	onSocketOpen: function() {
		// Base class' onSocketOpen handles authentication
		this.base.call(this);
		this.socket.send('module templaterepo getModuleVersion');
	},

	onSocketMessage: function(e) {
		var textMsg;
		if (typeof e.data === 'string')
		{
			textMsg = e.data;
		}
		else
		{
			textMsg = '';
		}

		if (textMsg.startsWith('getModuleVersion '))
		{
			var version = textMsg.split('getModuleVersion')[1];
			$('#moduleVersion').html('模組版本: ' + version);
		}
	},

	onSocketClose: function() {
		clearInterval(this._intervalId);
		this.base.call(this);
	}
});

Admin.Templaterepo = function(host)
{
	return new AdminSocketTemplaterepo(host);
};
