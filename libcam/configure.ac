AC_INIT(libcam, 1.0.0, simo.xan@gmail.com)
AC_LANG([C++])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE
AM_PROG_AR
AM_CONFIG_HEADER(src/config.h)
AC_SUBST(VERSION)
ISODATE=`date +%Y-%m-%d`
AC_SUBST(ISODATE)
AC_PREFIX_DEFAULT(/usr/local)
if test "${prefix}" == "NONE" ; then
	prefix=/usr/local
fi
AC_ARG_ENABLE(debug, [  --enable-debug    Enable debug compilation])
AC_ARG_ENABLE(optdebug, [  --enable-optdebug    Enable debug compilation])
AC_ARG_ENABLE(printdebug, [  --enable-printdebug    Enable the print debug and the debug compilation])
AC_ARG_ENABLE(cxxdebug, [  --enable-cxxdebug    Enable debugging with c++ pretty printers])
AC_ARG_ENABLE(camtest, [  --enable-camtest    Enable output of camtest binary for testing tracers])
AC_ARG_ENABLE(cambinary, [  --enable-cambinary    Enable generation of cam binary])
AC_ARG_ENABLE(profile, [  --enable-profile    Enable the compilation for the automatically profiler tools])
AC_ARG_ENABLE(memdebug, [  --enable-memdebug    Enable debugging of memory usage])
AC_ARG_ENABLE(track_mem_alloc, AS_HELP_STRING([--disable-track-mem-alloc], [Disable tracking of memory allocation]))
AM_CONDITIONAL(DEBUG, test "$enable_debug" = "yes")
AM_CONDITIONAL(OPTDEBUG, test "$enable_optdebug" = "yes")
AM_CONDITIONAL(PRINTDEBUG, test "$enable_printdebug" = "yes")
AM_CONDITIONAL(CXXDEBUG, test "$enable_cxxdebug" = "yes")
AM_CONDITIONAL(CAMTEST, test "$enable_camtest" = "yes")
AM_CONDITIONAL(CAMBINARY, test "$enable_cambinary" = "yes")
AM_CONDITIONAL(PROFILE, test "$enable_profile" = "yes")
AM_CONDITIONAL(MEMDEBUG, test "$enable_memdebug" = "yes")
AC_DEFINE_UNQUOTED(PREFIX,		"${prefix}",				[Prefix directory])
AC_DEFINE_UNQUOTED(DATADIR,		"${prefix}/share",			[Data directory])
AS_IF([test "x$enable_track_mem_alloc" != "xno"], [
  AC_DEFINE_UNQUOTED(TRACK_MEM_ALLOCATIONS, [1], [Tracking of memory allocations])
])
AC_CANONICAL_HOST

##############################################################################################################################
#						Initialize compiler default options
##############################################################################################################################
AM_INIT_AUTOMAKE(-Wall -Werror)
CFLAGS="$CFLAGS -Wall"
CXXFLAGS="$CXXFLAGS -Wall"
AC_SUBST(CFLAGS)
AC_SUBST(CXXFLAGS)


##############################################################################################################################
#						Checks for programs.
##############################################################################################################################
AC_PROG_INSTALL
AC_PROG_CXXCPP
AC_PROG_CXX
AC_PROG_LIBTOOL
AC_PROG_MAKE_SET
m4_pattern_allow([AC_MSG_ERROR])
if ! test -z "$MAKE" ; then
	AC_ERROR([Error: make program not exist, you have to install it])
fi
AC_CHECK_TOOL(PKGVAR, pkg-config)
if test -z "$PKGVAR" ; then
	echo "ERROR: I can't find the pkg-config program, then you have to install it" ;
	exit 1 ;
fi
AC_PROG_LEX
AC_CHECK_PROGS(XLEX,$LEX flex lex,none)
if test "$XLEX" = "none" ; then
	echo "*** Error: No suitable flex found. ***"
	echo "    Please install the 'flex' package."
	exit 1
fi
AC_CHECK_LIB(bz2 , BZ2_bzRead , [], AC_MSG_ERROR(missing libbz2

***ERROR***
Please install libbz2))

##############################################################################################################################
#						Checks for libraries.
##############################################################################################################################
PKG_CHECK_MODULES(PLATFORM, libplatform >= 0.6.0)
AC_SUBST(PLATFORM_CFLAGS)
AC_SUBST(PLATFORM_LIBS)

PKG_CHECK_MODULES(XAN, libxan >= 0.2.1)
AC_SUBST(XAN_CFLAGS)
AC_SUBST(XAN_LIBS)


##############################################################################################################################
##						Checks for headers.
##############################################################################################################################
AC_HEADER_STDC
AC_CHECK_HEADERS(unistd.h sys/param.h sys/time.h time.h sys/mkdev.h sys/sysmacros.h string.h memory.h fcntl.h dirent.h sys/ndir.h ndir.h alloca.h locale.h )

AC_HEADER_MAJOR
AC_FUNC_ALLOCA
AC_STRUCT_TM
AC_STRUCT_ST_BLOCKS

AC_OUTPUT(
	Makefile 
	src/Makefile
	include/Makefile
	pkgconfig/libcam.pc
	pkgconfig/Makefile
)
