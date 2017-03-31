AC_INIT(libplatform, 2.0.0, simo.xan@gmail.com)
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE
AM_PROG_AR
AM_CONFIG_HEADER(src/config.h)
AC_CONFIG_MACRO_DIR([m4])
AC_SUBST(VERSION)
ISODATE=`date +%Y-%m-%d`
AC_SUBST(ISODATE)
AC_PREFIX_DEFAULT(/usr/local)
if test "${prefix}" == "NONE" ; then
	prefix=/usr/local
fi
AC_ARG_ENABLE(debug, [  --enable-debug    Enable debug compilation])
AC_ARG_ENABLE(printdebug, [  --enable-printdebug    Enable the print debug and the debug compilation])
AC_ARG_ENABLE(profile, [  --enable-profile    Enable the compilation for the automatically profiler tools])
AC_ARG_ENABLE(mingw32, [  --enable-mingw32    Enable mingw32 compiling])
AM_CONDITIONAL(DEBUG, test "$enable_debug" = "yes")
AM_CONDITIONAL(PRINTDEBUG, test "$enable_printdebug" = "yes")
AM_CONDITIONAL(PROFILE, test "$enable_profile" = "yes")
AM_CONDITIONAL(MINGW32, test "$enable_mingw32" = "yes")
AC_DEFINE_UNQUOTED(PREFIX,		"${prefix}",				[Prefix directory])
AC_DEFINE_UNQUOTED(DATADIR,		"${prefix}/share",			[Data directory])
AC_CANONICAL_HOST
case $host_os in
  *mingw*)
    AM_CONDITIONAL(MINGW,true)
    ;;
    *)
    AM_CONDITIONAL(MINGW,false)
    ;;
esac
case $host_os in
  *cygwin*)
    AM_CONDITIONAL(CYGWIN,true)
    ;;
  *solaris*)
    AM_CONDITIONAL(CYGWIN,false)
    ;;
    *)
    AM_CONDITIONAL(CYGWIN,false)
    ;;
esac

AC_ARG_ENABLE([api-doc],
              AS_HELP_STRING([--enable-api-doc], [generate api documentation]))
AM_CONDITIONAL(API_DOC, test "$enable_api_doc" = "yes")

##############################################################################################################################
#						Initialize compiler default options
##############################################################################################################################
AM_INIT_AUTOMAKE(-Wall -Werror)
CFLAGS="$CFLAGS -Wall"
AC_SUBST(CFLAGS)

##############################################################################################################################
#						Checks for programs.
##############################################################################################################################
AC_PROG_INSTALL
AC_PROG_CC
AC_PROG_LIBTOOL
AC_PROG_MAKE_SET
if ! test -z "$MAKE" ; then
	AC_ERROR([Error: make program not exist, you have to install it])
fi
AC_CHECK_TOOL(BZIP2, bzip2, no)
if test "$BZIP2" = "no" ; then
	AC_ERROR([Error: you have to install bzip2 program])
fi

##############################################################################################################################
#						Checks for libraries.
##############################################################################################################################
AC_CHECK_FUNC(clock_gettime,clockgettime="yes",clockgettime="no")
AC_CHECK_LIB(rt, clock_gettime, clockgettime="yes";needrt="yes",clockgettime="no")

if test "$clockgettime" == "no" ; then
	AC_MSG_RESULT([Building the profile with the gettimeofday function])
else 
	AC_MSG_RESULT([Building the profile with the clock_gettime function])
	AC_DEFINE(PROFILE_CLOCKGETTIME, 1, [Define to use the gettimeofday function to make the profile])
fi
AM_CONDITIONAL(CLOCKGETTIME, test "$clockgettime" = "yes")
AM_CONDITIONAL(NEEDRT, test "$needrt" = "yes")

##############################################################################################################################
##						Checks for headers.
##############################################################################################################################
AC_HEADER_STDC
AC_CHECK_HEADERS(unistd.h sys/param.h sys/time.h time.h sys/mkdev.h sys/sysmacros.h string.h memory.h fcntl.h dirent.h sys/ndir.h ndir.h alloca.h locale.h )

