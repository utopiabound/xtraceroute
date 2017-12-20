# -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([xtraceroute], [0.9.2])
AC_CONFIG_SRCDIR([main.c])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE
AC_CONFIG_MACRO_DIRS([m4])

# Checks for programs.
AC_PROG_CC
AC_PROG_LN_S
AC_PROG_INSTALL
AC_CANONICAL_HOST
AC_USE_SYSTEM_EXTENSIONS

LT_PREREQ([2.2.2])
LT_INIT([disable-static])
AC_SUBST([LIBTOOL_DEPS])

# Checks for header files.
AC_FUNC_ALLOCA
AC_CHECK_HEADERS([argz.h arpa/inet.h fcntl.h inttypes.h libintl.h limits.h locale.h malloc.h netdb.h netinet/in.h nl_types.h stddef.h stdlib.h string.h strings.h sys/param.h sys/socket.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_FUNC_REALLOC
#AC_CHECK_FUNCS([__argz_count __argz_next __argz_stringify dup2 floor getcwd gethostbyname inet_ntoa mempcpy memset mkdir munmap putenv rint setenv setlocale sqrt stpcpy strcasecmp strchr strdup strncasecmp uname])
AC_SEARCH_LIBS([sqrt], [m])
AM_SILENT_RULES([yes])

xtraceroutedatadir=${datadir}/xtraceroute
AC_SUBST(xtraceroutedatadir)
xtraceroutelocaledir=${datadir}/locale
AC_SUBST(xtraceroutelocaledir)

AC_ARG_ENABLE(debug,
AS_HELP_STRING([--enable-debug],
               [enable debugging, default: no]),
[case "${enableval}" in
             yes|"") debug=true ;;
             no)  debug=false ;;
             *)   AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
esac],
[debug=false])

AM_CONDITIONAL(DEBUG, test x"$debug" = x"true")


dnl Find traceroute.

AC_ARG_WITH(traceroute, AS_HELP_STRING([--with-traceroute=PATH],[Where traceroute is.]))
AS_IF([test "x$with_traceroute" = "x"], [
	MYPATH=$PATH:/usr/etc:/usr/sbin
	AC_PATH_PROG(TP,
		traceroute, [""],
		[$MYPATH])
	if test "x$TP" = "x" ; then
		AC_MSG_ERROR([Can't find traceroute. Use '--with-traceroute'])
        fi
], [
	TP=$with_traceroute
])

AC_DEFINE_UNQUOTED(TRACEPGM, "$TP", "Where traceroute is.")

dnl Find host.

AC_ARG_WITH(host, AS_HELP_STRING([--with-host=PATH],[Where host is.]))

if test "x$with_host" = "x" ; then
	MYPATH=$PATH:/usr/etc:/usr/sbin:/usr/local:/usr/pd/bin
	AC_PATH_PROG(HP,
		host, [""],
		[$MYPATH])
	if test "x$HP" = "x" ; then
		AC_MSG_ERROR([Can't find host. Use '--with-host'])
	fi
else
	HP=$with_host
fi

AC_DEFINE_UNQUOTED(HOSTPGM, "$HP", "path to host program.")
hostbinary=$HP
AC_SUBST(hostbinary)

dnl Checks for libraries.

PKG_CHECK_MODULES(GDK_PIXBUF, gdk-pixbuf-2.0)
PKG_CHECK_MODULES(GTKGL, gtkgl-2.0)
PKG_CHECK_MODULES(GTK, gtk+-2.0)

AC_ARG_WITH([geoip], AS_HELP_STRING([--with-geoip], [Build with GEOIP Support]))
AS_IF([test "x$with_geoip" = "xyes"], [
   PKG_CHECK_MODULES([GEOIP], [geoip])
   AC_DEFINE([HAVE_GEOIP], [1], [Define if GeoIP exists.])
])

AC_CONFIG_FILES([Makefile
		 share/Makefile
		 share/xtraceroute-resolve-location.sh
		 xtraceroute.1
		 xtraceroute.spec])
AC_OUTPUT
