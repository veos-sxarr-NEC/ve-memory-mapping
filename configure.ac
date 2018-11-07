#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([VE memory mapping], [1.3.1])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AC_ARG_WITH([release-id], [AS_HELP_STRING([--with-release-id],
                                          [Specify the release ID [1]])],
            [RELEASE=${with_release_id}],
            [RELEASE=1])
AC_SUBST(RELEASE)

AC_ARG_WITH([systemd-unitdir],
            AS_HELP_STRING([--with-systemd-unitdir=DIR],
                           [Directory for systemd unit configuration files]),
            [systemd_unitdir="$withval"],
            [systemd_unitdir='/lib/systemd/system'])
AC_SUBST(systemd_unitdir)

AC_ARG_WITH([os-launcher-confdir],
            AS_HELP_STRING([--with-os-launcher-confdir=DIR],
            [Directory for OS launcher options [sysconfdir/ve-os-launcher.d]]),
            [os_launcher_confdir="$withval"],
            [os_launcher_confdir="${sysconfdir}/ve-os-launcher.d"])
AC_SUBST(os_launcher_confdir)

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AM_PROG_LIBTOOL

AX_CXX_COMPILE_STDCXX_11([noext], [mandatory])

# Checks for header files.
test x"${prefix}" = xNONE && prefix="$ac_default_prefix"
test x"${exec_prefix}" = xNONE && exec_prefix="$prefix"
extra_includedir=$(eval echo $includedir)
extra_libdir=$(eval echo $libdir)
if test "$extra_includedir" != /usr/include; then
  CPPFLAGS="$CPPFLAGS -I${extra_includedir}"
fi
LDFLAGS="$LDFLAGS -L${extra_libdir}"
AC_CHECK_HEADERS([velayout.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AX_BOOST_BASE([], [], [AC_MSG_ERROR([Boost is missing.])])
AX_BOOST_PROGRAM_OPTIONS
AX_BOOST_ASIO
PKG_CHECK_MODULES([PROTOBUF], [protobuf], [],
                  [AC_MSG_ERROR([protobuf-devel is missing.])])
AC_CHECK_LIB([log4c], [log4c_init], [],
             [AC_MSG_ERROR([log4c library is missing.])])
PKG_CHECK_MODULES([SYSTEMD], [libsystemd], [],
                  [AC_MSG_ERROR([systemd-devel is missing.])])

AX_LIB_EV([], [], [AC_MSG_ERROR([libev-devel is missing.])])

AC_CHECK_HEADER([velayout.h], [], [AC_MSG_ERROR([velayout-devel is missing.])])
AC_CHECK_LIB([velayout], [ve_layout], [],
             [AC_MSG_ERROR([velayout is missing.])])

AC_CONFIG_FILES([Makefile
                 debian/Makefile
                 vemmd/Makefile
                 vemmagent/Makefile])
AC_OUTPUT
