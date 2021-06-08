ETAGS=ctags -e

if ENABLE_MOBILEAPP

if ENABLE_GTKAPP
SUBDIRS = gtk loleaflet
else
SUBDIRS = loleaflet
endif

else

SUBDIRS = . loleaflet extensions fonts support modules

export ENABLE_DEBUG

bin_PROGRAMS = \
	ndcodfwebforkit \
	ndcodfwebmount \
	ndcodfwebconvert ndcodfwebconfig

if ENABLE_LIBFUZZER
else
bin_PROGRAMS += ndcodfweb
endif

dist_bin_SCRIPTS = ndcodfweb-systemplate-setup \
                   ndcodfweb-generate-proof-key

man_MANS = man/ndcodfweb.1 \
           man/ndcodfwebforkit.1 \
           man/ndcodfwebconvert.1 \
           man/ndcodfwebconfig.1 \
           man/ndcodfweb-systemplate-setup.1 \
           man/ndcodfweb-generate-proof-key.1

dist_doc_DATA = wsd/README \
                wsd/README.vars \
                wsd/protocol.txt \
                wsd/reference.md \
                wsd/metrics.txt

oxoolwsddatadir = @LOOLWSD_DATADIR@

oxoolwsddata_DATA = discovery.xml \
                   favicon.ico

sudoersdir = ${sysconfdir}/sudoers.d
sudoers_DATA = etc/sudoers.d/lool

servicesysconfigdir = ${sysconfdir}/sysconfig
servicesysconfig_DATA = etc/sysconfig/ndcodfweb

oxoolwsdconfigdir = @LOOLWSD_CONFIGDIR@

oxool_modulesdir = @LOOLWSD_MODULES_DIR@

oxool_module_confdir = @LOOLWSD_MODULE_CONFIG_DIR@

oxool_module_datadir = @LOOLWSD_MODULE_DATA_DIR@

oxoolwsdconfig_DATA = ndcodfweb.xml \
		     ndcodfwebkitconfig.xcu \
		     etc/ca-chain.cert.pem \
		     etc/cert.pem \
		     etc/key.pem

ACLOCAL_AMFLAGS = -I m4

# quick and easy for now.
include_paths = -I${top_srcdir}/common -I${top_srcdir}/net -I${top_srcdir}/wsd -I${top_srcdir}/kit ${ZLIB_CFLAGS} ${FONTCONFIG_CFLAGS}

AM_CPPFLAGS = -pthread -DLOOLWSD_DATADIR='"@LOOLWSD_DATADIR@"' \
	      -DLOOLWSD_CONFIGDIR='"@LOOLWSD_CONFIGDIR@"' \
	      -DDEBUG_ABSSRCDIR='"@abs_srcdir@"' \
	      ${include_paths}

if !ENABLE_DEBUG
AM_CPPFLAGS += -DNDEBUG
endif

AM_LDFLAGS = -pthread -Wl,-E,-rpath,/snap/ndcodfweb/current/usr/lib -lpam $(ZLIB_LIBS) ${FONTCONFIG_LIBS}

if ENABLE_SSL
AM_LDFLAGS += -lssl -lcrypto
else
ndcodfweb_fuzzer_LDADD = -lssl -lcrypto
ndcodfweb_LDADD = -lssl -lcrypto
ndcodfwebconfig_LDADD = -lssl -lcrypto
endif

ndcodfweb_fuzzer_CPPFLAGS = -DKIT_IN_PROCESS=1 -DFUZZER=1 -DTDOC=\"$(abs_top_srcdir)/test/data\" $(AM_CPPFLAGS)

AM_ETAGSFLAGS = --c++-kinds=+p --fields=+iaS --extra=+q -R --totals=yes --exclude=loleaflet *
AM_CTAGSFLAGS = $(AM_ETAGSFLAGS)

