
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

#include "BZ2ParserState.h"
#include <cstdlib>
#include <cstring>
#include <iostream>
using namespace std;

BZ2ParserState::BZ2ParserState(const char * _filename
                             , int (*_lex)(void*)
                             , YY_BUFFER_STATE (*_scan_buffer)(char*, yy_size_t)
                             , void (*_switch_to_buffer)(YY_BUFFER_STATE)
                             , void (*_delete_buffer)(YY_BUFFER_STATE)
                             , void *_arg
  ) : f(NULL)
    , fPosition(0)
    , eof(false)
    , bzf(NULL)
    , unused(malloc(BZ_MAX_UNUSED))
    , nUnused(0)
    , status(BZ_OK)
    , rem(0)
    , count(0)
    , lex(_lex)
    , scan_buffer(_scan_buffer)
    , switch_to_buffer(_switch_to_buffer)
    , delete_buffer(_delete_buffer)
    , activeParser(false)
    , arg(_arg)
  {
    filename = (char *)malloc(strlen(_filename)+1);
    strcpy(filename, _filename);
  }

BZ2ParserState::~BZ2ParserState(){
  free(filename);
  free(unused);
}

int BZ2ParserState::doLexing(int bufLength){
  if(activeParser)
    switch_to_buffer (yybuf);
  else
    yybuf = scan_buffer (buf, bufLength);
  int lexres = lex(arg);
  if(lexres){
    activeParser = true;
    return 1;
  }
  else{
    activeParser = false;
    delete_buffer(yybuf);
    if((eof || feof(f)) && nUnused == 0 && bzf == NULL)
      return 0;
    else
      return 1;
  }
}

void BZ2ParserState::openFile(){
  f = fopen(filename, "r");
  if(!f){
    cerr << "Error opening file: " << filename << endl;
    perror(NULL);
    abort();
  }
  fseeko(f, fPosition, SEEK_SET);
}

void BZ2ParserState::closeFile(){
  if(feof(f))
    eof = true;
  else
    fPosition = ftello(f);
  fclose(f);
  f = NULL;
}

int BZ2ParserState::parseBlock2(){
  if(activeParser){
    return doLexing();
  }
  else if(!(eof && nUnused == 0 && bzf == NULL)){
    /* copy any remaining chars into buf */
    memcpy(buf, remainder, rem);
    /* If there is no stream open, open a new stream */
    if(bzf == NULL){
      bzf = BZ2_bzReadOpen(&status, f, 0, 0, unused, nUnused);
      if(status != BZ_OK){
        cout << "Error opening for decompression: " << filename << " ( error:" << status << " ) " << endl;
        abort();
      }
      if(*(FILE**)bzf != f){
        cout << "bzf format not as expected\n";
        abort();
      }
    }
    /* This is a horrible hack due to the fact that f must be closed and reopened while 
     * the compression stream remains open. There is no way in the bzlib API to bind the reopened
     * file stream to the open compression stream so I do so manually here. Hopefully the format
     * of BZFILE (currently has the file handle as first field) will not change.
    */
    (*(FILE**)bzf) = f;
    /* Fill the rest of buf with new data */
    count = BZ2_bzRead(&status, bzf, buf + rem, BZ_BUFSIZE - rem) + rem;
    /* Check if we reached the end of this stream */
    if(status != BZ_OK){
      if(status != BZ_STREAM_END){ abort(); }
      void *unusedTemp;
      BZ2_bzReadGetUnused(&status, bzf, &unusedTemp, &nUnused );
      memcpy(unused, unusedTemp, nUnused);
      if(status != BZ_OK){ abort(); }
      BZ2_bzReadClose(&status, bzf);
      if(status != BZ_OK){ abort(); }
      bzf = NULL;
      if(count == 0) //empty stream
        return 1;
    }
    /* Find a token to split on near the back and copy the remainder */
    rem = 1;
    while(buf[count - rem] != ' ' && buf[count-rem] != ')' && buf[count-rem] != ',' && buf[count-rem] != '\n'){
      rem++;
      if(rem > MAXREMAINDER || rem > count){
        cerr << "Could not find token to split on\n";
        exit(-1);
      }
    }
    memcpy(remainder, buf + count - rem, rem);
    /* Write NULLS to the end of the buffer as required by flex */
    buf[count-rem] = buf[count-rem+1] = 0;
    /* Set up the flex buffer and parse */
    return doLexing(count-rem+2);
  }
  else{
    /* parse anything that's left over */
    memcpy(buf, remainder, rem);
    buf[rem] = buf[rem+1] = 0;
    return doLexing(rem+2);
  }
}

int BZ2ParserState::parseBlock(){
  openFile();
  int res = parseBlock2();
  closeFile();
  return res;
}

void BZ2ParserState::parseAll(){
  while(parseBlock());
}

bool BZ2ParserState::isActive(){
  return activeParser;
}
