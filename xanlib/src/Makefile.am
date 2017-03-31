libxan_LTLIBRARIES = libxan.la
libxandir = $(libdir)

AM_CFLAGS= -Wall -Wextra -D_GNU_SOURCE $(PLATFORM_CFLAGS)

if MULTIAPP
AM_CFLAGS	+= -DMULTIAPP
endif

if PROFILE
AM_CFLAGS 	+= -DNDEBUG -DPROFILE -O0 -g
else
if PRINTDEBUG
AM_CFLAGS	+= -g -DDEBUG -DPRINTDEBUG -O0
else 
if DEBUG
AM_CFLAGS 	+= -DDEBUG -O0 -g
else 
AM_CFLAGS	+= -O3 -DNDEBUG 
endif
endif
endif

libxan_la_SOURCES=			 				\
		xanlib.h			xanlib.c		\
		xan-system.h			xanlist.c		\
		xanvar.c			xanpipe.c		\
		xanqueue.c			xanstack.c		\
		xantable.c			xangraph.c		\
		xanbitset.c

libxan_la_LDFLAGS= -fPIC -lpthread -lm
libxan_la_LIBADD= $(PLATFORM_LIBS)