shared_sources = common/FileUtil.cpp \
                 common/JailUtil.cpp \
                 common/Log.cpp \
                 common/Protocol.cpp \
                 common/StringVector.cpp \
                 common/Session.cpp \
                 common/Seccomp.cpp \
                 common/MessageQueue.cpp \
                 common/MobileApp.cpp \
                 common/SigUtil.cpp \
                 common/SpookyV2.cpp \
                 common/Unit.cpp \
                 common/Util.cpp \
                 common/Authorization.cpp \
                 net/DelaySocket.cpp \
                 net/Socket.cpp
if ENABLE_SSL
shared_sources += net/Ssl.cpp
endif

shared_json = $(patsubst %.cpp,%.cmd,$(shared_sources))

ndcodfweb_sources = common/Crypto.cpp \
                  wsd/Admin.cpp \
                  wsd/AdminModel.cpp \
                  wsd/Auth.cpp \
                  wsd/DocumentBroker.cpp \
                  wsd/ProxyProtocol.cpp \
                  wsd/LOOLWSD.cpp \
                  wsd/ClientSession.cpp \
                  wsd/FileServer.cpp \
                  wsd/FileServerUtil.cpp \
                  wsd/RequestDetails.cpp \
                  wsd/Storage.cpp \
                  wsd/TileCache.cpp \
                  wsd/ProofKey.cpp

ndcodfweb_json = $(patsubst %.cpp,%.cmd,$(ndcodfweb_sources))

ndcodfweb_SOURCES = $(ndcodfweb_sources) \
                  $(shared_sources)

noinst_PROGRAMS = clientnb \
                  connect \
                  ndcodfwebkitclient \
                  ndcodfwebmap \
                  ndcodfwebstress \
                  ndcodfwebsocketdump

if ENABLE_LIBFUZZER
noinst_PROGRAMS += \
		   admin_fuzzer \
		   clientsession_fuzzer
else
noinst_PROGRAMS += ndcodfweb_fuzzer
endif

connect_SOURCES = tools/Connect.cpp \
                  common/Log.cpp \
                  common/Protocol.cpp \
                  common/StringVector.cpp \
                  common/Util.cpp

ndcodfwebkitclient_SOURCES = common/Log.cpp \
                      tools/KitClient.cpp \
                      common/Protocol.cpp \
                      common/StringVector.cpp \
                      common/Util.cpp

ndcodfwebforkit_sources = kit/ChildSession.cpp \
                     kit/ForKit.cpp \
                     kit/Kit.cpp

ndcodfwebforkit_json = $(patsubst %.cpp,%.cmd,$(ndcodfwebforkit_sources))

ndcodfwebforkit_SOURCES = $(ndcodfwebforkit_sources) \
                     $(shared_sources)

ndcodfweb_fuzzer_SOURCES = $(ndcodfweb_sources) \
                         $(ndcodfwebforkit_sources) \
                         $(shared_sources) \
                         kit/DummyLibreOfficeKit.cpp

admin_fuzzer_CPPFLAGS = \
			-DKIT_IN_PROCESS=1 \
			$(AM_CPPFLAGS)
admin_fuzzer_SOURCES = \
		       $(ndcodfweb_sources) \
		       $(ndcodfwebforkit_sources) \
		       $(shared_sources) \
		       fuzzer/Admin.cpp
admin_fuzzer_LDFLAGS = -fsanitize=fuzzer $(AM_LDFLAGS)

clientsession_fuzzer_CPPFLAGS = \
				-DKIT_IN_PROCESS=1 \
				$(AM_CPPFLAGS)
clientsession_fuzzer_SOURCES = \
			       $(ndcodfweb_sources) \
			       $(ndcodfwebforkit_sources) \
			       $(shared_sources) \
			       fuzzer/ClientSession.cpp
clientsession_fuzzer_LDFLAGS = -fsanitize=fuzzer $(AM_LDFLAGS)

clientnb_SOURCES = net/clientnb.cpp \
                   common/Log.cpp \
		   common/StringVector.cpp \
                   common/Util.cpp

ndcodfwebmount_SOURCES = tools/mount.cpp

