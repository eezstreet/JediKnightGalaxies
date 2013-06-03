/*
|*	Base64 encoder/decoder
|*
|*	Copyright (c) 2010 Lourens "BobaFett" Elzinga
|*
\*/

/* 
|* Benchmark results:
|*
|* Optimized Release compile:
|*  Encode speed: 628 MB/s
|*  Decode speed: 733 MB/s
|*
|* Debug compile:
|*  Encode speed: 71 MB/s
|*  Decode speed: 93 MB/s
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

#include "base64.h"

// Encoder characters
static unsigned char *bec64 = (unsigned char *)"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static unsigned char *bec64url = (unsigned char *)"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

// Decoder characters
static unsigned char *bdc64 = (unsigned char *)"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x3e\x00\x00\x00\x3f\x34\x35\x36\x37\x38\x39\x3a\x3b\x3c\x3d\x00\x00\x00\x00\x00\x00\x00\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x00\x00\x00\x00\x00\x00\x1a\x1b\x1c\x1d\x1e\x1f\x20\x21\x22\x23\x24\x25\x26\x27\x28\x29\x2a\x2b\x2c\x2d\x2e\x2f\x30\x31\x32\x33\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
static unsigned char *bdc64url = (unsigned char *)"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x3e\x00\x00\x34\x35\x36\x37\x38\x39\x3a\x3b\x3c\x3d\x00\x00\x00\x00\x00\x00\x00\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x00\x00\x00\x00\x3f\x00\x1a\x1b\x1c\x1d\x1e\x1f\x20\x21\x22\x23\x24\x25\x26\x27\x28\x29\x2a\x2b\x2c\x2d\x2e\x2f\x30\x31\x32\x33\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";

/* Encodes a block of (up to) 3 bytes, encoding them into 4 base64 characters
|*
|* Method:
|*
|* First, the 3 bytes are split up in sets of 6 bits:
|*
|* Binary: 01011100 10110101 11110000
|* Base64: 010111 001011 010111 110000
|*
|* These have a range of 0 to 63, which corresponds with a character in bec64
|*
|*
\*/

static void encodeblock(unsigned char in[3], unsigned char out[4])
{
	out[0] = bec64[ in[0] >> 2 ];
	out[1] = bec64[ ((in[0] & 3) << 4) | ((in[1] & 0xF0) >> 4) ];
	out[2] = bec64[ ((in[1] & 0xF) << 2) | ((in[2] & 0xC0) >> 6) ];
	out[3] = bec64[ (in[2] & 0x3F) ];
}

static void encodeblockurl(unsigned char in[3], unsigned char out[4])
{
	out[0] = bec64url[ in[0] >> 2 ];
	out[1] = bec64url[ ((in[0] & 3) << 4) | ((in[1] & 0xF0) >> 4) ];
	out[2] = bec64url[ ((in[1] & 0xF) << 2) | ((in[2] & 0xC0) >> 6) ];
	out[3] = bec64url[ (in[2] & 0x3F) ];
}


/* Returns the buffer size required to encode data of the specified size, including NULL terminator */
unsigned int Base64_EncodeLength(unsigned int length)
{
	double temp = (double)length;
	temp = ceil(temp/3);
	temp *= 4;
	return (unsigned int)temp + 1;
}

// Encodes binary to base64 
// Returns 1 if successful, 0 if not 
// NOTE: The base64 buffer must be large enough to fit the encoded string
//       use Base64_EncodeLength to determine the required size
int Base64_Encode(const char *data, unsigned int length, char *base64, unsigned int base64len)
{
	unsigned char *r = (unsigned char *)data;
	unsigned char *w = (unsigned char *)base64;

	if (!r || !w || base64len < Base64_EncodeLength(length)) {
		return 0;
	}

	for (;;) {
		// Encode a block
		if (length > 3) {
			encodeblock(r, w);
			r += 3;
			w += 4;
			length -= 3;
		} else {
			// Reached the end, get the remaining bytes and encode them accordingly, then null terminate and bail
			encodeblock(r, w);
			switch (length) {
				case 1:
					w[2] = '=';
				case 2:
					w[3] = '=';
			}
			w += 4;	
			//length = 0;
			break;
		}
	}
	*w = 0;
	return 1;
}

int Base64_EncodeURL(const unsigned char *data, unsigned int length, char *base64, unsigned int base64len)
{
	unsigned char *r = (unsigned char *)data;
	unsigned char *w = (unsigned char *)base64;

	if (!r || !w || base64len < Base64_EncodeLength(length)) {
		return 0;
	}

	for (;;) {
		// Encode a block
		if (length > 3) {
			encodeblockurl(r, w);
			r += 3;
			w += 4;
			length -= 3;
		} else {
			// Reached the end, get the remaining bytes and encode them accordingly, then null terminate and bail
			encodeblockurl(r, w);
			switch (length) {
				case 1:
					w[2] = '.';
				case 2:
					w[3] = '.';
			}
			w += 4;	
			//length = 0;
			break;
		}
	}
	*w = 0;
	return 1;
}

