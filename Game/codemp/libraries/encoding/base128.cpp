/******************************************\
*
*  Base-128 Encoding Algorithm
*
*  Encodes data into base-128
*
*  The encoder will not use characters with special meaning (such as % \ and ") or any non-printable or whitespace characters
*
*  When decoding, invalid characters will be treated as if they represent the value 0
*
*  Copyright (c) 2010 Lourens "BobaFett" Elzinga
*
\******************************************/

/* 
|* Benchmark results:
|*
|* Optimized Release compile:
|*  Encode speed: 612 MB/s
|*  Decode speed: 606 MB/s
|*
|* Debug compile:
|*  Encode speed: 128 MB/s
|*  Decode speed: 136 MB/s
|*
\*/

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <float.h>
#include <limits.h>
#include <ctype.h>
#include <stdarg.h>

#include "base128.h"

// Encoder characters
                                               //---------|---------|---------|---------|---------|---------|---------|---------|---------|---------|---------|---------|-------+
static unsigned char *b128ec = (unsigned char *)"!#$^&'()*+,-.0123456789:<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ_`abcdefghjiklmnopqrstuvwxyz[]{|}~ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖØÙÚÛÜÝÞàáâãäåæéè";

// Decoder table
//
// Conversion formula: val =  b128dc[in];
//
static unsigned char *b128dc = (unsigned char *) "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x02\x00\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x00\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x00\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f\x20\x21\x22\x23\x24\x25\x26\x27\x28\x29\x2a\x2b\x2c\x2d\x2e\x2f\x30\x31\x32\x33\x34\x35\x36\x53\x00\x54\x03\x37\x38\x39\x3a\x3b\x3c\x3d\x3e\x3f\x40\x42\x41\x43\x44\x45\x46\x47\x48\x49\x4a\x4b\x4c\x4d\x4e\x4f\x50\x51\x52\x55\x56\x57\x58\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x59\x5a\x5b\x5c\x5d\x5e\x5f\x60\x61\x62\x63\x64\x65\x66\x67\x68\x69\x6a\x6b\x6c\x6d\x6e\x6f\x00\x70\x71\x72\x73\x74\x75\x76\x00\x77\x78\x79\x7a\x7b\x7c\x7d\x00\x7f\x7e\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";


/* Encodes a block of (up to) 7 bytes, encoding them into (up to) 8 base128 characters
|*
|* Method:
|*
|* First, the 7 bytes are split up in sets of 7 bits:
|*
|* Binary:  01011100 10110101 11110000 10110101 10110111 11110000 10101010
|* Base128: 0101110 0|101101 01|11110 000|1011 0101|101 10111|11 110000|1 0101010  (| chars represent the locations where the next byte starts)
|*
|* These values correspond to an entry in b128ec, which is the symbol representing that value
|*
|* The encoder will always output 8 bytes, regardless of the length of the input
|* If less than 7 bytes have been encoded:
|* The amount of bytes generated is 1 + sourceLen, so when encoding 3 bytes, only use the first 4 bytes of output, instead of all 8.
|*
\*/

static void encodeblock(unsigned char in[7], unsigned char out[8])
{
	out[0] = b128ec[ in[0] >> 1 ];
	out[1] = b128ec[ ( (in[0] & 0x01) << 6 ) | ( in[1] >> 2 ) ];
	out[2] = b128ec[ ( (in[1] & 0x03) << 5 ) | ( in[2] >> 3 ) ];
	out[3] = b128ec[ ( (in[2] & 0x07) << 4 ) | ( in[3] >> 4 ) ];
	out[4] = b128ec[ ( (in[3] & 0x0F) << 3 ) | ( in[4] >> 5 ) ];
	out[5] = b128ec[ ( (in[4] & 0x1F) << 2 ) | ( in[5] >> 6 ) ];
	out[6] = b128ec[ ( (in[5] & 0x3F) << 1 ) | ( in[6] >> 7 ) ];
	out[7] = b128ec[ in[6] & 0x7F ];
}


