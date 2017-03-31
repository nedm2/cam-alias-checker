# Enable binary for dependence analysis (configure --enable-cam)
if CAMBINARY
bin_PROGRAMS = cam
endif

libcam_LTLIBRARIES = libcam.la
libcamdir = $(libdir)

AM_CPPFLAGS	= -std=c++0x -Wall -I../include -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64		\
		$(XAN_CFLAGS)						\
		$(PLATFORM_CFLAGS)      \
    -I ./intervaltree/

if PRINTDEBUG
AM_CPPFLAGS	+= -ggdb -DDEBUG -DPRINTDEBUG -O0 
else 
if DEBUG
AM_CPPFLAGS	+= -ggdb -DDEBUG -O0
else 
if OPTDEBUG
AM_CPPFLAGS	+= -ggdb -DDEBUG -O3
else 
AM_CPPFLAGS	+= -O3 -DNDEBUG 
endif
endif
endif

if CXXDEBUG
AM_CPPFLAGS	+= -D_GLIBCXX_DEBUG
endif

if MEMDEBUG
AM_CPPFLAGS	+= -DDEBUG_MEMUSE
endif

if PROFILE
AM_CPPFLAGS += -pg
endif

libcam_la_SOURCES=								\
		MemoryTracer.cpp		MemoryTracer.h		\
		loop_trace.cpp			loop_trace.hh			\
		ControlFlowCompressor.cpp			ControlFlowCompressor.h			\
		memory_allocator.cpp		memory_allocator.hh		\
		cam.cpp				cam.h				\
		cam_system.h                \
    TimeoutCounter.h

libcam_la_LIBADD	= $(XAN_LIBS) $(PLATFORM_LIBS) -lbz2 -lrt
libcam_la_LDFLAGS	= -shared -fPIC

cam_SHARED_SOURCES =                   \
		static_inst_rec.cpp		static_inst_rec.h		\
		loop_trace_cfc_parser.cpp		loop_trace_cfc_parser.h		\
		call_trace_cfc_parser.cpp		call_trace_cfc_parser.h		\
		memory_trace_parser.cpp		memory_trace_parser.h		\
    MemoryTraceStreamer.cpp  MemoryTraceStreamer.h    \
    LoopTraceStreamer.cpp  LoopTraceStreamer.h    \
    CallTraceStreamer.cpp  CallTraceStreamer.h    \
    BZ2ParserState.cpp  BZ2ParserState.h    \
		dependence_pairs_parser.cpp		dependence_pairs_parser.h		\
		static_ddg_parser.cpp		static_ddg_parser.h		\
		parser_wrappers.cpp		parser_wrappers.h		\
		CompressionPattern.cpp			CompressionPattern.h			\
    dependence_analysis.cpp dependence_analysis.h  \
    StaticLoopRec.cpp StaticLoopRec.h      \
    CallTrace.cpp CallTrace.h      \
    RepetitionPattern.cpp RepetitionPattern.h      \
    dynamic_gcd.cpp dynamic_gcd.h \
    unit_tests.cpp unit_tests.h 

cam_SOURCES=            \
    main.cpp            \
		TimeoutCounter.h     \
		$(cam_SHARED_SOURCES)  

cam_CXXFLAGS = $(AM_CPPFLAGS)

cam_LDADD = $(libcam_la_LIBADD) 
cam_LDFLAGS = -Wl,--no-as-needed -pthread

# Enable binary for testing tracers (configure --enable-camtest)
if CAMTEST
bin_PROGRAMS = cam camtest

camtest_SOURCES=            \
    camtest.cpp            \
    $(cam_SHARED_SOURCES)   \
    $(libcam_la_SOURCES)

camtest_CXXFLAGS = $(AM_CPPFLAGS)

camtest_LDADD = $(libcam_la_LIBADD) 
camtest_LDFLAGS = -Wl,--no-as-needed -pthread
endif

CLEANFILES = 							\
    memory_trace_parser.cpp \
    memory_trace_parser.h   \
    loop_trace_cfc_parser.cpp  \
    loop_trace_cfc_parser.h    \
    call_trace_cfc_parser.cpp  \
    call_trace_cfc_parser.h    \
    dependence_pairs_parser.cpp \
    dependence_pairs_parser.h   \
    static_ddg_parser.cpp  \
    static_ddg_parser.h 

memory_trace_parser.cpp memory_trace_parser.h: memory_trace_parser.lex
	$(LEX) $<

loop_trace_cfc_parser.cpp loop_trace_cfc_parser.h: loop_trace_cfc_parser.lex
	$(LEX) $<

call_trace_cfc_parser.cpp call_trace_cfc_parser.h: call_trace_cfc_parser.lex
	$(LEX) $<

dependence_pairs_parser.cpp dependence_pairs_parser.h: dependence_pairs_parser.lex
	$(LEX) $<

static_ddg_parser.cpp static_ddg_parser.h: static_ddg_parser.lex
	$(LEX) $<
