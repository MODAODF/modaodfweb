# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

SUBDIRS = Alive Announce ModuleTesting

install-data-local:
	$(MKDIR_P) $(DESTDIR)@OXOOLWSD_MODULE_DATA_DIR@

uninstall-local:
	rm -rf $(DESTDIR)@OXOOLWSD_MODULE_DATA_DIR@

# vim: set noet sw=4 ts=4:
