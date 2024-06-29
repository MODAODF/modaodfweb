# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# Copyright the OxOffice Online contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

MODULE_NAME = MergeODF
# JavaScript files admin/
JS_FILES = $(shell find admin/ -name '*.js')
# Generate POT file
pot:
	@$(MKDIR_P) $(srcdir)/po
	@xgettext --from-code=UTF-8 --language=JavaScript --keyword=_ --output="po/${MODULE_NAME}.pot" \
	$(JS_FILES)

	@msguniq -w 78 -t UTF-8 po/${MODULE_NAME}.pot > po/${MODULE_NAME}.pot.new; mv po/${MODULE_NAME}.pot.new po/${MODULE_NAME}.pot

l10n: pot
	@for i in $(wildcard $(srcdir)/po/*.po); do \
		msgmerge --update --backup=off $$i po/${MODULE_NAME}.pot; \
		echo "Updated $$i"; \
	done

convert_po2json:
	@$(MKDIR_P) $(srcdir)/browser/l10n
	@for i in $(wildcard $(srcdir)/po/*.po); do \
		lang=$$(basename $$i .po); \
		$(top_srcdir)/ossii/scripts/po2json.py --quiet $$i -o $(srcdir)/browser/l10n/$$lang.json; \
		echo "Converted $$i to browser/l10n/$$lang.json"; \
	done

.PHONY: build-another

all-local: build-another

build-another: \
	convert_po2json

configdir = @OXOOL_MODULE_CONFIG_DIR@
dist_config_DATA = ${MODULE_NAME}.xml

# Module install path
moduledir = @OXOOL_MODULE_DIR@
module_LTLIBRARIES = MergeODF.la

MergeODF_la_CPPFLAGS = -pthread $(OSSII_CFLAGS)

MergeODF_la_LDFLAGS = -avoid-version -module $(OXOOL_LIBS) -lPocoDataSQLite

noinst_HEADERS = \
	src/MergeODF.h \
	src/MergeODFParser.h

MergeODF_la_SOURCES = \
	src/MergeODF.cpp \
	src/MergeODFParser.cpp

admindir = @OXOOL_MODULE_DATA_DIR@/${MODULE_NAME}/admin
dist_admin_DATA = admin/admin.html admin/admin.js

l10ndir = @OXOOL_MODULE_DATA_DIR@/${MODULE_NAME}/browser/l10n
# json 是依據 po 檔案轉換而來，所以不需要加上 dist_l10n_DATA
l10n_DATA = $(wildcard browser/l10n/*.json)

cssdir = @OXOOL_MODULE_DATA_DIR@/${MODULE_NAME}/admin/css
dist_css_DATA = admin/css/*.css

jsdir = @OXOOL_MODULE_DATA_DIR@/${MODULE_NAME}/admin/js
dist_js_DATA = admin/js/*.js

jsl10ndir = @OXOOL_MODULE_DATA_DIR@/${MODULE_NAME}/admin/js/l10n
dist_jsl10n_DATA = admin/js/l10n/*.json

EXTRA_DIST = $(wildcard $(srcdir)/po/*.po)

# vim: set noet sw=4 ts=4:

