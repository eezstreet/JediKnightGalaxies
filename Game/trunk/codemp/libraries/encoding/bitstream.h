/******************************************\
*
*  Bitstream implementation
*
*  Copyright (c) 2010 Lourens "BobaFett" Elzinga
*
\******************************************/

#ifndef bitstream__h
#define bitstream__h

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef _WIN32
	#ifndef INT64
		#define INT64 __int64
	#endif
	#ifndef UINT64
		#define UINT64 unsigned __int64
	#endif
#else
	#ifndef INT64
		#define INT64 long long
	#endif
	#ifndef UINT64
		#define UINT64 unsigned long long
	#endif
#endif

typedef struct {
	unsigned char	*data;
	unsigned int	maxsize;
	unsigned int	cursize;
	unsigned int	readcount;
	unsigned int	bit;
} bitstream_t;

// Initializes a bitstream and links it to a buffer
void BitStream_Init(bitstream_t *stream, unsigned char *buffer, unsigned int size);

/* Write functions */

/* Write a certain amount of bits to the stream (max of 32), use a negative value for bits to denote signed values */
void BitStream_WriteBits(bitstream_t *stream, int bits, unsigned int value);

/* Wrapper functions */
void BitStream_WriteBool(bitstream_t *stream, int val);					// Bool - 1 bit
void BitStream_WriteNibble(bitstream_t *stream, char val);				// Nibble - 4 bits
void BitStream_WriteUNibble(bitstream_t *stream, unsigned char val);
void BitStream_WriteChar(bitstream_t *stream, char val);				// Char - 8 bits
void BitStream_WriteByte(bitstream_t *stream, unsigned char val);		// Byte - 8 bits
void BitStream_WriteShort(bitstream_t *stream, short val);				// Short - 16 bits
void BitStream_WriteUShort(bitstream_t *stream, unsigned short val);
void BitStream_WriteTriByte(bitstream_t *stream, int val);				// TriByte - 24 bits
void BitStream_WriteUTriByte(bitstream_t *stream, unsigned int val);
void BitStream_WriteInt(bitstream_t *stream, int val);					// Int - 32 bits
void BitStream_WriteUInt(bitstream_t *stream, unsigned int val);
void BitStream_WriteInt64(bitstream_t *stream, INT64 val);				// Int64 - 64 bits
void BitStream_WriteUInt64(bitstream_t *stream, UINT64 val);
void BitStream_WriteFloat(bitstream_t *stream, float val);				// Float - 32 bits
void BitStream_WriteDouble(bitstream_t *stream, double val);			// Double - 64 bits
void BitStream_WriteString(bitstream_t *stream, const char *str);		// String - (8 * length) bits
void BitStream_WriteData(bitstream_t *stream, const unsigned char *data, unsigned int len); // Data - (8 * length) bits


// Begin reading (use this before using any read functions)
void BitStream_BeginReading(bitstream_t *stream);

unsigned int BitStream_ReadBits(bitstream_t *stream, int bits);

int				BitStream_ReadBool(bitstream_t *stream);
char			BitStream_ReadNibble(bitstream_t *stream);
unsigned char	BitStream_ReadUNibble(bitstream_t *stream);
char			BitStream_ReadChar(bitstream_t *stream);
unsigned char	BitStream_ReadByte(bitstream_t *stream);
short			BitStream_ReadShort(bitstream_t *stream);
unsigned short	BitStream_ReadUShort(bitstream_t *stream);
int				BitStream_ReadTriByte(bitstream_t *stream);
unsigned int	BitStream_ReadUTriByte(bitstream_t *stream);
int				BitStream_ReadInt(bitstream_t *stream);
unsigned int	BitStream_ReadUInt(bitstream_t *stream);
INT64			BitStream_ReadInt64(bitstream_t *stream);
UINT64			BitStream_ReadUInt64(bitstream_t *stream);
float			BitStream_ReadFloat(bitstream_t *stream);
double			BitStream_ReadDouble(bitstream_t *stream);
int				BitStream_ReadString(bitstream_t *stream, char *str, unsigned int len);  // Returns chars read
const char*		BitStream_ReadStringBuffered(bitstream_t *stream);						 // Uses 4 32kb buffers
void			BitStream_ReadData(bitstream_t *stream, unsigned char *data, unsigned int len);

#ifdef __cplusplus
}
#endif

#endif