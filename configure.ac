AC_INIT(wayland-tbm, [0.1.0], lsj119@samsung.com)

AC_CONFIG_SRCDIR(configure.ac)
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE(1.6 foreign subdir-objects)
AM_SILENT_RULES([yes])

# Checks for programs.
AC_PROG_CC
AC_PROG_LIBTOOL

AC_PATH_PROG([wayland_scanner], [wayland-scanner])
if test x$wayland_scanner = x; then
      AC_MSG_ERROR([wayland-scanner is needed to compile wayland-tbm])
fi

PKG_CHECK_MODULES([WL_TBM_COMMON], [libtbm capi-base-common dlog])
PKG_CHECK_MODULES([WL_TBM_CLIENT], [wayland-client])
PKG_CHECK_MODULES([WL_TBM_SERVER], [wayland-server])
PKG_CHECK_MODULES( WAYLAND_SCANNER, wayland-scanner)

AC_CONFIG_FILES([
	Makefile
    src/Makefile
    test/Makefile
    tool/Makefile
	wayland-tbm-server.pc
	wayland-tbm-client.pc
])
AC_OUTPUT
