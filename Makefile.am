ETAGS=ctags -e

SUBDIRS = . src loleaflet extensions fonts support

export ENABLE_DEBUG

bin_PROGRAMS = \
	modaodfwebforkit \
	modaodfwebmount \
	modaodfwebconvert modaodfwebconfig

bin_PROGRAMS += modaodfweb

dist_bin_SCRIPTS = modaodfweb-systemplate-setup \
                   modaodfweb-generate-proof-key

man_MANS = man/modaodfweb.1 \
           man/modaodfwebforkit.1 \
           man/modaodfwebconvert.1 \
           man/modaodfwebconfig.1 \
           man/modaodfweb-systemplate-setup.1 \
           man/modaodfweb-generate-proof-key.1

dist_doc_DATA = wsd/README \
                wsd/README.vars \
                wsd/protocol.txt \
                wsd/reference.md \
                wsd/metrics.txt

modaodfwebdatadir = @LOOLWSD_DATADIR@

modaodfwebdata_DATA = discovery.xml \
                   favicon.ico

sudoersdir = ${sysconfdir}/sudoers.d
sudoers_DATA = etc/sudoers.d/lool

servicesysconfigdir = ${sysconfdir}/sysconfig
servicesysconfig_DATA = etc/sysconfig/modaodfweb

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = modaodfweb.pc

modaodfwebconfigdir = @LOOLWSD_CONFIGDIR@

modaodfweb_modulesdir = @LOOLWSD_MODULE_DIR@

modaodfweb_module_confdir = @LOOLWSD_MODULE_CONFIG_DIR@

modaodfweb_module_datadir = @LOOLWSD_MODULE_DATA_DIR@

modaodfwebconfig_DATA = modaodfweb.xml \
		     modaodfwebkitconfig.xcu \
		     etc/ca-chain.cert.pem \
		     etc/cert.pem \
		     etc/key.pem

ACLOCAL_AMFLAGS = -I m4

# quick and easy for now.
include_paths = -I${top_srcdir}/common -I${top_srcdir}/net -I${top_srcdir}/wsd -I${top_srcdir}/kit -I${top_srcdir}/src/include ${ZLIB_CFLAGS} ${FONTCONFIG_CFLAGS}

AM_CPPFLAGS = -pthread -DLOOLWSD_DATADIR='"@LOOLWSD_DATADIR@"' \
	      -DLOOLWSD_CONFIGDIR='"@LOOLWSD_CONFIGDIR@"' \
	      -DDEBUG_ABSSRCDIR='"@abs_top_builddir@"' \
	      ${include_paths}

if !ENABLE_DEBUG
AM_CPPFLAGS += -DNDEBUG
endif

AM_LDFLAGS = -pthread -lpam $(ZLIB_LIBS) ${FONTCONFIG_LIBS}

if ENABLE_DEBUG
AM_LDFLAGS += -Wl,-E,-rpath,"@abs_top_builddir@"/.libs
endif

if ENABLE_SSL
AM_LDFLAGS += -lssl -lcrypto
endif

AM_ETAGSFLAGS = --c++-kinds=+p --fields=+iaS --extra=+q -R --totals=yes --exclude=loleaflet *
AM_CTAGSFLAGS = $(AM_ETAGSFLAGS)

shared_sources = common/Crypto.cpp \
                 common/FileUtil.cpp \
                 common/JailUtil.cpp \
                 common/Log.cpp \
                 common/Protocol.cpp \
                 common/StringVector.cpp \
                 common/Session.cpp \
                 common/Seccomp.cpp \
                 common/MessageQueue.cpp \
                 common/MobileApp.cpp \
                 common/TraceEvent.cpp \
                 common/SigUtil.cpp \
                 common/SpookyV2.cpp \
                 common/Unit.cpp \
                 common/Util.cpp \
                 common/Common.cpp \
                 common/ConfigUtil.cpp \
                 common/Authorization.cpp \
                 net/DelaySocket.cpp \
                 net/HttpRequest.cpp \
                 net/HttpHelper.cpp \
                 net/NetUtil.cpp \
                 net/Socket.cpp \
                 wsd/Exceptions.cpp
if ENABLE_SSL
shared_sources += net/Ssl.cpp
endif

shared_json = $(patsubst %.cpp,%.cmd,$(shared_sources))

