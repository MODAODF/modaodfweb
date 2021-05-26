lib_LTLIBRARIES = libtemplaterepo.la libmergeodf.la libtbl2sc.la

lib_path = $(abs_top_srcdir)
AM_CPPFLAGS = -I$(lib_path) \
	-I$(lib_path)/src/include \
	-I${lib_path}/common \
	-I${lib_path}/net \
	-I${lib_path}/wsd \
	-I${lib_path}/kit \
	-std=c++11 \
	-DDEV_DIR=\"$(abs_top_srcdir)/modules\"


libtemplaterepo_la_SOURCES = templaterepo.cpp \
							 templaterepo_file_db.cpp \
							 templaterepo_db.cpp

libmergeodf_la_SOURCES = mergeodf.cpp \
						 mergeodf_file_db.cpp \
						 mergeodf_parser.cpp \
						 mergeodf_access_db.cpp \
						 mergeodf_logging_db.cpp

libtbl2sc_la_SOURCES = tbl2sc.cpp \
						tbl2sc_logging_db.cpp