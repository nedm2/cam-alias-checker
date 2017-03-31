
/*
 * Copyright (C) 2012 - 2015  Niall Murphy
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef BZ2PARSERSTATE_H
#define BZ2PARSERSTATE_H

#include <sys/types.h>
#include <bzlib.h>
//#include "memory_trace_parser.h"

#define BZ_BUFSIZE 1000000
#define MAXREMAINDER 100

#ifndef YY_TYPEDEF_YY_BUFFER_STATE
#define YY_TYPEDEF_YY_BUFFER_STATE
typedef struct yy_buffer_state *YY_BUFFER_STATE;
#endif

#ifndef YY_TYPEDEF_YY_SIZE_T
#define YY_TYPEDEF_YY_SIZE_T
typedef size_t yy_size_t;
#endif

#ifndef YY_STRUCT_YY_BUFFER_STATE
#define YY_STRUCT_YY_BUFFER_STATE
struct yy_buffer_state
	{
	FILE *yy_input_file;

	char *yy_ch_buf;		/* input buffer */
	char *yy_buf_pos;		/* current position in input buffer */

	/* Size of input buffer in bytes, not including room for EOB
	 * characters.
	 */
	yy_size_t yy_buf_size;

	/* Number of characters read into yy_ch_buf, not including EOB
	 * characters.
	 */
	int yy_n_chars;

	/* Whether we "own" the buffer - i.e., we know we created it,
	 * and can realloc() it to grow it, and should free() it to
	 * delete it.
	 */
	int yy_is_our_buffer;

	/* Whether this is an "interactive" input source; if so, and
	 * if we're using stdio for input, then we want to use getc()
	 * instead of fread(), to make sure we stop fetching input after
	 * each newline.
	 */
	int yy_is_interactive;

	/* Whether we're considered to be at the beginning of a line.
	 * If so, '^' rules will be active on the next match, otherwise
	 * not.
	 */
	int yy_at_bol;

    int yy_bs_lineno; /**< The line count. */
    int yy_bs_column; /**< The column count. */
    
	/* Whether to try to fill the input buffer when we reach the
	 * end of it.
	 */
	int yy_fill_buffer;

	int yy_buffer_status;

#define YY_BUFFER_NEW 0
#define YY_BUFFER_NORMAL 1
  /* When an EOF's been seen but there's still some text to process
   * then we mark the buffer as YY_EOF_PENDING, to indicate that we
   * shouldn't try reading from the input source any more.  We might
   * still have a bunch of tokens to match, though, because of
   * possible backing-up.
   *
   * When we actually see the EOF, we change the status to "new"
   * (via memorytracerestart()), so that the user can continue scanning by
   * just pointing memorytracein at a new input file.
   */
#define YY_BUFFER_EOF_PENDING 2

	};
#endif /* !YY_STRUCT_YY_BUFFER_STATE */


class BZ2ParserState{
  char * filename;
  FILE *f;
  off_t fPosition;
  bool eof;
  BZFILE *bzf;
  void *unused;
  int nUnused;
  int status;
  char buf[BZ_BUFSIZE+2];
  char remainder[MAXREMAINDER];
  yy_size_t rem;
  yy_size_t count;
  int (*lex)(void*);
  YY_BUFFER_STATE (*scan_buffer)(char*, yy_size_t);
  void (*switch_to_buffer)(YY_BUFFER_STATE);
  void (*delete_buffer)(YY_BUFFER_STATE);
  bool activeParser;
  YY_BUFFER_STATE yybuf;
  void *arg;

  int doLexing(int bufLength = 0);
  int parseBlock2();
  void openFile();
  void closeFile();

public:
  BZ2ParserState(const char * _filename
                 , int (*_lex)(void*)
                 , YY_BUFFER_STATE (*_scan_buffer)(char*, yy_size_t)
                 , void (*_switch_to_buffer)(YY_BUFFER_STATE)
                 , void (*_delete_buffer)(YY_BUFFER_STATE)
                 , void *_arg
    );  

  ~BZ2ParserState();

  int parseBlock();

  void parseAll();

  bool isActive();
};

#endif
