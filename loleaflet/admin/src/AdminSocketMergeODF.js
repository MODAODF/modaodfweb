/* -*- js-indent-level: 8 -*- */
/*
	View logs in the admin console.
*/
/* global $ AdminSocketBase Admin loggingTable */
var AdminSocketMergeODF = AdminSocketBase.extend({
	constructor: function(host) {
		this.base(host);
	},

	_intervalId: 0,
	_recId: 0,
	_pause:  false,	// 是否暫停

	getLog: function() {
		if (!this._pause)
		{
			this.socket.send('module mergeodf get_log ' + this._recId);
		}
	},

	onSocketOpen: function() {
		// Base class' onSocketOpen handles authentication
		this.base.call(this);
		this.socket.send('module mergeodf get_log ' + this._recId);
		this.socket.send('module mergeodf getModuleVersion');
		var socketLogView = this;

		//初始化畫面的按鈕事件
		$('#Pause').click(function ()
		{
			socketLogView._pause = !socketLogView._pause;
			$('#Pause').text(socketLogView._pause ? '持續更新' : '暫停');
			$('#Pause').removeClass(socketLogView._pause ? 'btn-warning' : 'btn-success');
			$('#Pause').addClass(socketLogView._pause ? 'btn-success' : 'btn-warning');
		});
		$('#get_signature').click(function()
		{
			this.socket.send('module mergeodf get_signature');
		});
		$('#send_authorize').click(function()
		{
			var authorizeFile = $('input[id=authorizeFile]')[0].files[0];
			var reader = new FileReader();
			reader.onload = function(e)
			{
				var command = 'module mergeodf send_authorize ';
				command = command + btoa(e.target.result);
				this.socket.send(command);
			};
			reader.readAsBinaryString(authorizeFile);
		});
		this._intervalId = setInterval(function() {
			return socketLogView.getLog();
		}, 5000);
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

		if (textMsg.startsWith('get_log '))
		{
			var logData = textMsg.split('get_log')[1];
			var result = JSON.parse(logData.trim());
			for (var id in result)
			{
				var data = result[id];
				loggingTable.row.add([
					data['status'] == 1 ? '成功' : '失敗',
					data['to_pdf'] == 1 ? 'V' : 'X',
					data['timestamp'],
					data['source_ip'],
					data['file_name'],
					data['file_ext'],
				]).node().id = ('rec_id_' + data['rec_id']);
				if (Number(data['rec_id']) > this._recId)
					this._recId = Number(data['rec_id']);

			}
			loggingTable.draw();
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

Admin.MergeODF = function(host)
{
	return new AdminSocketMergeODF(host);
};
