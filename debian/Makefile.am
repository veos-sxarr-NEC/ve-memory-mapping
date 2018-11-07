EXTRA_DIST = compat control.in copyright rules.in \
             ve-memory-mapping.install.in \
             ve-memory-mapping-dev.install.in \
             ve-memory-mapping.postinst \
             ve-memory-mapping.postrm \
             ve-memory-mapping.preinst \
             ve-memory-mapping.prerm

dist_noinst_DATA = control rules \
                   ve-memory-mapping.install \
                   ve-memory-mapping-dev.install

CLEANFILES = $(dist_noinst_DATA)

edit = $(SED) -e 's|@PACKAGE[@]|@PACKAGE@|g' \
              -e 's|@VERSION[@]|@VERSION@|g' \
              -e 's|@RELEASE[@]|@RELEASE@|g' \
              -e 's|@prefix[@]|@prefix@|g' \
              -e 's|@sysconfdir[@]|@sysconfdir@|g' \
              -e 's|@includedir[@]|@includedir@|g' \
              -e 's|@localstatedir[@]|@localstatedir@|g' \
              -e 's|@libdir[@]|@libdir@|g' \
              -e 's|@sbindir[@]|@sbindir@|g' \
              -e 's|@systemd_unitdir[@]|@systemd_unitdir@|g' \
              -e 's|@os_launcher_confdir[@]|@os_launcher_confdir@|g'

rules: $(srcdir)/rules.in Makefile
	$(edit) $< > $@
	chmod a+x $@

control: $(srcdir)/control.in Makefile
	$(edit) $< > $@

ve-memory-mapping.install: $(srcdir)/ve-memory-mapping.install.in Makefile
	$(edit) $< > $@

ve-memory-mapping-dev.install: $(srcdir)/ve-memory-mapping-dev.install.in \
                               Makefile
	$(edit) $< > $@