/* Decodes a block of 4 base64 characters back into bytes
|* The values to decode have to be translated first using the bdc64 table
|*
|* Method:
|*
|*
|* Base64: 010111 001011 010111 110000
|* Binary: 01011100 10110101 11110000
|*
|*
\*/

static void decodeblock(unsigned char in[4], unsigned char out[3])
{
	out[0] = bdc64[in[0]] << 2 | bdc64[in[1]] >> 4;
	out[1] = bdc64[in[1]] << 4 | bdc64[in[2]] >> 2;
	out[2] = ((bdc64[in[2]] & 3) << 6) | bdc64[in[3]];
}

static void decodeblockurl(unsigned char in[4], unsigned char out[3])
{
	out[0] = bdc64url[in[0]] << 2 | bdc64url[in[1]] >> 4;
	out[1] = bdc64url[in[1]] << 4 | bdc64url[in[2]] >> 2;
	out[2] = ((bdc64url[in[2]] & 3) << 6) | bdc64url[in[3]];
}

/* Returns the buffer size required to decode the base64 data */
/* You can pass either the length, the actual base64 string, or both */
unsigned int Base64_DecodeLength(unsigned int length, const char *base64, int urlencoded)
{
	double temp;
	if (!length && !base64) {
		return 0;
	}
	if (!length) {
		length = strlen(base64);
	}

	temp = (double)length;
	if (length % 4) {
		return 0;	// Size is not a multiple of 4, bad base64 string
	}
	temp = temp / 4;
	temp *= 3;
	if (base64) {
		if (base64[length-1] == (urlencoded ? '.' : '=')) temp--;
		if (base64[length-2] == (urlencoded ? '.' : '=')) temp--;
	}
	return (unsigned int)temp;	
}

// Decodes base64 to binary
// Returns 1 if successful, 0 if not. If the buffer was not big enough, 2 will be returned.
// NOTE: If the target buffer is not large enough, it will be cut off prematurely!
//       Use Base64_DecodeSize to determine the required size
int Base64_Decode(const char *base64, unsigned int b64len, char *data, unsigned int length)
{
	unsigned char output[3];

	unsigned int base64len = b64len ? b64len : strlen(base64);
	unsigned int maxchars;

	unsigned char *r = (unsigned char *)base64;
	unsigned char *w = (unsigned char *)data;
	
	if (!w || !r) {
		return 0;
	}
	
	// Process padding
	if (base64[base64len-1] == '=') base64len--;
	if (base64[base64len-1] == '=') base64len--;

	// Speed hack: Use the target length to calculate how many bytes we should process from the base64 string
	maxchars = (Base64_EncodeLength(length) - 1);
	if (length % 3) {
		maxchars -= (3 - (length % 3));	// Account for padding
	}
	if (maxchars < base64len) {
		base64len = maxchars;
	}

	while (base64len) {
		// Decode a block
		if (base64len > 3) {
			decodeblock(r, w);
			
			base64len -= 4;
			r += 4;
			w += 3;
		} else {
			// Reached the end, get the remaining bytes and decode them accordingly
			decodeblock(r, output);
			memcpy(w, output, base64len - 1);
			base64len = 0;
		}
	}
	return 1;
}

int Base64_DecodeURL(const char *base64, unsigned int b64len, char *data, unsigned int length)
{
	unsigned char output[3];

	unsigned int base64len = b64len ? b64len : strlen(base64);
	unsigned int maxchars;

	unsigned char *r = (unsigned char *)base64;
	unsigned char *w = (unsigned char *)data;
	
	if (!w || !r) {
		return 0;
	}
	
	// Process padding
	if (base64[base64len-1] == '.') base64len--;
	if (base64[base64len-1] == '.') base64len--;

	// Speed hack: Use the target length to calculate how many bytes we should process from the base64 string
	maxchars = (Base64_EncodeLength(length) - 1);
	if (length % 3) {
		maxchars -= (3 - (length % 3));	// Account for padding
	}
	if (maxchars < base64len) {
		base64len = maxchars;
	}

	while (base64len) {
		// Decode a block
		if (base64len > 3) {
			decodeblockurl(r, w);
			
			base64len -= 4;
			r += 4;
			w += 3;
		} else {
			// Reached the end, get the remaining bytes and decode them accordingly
			decodeblockurl(r, output);
			memcpy(w, output, base64len - 1);
			base64len = 0;
		}
	}
	return 1;
}