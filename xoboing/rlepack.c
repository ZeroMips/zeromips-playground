/*
 *The RLE Toolkit is released under a slightly modified BSD license:
 *
 *Copyright (c) 2004, Per Olofsson
 *All rights reserved.
 *
 *Redistribution and use in source and binary forms, with or without
 *modification, are permitted provided that the following conditions are
 *met:
 *
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *
 *THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>


// global variables for the input stream
static unsigned char inbuf[1024];
static unsigned char *inptr = inbuf;
static unsigned int inlen = 0;
static unsigned int incount = 0;
static FILE *infile;

// global variables for the output stream
static unsigned char outbuf[1024];
static unsigned char *outptr = outbuf;
static unsigned int outlen = 0;
static unsigned int outcount = 0;
static FILE *outfile;


// read a byte from the input stream
signed int readdata(void) {
  if (inlen == 0) {
    inlen = fread(inbuf, 1, sizeof(inbuf), infile);
    inptr = inbuf;
  }
  if (inlen) {
    ++incount;
    --inlen;
    return(*inptr++);
  } else {
    return(EOF);
  }
}


// flush the output buffer to disk
unsigned int flushdata(void) {
  unsigned int l;

  if (outlen) {
    l = fwrite(outbuf, 1, outlen, outfile);
    outlen = 0;
    outptr = outbuf;
    return(l);
  } else {
    return(0);
  }
}


// write a byte to the output stream
unsigned int writedata(unsigned char byte) {
  ++outcount;
  if (outlen >= sizeof(outbuf)) {
    if (flushdata() == 0) {
      return(0);
    }
  }
  *outptr++ = byte;
  ++outlen;
  return(1);
}


void usage(void) {
  puts("Usage: rlepack infile outfile\r\n");
}


int main(int argc, char **argv) {
  signed int c;
  signed int lastbyte;
  int count;
  char *inname, *outname;

  if (argc != 3) {
    usage();
    return(1);
  }

  inname = argv[1];
  outname = argv[2];

  if ((infile = fopen(inname, "rb")) == NULL) {
    printf("%s: ", inname);
    perror("couldn't open for reading");
    return(1);
  }

  if ((outfile = fopen(outname, "wb")) == NULL) {
    fclose(infile);
    printf("%s: ", inname);
    perror("couldn't open for writing");
    return(1);
  }

  if ((c = readdata()) < 0) {
    puts("No input");
    goto error;
  }
  lastbyte = c;
  if (writedata(c) == 0) {
    perror("Write error");
    goto error;
  }
  while ((c = readdata()) >= 0) {
    if (c == lastbyte) {
      count = 1;
      while ((c = readdata()) == lastbyte) {
	if (++count > 255) {
	  writedata(lastbyte);
	  writedata(255);
	  count = 1;
	}
      }
      writedata(lastbyte);
      writedata(count);
      if (c < 0) {
	break;
      }
    }
    lastbyte = c;
    if (writedata(c) == 0) {
      perror("Write error");
      goto error;
    }
  }
  writedata(lastbyte);
  writedata(0);

  if (outlen) {
    if (flushdata() == 0) {
      perror("Write error");
      goto error;
    }
  }

  printf("Input %d bytes, output %d bytes, ratio %d%%\r\n", incount, outcount, ((100 * outcount) / incount));

  fclose(infile);
  fclose(outfile);
  return(0);

 error:
  fclose(infile);
  fclose(outfile);
  return(2);
}