ndcodfwebmap_SOURCES = tools/map.cpp

ndcodfwebconvert_SOURCES = tools/Tool.cpp

ndcodfwebstress_CPPFLAGS = -DTDOC=\"$(abs_top_srcdir)/test/data\" ${include_paths}
ndcodfwebstress_SOURCES = tools/Stress.cpp \
                     common/Protocol.cpp \
                     common/StringVector.cpp \
                     common/Log.cpp \
		     common/Util.cpp

ndcodfwebconfig_SOURCES = tools/Config.cpp \
		     common/Crypto.cpp \
		     common/Log.cpp \
		     common/StringVector.cpp \
		     common/Util.cpp

ndcodfwebsocketdump_SOURCES = tools/WebSocketDump.cpp \
			 $(shared_sources)

wsd_headers = wsd/Admin.hpp \
              wsd/AdminModel.hpp \
              wsd/Auth.hpp \
              wsd/ClientSession.hpp \
              wsd/DocumentBroker.hpp \
              wsd/ProxyProtocol.hpp \
              wsd/Exceptions.hpp \
              wsd/FileServer.hpp \
              wsd/LOOLWSD.hpp \
              wsd/ProofKey.hpp \
              wsd/RequestDetails.hpp \
              wsd/SenderQueue.hpp \
              wsd/ServerURL.hpp \
              wsd/Storage.hpp \
              wsd/TileCache.hpp \
              wsd/TileDesc.hpp \
              wsd/TraceFile.hpp \
              wsd/UserMessages.hpp

shared_headers = common/Common.hpp \
                 common/Clipboard.hpp \
                 common/Crypto.hpp \
                 common/JsonUtil.hpp \
                 common/FileUtil.hpp \
                 common/JailUtil.hpp \
                 common/Log.hpp \
                 common/LOOLWebSocket.hpp \
                 common/Protocol.hpp \
                 common/StringVector.hpp \
                 common/Seccomp.hpp \
                 common/Session.hpp \
                 common/Unit.hpp \
                 common/UnitHTTP.hpp \
                 common/Util.hpp \
                 common/Authorization.hpp \
                 common/MessageQueue.hpp \
                 common/Message.hpp \
                 common/MobileApp.hpp \
                 common/Png.hpp \
                 common/Rectangle.hpp \
                 common/RenderTiles.hpp \
                 common/SigUtil.hpp \
                 common/security.h \
                 common/SpookyV2.h \
                 net/DelaySocket.hpp \
                 net/FakeSocket.hpp \
                 net/ServerSocket.hpp \
                 net/Socket.hpp \
                 net/WebSocketHandler.hpp \
                 tools/Replay.hpp
if ENABLE_SSL
shared_headers += net/Ssl.hpp \
                  net/SslSocket.hpp
endif

kit_headers = kit/ChildSession.hpp \
              kit/Delta.hpp \
              kit/DummyLibreOfficeKit.hpp \
              kit/Kit.hpp \
              kit/KitHelper.hpp \
              kit/SetupKitEnvironment.hpp \
              kit/Watermark.hpp

noinst_HEADERS = $(wsd_headers) $(shared_headers) $(kit_headers) \
                 bundled/include/LibreOfficeKit/LibreOfficeKit.h \
                 bundled/include/LibreOfficeKit/LibreOfficeKit.hxx \
                 bundled/include/LibreOfficeKit/LibreOfficeKitEnums.h \
                 bundled/include/LibreOfficeKit/LibreOfficeKitInit.h \
                 bundled/include/LibreOfficeKit/LibreOfficeKitTypes.h \
                 test/WopiTestServer.hpp \
                 test/countloolkits.hpp \
                 test/lokassert.hpp \
                 test/test.hpp \
                 test/helpers.hpp

dist-hook:
	git log -1 --format=%h > $(distdir)/dist_git_hash 2> /dev/null || rm $(distdir)/dist_git_hash
	git rev-parse --abbrev-ref HEAD | cut -d '-' -f 1 > $(distdir)/dist_git_branch 2> /dev/null || rm $(distdir)/dist_git_branch

