sbin_PROGRAMS = vemmd
systemd_unit_DATA = vemmd.service
os_launcher_conf_DATA = vemmd.options
vemmd_SOURCES = \
                main.cpp \
                vemmd_main.cpp \
                vemmd_main.hpp \
                OSDaemonProxy.cpp \
                UpcallReceiver.cpp \
                log.cpp \
                ../common/log_wrapper.c \
                log.hpp

noinst_HEADERS = \
                       Dispatcher.hpp \
                       Dispatcher.ipp \
                       OSDaemonProxy.hpp \
                       UpcallReceiver.hpp

nodist_vemmd_SOURCES = $(builddir)/vemm.pb.cc \
                       $(builddir)/vemm.pb.h

BUILT_SOURCES = vemm.pb.h

vemmd_CXXFLAGS = $(PROTOBUF_CFLAGS) $(LOG4C_CFLAGS) \
                 $(BOOST_CPPFLAGS) $(libev_CFLAGS) -I$(top_srcdir)/common \
                 $(SYSTEMD_CFLAGS) -Wall
vemmd_LDADD = $(PROTOBUF_LIBS) $(LOG4C_LIBS) \
              $(BOOST_LDFLAGS) $(BOOST_PROGRAM_OPTIONS_LIB) $(libev_LIBS) \
              $(SYSTEMD_LIBS) \
              -lvelayout

CLEANFILES = vemm.pb.cc vemm.pb.h $(systemd_unit_DATA)

EXTRA_DIST = vemmd.service.in vemmd.options.in

edit = $(SED) -e 's|@sbindir[@]|$(sbindir)|g' \
              -e 's|@localstatedir[@]|$(localstatedir)|g' \
              -e 's|@sysconfdir[@]|$(sysconfdir)|g'

vemm.pb.cc vemm.pb.h: $(srcdir)/../common/vemm.proto
	protoc --cpp_out . -I$(srcdir)/../common $<

vemmd.service: $(srcdir)/vemmd.service.in
	$(edit) $< > $@

vemmd.options: $(srcdir)/vemmd.options.in
	$(edit) $< > $@
