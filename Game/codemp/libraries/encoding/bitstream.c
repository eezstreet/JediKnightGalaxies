/******************************************\
*
*  Bitstream implementation
*
*  Copyright (c) 2010 Lourens "BobaFett" Elzinga
*
\******************************************/

#include "bitstream.h"

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <float.h>
#include <limits.h>
#include <ctype.h>
#include <stdarg.h>

int bitmasks[9] = {0x0, 0x1, 0x3, 0x7, 0xF, 0x1F, 0x3F, 0x7F, 0xFF};

void BitStream_Init(bitstream_t *stream, unsigned char *buffer, unsigned int size)
{
	stream->data = buffer;
	stream->maxsize = size;
	stream->cursize = 0;
	stream->readcount = 0;
	stream->bit = 0;
}

void BitStream_BeginReading(bitstream_t *stream)
{
	stream->readcount = 0;
	stream->bit = 0;
}

void BitStream_WriteBits(bitstream_t *stream, int bits, unsigned int value)
{
	int BitsToWrite;
	// Check for invalid bit counts
	if (!bits || bits < -32 || bits > 32) {
		return;
	}

	if (bits < 0) bits = -bits;

	if ((stream->bit + bits) >> 3 >= stream->maxsize) {
		return;
	}

	value &= (0xffffffff>>(32-bits)); // 0 out irrelevant bits

	// Step 1: Try to fill up the current byte (if possible)
	BitsToWrite = 8 - (stream->bit & 7);
	if (BitsToWrite & 7 && bits >= (BitsToWrite & 7)) {
		stream->data[ stream->bit >> 3 ] |= (value & (0xffffffff>>(32-BitsToWrite)));
		bits -= BitsToWrite;
		stream->bit += BitsToWrite;
		value >>= BitsToWrite;
	}

	// Step 2: Copy alligned bytes
	switch (bits & ~7)
	{
		case 8:
			memcpy(&stream->data[stream->bit >> 3], &value, 1);
			bits -= 8;
			stream->bit += 8;
			value >>= 8;
			break;
		case 16:
			memcpy(&stream->data[stream->bit >> 3], &value, 2);
			bits -= 16;
			stream->bit += 16;
			value >>= 16;
			break;
		case 24:
			memcpy(&stream->data[stream->bit >> 3], &value, 3);
			bits -= 24;
			stream->bit += 24;
			value >>= 24;
			break;
		case 32:
			memcpy(&stream->data[stream->bit >> 3], &value, 4);
			bits -= 32;
			stream->bit += 32;
			value = 0;
			break;
		default:
			break;
	}

	// Step 3: Check for any remaining bits and put them in place
	if (bits) {
		if (!(stream->bit & 7)) {
			// New byte, clear it
			stream->data[ stream->bit >> 3 ] = 0;
		}
		stream->data[ stream->bit >> 3 ] |= (value << (8 - (stream->bit & 7) - bits) );
		stream->bit += bits;
	}
	stream->cursize = (stream->bit >> 3) + ((stream->bit & 7) ? 1 : 0);
}

void BitStream_WriteBool(bitstream_t *stream, int val)
{
	BitStream_WriteBits(stream, 1, !!val);
}

void BitStream_WriteNibble(bitstream_t *stream, char val)
{
	BitStream_WriteBits(stream, -4, val);
}

void BitStream_WriteUNibble(bitstream_t *stream, unsigned char val)
{
	BitStream_WriteBits(stream, 4, val);
}

void BitStream_WriteChar(bitstream_t *stream, char val)
{
	BitStream_WriteBits(stream, -8, val);
}

void BitStream_WriteByte(bitstream_t *stream, unsigned char val)
{
	BitStream_WriteBits(stream, 8, val);
}

void BitStream_WriteShort(bitstream_t *stream, short val)
{
	BitStream_WriteBits(stream, -16, val);
}

void BitStream_WriteUShort(bitstream_t *stream, unsigned short val)
{
	BitStream_WriteBits(stream, 16, val);
}

void BitStream_WriteTriByte(bitstream_t *stream, int val)
{
	BitStream_WriteBits(stream, -24, val);
}

void BitStream_WriteUTriByte(bitstream_t *stream, unsigned int val)
{
	BitStream_WriteBits(stream, 24, val);
}