EXTRA_DIST = discovery.xml \
             favicon.ico \
             ndcodfwebkitconfig.xcu \
             ndcodfweb.spec \
             ndcodfweb.xml.in \
             ndcodfweb.service \
             debian/changelog \
             debian/compat \
             debian/control \
             debian/copyright \
             debian/ndcodfweb.cron.d \
             debian/ndcodfweb.postinst \
             debian/ndcodfweb.postrm \
             debian/ndcodfweb.preinst \
             debian/ndcodfweb.service \
             debian/rules \
             debian/source/format \
             etc/key.pem \
             etc/cert.pem \
             etc/ca-chain.cert.pem \
             etc/sudoers.d/lool \
             etc/sysconfig/ndcodfweb \
             scripts/unocommands.py \
             $(man_MANS) \
			 modules

if ENABLE_LIBFUZZER
CLEANUP_COMMAND=true
else
CLEANUP_COMMAND=true
#CLEANUP_COMMAND=if test -s ./ndcodfweb; then echo "Cleaning up..." && ./ndcodfweb --disable-lool-user-checking --cleanup --o:logging.file[@enable]=false --o:logging.level=trace; fi
endif

if HAVE_LO_PATH

SYSTEM_STAMP = @SYSTEMPLATE_PATH@/system_stamp
CAPABILITIES = $(if @ENABLE_SETCAP@,true,false)
RUN_GDB = $(if $(GDB_FRONTEND),$(GDB_FRONTEND),gdb --tui --args)

if ENABLE_SETCAP
SET_CAPS_COMMAND=sudo @SETCAP@ cap_fowner,cap_mknod,cap_sys_chroot=ep ndcodfwebforkit && sudo @SETCAP@ cap_sys_admin=ep ndcodfwebmount && sudo @SETCAP@ cap_dac_override,cap_net_bind_service=+ep ndcodfweb
else
SET_CAPS_COMMAND=echo "Skipping capability setting"
endif

if ENABLE_LIBFUZZER
CLEANUP_DEPS=
else
CLEANUP_DEPS=ndcodfweb ndcodfwebmount ndcodfwebforkit
endif

# Build ndcodfweb and ndcodfwebmount first, so we can cleanup before updating
# the systemplate directory, which we can't rm if it's mounted.
$(SYSTEM_STAMP): ${top_srcdir}/ndcodfweb-systemplate-setup $(CLEANUP_DEPS)
	$(SET_CAPS_COMMAND)
	$(CLEANUP_COMMAND)
	if test "z@SYSTEMPLATE_PATH@" != "z"; then rm -rf "@SYSTEMPLATE_PATH@" && \
	${top_srcdir}/ndcodfweb-systemplate-setup "@SYSTEMPLATE_PATH@" "@LO_PATH@" && touch $@; fi

@JAILS_PATH@:
	$(CLEANUP_COMMAND)
	mkdir -p $@

cleanup:
	$(CLEANUP_COMMAND)

# Always cleanup before removing the binaries.
# Note: this can break with -j option!
clean-am: cleanup clean-binPROGRAMS clean-generic clean-libtool clean-local clean-noinstPROGRAMS mostlyclean-am

clean-local:
	$(CLEANUP_COMMAND)
	if test "z@JAILS_PATH@" != "z"; then rm -rf "@JAILS_PATH@"; fi
	if test "z@SYSTEMPLATE_PATH@" != "z"; then rm -rf "@SYSTEMPLATE_PATH@"; fi