modaodfweb_sources = \
                  wsd/Admin.cpp \
                  wsd/AdminModel.cpp \
                  wsd/Auth.cpp \
                  wsd/DocumentBroker.cpp \
                  wsd/ProxyProtocol.cpp \
                  wsd/LOOLWSD.cpp \
                  wsd/ClientSession.cpp \
                  wsd/FileServer.cpp \
                  wsd/ProxyRequestHandler.cpp \
                  wsd/FileServerUtil.cpp \
                  wsd/RequestDetails.cpp \
                  wsd/Storage.cpp \
                  wsd/HostUtil.cpp \
                  wsd/TileCache.cpp \
                  wsd/ProofKey.cpp \
                  wsd/QuarantineUtil.cpp

modaodfweb_json = $(patsubst %.cpp,%.cmd,$(modaodfweb_sources))

oxoolextra_sources = \
	src/lib/OxOOL/Module/Base.cpp \
	src/lib/OxOOL/Module/Map.cpp \
	src/lib/OxOOL/Net/Util.cpp \
	src/lib/OxOOL/ModuleManager.cpp \
	src/lib/OxOOL/HttpHelper.cpp \
	src/lib/OxOOL/L10NTranslator.cpp \
	src/lib/OxOOL/Logger.cpp \
	src/lib/OxOOL/XMLConfig.cpp \
	src/lib/OxOOL/ConvertBroker.cpp \
	src/lib/OxOOL/Util.cpp \
	src/lib/OxOOL/Util/OptionlValidator.cpp \
	src/lib/OxOOL/Util/FileValidator.cpp

lib_LTLIBRARIES = @LIB_NAME@.la

@LIB_NAME@_la_SOURCES = \
	$(shared_sources) \
	$(modaodfweb_sources) \
	$(oxoolextra_sources)

modaodfweb_SOURCES = wsd/MainLOOLWSD.cpp

modaodfweb_LDADD = @LIB_NAME@.la

noinst_PROGRAMS = clientnb \
                  #connect \
                  lokitclient \
                  modaodfwebmap
                  #modaodfwebstress \
                  #modaodfwebsocketdump

#connect_SOURCES = tools/Connect.cpp \
#                  common/DummyTraceEventEmitter.cpp \
#                  common/Log.cpp \
#                  common/Protocol.cpp \
#                  common/StringVector.cpp \
#                  common/Util.cpp

lokitclient_SOURCES = tools/KitClient.cpp \
                      common/DummyTraceEventEmitter.cpp

lokitclient_LDADD = @LIB_NAME@.la

modaodfwebforkit_sources = kit/ChildSession.cpp \
                     kit/ForKit.cpp \
                     kit/Kit.cpp

modaodfwebforkit_json = $(patsubst %.cpp,%.cmd,$(modaodfwebforkit_sources))

modaodfwebforkit_SOURCES = $(modaodfwebforkit_sources)

modaodfwebforkit_LDADD = @LIB_NAME@.la

clientnb_SOURCES = net/clientnb.cpp \
                   common/DummyTraceEventEmitter.cpp
clientnb_LDADD = @LIB_NAME@.la

modaodfwebmount_SOURCES = tools/mount.cpp
modaodfwebmount_LDADD = @LIB_NAME@.la

modaodfwebmap_SOURCES = tools/map.cpp

modaodfwebconvert_SOURCES = tools/Tool.cpp

#modaodfwebstress_CPPFLAGS = -DTDOC=\"$(abs_top_srcdir)/test/data\" ${include_paths}
#modaodfwebstress_SOURCES = tools/Stress.cpp \
#                     common/DummyTraceEventEmitter.cpp \
#                     common/Protocol.cpp \
#                     common/StringVector.cpp \
#                     common/Log.cpp \
#		     common/Util.cpp

modaodfwebconfig_SOURCES = tools/Config.cpp \
             common/DummyTraceEventEmitter.cpp
modaodfwebconfig_LDADD = @LIB_NAME@.la

#modaodfwebsocketdump_SOURCES = tools/WebSocketDump.cpp \
#			 common/DummyTraceEventEmitter.cpp \
#

wsd_headers = wsd/Admin.hpp \
              wsd/AdminModel.hpp \
              wsd/Auth.hpp \
              wsd/ClientSession.hpp \
              wsd/DocumentBroker.hpp \
              wsd/ProxyProtocol.hpp \
              wsd/Exceptions.hpp \
              wsd/FileServer.hpp \
              wsd/ProxyRequestHandler.hpp \
              wsd/LOOLWSD.hpp \
              wsd/ProofKey.hpp \
              wsd/RequestDetails.hpp \
              wsd/SenderQueue.hpp \
              wsd/ServerURL.hpp \
              wsd/Storage.hpp \
              wsd/TileCache.hpp \
              wsd/TileDesc.hpp \
              wsd/TraceFile.hpp \
              wsd/UserMessages.hpp \
              wsd/QuarantineUtil.hpp \
              wsd/HostUtil.hpp

