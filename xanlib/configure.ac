AC_INIT(libxan, 2.0.0, simo.xan@gmail.com)
AM_INIT_AUTOMAKE
AM_PROG_AR
AM_CONFIG_HEADER(src/config.h)
AC_SUBST(VERSION)
ISODATE=`date +%Y-%m-%d`
AC_SUBST(ISODATE)

AC_PREFIX_DEFAULT(/usr)
if test "${prefix}" == "NONE" ; then
	prefix=/usr
fi
AC_ARG_ENABLE(debug, [  --enable-debug    Enable debug compilation])
AC_ARG_ENABLE(printdebug, [  --enable-printdebug    Enable the print debug and the debug compilation])
AC_ARG_ENABLE(profile, [  --enable-profile    Enable the compilation for the automatically profiler tools])
AC_ARG_ENABLE(multiapp, [  --enable-multiapp    Enable support for multi application environment])
AM_CONDITIONAL(DEBUG, test "$enable_debug" = "yes")
AM_CONDITIONAL(PRINTDEBUG, test "$enable_printdebug" = "yes")
AM_CONDITIONAL(PROFILE, test "$enable_profile" = "yes")
AM_CONDITIONAL(MULTIAPP, test "$enable_multiapp" = "yes")
AC_DEFINE_UNQUOTED(PREFIX,		"${prefix}",				[Prefix directory])
AC_DEFINE_UNQUOTED(DATADIR,		"${prefix}/share",			[Data directory])
AC_CANONICAL_HOST
AM_PROG_LIBTOOL

##############################################################################################################################
#						Initialize compiler default options
##############################################################################################################################
AM_INIT_AUTOMAKE(-Wall -Werror)
CFLAGS="$CFLAGS -Wall"
AC_SUBST(CFLAGS)

##############################################################################################################################
##						Checks for libraries.
##############################################################################################################################
PKG_CHECK_MODULES(PLATFORM, libplatform >= 2.0.0)
AC_SUBST(PLATFORM_CFLAGS)
AC_SUBST(PLATFORM_LIBS)

##############################################################################################################################
# 						Checks for programs.
##############################################################################################################################
AC_PROG_INSTALL
AC_PROG_CC
AC_PROG_LIBTOOL
if test "$BZIP2" = "no" ; then
	AC_ERROR([Error: you have to install bzip2 program])
fi

##############################################################################################################################
#	 					Checks for header files.
##############################################################################################################################
AC_HEADER_STDC
AC_CHECK_HEADERS(unistd.h sys/mman.h string.h)

AC_OUTPUT(
	Makefile 
	src/Makefile
	include/Makefile
	doc/Makefile
	pkgconfig/libxan.pc
	pkgconfig/Makefile
)