run: all @JAILS_PATH@
	@echo "Launching ndcodfweb"
	@fc-cache "@LO_PATH@"/share/fonts/truetype
	@cp $(abs_top_srcdir)/test/data/hello.odt $(abs_top_srcdir)/test/data/hello-world.odt
	@cp $(abs_top_srcdir)/test/data/hello.ods $(abs_top_srcdir)/test/data/hello-world.ods
	@cp $(abs_top_srcdir)/test/data/hello.odp $(abs_top_srcdir)/test/data/hello-world.odp
	@cp $(abs_top_srcdir)/modules/.libs/libtemplaterepo.so.0.0.0 $(abs_top_srcdir)/libtemplaterepo.so
	@cp $(abs_top_srcdir)/modules/.libs/libmergeodf.so.0.0.0 $(abs_top_srcdir)/libmergeodf.so
	@cp $(abs_top_srcdir)/modules/.libs/libtbl2sc.so.0.0.0 $(abs_top_srcdir)/libtbl2sc.so
	@echo
	./ndcodfweb --o:sys_template_path="@SYSTEMPLATE_PATH@" \
			  --o:security.capabilities="$(CAPABILITIES)" \
			  --o:child_root_path="@JAILS_PATH@" --o:storage.filesystem[@allow]=true \
			  --config-file="$(abs_top_srcdir)/ndcodfweb.xml" \
			  --o:file_server_root_path="$(abs_top_srcdir)" \
			  --o:ssl.cert_file_path="$(abs_top_srcdir)/etc/cert.pem" \
			  --o:ssl.key_file_path="$(abs_top_srcdir)/etc/key.pem" \
			  --o:ssl.ca_file_path="$(abs_top_srcdir)/etc/ca-chain.cert.pem" \
			  --o:admin_console.username=admin --o:admin_console.password=admin \
			  --o:logging.file[@enable]=true --o:logging.level=trace

if ENABLE_DEBUG
run-one: all @JAILS_PATH@
	@echo "Launching ndcodfweb"
	@fc-cache "@LO_PATH@"/share/fonts/truetype
	@cp $(abs_top_srcdir)/test/data/hello.odt $(abs_top_srcdir)/test/data/hello-world.odt
	@cp $(abs_top_srcdir)/test/data/hello.ods $(abs_top_srcdir)/test/data/hello-world.ods
	@cp $(abs_top_srcdir)/test/data/hello.odp $(abs_top_srcdir)/test/data/hello-world.odp
	@echo
	./ndcodfweb --o:sys_template_path="@SYSTEMPLATE_PATH@" \
			  --o:security.capabilities="$(CAPABILITIES)" \
			  --o:child_root_path="@JAILS_PATH@" --o:storage.filesystem[@allow]=true \
			  --o:ssl.cert_file_path="$(abs_top_srcdir)/etc/cert.pem" \
			  --o:ssl.key_file_path="$(abs_top_srcdir)/etc/key.pem" \
			  --o:ssl.ca_file_path="$(abs_top_srcdir)/etc/ca-chain.cert.pem" \
			  --o:admin_console.username=admin --o:admin_console.password=admin \
			  --o:logging.file[@enable]=true --o:logging.level=trace \
			  --singlekit
endif

sync-writer:
	browser-sync start --config browsersync-config.js --startPath "loleaflet/96c23f663/loleaflet.html?file_path=file://$(abs_top_srcdir)/test/data/hello-world.odt"

sync-calc:
	browser-sync start --config browsersync-config.js --startPath "loleaflet/96c23f663/loleaflet.html?file_path=file://$(abs_top_srcdir)/test/data/hello-world.ods"

sync-impress:
	browser-sync start --config browsersync-config.js --startPath "loleaflet/96c23f663/loleaflet.html?file_path=file://$(abs_top_srcdir)/test/data/hello-world.odp"

