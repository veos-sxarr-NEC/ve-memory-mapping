ACLOCAL_AMFLAGS=-I m4
SUBDIRS=vemmd vemmagent debian
EXTRA_DIST=common vemm.spec.in
dist_noinst_DATA=vemm.spec
CLEANFILES=vemm.spec
edit = $(SED) -e 's|@PACKAGE[@]|@PACKAGE@|g' \
              -e 's|@VERSION[@]|@VERSION@|g' \
              -e 's|@RELEASE[@]|@RELEASE@|g' \
              -e 's|@prefix[@]|@prefix@|g' \
              -e 's|@localstatedir[@]|@localstatedir@|g' \
              -e 's|@sysconfdir[@]|@sysconfdir@|g' \
              -e 's|@systemd_unitdir[@]|@systemd_unitdir@|g' \
              -e 's|@os_launcher_confdir[@]|@os_launcher_confdir@|g'

vemm.spec: $(srcdir)/vemm.spec.in
	$(edit) $< > $@
