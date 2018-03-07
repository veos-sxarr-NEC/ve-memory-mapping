lib_LTLIBRARIES = libvemmagent.la
libvemmagent_la_SOURCES = vemmagent.cpp
include_HEADERS = vemmagent.h
noinst_HEADERS = VEMMAgent.hpp
nodist_libvemmagent_la_SOURCES = vemm.pb.cc
CLEANFILES = vemm.pb.cc vemm.pb.h
libvemmagent_la_CXXFLAGS = $(PROTOBUF_CFLAGS) \
                           $(BOOST_CPPFLAGS) \
                           -I$(top_srcdir)/common \
                           -Wall
libvemmagent_la_LIBADD = $(PROTOBUF_LIBS) $(BOOST_LDFLAGS) $(BOOST_ASIO_LIB) \
                         -lvelayout
libvemmagent_la_LDFLAGS = -version-info 1:0:0

vemmagent.cpp: vemm.pb.h
vemm.pb.cc vemm.pb.h: $(srcdir)/../common/vemm.proto
	protoc --cpp_out . -I$(srcdir)/../common $<
