/* -*- js-indent-level: 8 -*- */
/*
 * L.dialog.PdfWatermarkText
 * 設定列印或下載為 pdf 的浮水印文字
 *
 * Author: Firefly <firefly@ossii.com.tw>
 */
/* global app $ _ */
L.dialog.PdfWatermarkText = {
	_dialog: L.DomUtil.create('div', '', document.body),

	_hasScreenWatermark: false, // 有無螢幕浮水印

	l10n: [
		_('Watermark text'), // 浮水印文字
		_('Use screen watermark'), // 採用螢幕浮水印
		_('If no needed, please leave it blank.'), // 如果不需要，請保持空白
		_('Direction'), // 方向
		// 替文件中的超連結(如果有的話)製作 QR Code.
		_('Create a QR code for hyperlinks (if any) in the document.'),
	],

	// init 只會在載入的第一次執行
	initialize: function() {
		var that = this;
		this._id = $(this._dialog).uniqueId().attr('id');
		this._watermarkTextId = this._id + '_watermarkText';
		this._useScreenId = this._id + '_useScreen';
		this._hyperlinkQRCode = this._id + '_hyperlinkQRCode';
		this._hasScreenWatermark = (this._map.options.watermark !== undefined && this._map.options.watermark.editing === true);
		this._dialog.innerHTML =
			'<label for="' + this._watermarkTextId + '" _="Watermark text"></label>' +
			'<textarea id="' + this._watermarkTextId + '" rows=3 maxlength=128 style="resize:none; width:100%" title="If no needed, please leave it blank."></textarea><br>' +
			'<button id="' + this._useScreenId + '" style="color:blue; font-size:12px; float:right; cursor:pointer; display:none" _="Use screen watermark"></button><br>' +

			'<fieldset style="margin-top:8px;"><legend _="Direction"></legend>' +
			'<label style="margin-right:24px;"><input type="radio" name="watermarkAngle" value="45" checked> <span _="Diagonal"></span></label>' +
			'<label><input type="radio" name="watermarkAngle" value="0"> <span _="Horizontal"></span></label>' +
			'</fieldset>';
		this._dialog.innerHTML +=
			'<input type="checkbox" id="' + this._hyperlinkQRCode + '" name="hyperlinkQRCode">' +
			'<label for="' + this._hyperlinkQRCode + '" _="Create a QR Code for hyperlinks (if any) in the document."></label>';

		this._map.translationElement(this._dialog);

		// 有螢幕浮水印，啟用使用螢幕浮水印按鈕
		if (this._hasScreenWatermark) {
			var $useScreenButton = $('#' + this._useScreenId);
			$useScreenButton.click(function() {
				var watermarkText = document.getElementById(that._watermarkTextId);
				// 複製螢幕浮水印文字到輸入區
				watermarkText.value = that._map.options.watermark.text;
				watermarkText.focus(); // 聚焦
			}).show();
		}

		$(this._dialog).dialog({
			title: _('Add watermark'),
			closeText: _('Close'),
			position: {my: 'center', at: 'center', of: window},
			minWidth: 250,
			autoOpen: false, // 不自動顯示對話框
			modal: true,	// 獨占模式
			resizable: false, // 不能縮放視窗
			draggable: true, // 可以拖放視窗
			closeOnEscape: true, // 按 esc 視同關閉視窗
			buttons: [
				{
					text: _('Without watermark'), // 不用浮水印
					click: function() {
						that.needInsertQRCode();
						that._map.showBusy(_('Downloading...'), false);
						app.socket.sendMessage('downloadas ' +
							'name=' + encodeURIComponent(that._args.name) + ' ' +
							'id=' + that._args.id + ' ' +
							'format=pdf ' +
							'options=' + that._args.options);
						$(this).dialog('close');
					}
				},
				{
					text: _('OK'),
					click: function() {
						that.needInsertQRCode();
						// 文字
						var text = document.getElementById(that._watermarkTextId).value.trim();
						// 角度
						var angle = document.querySelector('input[name="watermarkAngle"]:checked').value;
						var watermark = {
							text: text,
							angle: angle,
							familyname: 'Carlito',
							color: '#000000',
							opacity: 0.2
						};
						var jsonStr = JSON.stringify(watermark);
						var watermarkText = (text !== '') ? ',Watermark=' + jsonStr + 'WATERMARKEND' : '';
						that._map.showBusy(_('Downloading...'), false);
						app.socket.sendMessage('downloadas ' +
							'name=' + encodeURIComponent(that._args.name) + ' ' +
							'id=' + that._args.id + ' ' +
							'format=pdf ' +
							'options=' + that._args.options + watermarkText);
						$(this).dialog('close');
					}
				},
				{
					text: _('Cancel'),
					click: function() {
						$(this).dialog('close');
					}
				}
			]
		});
	},

	needInsertQRCode: function() {
		var isNeed = document.getElementById(this._hyperlinkQRCode).checked;
		// 如果需要製作超連結 QR Codes
		if (isNeed) {
			// 1. 如果檔案有修改過，要先存檔
			if (this._map._everModified) {
				this._map.save(true, true);
			}

			// 2. 呼叫製作超連結 QR Code 巨集
			this._map.sendUnoCommand('macro:///QRcodePrint.QRcodePrint.QRcodeRun()');
		}
	},

	// 每次都會從這裡開始
	run: function(param) {
		this._args = param.args;
		if (this._args === undefined) {
			return;
		}
		this._map.hideBusy();
		$(this._dialog).dialog('open');
	},
};