void BitStream_WriteInt(bitstream_t *stream, int val)
{
	BitStream_WriteBits(stream, -32, val);
}

void BitStream_WriteUInt(bitstream_t *stream, unsigned int val)
{
	BitStream_WriteBits(stream, 32, val);
}

void BitStream_WriteInt64(bitstream_t *stream, INT64 val)
{
	union {
		INT64 i64;
		unsigned int i[2];
	} t;

	t.i64 = val;
	BitStream_WriteBits(stream, 32, t.i[0]);
	BitStream_WriteBits(stream, 32, t.i[1]);
}

void BitStream_WriteUInt64(bitstream_t *stream, UINT64 val)
{
	union {
		UINT64 i64;
		unsigned int i[2];
	} t;

	t.i64 = val;
	BitStream_WriteBits(stream, 32, t.i[0]);
	BitStream_WriteBits(stream, 32, t.i[1]);
}

void BitStream_WriteFloat(bitstream_t *stream, float val)
{
	BitStream_WriteBits(stream, 32, *(unsigned int *)&val);
}

void BitStream_WriteDouble(bitstream_t *stream, double val)
{
	union {
		double d;
		unsigned int i[2];
	} t;

	t.d = val;
	BitStream_WriteBits(stream, 32, t.i[0]);
	BitStream_WriteBits(stream, 32, t.i[1]);
}

void BitStream_WriteString(bitstream_t *stream, const char *str)
{
	const char *s = str;
	while (*s) {
		BitStream_WriteBits(stream, 8, *s++);
	}
	BitStream_WriteBits(stream, 8, 0);
}

void BitStream_WriteData(bitstream_t *stream, const unsigned char *data, unsigned int len)
{
	const unsigned char *s = data;

	while (len >= 4) {
		BitStream_WriteUInt(stream, *(unsigned int *)s);
		s += 4;
		len -= 4;
	}
	switch (len)
	{
		case 3:
			BitStream_WriteByte(stream, *s++);
		case 2:
			BitStream_WriteByte(stream, *s++);
		case 1:
			BitStream_WriteByte(stream, *s++);
		default:
			break;
	}

}

unsigned int BitStream_ReadBits(bitstream_t *stream, int bits)
{
	int BitsToWrite;
	int sgn = 0;
	int tbits;
	unsigned int val = 0;
	unsigned int tmp;

	unsigned int sh = 0;

	// Check for invalid bit counts
	if (!bits || bits < -32 || bits > 32) {
		return 0;
	}

	if (bits < 0) bits = -bits, sgn = 1;

	tbits = bits;
	if ((stream->bit + bits) >> 3 >= stream->maxsize) {
		return 0;
	}


	// Step 1: Determine if there's data in the current byte
	BitsToWrite = 8 - (stream->bit & 7);
	if (BitsToWrite & 7 && bits >= (BitsToWrite & 7)) {
		val = stream->data[ stream->bit >> 3 ] & (0xffffffff>>(32-BitsToWrite));
		bits -= BitsToWrite;
		stream->bit += BitsToWrite;
		sh += BitsToWrite;
	}

	// Step 2: Copy alligned bytes
	switch (bits & ~7)
	{
		case 8:
			tmp = 0;
			memcpy(&tmp, &stream->data[stream->bit >> 3], 1);
			bits -= 8;
			stream->bit += 8;
			val |= tmp << sh;
			sh += 8;
			break;
		case 16:
			tmp = 0;
			memcpy(&tmp, &stream->data[stream->bit >> 3], 2);
			bits -= 16;
			stream->bit += 16;
			val |= tmp << sh;
			sh += 16;
			break;
		case 24:
			tmp = 0;
			memcpy(&tmp, &stream->data[stream->bit >> 3], 3);
			bits -= 24;
			stream->bit += 24;
			val |= tmp << sh;
			sh += 24;
			break;
		case 32:
			tmp = 0;
			memcpy(&tmp, &stream->data[stream->bit >> 3], 4);
			bits -= 32;
			stream->bit += 32;
			val |= tmp << sh;
			sh += 32;
			break;
		default:
			break;
	}

	// Step 3: Check for any remaining bits and put them in place
	if (bits) {
		int shift = (8 - (stream->bit & 7) - bits);
		int mask = bitmasks[bits] << shift;
		val |= ((stream->data[ stream->bit >> 3 ] & mask) >> shift) << sh;
		stream->bit += bits;
	}
	stream->readcount = (stream->bit >> 3) + ((stream->bit & 7) ? 1 : 0);

	if ( sgn ) {
		if ( val & ( 1 << ( tbits - 1 ) ) ) {
			val |= -1 ^ ( ( 1 << tbits ) - 1 );
		}
	}
	return val;
}

