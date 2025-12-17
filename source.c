#include <errno.h>
#include "shell.h"
#include "source.h"

void unget_char(struct source_s *src) {
    /* 
   * Undo last next_char() call - move position back one step
   * Allows parser to backtrack when it reads too far
   */
  if (src->curpos < 0) {
    return;
  }

  src->curpos--;
}

char next_char(struct source_s *src) {
    if(!src || !src->buffer) {
        errno = EINVAL;
        return ERRCHAR;
    }
    
    /* Handle initial position */
    if(src->curpos < 0) {
        src->curpos = 0;  /* Start at first character */
    } else {
        src->curpos++;    /* Move to next character */
    }
    
    /* Check if at end */
    if(src->curpos >= src->bufsize) {
        src->curpos = src->bufsize;  /* Lock at end */
        return EOF;
    }
    
    /* Return character at CURRENT position */
    return src->buffer[src->curpos];
}

char peek_char(struct source_s *src)
{
    /* Purpose: Look at next character without consuming it */
    
    /* 1. Safety check - ensure source exists */
    if(!src || !src->buffer)
    {
        errno = ENODATA;
        return ERRCHAR;
    }

    /* 2. Make copy of current position (don't modify original) */
    long pos = src->curpos;

    /* 3. Handle initial position (-2) */
    if(pos == INIT_SRC_POS)
    {
        pos++;  /* Move from -2 to -1 */
    }
    pos++;  /* Move to position we want to peek at */

    /* 4. Check if peek position is past end */
    if(pos >= src->bufsize)
    {
        return EOF;  /* Nothing to peek at */
    }

    /* 5. Return character at peek position WITHOUT changing src->curpos */
    return src->buffer[pos];
}

void skip_white_spaces(struct source_s *src)
{
    char c;

    if(!src || !src->buffer)
    {
        return;
    }

    while(((c = peek_char(src)) != EOF) && (c == ' ' || c == '\t'))
    {
        next_char(src);
    }
}
