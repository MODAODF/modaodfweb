/* -*- js-indent-level: 8 -*- */
/*
 * Copyright the OxOffice Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/* global L _ $ */

L.AdminModule.Table2SC = L.AdminModule.extend({

	// 完整的 API 位址
	_fullServiceURI: "",

	_l10n: [
		_('Table conversion spreadsheet'), // 表格轉試算表

		_('Module overview'), // 模組概覽
		_('API Server address'), // API 伺服器位址
		_('Usage'), // 使用方式
		_('Use Javascript to obtain more than one HTML table in a web page and treat it as text content. The content can also contain other types of HTML tags.'), // 利用 Javascript 取得網頁內一個以上的 HTML 表格，當成文字內容，內容中亦>可含有其他類型的 HTML 標籤。
		_('Use the HTTP POST method to send the text to the API server address as the form content.'), // 將該文字當成表單內容，利用 HTTP POST 方法，送至 API 伺服器位址。
		_('The form has three fields:'), // 表單有三個欄位
		_('Title (optional), used for records, blank is not recommended.'), // 標題(可選)，用作紀錄用，不建議空白。
		_('Conversion format (optional), the default is "ods", you can also specify "xls", "xlsx" or "pdf".'), // 轉換格式(可選)，預設為 "ods"，亦可指定 "xls"、"xlsx" 或 "pdf"。
		_('Table content (required field).'), // 表格內容(必要欄位)。
		_('Form example'), // 表單範例

		_('View log'), // 檢視日誌
		_('Refresh Log'), // 更新日誌

		_('API testing'), // API 測試
		_('HTML Table'), // HTML 表格
		_('Download as ODF'), // 下載 ODF 格式
        _('Download as XLSX'), // 下載 Excel 格式
        _('Download as PDF'), // 下載 PDF 格式
	],

	/**
	 * Initialize the module.
	 *
	 * When implementing this module, the system has built-in several methods for you:
	 * -------------------------------------------------------------------------------
	 * 1. this.getDetail() get the detail object of this module
	 * 2. this.sendMessage(textMsg) send a message to own module's handleAdminMessage() function.
	 */
	initialize: function() {
		// 紀錄完整的 API 位址
		this._fullServiceURI = window.location.origin + this.getDetail().serviceURI;
		this.initializeComponent();
		this.sendMessage('getLog');
	},

	/**
	 * Process messages from the owning service module.
	 *
	 * @param {string} textMsg - the message text
	 */
	onMessage: function(textMsg) {
		// 日誌內容
		if (textMsg.startsWith('log ')) {
			const logElm = document.getElementById('log');
			logElm.textContent = textMsg.substring(4);
		} else {
			console.debug('warning! received an unknown message:"' + textMsg + '"');
		}
	},

	/**
	 * When this module is to be terminated.
	 */
	terminate: function () {
		// put your termination code here
	},

	initializeComponent: function() {
		$('#tbl2scURI').text(this._fullServiceURI);

		document.getElementById('demoForm').setAttribute('action', this._fullServiceURI);
		document.getElementById('demoForm').removeAttribute('id');
		// 把表單範例 HTML 轉成普通文字
		var formExample = document.getElementById('formExample');
		var innerHTML = formExample.innerHTML;
		formExample.innerHTML = "";
		formExample.innerText = innerHTML;
		formExample.classList.remove('d-none'); // 顯示

		var $form = $('#convertDemo');
		$form.attr('action', this._fullServiceURI);

		// 找出所有 attribute 為 'data-fotmat' 的元素，不要用 jQuery 的方法
		var formatButtons = document.querySelectorAll('[data-format]');
		for (var i = 0; i < formatButtons.length; i++) {
			formatButtons[i].addEventListener('click', function(e) {
				var format = e.target.getAttribute('data-format');
				this.getTableContent(format);
			}.bind(this));
		}

		// log
		$('#refreshLog').click(function(e) {
			this.sendMessage('getLog');
		}.bind(this));
	},

	getTableContent: function (format) {
		var tableDemo = document.getElementById("tableDemo");
		var tableFormat = document.getElementById("tableFormat");
		var tableContent = document.getElementById("tableContent");
		tableFormat.value = format;
		tableContent.value = tableDemo.outerHTML;
		document.forms['convertDemo'].submit();
	},
});