shared_headers = common/Common.hpp \
                 common/Clipboard.hpp \
                 common/Crypto.hpp \
                 common/JsonUtil.hpp \
                 common/FileUtil.hpp \
                 common/JailUtil.hpp \
                 common/Log.hpp \
                 common/Protocol.hpp \
                 common/StateEnum.hpp \
                 common/StringVector.hpp \
                 common/Seccomp.hpp \
                 common/Session.hpp \
                 common/Unit.hpp \
                 common/UnitHTTP.hpp \
                 common/Util.hpp \
                 common/ConfigUtil.hpp \
                 common/Authorization.hpp \
                 common/MessageQueue.hpp \
                 common/Message.hpp \
                 common/MobileApp.hpp \
                 common/Png.hpp \
                 common/TraceEvent.hpp \
                 common/Rectangle.hpp \
                 common/RenderTiles.hpp \
                 common/SigUtil.hpp \
                 common/security.h \
                 common/SpookyV2.h \
                 net/Buffer.hpp \
                 net/DelaySocket.hpp \
                 net/FakeSocket.hpp \
		 net/HttpRequest.hpp \
                 net/HttpHelper.hpp \
                 net/NetUtil.hpp \
                 net/ServerSocket.hpp \
                 net/Socket.hpp \
                 net/WebSocketHandler.hpp \
                 net/WebSocketSession.hpp \
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
                 test/WopiTestServer.hpp \
                 test/countloolkits.hpp \
                 test/lokassert.hpp \
                 test/test.hpp \
                 test/testlog.hpp \
                 test/helpers.hpp

dist-hook:
	git log -1 --format=%h > $(distdir)/dist_git_hash 2> /dev/null || rm $(distdir)/dist_git_hash
	git rev-parse --abbrev-ref HEAD | cut -d '-' -f 1 > $(distdir)/dist_git_branch 2> /dev/null || rm $(distdir)/dist_git_branch
	( if test -f dist_project; then mv dist_project $(distdir); fi ) || rm $(distdir)/dist_project
	cp -aR bundled $(distdir)/

EXTRA_DIST = discovery.xml \
             favicon.ico \
             modaodfwebkitconfig.xcu \
             modaodfweb.spec \
             modaodfweb.xml.in \
             modaodfweb.service \
             debian/changelog \
             debian/compat \
             debian/control \
             debian/copyright \
             debian/modaodfweb.install \
             debian/modaodfweb-dev.install \
             debian/modaodfweb.postinst \
             debian/modaodfweb.postrm \
             debian/modaodfweb.preinst \
             debian/modaodfweb.service \
             debian/rules \
             debian/source/format \
             etc/key.pem \
             etc/cert.pem \
             etc/ca-chain.cert.pem \
             etc/sudoers.d/lool \
             etc/sysconfig/modaodfweb \
             scripts/unocommands.py \
             $(man_MANS)

CLEANUP_COMMAND=true

if HAVE_LO_PATH

SYSTEM_STAMP = @SYSTEMPLATE_PATH@/system_stamp
CAPABILITIES = $(if @ENABLE_SETCAP@,true,false)
RUN_GDB = $(if $(GDB_FRONTEND),$(GDB_FRONTEND),gdb --tui --args)

if ENABLE_SETCAP
SET_CAPS_COMMAND=sudo @SETCAP@ cap_fowner,cap_chown,cap_mknod,cap_sys_chroot=ep .libs/modaodfwebforkit && sudo @SETCAP@ cap_sys_admin=ep .libs/modaodfwebmount && sudo @SETCAP@ cap_dac_override,cap_net_bind_service=+ep .libs/modaodfweb
else
SET_CAPS_COMMAND=echo "Skipping capability setting"
endif

CLEANUP_DEPS=modaodfweb modaodfwebmount modaodfwebforkit