/* Decodes a block of (up to) 8 base128 characters, encoding them into (up to) 7 bytes
|*
|* Method:
|*
|* Base128: 0101110 0|101101 01|11110 000|1011 0101|101 10111|11 110000|1 101010  (| chars represent the locations where the next byte starts)
|* Binary:  01011100 10110101 11110000 10110101 10110111 11110000 10101010
|*
|*
|* The decoder will always output 7 bytes, regardless of the length of the input
|* If less than 8 characters have been provided:
|* The amount of bytes generated is 1 - sourceLen, so when decoding 3 characters, only use the first 2 bytes of output, instead of all 7.
|*
\*/

static void decodeblock(unsigned char in[8], unsigned char out[7])
{
	out[0] = (b128dc[in[0]] << 1) | (b128dc[in[1]] >> 6);
	out[1] = (b128dc[in[1]] << 2) | (b128dc[in[2]] >> 5);
	out[2] = (b128dc[in[2]] << 3) | (b128dc[in[3]] >> 4);
	out[3] = (b128dc[in[3]] << 4) | (b128dc[in[4]] >> 3);
	out[4] = (b128dc[in[4]] << 5) | (b128dc[in[5]] >> 2);
	out[5] = (b128dc[in[5]] << 6) | (b128dc[in[6]] >> 1);
	out[6] = (b128dc[in[6]] << 7) | (b128dc[in[7]]);
}

/* Returns the size of the buffer required to encode a binary stream with the given size (including NULL terminator) */
unsigned int Base128_EncodeLength(unsigned int length)
{
	return 1 + ((length / 7) * 8) + ((length % 7) ? (length % 7) + 1 : 0);
}

/* Returns the size of the buffer required to decode a base-128 string with the given length */
unsigned int Base128_DecodeLength(unsigned int length)
{
	return ( (length >> 3) * 7 ) + ((length & 7) ? (length & 7) - 1 : 0);
}

/* Base-128 encodes the data */
/* Returns 1 if successful, 0 otherwise */
int Base128_Encode(const void *data, unsigned int length, char *base128, unsigned int base128len)
{
	unsigned char input[7];
	unsigned char output[8];

	unsigned char *r = (unsigned char *)data;
	unsigned char *w = (unsigned char *)base128;

	if (!r || !w || base128len < Base128_EncodeLength(length)) {
		return 0;
	}

	while (length) {
		// Encode a block
		if (length >= 7) {
			encodeblock(r, w);	
			r += 7;
			w += 8;
			length -= 7;
		} else {
			// Reached the end, get the remaining bytes and encode them accordingly, then null terminate and bail
			memcpy(input, (void *)r, length);
			encodeblock(input, output);
			memcpy(w, output, length + 1);
			w += (length + 1);	
			length = 0;
		}
	}
	*w = 0;
	return 1;
}

/* Base-128 decodes the data */
/* Returns 1 if successful, 0 otherwise */
int Base128_Decode(const char *base128, unsigned int b128len, void *data, unsigned int length )
{
	unsigned char input[8];
	unsigned char output[7];

	unsigned int base128len = b128len ? b128len : strlen(base128);
	unsigned int maxchars;

	unsigned char *r = (unsigned char *)base128;
	unsigned char *w = (unsigned char *)data;
	
	if (!w || !r) {
		return 0;
	}

	// Speed hack: Use the target length to calculate how many bytes we should process from the base128 string
	maxchars = Base128_EncodeLength(length) - 1;
	if (maxchars < base128len) {
		base128len = maxchars;
	}

	while (base128len) {
		// Decode a block
		if (base128len > 7) {
			decodeblock(r, w);
			
			base128len -= 8;
			r += 8;
			w += 7;
		} else {
			// Reached the end, get the remaining bytes and decode them accordingly
			memcpy(input, (void *)r, base128len);
			decodeblock(input, output);
			memcpy(w, output, base128len - 1);
			base128len = 0;
		}
	}
	return 1;
}