int BitStream_ReadBool(bitstream_t *stream)
{
	return BitStream_ReadBits(stream, 1);
}

char BitStream_ReadNibble(bitstream_t *stream)
{
	return (char)BitStream_ReadBits(stream, -4);
}

unsigned char BitStream_ReadUNibble(bitstream_t *stream)
{
	return (unsigned char)BitStream_ReadBits(stream, 4);
}

char BitStream_ReadChar(bitstream_t *stream)
{
	return (char)BitStream_ReadBits(stream, -8);
}

unsigned char BitStream_ReadByte(bitstream_t *stream)
{
	return (unsigned char)BitStream_ReadBits(stream, 8);
}

short BitStream_ReadShort(bitstream_t *stream)
{
	return (short)BitStream_ReadBits(stream, -16);
}

unsigned short BitStream_ReadUShort(bitstream_t *stream)
{
	return (unsigned short)BitStream_ReadBits(stream, 16);
}

int BitStream_ReadTriByte(bitstream_t *stream)
{
	return BitStream_ReadBits(stream, -24);
}

unsigned int BitStream_ReadUTriByte(bitstream_t *stream)
{
	return BitStream_ReadBits(stream, 24);
}

int BitStream_ReadInt(bitstream_t *stream)
{
	return BitStream_ReadBits(stream, -32);
}

unsigned int BitStream_ReadUInt(bitstream_t *stream)
{
	return BitStream_ReadBits(stream, 32);
}

INT64 BitStream_ReadInt64(bitstream_t *stream)
{
	union {
		INT64 i64;
		unsigned int i[2];
	} t;
	
	t.i[0] = BitStream_ReadBits(stream, 32);
	t.i[1] = BitStream_ReadBits(stream, 32);
	return t.i64;
}


UINT64 BitStream_ReadUInt64(bitstream_t *stream)
{
	union {
		UINT64 i64;
		unsigned int i[2];
	} t;
	
	t.i[0] = BitStream_ReadBits(stream, 32);
	t.i[1] = BitStream_ReadBits(stream, 32);
	return t.i64;
}

float BitStream_ReadFloat(bitstream_t *stream)
{
	unsigned int tmp = BitStream_ReadBits(stream, 32);
	return *(float *)&tmp;
}

double BitStream_ReadDouble(bitstream_t *stream)
{
	union {
		double d;
		unsigned int i[2];
	} t;
	t.i[0] = BitStream_ReadBits(stream, 32);
	t.i[1] = BitStream_ReadBits(stream, 32);
	return t.d;
}

int BitStream_ReadString(bitstream_t *stream, char *str, unsigned int len)
{
	char *s = str;
	char ch;
	int l = 0;
	for (;;) {
		ch = BitStream_ReadChar(stream);
		if (!ch) {
			break;
		}
		if (len > 1) {	// Always read the entire string, even if the buffer is not big enough, as we'll get alignment errors if we dont
			*s++ = ch;
			l++;
			len--;
		}
	}
	*s = 0;
	return l;
}

const char* BitStream_ReadStringBuffered(bitstream_t *stream)
{
	static char buffer[4][32768];
	static int index = 0;
	
	index += 1;
	index &= 3;

	BitStream_ReadString(stream, buffer[index], 32768);

	return buffer[index];
}

void BitStream_ReadData(bitstream_t *stream, unsigned char *data, unsigned int len)
{
	unsigned char *s = data;

	while (len >= 4) {
		*(unsigned int *)s = BitStream_ReadUInt(stream);
		s += 4;
		len -= 4;
	}
	switch (len)
	{
		case 3:
			*s++ = BitStream_ReadByte(stream);
		case 2:
			*s++ = BitStream_ReadByte(stream);
		case 1:
			*s++ = BitStream_ReadByte(stream);
		default:
			break;
	}
}