# Build modaodfweb and modaodfwebmount first, so we can cleanup before updating
# the systemplate directory, which we can't rm if it's mounted.
$(SYSTEM_STAMP): ${top_srcdir}/modaodfweb-systemplate-setup $(CLEANUP_DEPS)
	$(SET_CAPS_COMMAND)
	$(CLEANUP_COMMAND)
	if test "z@SYSTEMPLATE_PATH@" != "z"; then chmod u+w -R --silent "@SYSTEMPLATE_PATH@" ; rm -rf "@SYSTEMPLATE_PATH@" && \
	${top_srcdir}/modaodfweb-systemplate-setup "@SYSTEMPLATE_PATH@" "@LO_PATH@" && touch $@; fi

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
	@echo "Launching modaodfweb"
	@fc-cache "@LO_PATH@"/share/fonts/truetype
	@cp $(abs_top_srcdir)/test/data/hello.odt $(abs_top_srcdir)/test/data/hello-world.odt
	@cp $(abs_top_srcdir)/test/data/hello.ods $(abs_top_srcdir)/test/data/hello-world.ods
	@cp $(abs_top_srcdir)/test/data/hello.odp $(abs_top_srcdir)/test/data/hello-world.odp
	@echo
	.libs/modaodfweb --o:sys_template_path="@SYSTEMPLATE_PATH@" \
			  --o:security.capabilities="$(CAPABILITIES)" \
			  --o:child_root_path="@JAILS_PATH@" --o:storage.filesystem[@allow]=true \
			  --config-file="$(abs_top_srcdir)/modaodfweb.xml" \
			  --o:file_server_root_path="$(abs_top_srcdir)" \
			  --o:ssl.cert_file_path="$(abs_top_srcdir)/etc/cert.pem" \
			  --o:ssl.key_file_path="$(abs_top_srcdir)/etc/key.pem" \
			  --o:ssl.ca_file_path="$(abs_top_srcdir)/etc/ca-chain.cert.pem" \
			  --o:admin_console.username=admin --o:admin_console.password=admin \
			  --o:logging.file[@enable]=true --o:logging.level=trace

if ENABLE_DEBUG
run-one: all @JAILS_PATH@
	@echo "Launching modaodfweb"
	@fc-cache "@LO_PATH@"/share/fonts/truetype
	@cp $(abs_top_srcdir)/test/data/hello.odt $(abs_top_srcdir)/test/data/hello-world.odt
	@cp $(abs_top_srcdir)/test/data/hello.ods $(abs_top_srcdir)/test/data/hello-world.ods
	@cp $(abs_top_srcdir)/test/data/hello.odp $(abs_top_srcdir)/test/data/hello-world.odp
	@echo
	.libs/modaodfweb --o:sys_template_path="@SYSTEMPLATE_PATH@" \
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
	@echo "Launching modaodfweb under valgrind (but not forkit/modaodfwebkit, yet)"
	@fc-cache "@LO_PATH@"/share/fonts/truetype
	@cp $(abs_top_srcdir)/test/data/hello.odt $(abs_top_srcdir)/test/data/hello-world.odt
	@cp $(abs_top_srcdir)/test/data/hello.ods $(abs_top_srcdir)/test/data/hello-world.ods
	@cp $(abs_top_srcdir)/test/data/hello.odp $(abs_top_srcdir)/test/data/hello-world.odp
	valgrind --tool=memcheck --trace-children=no -v --read-var-info=yes \
		.libs/modaodfweb --o:sys_template_path="@SYSTEMPLATE_PATH@" \
			  --o:child_root_path="@JAILS_PATH@" --o:storage.filesystem[@allow]=true \
			  --config-file="$(abs_top_srcdir)/modaodfweb.xml" \
			  --o:file_server_root_path="$(abs_top_srcdir)" \
			  --o:ssl.cert_file_path="$(abs_top_srcdir)/etc/cert.pem" \
			  --o:ssl.key_file_path="$(abs_top_srcdir)/etc/key.pem" \
			  --o:ssl.ca_file_path="$(abs_top_srcdir)/etc/ca-chain.cert.pem" \
			  --o:admin_console.username=admin --o:admin_console.password=admin \
			  --o:logging.file[@enable]=false --o:logging.level=trace

run-gdb: all @JAILS_PATH@
	@echo "Launching modaodfweb under gdb"
	@fc-cache "@LO_PATH@"/share/fonts/truetype
	@cp $(abs_top_srcdir)/test/data/hello.odt $(abs_top_srcdir)/test/data/hello-world.odt
	@cp $(abs_top_srcdir)/test/data/hello.ods $(abs_top_srcdir)/test/data/hello-world.ods
	@cp $(abs_top_srcdir)/test/data/hello.odp $(abs_top_srcdir)/test/data/hello-world.odp
	$(RUN_GDB) \
		.libs/modaodfweb --o:security.capabilities="false" \
			  --o:sys_template_path="@SYSTEMPLATE_PATH@" \
			  --o:child_root_path="@JAILS_PATH@" --o:storage.filesystem[@allow]=true \
			  --config-file="$(abs_top_srcdir)/modaodfweb.xml" \
			  --o:file_server_root_path="$(abs_top_srcdir)" \
			  --o:ssl.cert_file_path="$(abs_top_srcdir)/etc/cert.pem" \
			  --o:ssl.key_file_path="$(abs_top_srcdir)/etc/key.pem" \
			  --o:ssl.ca_file_path="$(abs_top_srcdir)/etc/ca-chain.cert.pem" \
			  --o:admin_console.username=admin --o:admin_console.password=admin \
			  --o:logging.file[@enable]=false --o:logging.level=error

