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

L.AdminModule.MergeODF = L.AdminModule.extend({

	_l10n: [
		_('ODF report template'), // ODF 報表管理

		_('Module overview'), // 模組概覽
		_('How to use the Reporting API'), // 如何使用報表 API
		_('On the ODF report management website, upload the template file, and obtain the API address to convert the file.'), // 在 ODF 報表管理網站上，上傳報表範本檔案，取得轉檔 API 位址。
		_('API list'), // API 列表
		_('(Note: The {doc_id} of the API needs to be obtained from the ODF report management website.)'), // (註：API 的 {doc_id} 需要從 ODF 報表管理網站取得。)
		_('API URL'), // API 位址
		_('Explain'), // 說明
		_('Get the detailed structure of all reports (JSON format)'), // 取得所有報表的詳細結構（JSON 格式）
		_('Get the detailed structure of all reports (YAML format)'), // 取得所有報表的詳細結構（YAML 格式）
		_('Get the detailed structure of individual report (JSON format)'), // 取得單一報表的詳細結構（JSON 格式）
		_('Get the detailed structure of individual report (YAML format)'), // 取得單一報表的詳細結構（YAML 格式）
		_('Get an example JSON data source for individual report.'), // 取得單一報表的 JSON 格式範例。
		_('Get the number of times an individual report was executed.'), // 取得單一報表的轉檔次數。
		_('Using a JSON data source, Make ODF report file.(If add ?outputPDF after {doc_id}, it will output PDF file)'), // 傳送 JSON 資料，製作 ODF 報表檔案。（若在 {doc_id} 後加上 "?outputPDF"，則會輸出 PDF 檔案）

		_('Convert log'), // 轉檔紀錄
		_('Refresh log'), // 更新日誌
		_('State'), // 狀態
		_('Date'), // 日期
		_('Source IP'), // 來源 IP
		_('File name'), // 檔案名稱
		_('File type'), // 檔案類型
		_('To PDF'), // 輸出 PDF
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
		this.initializeLogginTable();
	},

	/**
	 * Process messages from the owning service module.
	 *
	 * @param {string} textMsg - the message text
	 */
	onMessage: function(textMsg) {
		// 日誌內容
		if (textMsg.startsWith('logData ')) {
			var jsonIdx = textMsg.indexOf('[');
			if (jsonIdx > 0) {
				var dataArray = JSON.parse(textMsg.substring(jsonIdx));
				this._loggingTable.clear();
				if (dataArray.length > 0) {
					this._loggingTable.rows.add(dataArray).draw();
				}
			}
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

	initializeLogginTable: function() {
		this._loggingTable = $("#logging_table").DataTable({
			order: [[1, 'desc']],
			columns: [
				{data: 'status'},
				{data: 'timestamp'},
				{data: 'source_ip'},
                {data: 'file_name'},
                {data: 'file_ext'},
                {data: 'to_pdf'}
			],
			columnDefs: [
				{
					targets: 0, // status
					render: function(data, type, row) {
						return data ? '<span class="text-success">' + _('Success') + '</span>' :
									  '<span class="text-danger">' + _('Fail') + '</span>';
					},
				},
				{
					targets: 1, // timestamp
					render: function(data, type, row) {
						return new Date(Date.parse(data)).toLocaleString();
					},
				},
                {
					targets: 5, // to_pdf
					render: function(data, type, row) {
						return data ? '<span class="text-success">' + _('Yes') + '</span>' : '';
					},
				},
				{	// 標題和訊息不需排序功能
					targets: [4, 5],
					orderable: false
				}
			],
			language: {
				url: this.getDetail().adminURI + 'js/l10n/' + String.locale + '.json'
			}
		});

		$('#refreshLog').click(function() {
			this.sendMessage('refreshLog');
		}.bind(this));
	},

});