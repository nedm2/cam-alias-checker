MAINTAINERCLEANFILES = Makefile.in config.h.in

libplatform_LTLIBRARIES = libplatform.la
libplatformdir = $(libdir)
AM_CFLAGS= -I../include -Wall -Wextra -D_GNU_SOURCE

if MINGW
  AM_CFLAGS += -DWINDEBUG -lws2_32 -lpthreadGC2 -lregex -L/lib/dll
endif

if CYGWIN
  AM_CFLAGS += -shared
endif

if PROFILE
AM_CFLAGS	+= -DNDEBUG -DPROFILE -O0 -g
else
if PRINTDEBUG
AM_CFLAGS += -g -DDEBUG -DPRINTDEBUG -O0
else 
if DEBUG
AM_CFLAGS += -g -DDEBUG -O0
else 
AM_CFLAGS+= -O3 -DNDEBUG
endif
endif
endif

libplatform_la_SOURCES=			 				\
		platform_API.c			platform_API.h		\
		jitsystem.h

if MINGW
libplatform_la_LDFLAGS=-fPIC -no-undefined
else
libplatform_la_LDFLAGS= -fPIC -no-undefined -ldl
endif

if CYGWIN
  libplatform_la_LDFLAGS += -Wl,--enable-auto-import
endif

libplatform_la_LIBADD = -L$(libdir) -L/usr/lib 