run-callgrind: all @JAILS_PATH@
	@echo "Launching modaodfweb under valgrind's callgrind"
	@fc-cache "@LO_PATH@"/share/fonts/truetype
	@cp $(abs_top_srcdir)/test/data/hello.odt $(abs_top_srcdir)/test/data/hello-world.odt
	@cp $(abs_top_srcdir)/test/data/hello.ods $(abs_top_srcdir)/test/data/hello-world.ods
	@cp $(abs_top_srcdir)/test/data/hello.odp $(abs_top_srcdir)/test/data/hello-world.odp
	valgrind --tool=callgrind --simulate-cache=yes --dump-instr=yes --num-callers=50 --error-limit=no --trace-children=yes \
		.libs/modaodfweb --o:security.capabilities="false" \
			  --o:sys_template_path="@SYSTEMPLATE_PATH@" \
			  --o:child_root_path="@JAILS_PATH@" --o:storage.filesystem[@allow]=true \
			  --config-file="$(abs_top_srcdir)/modaodfweb.xml" \
			  --o:file_server_root_path="$(abs_top_srcdir)" \
			  --o:ssl.cert_file_path="$(abs_top_srcdir)/etc/cert.pem" \
			  --o:ssl.key_file_path="$(abs_top_srcdir)/etc/key.pem" \
			  --o:ssl.ca_file_path="$(abs_top_srcdir)/etc/ca-chain.cert.pem" \
			  --o:admin_console.username=admin --o:admin_console.password=admin \
			  --o:logging.file[@enable]=false --o:logging.level=error

run-strace: all @JAILS_PATH@
	@echo "Launching modaodfweb under strace"
	@fc-cache "@LO_PATH@"/share/fonts/truetype
	@cp $(abs_top_srcdir)/test/data/hello.odt $(abs_top_srcdir)/test/data/hello-world.odt
	@cp $(abs_top_srcdir)/test/data/hello.ods $(abs_top_srcdir)/test/data/hello-world.ods
	@cp $(abs_top_srcdir)/test/data/hello.odp $(abs_top_srcdir)/test/data/hello-world.odp
	strace -o strace.log -f -tt -s 256 \
		.libs/modaodfweb --o:security.capabilities="false" \
			  --o:sys_template_path="@SYSTEMPLATE_PATH@" \
			  --o:child_root_path="@JAILS_PATH@" --o:storage.filesystem[@allow]=true \
			  --config-file="$(abs_top_srcdir)/modaodfweb.xml" \
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

ALL_LOCAL_DEPS=modaodfweb

# After building modaodfwebforkit, set its capabilities as required. Do it
# already after a plain 'make' to allow for testing without
# installing. When building for packaging, no need for this, as the
# capabilities won't survive packaging anyway. Instead, handle it when
# installing the RPM or Debian package.
all-local: $(ALL_LOCAL_DEPS) @JAILS_PATH@ $(SYSTEM_STAMP)
	$(SET_CAPS_COMMAND)

# just run the build without any tests
build-nocheck: all-am

install-data-local:
	$(MKDIR_P) $(DESTDIR)$(LOOLWSD_MODULE_DIR)
	$(MKDIR_P) $(DESTDIR)$(LOOLWSD_MODULE_CONFIG_DIR)
	$(MKDIR_P) $(DESTDIR)$(LOOLWSD_MODULE_DATA_DIR)

uninstall-local:
	rmdir $(DESTDIR)$(LOOLWSD_MODULE_DIR)
	rmdir $(DESTDIR)$(LOOLWSD_MODULE_CONFIG_DIR)
	rmdir $(DESTDIR)$(LOOLWSD_MODULE_DATA_DIR)

define file_target
$(1): $(2)

endef

define file_targets
$(foreach file,$(1),$(call file_target,$(file),$(patsubst %.cmd,%.cpp,$(file))))
endef

CLANGXX_COMPILE_FLAGS=clang++ $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
	$(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS)

JSON_COMPILE_FLAGS=$(subst ",\",$(subst \,\\,$(CLANGXX_COMPILE_FLAGS)))

JSON_COMPILE_SRC = $(modaodfweb_json) $(modaodfwebforkit_json) $(shared_json)

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
