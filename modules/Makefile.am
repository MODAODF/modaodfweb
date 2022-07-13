
AM_LDFLAGS = -avoid-version -module -pthread -lPocoDataSQLite${POCO_DEBUG_SUFFIX}

AM_CPPFLAGS = -pthread \
	-I$(top_srcdir)/bundled/include \
	-I$(top_srcdir)/common \
	-I$(top_srcdir)/net \
	-I$(top_srcdir)/wsd \
	-I$(top_srcdir)/kit \
	-DLOOLWSD_MODULES_DIR='"@LOOLWSD_MODULES_DIR@"' \
	-DLOOLWSD_MODULE_CONFIG_DIR='"@LOOLWSD_MODULE_CONFIG_DIR@"' \
	-DDEV_DIR='"$(abs_top_srcdir)/modules"'

modules_LTLIBRARIES = templaterepo.la mergeodf.la tbl2sc.la
modulesdir = $(LOOLWSD_MODULES_DIR)

modulesconfigdir = $(LOOLWSD_MODULE_CONFIG_DIR)
modulesconfig_DATA = templaterepo.xml mergeodf.xml tbl2sc.xml

templaterepo_la_SOURCES = oxoolmodule.h \
	templaterepo.cpp \
	templaterepo_db.cpp \
	templaterepo_db.h \
	templaterepo_file_db.cpp \
	templaterepo_file_db.h \
	templaterepo.h

mergeodf_la_SOURCES = oxoolmodule.h \
	mergeodf_access_db.cpp \
	mergeodf_access_db.h \
	mergeodf.cpp \
	mergeodf_file_db.cpp \
	mergeodf_file_db.h \
	mergeodf.h \
	mergeodf_logging_db.cpp \
	mergeodf_logging_db.h \
	mergeodf_parser.cpp \
	mergeodf_parser.h

tbl2sc_la_SOURCES = oxoolmodule.h \
	tbl2sc.cpp \
	tbl2sc.h \
	tbl2sc_logging_db.cpp \
	tbl2sc_logging_db.h