run-valgrind: all @JAILS_PATH@
	@echo "Launching ndcodfweb under valgrind (but not forkit/ndcodfwebkit, yet)"
	@fc-cache "@LO_PATH@"/share/fonts/truetype
	@cp $(abs_top_srcdir)/test/data/hello.odt $(abs_top_srcdir)/test/data/hello-world.odt
	@cp $(abs_top_srcdir)/test/data/hello.ods $(abs_top_srcdir)/test/data/hello-world.ods
	@cp $(abs_top_srcdir)/test/data/hello.odp $(abs_top_srcdir)/test/data/hello-world.odp
	valgrind --tool=memcheck --trace-children=no -v --read-var-info=yes \
		./ndcodfweb --o:sys_template_path="@SYSTEMPLATE_PATH@" \
			  --o:child_root_path="@JAILS_PATH@" --o:storage.filesystem[@allow]=true \
			  --config-file="$(abs_top_srcdir)/ndcodfweb.xml" \
			  --o:file_server_root_path="$(abs_top_srcdir)" \
			  --o:ssl.cert_file_path="$(abs_top_srcdir)/etc/cert.pem" \
			  --o:ssl.key_file_path="$(abs_top_srcdir)/etc/key.pem" \
			  --o:ssl.ca_file_path="$(abs_top_srcdir)/etc/ca-chain.cert.pem" \
			  --o:admin_console.username=admin --o:admin_console.password=admin \
			  --o:logging.file[@enable]=false --o:logging.level=trace

run-gdb: all @JAILS_PATH@
	@echo "Launching ndcodfweb under gdb"
	@fc-cache "@LO_PATH@"/share/fonts/truetype
	@cp $(abs_top_srcdir)/test/data/hello.odt $(abs_top_srcdir)/test/data/hello-world.odt
	@cp $(abs_top_srcdir)/test/data/hello.ods $(abs_top_srcdir)/test/data/hello-world.ods
	@cp $(abs_top_srcdir)/test/data/hello.odp $(abs_top_srcdir)/test/data/hello-world.odp
	$(RUN_GDB) \
		./ndcodfweb --o:security.capabilities="false" \
			  --o:sys_template_path="@SYSTEMPLATE_PATH@" \
			  --o:child_root_path="@JAILS_PATH@" --o:storage.filesystem[@allow]=true \
			  --config-file="$(abs_top_srcdir)/ndcodfweb.xml" \
			  --o:file_server_root_path="$(abs_top_srcdir)" \
			  --o:ssl.cert_file_path="$(abs_top_srcdir)/etc/cert.pem" \
			  --o:ssl.key_file_path="$(abs_top_srcdir)/etc/key.pem" \
			  --o:ssl.ca_file_path="$(abs_top_srcdir)/etc/ca-chain.cert.pem" \
			  --o:admin_console.username=admin --o:admin_console.password=admin \
			  --o:logging.file[@enable]=false --o:logging.level=error

run-callgrind: all @JAILS_PATH@
	@echo "Launching ndcodfweb under valgrind's callgrind"
	@fc-cache "@LO_PATH@"/share/fonts/truetype
	@cp $(abs_top_srcdir)/test/data/hello.odt $(abs_top_srcdir)/test/data/hello-world.odt
	@cp $(abs_top_srcdir)/test/data/hello.ods $(abs_top_srcdir)/test/data/hello-world.ods
	@cp $(abs_top_srcdir)/test/data/hello.odp $(abs_top_srcdir)/test/data/hello-world.odp
	valgrind --tool=callgrind --simulate-cache=yes --dump-instr=yes --num-callers=50 --error-limit=no --trace-children=yes \
		./ndcodfweb --o:security.capabilities="false" \
			  --o:sys_template_path="@SYSTEMPLATE_PATH@" \
			  --o:child_root_path="@JAILS_PATH@" --o:storage.filesystem[@allow]=true \
			  --config-file="$(abs_top_srcdir)/ndcodfweb.xml" \
			  --o:file_server_root_path="$(abs_top_srcdir)" \
			  --o:ssl.cert_file_path="$(abs_top_srcdir)/etc/cert.pem" \
			  --o:ssl.key_file_path="$(abs_top_srcdir)/etc/key.pem" \
			  --o:ssl.ca_file_path="$(abs_top_srcdir)/etc/ca-chain.cert.pem" \
			  --o:admin_console.username=admin --o:admin_console.password=admin \
			  --o:logging.file[@enable]=false --o:logging.level=error