AC_MSG_CHECKING([for architecture])
case "$host" in 
	i?86-* | k?-* | x86_64-*)
		case "$host" in
			i386-*)
			echo "Intel x86 32 bit i386"
			UINT64="unsigned long long int"
			AC_SUBST(UINT64)
			UINT32="unsigned int"
			AC_SUBST(UINT32)
			UINT16="unsigned short int"
			AC_SUBST(UINT16)
			UINT8="unsigned char"
			AC_SUBST(UINT8)
			NUINT="unsigned int"
			AC_SUBST(NUINT)
			INT64="long long int"
			AC_SUBST(INT64)
			INT32="int"
			AC_SUBST(INT32)
			INT16="short int"
			AC_SUBST(INT16)
			INT8="signed char"
			AC_SUBST(INT8)
			NINT="int"
			AC_SUBST(NINT)
			FLOAT32="float"
			AC_SUBST(FLOAT32)
			FLOAT64="double"
			AC_SUBST(FLOAT64)
			NFLOAT="float"
			AC_SUBST(NFLOAT)
			
			MAX_INT8="SCHAR_MAX"
			AC_SUBST(MAX_INT8)
			MIN_INT8="SCHAR_MIN"
			AC_SUBST(MIN_INT8)
			MAX_UINT8="CHAR_MAX"
			AC_SUBST(MAX_UINT8)			
			MIN_UINT8="CHAR_MIN"
			AC_SUBST(MIN_UINT8)
			MAX_INT16="SHRT_MAX"
			AC_SUBST(MAX_INT16)			
			MIN_INT16="SHRT_MIN"
			AC_SUBST(MIN_INT16)
			MAX_UINT16="USHRT_MAX"
			AC_SUBST(MAX_UINT16)			
			MIN_UINT16="USHRT_MIN"
			AC_SUBST(MIN_UINT16)
			MAX_INT32="INT_MAX"
			AC_SUBST(MAX_INT32)			
			MIN_INT32="INT_MIN"
			AC_SUBST(MIN_INT32)
			MAX_UINT32="UINT_MAX"
			AC_SUBST(MAX_UINT32)			
			MIN_UINT32="UINT_MIN"
			AC_SUBST(MIN_UINT32)									
			MAX_INT64="LLONG_MAX"
			AC_SUBST(MAX_INT64)			
			MIN_INT64="LLONG_MIN"
			AC_SUBST(MIN_INT64)
			MAX_UINT64="ULLONG_MAX"
			AC_SUBST(MAX_UINT64)			
			MIN_UINT64="ULLONG_MIN"
			AC_SUBST(MIN_UINT64)
			MAX_NUINT="UINT_MAX"
			AC_SUBST(MAX_NUINT)		
			MAX_FLOAT32="FLT_MAX"
			AC_SUBST(MAX_FLOAT32)			
			MIN_FLOAT32="FLT_MIN"
			AC_SUBST(MIN_FLOAT32)
			MAX_FLOAT64="DBL_MAX"
			AC_SUBST(MAX_FLOAT64)			
			MIN_FLOAT64="DBL_MIN"
			AC_SUBST(MIN_FLOAT64)
			MAX_NFLOAT="FLT_MAX"
			AC_SUBST(MAX_NFLOAT)			
			MIN_NFLOAT="FLT_MIN"
			AC_SUBST(MIN_NFLOAT)					
			;;
			i486-*)
			echo "Intel x86 32 bit i486"
			UINT64="unsigned long long int"
			AC_SUBST(UINT64)
			UINT32="unsigned int"
			AC_SUBST(UINT32)
			UINT16="unsigned short int"
			AC_SUBST(UINT16)
			UINT8="unsigned char"
			AC_SUBST(UINT8)
			NUINT="unsigned int"
			AC_SUBST(NUINT)
			INT64="long long int"
			AC_SUBST(INT64)
			INT32="int"
			AC_SUBST(INT32)
			INT16="short int"
			AC_SUBST(INT16)
			INT8="signed char"
			AC_SUBST(INT8)
			NINT="int"
			AC_SUBST(NINT)
			FLOAT32="float"
			AC_SUBST(FLOAT32)
			FLOAT64="double"
			AC_SUBST(FLOAT64)
			NFLOAT="float"
			AC_SUBST(NFLOAT)
			
			MAX_INT8="SCHAR_MAX"
			AC_SUBST(MAX_INT8)
			MIN_INT8="SCHAR_MIN"
			AC_SUBST(MIN_INT8)
			MAX_UINT8="CHAR_MAX"
			AC_SUBST(MAX_UINT8)			
			MIN_UINT8="CHAR_MIN"
			AC_SUBST(MIN_UINT8)
			MAX_INT16="SHRT_MAX"
			AC_SUBST(MAX_INT16)			
			MIN_INT16="SHRT_MIN"
			AC_SUBST(MIN_INT16)
			MAX_UINT16="USHRT_MAX"
			AC_SUBST(MAX_UINT16)			
			MIN_UINT16="USHRT_MIN"
			AC_SUBST(MIN_UINT16)
			MAX_INT32="INT_MAX"
			AC_SUBST(MAX_INT32)			
			MIN_INT32="INT_MIN"
			AC_SUBST(MIN_INT32)
			MAX_UINT32="UINT_MAX"
			AC_SUBST(MAX_UINT32)			
			MIN_UINT32="UINT_MIN"
			AC_SUBST(MIN_UINT32)									
			MAX_INT64="LLONG_MAX"
			AC_SUBST(MAX_INT64)			
			MIN_INT64="LLONG_MIN"
			AC_SUBST(MIN_INT64)
			MAX_UINT64="ULLONG_MAX"
			AC_SUBST(MAX_UINT64)			
			MIN_UINT64="ULLONG_MIN"
			AC_SUBST(MIN_UINT64)
			MAX_NUINT="UINT_MAX"
			AC_SUBST(MAX_NUINT)		
			MAX_FLOAT32="FLT_MAX"
			AC_SUBST(MAX_FLOAT32)			
			MIN_FLOAT32="FLT_MIN"
			AC_SUBST(MIN_FLOAT32)
			MAX_FLOAT64="DBL_MAX"
			AC_SUBST(MAX_FLOAT64)			
			MIN_FLOAT64="DBL_MIN"
			AC_SUBST(MIN_FLOAT64)
			MAX_NFLOAT="FLT_MAX"
			AC_SUBST(MAX_NFLOAT)			
			MIN_NFLOAT="FLT_MIN"
			AC_SUBST(MIN_NFLOAT)		
			;;
			i586-*)
			echo "Intel x86 32 bit i586"
			UINT64="unsigned long long int"
			AC_SUBST(UINT64)
			UINT32="unsigned int"
			AC_SUBST(UINT32)
			UINT16="unsigned short int"
			AC_SUBST(UINT16)
			UINT8="unsigned char"
			AC_SUBST(UINT8)
			NUINT="unsigned int"
			AC_SUBST(NUINT)
			INT64="long long int"
			AC_SUBST(INT64)
			INT32="int"
			AC_SUBST(INT32)
			INT16="short int"
			AC_SUBST(INT16)
			INT8="signed char"
			AC_SUBST(INT8)
			NINT="int"
			AC_SUBST(NINT)
			FLOAT32="float"
			AC_SUBST(FLOAT32)
			FLOAT64="double"
			AC_SUBST(FLOAT64)
			NFLOAT="float"
			AC_SUBST(NFLOAT)
			
			MAX_INT8="SCHAR_MAX"
			AC_SUBST(MAX_INT8)
			MIN_INT8="SCHAR_MIN"
			AC_SUBST(MIN_INT8)
			MAX_UINT8="CHAR_MAX"
			AC_SUBST(MAX_UINT8)			
			MIN_UINT8="CHAR_MIN"
			AC_SUBST(MIN_UINT8)
			MAX_INT16="SHRT_MAX"
			AC_SUBST(MAX_INT16)			
			MIN_INT16="SHRT_MIN"
			AC_SUBST(MIN_INT16)
			MAX_UINT16="USHRT_MAX"
			AC_SUBST(MAX_UINT16)			
			MIN_UINT16="USHRT_MIN"
			AC_SUBST(MIN_UINT16)
			MAX_INT32="INT_MAX"
			AC_SUBST(MAX_INT32)			
			MIN_INT32="INT_MIN"
			AC_SUBST(MIN_INT32)
			MAX_UINT32="UINT_MAX"
			AC_SUBST(MAX_UINT32)			
			MIN_UINT32="UINT_MIN"
			AC_SUBST(MIN_UINT32)									
			MAX_INT64="LLONG_MAX"
			AC_SUBST(MAX_INT64)			
			MIN_INT64="LLONG_MIN"
			AC_SUBST(MIN_INT64)
			MAX_UINT64="ULLONG_MAX"
			AC_SUBST(MAX_UINT64)			
			MIN_UINT64="ULLONG_MIN"
			AC_SUBST(MIN_UINT64)
			MAX_NUINT="UINT_MAX"
			AC_SUBST(MAX_NUINT)		
			MAX_FLOAT32="FLT_MAX"
			AC_SUBST(MAX_FLOAT32)			
			MIN_FLOAT32="FLT_MIN"
			AC_SUBST(MIN_FLOAT32)
			MAX_FLOAT64="DBL_MAX"
			AC_SUBST(MAX_FLOAT64)			
			MIN_FLOAT64="DBL_MIN"
			AC_SUBST(MIN_FLOAT64)
			MAX_NFLOAT="FLT_MAX"
			AC_SUBST(MAX_NFLOAT)			
			MIN_NFLOAT="FLT_MIN"
			AC_SUBST(MIN_NFLOAT)		
			;;
			i686-*)
			echo "Intel x86 32 bit i686"
			UINT64="unsigned long long int"
			AC_SUBST(UINT64)
			UINT32="unsigned int"
			AC_SUBST(UINT32)
			UINT16="unsigned short int"
			AC_SUBST(UINT16)
			UINT8="unsigned char"
			AC_SUBST(UINT8)
			NUINT="unsigned int"
			AC_SUBST(NUINT)
			INT64="long long int"
			AC_SUBST(INT64)
			INT32="int"
			AC_SUBST(INT32)
			INT16="short int"
			AC_SUBST(INT16)
			INT8="signed char"
			AC_SUBST(INT8)
			NINT="int"
			AC_SUBST(NINT)
			FLOAT32="float"
			AC_SUBST(FLOAT32)
			FLOAT64="double"
			AC_SUBST(FLOAT64)
			NFLOAT="float"
			AC_SUBST(NFLOAT)
			
			MAX_INT8="SCHAR_MAX"
			AC_SUBST(MAX_INT8)
			MIN_INT8="SCHAR_MIN"
			AC_SUBST(MIN_INT8)
			MAX_UINT8="CHAR_MAX"
			AC_SUBST(MAX_UINT8)			
			MIN_UINT8="CHAR_MIN"
			AC_SUBST(MIN_UINT8)
			MAX_INT16="SHRT_MAX"
			AC_SUBST(MAX_INT16)			
			MIN_INT16="SHRT_MIN"
			AC_SUBST(MIN_INT16)
			MAX_UINT16="USHRT_MAX"
			AC_SUBST(MAX_UINT16)			
			MIN_UINT16="USHRT_MIN"
			AC_SUBST(MIN_UINT16)
			MAX_INT32="INT_MAX"
			AC_SUBST(MAX_INT32)			
			MIN_INT32="INT_MIN"
			AC_SUBST(MIN_INT32)
			MAX_UINT32="UINT_MAX"
			AC_SUBST(MAX_UINT32)			
			MIN_UINT32="UINT_MIN"
			AC_SUBST(MIN_UINT32)									
			MAX_INT64="LLONG_MAX"
			AC_SUBST(MAX_INT64)			
			MIN_INT64="LLONG_MIN"
			AC_SUBST(MIN_INT64)
			MAX_UINT64="ULLONG_MAX"
			AC_SUBST(MAX_UINT64)			
			MIN_UINT64="ULLONG_MIN"
			AC_SUBST(MIN_UINT64)
			MAX_NUINT="UINT_MAX"
			AC_SUBST(MAX_NUINT)		
			MAX_FLOAT32="FLT_MAX"
			AC_SUBST(MAX_FLOAT32)			
			MIN_FLOAT32="FLT_MIN"
			AC_SUBST(MIN_FLOAT32)
			MAX_FLOAT64="DBL_MAX"
			AC_SUBST(MAX_FLOAT64)			
			MIN_FLOAT64="DBL_MIN"
			AC_SUBST(MIN_FLOAT64)
			MAX_NFLOAT="FLT_MAX"
			AC_SUBST(MAX_NFLOAT)			
			MIN_NFLOAT="FLT_MIN"
			AC_SUBST(MIN_NFLOAT)			
			;;
			k6-*)
			echo "Intel x86 32 bit k6"
			UINT64="unsigned long long int"
			AC_SUBST(UINT64)
			UINT32="unsigned int"
			AC_SUBST(UINT32)
			UINT16="unsigned short int"
			AC_SUBST(UINT16)
			UINT8="unsigned char"
			AC_SUBST(UINT8)
			NUINT="unsigned int"
			AC_SUBST(NUINT)
			INT64="long long int"
			AC_SUBST(INT64)
			INT32="int"
			AC_SUBST(INT32)
			INT16="short int"
			AC_SUBST(INT16)
			INT8="signed char"
			AC_SUBST(INT8)
			NINT="int"
			AC_SUBST(NINT)
			FLOAT32="float"
			AC_SUBST(FLOAT32)
			FLOAT64="double"
			AC_SUBST(FLOAT64)
			NFLOAT="float"
			AC_SUBST(NFLOAT)
			
			MAX_INT8="SCHAR_MAX"
			AC_SUBST(MAX_INT8)
			MIN_INT8="SCHAR_MIN"
			AC_SUBST(MIN_INT8)
			MAX_UINT8="CHAR_MAX"
			AC_SUBST(MAX_UINT8)			
			MIN_UINT8="CHAR_MIN"
			AC_SUBST(MIN_UINT8)
			MAX_INT16="SHRT_MAX"
			AC_SUBST(MAX_INT16)			
			MIN_INT16="SHRT_MIN"
			AC_SUBST(MIN_INT16)
			MAX_UINT16="USHRT_MAX"
			AC_SUBST(MAX_UINT16)			
			MIN_UINT16="USHRT_MIN"
			AC_SUBST(MIN_UINT16)
			MAX_INT32="INT_MAX"
			AC_SUBST(MAX_INT32)			
			MIN_INT32="INT_MIN"
			AC_SUBST(MIN_INT32)
			MAX_UINT32="UINT_MAX"
			AC_SUBST(MAX_UINT32)			
			MIN_UINT32="UINT_MIN"
			AC_SUBST(MIN_UINT32)									
			MAX_INT64="LLONG_MAX"
			AC_SUBST(MAX_INT64)			
			MIN_INT64="LLONG_MIN"
			AC_SUBST(MIN_INT64)
			MAX_UINT64="ULLONG_MAX"
			AC_SUBST(MAX_UINT64)			
			MIN_UINT64="ULLONG_MIN"
			AC_SUBST(MIN_UINT64)
			MAX_NUINT="UINT_MAX"
			AC_SUBST(MAX_NUINT)		
			MAX_FLOAT32="FLT_MAX"
			AC_SUBST(MAX_FLOAT32)			
			MIN_FLOAT32="FLT_MIN"
			AC_SUBST(MIN_FLOAT32)
			MAX_FLOAT64="DBL_MAX"
			AC_SUBST(MAX_FLOAT64)			
			MIN_FLOAT64="DBL_MIN"
			AC_SUBST(MIN_FLOAT64)
			MAX_NFLOAT="FLT_MAX"
			AC_SUBST(MAX_NFLOAT)			
			MIN_NFLOAT="FLT_MIN"
			AC_SUBST(MIN_NFLOAT)			
			;;
			x86_64-*)
			echo "Intel x86 64 bit"
			UINT64="unsigned long long int"
			AC_SUBST(UINT64)
			UINT32="unsigned int"
			AC_SUBST(UINT32)
			UINT16="unsigned short int"
			AC_SUBST(UINT16)
			UINT8="unsigned char"
			AC_SUBST(UINT8)
			NUINT="unsigned long int"
			AC_SUBST(NUINT)
			INT64="long long int"
			AC_SUBST(INT64)
			INT32="int"
			AC_SUBST(INT32)
			INT16="short int"
			AC_SUBST(INT16)
			INT8="signed char"
			AC_SUBST(INT8)
			NINT="long int"
			AC_SUBST(NINT)
			FLOAT32="float"
			AC_SUBST(FLOAT32)
			FLOAT64="double"
			AC_SUBST(FLOAT64)
			NFLOAT="double"
			AC_SUBST(NFLOAT)
			
			MAX_INT8="SCHAR_MAX"
			AC_SUBST(MAX_INT8)
			MIN_INT8="SCHAR_MIN"
			AC_SUBST(MIN_INT8)
			MAX_UINT8="CHAR_MAX"
			AC_SUBST(MAX_UINT8)			
			MIN_UINT8="CHAR_MIN"
			AC_SUBST(MIN_UINT8)
			MAX_INT16="SHRT_MAX"
			AC_SUBST(MAX_INT16)			
			MIN_INT16="SHRT_MIN"
			AC_SUBST(MIN_INT16)
			MAX_UINT16="USHRT_MAX"
			AC_SUBST(MAX_UINT16)			
			MIN_UINT16="USHRT_MIN"
			AC_SUBST(MIN_UINT16)
			MAX_INT32="INT_MAX"
			AC_SUBST(MAX_INT32)			
			MIN_INT32="INT_MIN"
			AC_SUBST(MIN_INT32)
			MAX_UINT32="UINT_MAX"
			AC_SUBST(MAX_UINT32)			
			MIN_UINT32="UINT_MIN"
			AC_SUBST(MIN_UINT32)									
			MAX_INT64="LLONG_MAX"
			AC_SUBST(MAX_INT64)			
			MIN_INT64="LLONG_MIN"
			AC_SUBST(MIN_INT64)
			MAX_UINT64="ULLONG_MAX"
			AC_SUBST(MAX_UINT64)			
			MIN_UINT64="ULLONG_MIN"
			AC_SUBST(MIN_UINT64)
			MAX_NUINT="ULONG_MAX"
			AC_SUBST(MAX_NUINT)		
			MAX_FLOAT32="FLT_MAX"
			AC_SUBST(MAX_FLOAT32)			
			MIN_FLOAT32="FLT_MIN"
			AC_SUBST(MIN_FLOAT32)
			MAX_FLOAT64="DBL_MAX"
			AC_SUBST(MAX_FLOAT64)			
			MIN_FLOAT64="DBL_MIN"
			AC_SUBST(MIN_FLOAT64)
			MAX_NFLOAT="DBL_MAX"
			AC_SUBST(MAX_NFLOAT)			
			MIN_NFLOAT="DBL_MIN"
			AC_SUBST(MIN_NFLOAT)			
			;;
		*)
			echo "ERROR: Cannot detect host architecture type!"
			;;
		esac
		;;
	arm-* | arm7l-*)
		echo "Generic ARM"
		UINT64="unsigned long long int"
		AC_SUBST(UINT64)
		UINT32="unsigned int"
		AC_SUBST(UINT32)
		UINT16="unsigned short int"
		AC_SUBST(UINT16)
		UINT8="unsigned char"
		AC_SUBST(UINT8)
		NUINT="unsigned int"
		AC_SUBST(NUINT)
		INT64="long long int"
		AC_SUBST(INT64)
		INT32="int"
		AC_SUBST(INT32)
		INT16="short int"
		AC_SUBST(INT16)
		INT8="signed char"
		AC_SUBST(INT8)
		NINT="int"
		AC_SUBST(NINT)
		FLOAT32="float"
		AC_SUBST(FLOAT32)
		FLOAT64="double"
		AC_SUBST(FLOAT64)
		NFLOAT="double"
		AC_SUBST(NFLOAT)
		
			MAX_INT8="SCHAR_MAX"
			AC_SUBST(MAX_INT8)
			MIN_INT8="SCHAR_MIN"
			AC_SUBST(MIN_INT8)
			MAX_UINT8="CHAR_MAX"
			AC_SUBST(MAX_UINT8)			
			MIN_UINT8="CHAR_MIN"
			AC_SUBST(MIN_UINT8)
			MAX_INT16="SHRT_MAX"
			AC_SUBST(MAX_INT16)			
			MIN_INT16="SHRT_MIN"
			AC_SUBST(MIN_INT16)
			MAX_UINT16="USHRT_MAX"
			AC_SUBST(MAX_UINT16)			
			MIN_UINT16="USHRT_MIN"
			AC_SUBST(MIN_UINT16)
			MAX_INT32="INT_MAX"
			AC_SUBST(MAX_INT32)			
			MIN_INT32="INT_MIN"
			AC_SUBST(MIN_INT32)
			MAX_UINT32="UINT_MAX"
			AC_SUBST(MAX_UINT32)			
			MIN_UINT32="UINT_MIN"
			AC_SUBST(MIN_UINT32)									
			MAX_INT64="LLONG_MAX"
			AC_SUBST(MAX_INT64)			
			MIN_INT64="LLONG_MIN"
			AC_SUBST(MIN_INT64)
			MAX_UINT64="ULLONG_MAX"
			AC_SUBST(MAX_UINT64)			
			MIN_UINT64="ULLONG_MIN"
			AC_SUBST(MIN_UINT64)
			MAX_NUINT="UINT_MAX"
			AC_SUBST(MAX_NUINT)			
			MAX_FLOAT32="FLT_MAX"
			AC_SUBST(MAX_FLOAT32)			
			MIN_FLOAT32="FLT_MIN"
			AC_SUBST(MIN_FLOAT32)
			MAX_FLOAT64="DBL_MAX"
			AC_SUBST(MAX_FLOAT64)			
			MIN_FLOAT64="DBL_MIN"
			AC_SUBST(MIN_FLOAT64)
			MAX_NFLOAT="DBL_MAX"
			AC_SUBST(MAX_NFLOAT)			
			MIN_NFLOAT="DBL_MIN"
			AC_SUBST(MIN_NFLOAT)		
		;;
	ppc-* | powerpc-*)
		echo "Power PC"
		;;
	sparc-* | sparc64-*)
		echo "Sparc"
		;;
	alpha*)
		echo "Alpha"
		;;
	*)
		echo "Warning: cannot detect host architecture type!"
		;;
	esac
AC_MSG_RESULT([$host]);

AC_HEADER_MAJOR
AC_FUNC_ALLOCA
AC_STRUCT_TM
AC_STRUCT_ST_BLOCKS

AC_OUTPUT(
	Makefile 
	src/Makefile
	doc/Makefile
	src/jitsystem.h
	include/Makefile
	pkgconfig/libplatform.pc
	pkgconfig/Makefile
)
