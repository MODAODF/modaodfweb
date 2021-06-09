/* -*- js-indent-level: 8 -*- */
/*
	View logs in the admin console.
*/
/* global vex $ AdminSocketBase Admin */
var AdminSocketMergeODF = AdminSocketBase.extend({
	constructor: function(host) {
		this.base(host);
	},

	_intervalId: 0,
    _rec_id: 0,
	_pause:  false,	// 是否暫停

	getLog: function() {
		if (!this._pause)
		{
			this.socket.send('module mergeodf get_log ' + this._rec_id);
		}
	},

	onSocketOpen: function() {
		// Base class' onSocketOpen handles authentication
		this.base.call(this);
        this.socket.send('module mergeodf get_log ' + this._rec_id);
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
        $("#get_signature").click(function()
            {
                mergeSocket.socket.send("module mergeodf get_signature");
            });
        $("#send_authorize").click(function()
            {
                authorize_file = $("input[id=authorize_file]")[0].files[0];
                var reader = new FileReader();
                reader.onload = function(e)
                {
                    var command = "module mergeodf send_authorize ";
                    command = command + btoa(e.target.result);
                    mergeSocket.socket.send(command);
                };
                reader.readAsBinaryString(authorize_file);
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

        if(textMsg.startsWith("get_log "))
        {
            log_data = textMsg.split("get_log")[1];
            result = JSON.parse(log_data.trim());
            for(id in result)
            {
                data = result[id];
                loggingTable.row.add([
                        data['status'] == 1 ? "成功" : "失敗",
                        data['to_pdf'] == 1 ? "V" : "X",
                        data['timestamp'],
                        data['source_ip'],
                        data['file_name'],
                        data['file_ext'],
                ]).node().id = ("rec_id_" + data['rec_id']);
                if (Number(data['rec_id']) > this._rec_id)
                    this._rec_id = Number(data['rec_id']);

            }
            loggingTable.draw();
        }
        if(textMsg.startsWith("getModuleVersion "))
        {
            version = textMsg.split("getModuleVersion")[1];
            $("#moduleVersion").html(`模組版本: ${version}`);
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
