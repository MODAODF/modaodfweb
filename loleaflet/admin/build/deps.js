/* -*- js-indent-level: 8 -*- */
var deps = {
	AdminCore: {
		src: ['src/Base.js',
		      'src/Admin.js',
		      'src/AdminSocketBase.js',
		      'src/AdminSocketGlobalFunction.js',
		      'src/AdminSocketBroker.js'],
		desc: 'Core admin scripts infrastructure'
	},

	Util: {
		src: ['src/Util.js'],
		desc: 'Utility class',
		deps: ['AdminCore']
	},

	AdminSocketOverview: {
		src: ['src/AdminSocketOverview.js'],
		desc: 'Socket to handle messages in overview page.',
		deps: ['AdminCore']
	},

	AdminSocketAnalytics: {
		src: ['src/AdminSocketAnalytics.js'],
		desc: 'Socket to handle messages in analytics page.',
		deps: ['AdminCore']
	},

	AdminSocketSettings: {
		src: ['src/AdminSocketSettings.js'],
		desc: 'Socket to handle settings from server',
		deps: ['AdminCore']
	},

	AdminSocketLog: {
		src: ['src/AdminSocketLog.js'],
		desc: 'View logs in the admin console.',
		deps: ['AdminCore']
	},

	AdminSocketSoftwareUpgrade: {
		src: ['src/AdminSocketSoftwareUpgrade.js'],
		desc: 'Socket to upgrade software.',
		deps: ['AdminCore']
	},

	AdminSocketFontManager: {
		src: ['src/AdminSocketFontManager.js'],
		desc: 'Socket to font manager.',
		deps: ['AdminCore']
	},

	AdminSocketMacIp: {
		src: ['src/AdminSocketMacIp.js'],
		desc: 'Socket to templaterepo admin page.',
		deps: ['AdminCore']
	},

	AdminSocketMergeODF: {
		src: ['src/AdminSocketMergeODF.js'],
		desc: 'Socket to mergeodf admin page.',
		deps: ['AdminCore']
	},

	AdminSocketTemplateRepo: {
		src: ['src/AdminSocketTemplateRepo.js'],
		desc: 'Socket to templaterepo admin page.',
		deps: ['AdminCore']
	},

	AdminSocketTbl2sc: {
		src: ['src/AdminSocketTbl2sc.js'],
		desc: 'Socket to tbl2sc admin page.',
		deps: ['AdminCore']
	},
};

if (typeof exports !== 'undefined') {
	exports.deps = deps;
}