run-strace: all @JAILS_PATH@
	@echo "Launching ndcodfweb under strace"
	@fc-cache "@LO_PATH@"/share/fonts/truetype
	@cp $(abs_top_srcdir)/test/data/hello.odt $(abs_top_srcdir)/test/data/hello-world.odt
	@cp $(abs_top_srcdir)/test/data/hello.ods $(abs_top_srcdir)/test/data/hello-world.ods
	@cp $(abs_top_srcdir)/test/data/hello.odp $(abs_top_srcdir)/test/data/hello-world.odp
	strace -o strace.log -f -tt -s 256 \
		./ndcodfweb --o:security.capabilities="false" \
			  --o:sys_template_path="@SYSTEMPLATE_PATH@" \
			  --o:child_root_path="@JAILS_PATH@" --o:storage.filesystem[@allow]=true \
			  --config-file="$(abs_top_srcdir)/ndcodfweb.xml" \
			  --o:file_server_root_path="$(abs_top_srcdir)" \
			  --o:ssl.cert_file_path="$(abs_top_srcdir)/etc/cert.pem" \
			  --o:ssl.key_file_path="$(abs_top_srcdir)/etc/key.pem" \
			  --o:ssl.ca_file_path="$(abs_top_srcdir)/etc/ca-chain.cert.pem" \
			  --o:admin_console.username=admin --o:admin_console.password=admin \
			  --o:logging.file[@enable]=false --o:logging.level=error

else

SYSTEM_STAMP =

endif

.PHONY: cleanup

if ENABLE_LIBFUZZER
ALL_LOCAL_DEPS=
else
ALL_LOCAL_DEPS=ndcodfweb
endif

# After building ndcodfwebforkit, set its capabilities as required. Do it
# already after a plain 'make' to allow for testing without
# installing. When building for packaging, no need for this, as the
# capabilities won't survive packaging anyway. Instead, handle it when
# installing the RPM or Debian package.
all-local: $(ALL_LOCAL_DEPS) @JAILS_PATH@ $(SYSTEM_STAMP)
	$(SET_CAPS_COMMAND)

# just run the build without any tests
build-nocheck: all-am

install-data-local:
	$(MKDIR_P) $(DESTDIR)$(LOOLWSD_MODULES_DIR)
	$(MKDIR_P) $(DESTDIR)$(LOOLWSD_MODULE_CONFIG_DIR)
	$(MKDIR_P) $(DESTDIR)$(LOOLWSD_MODULE_DATA_DIR)

uninstall-local:
	rmdir $(DESTDIR)$(LOOLWSD_MODULES_DIR)
	rmdir $(DESTDIR)$(LOOLWSD_MODULE_CONFIG_DIR)
	rmdir $(DESTDIR)$(LOOLWSD_MODULE_DATA_DIR)
endif

define file_target
$(1): $(2)

endef

define file_targets
$(foreach file,$(1),$(call file_target,$(file),$(patsubst %.cmd,%.cpp,$(file))))
endef

CLANGXX_COMPILE_FLAGS=clang++ $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
	$(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS)

JSON_COMPILE_FLAGS=$(subst ",\",$(subst \,\\,$(CLANGXX_COMPILE_FLAGS)))

JSON_COMPILE_SRC = $(ndcodfweb_json) $(ndcodfwebforkit_json) $(shared_json)

$(eval $(call file_targets,$(JSON_COMPILE_SRC)))

.cpp.cmd:
	@echo -n "{\"directory\":\"$(abs_builddir)\",\"command\":\"" > $@
	@echo -n $(JSON_COMPILE_FLAGS) >> $@
	@echo -n "\",\"file\":\"$<\"}" >> $@

$(abs_srcdir)/compile_commands.json: $(JSON_COMPILE_SRC)
	@echo -n "[" > $@
	@for file in $(JSON_COMPILE_SRC) ; \
	do \
		cat $$file; \
		echo ","; \
		rm -f $$file; \
	done >> $@
	@echo -n "]" >> $@

compile_commands: $(abs_srcdir)/compile_commands.json
