/*******************************************
*
*  JKG Challenge algorithm
*
*  Unlike BaseJKA, JKG now uses
*  Diffie-Hellman Key Exchange as challenge
*  method, this to add protection against 
*  fake clients.
*
\*******************************************/

#include <encoding/base128.h>
//#include "libs/dhkeyexchange.h"
#include <encoding/bitstream.h>
#include "gls_enginefuncs.h"

#include <string.h>

#include <openssl/dh.h>
#include <openssl/bn.h>

typedef unsigned char byte;

typedef enum {
	NA_BOT,
	NA_BAD,					// an address lookup failed
	NA_LOOPBACK,
	NA_BROADCAST,
	NA_IP,
	NA_IPX,
	NA_BROADCAST_IPX
} netadrtype_t;

typedef struct {
	netadrtype_t	type;


	byte	ip[4];
	byte	ipx[10];

	unsigned short	port;
} netadr_t;

typedef struct {
	netadr_t	adr;
	int			challenge;
	int			time;				// time the last packet was sent to the autherize server
	int			pingTime;			// time the challenge response was sent to client
	int			firstTime;			// time the adr was first used, for authorize timeout checks
	qboolean	connected;
} challenge_t;

// Diffie-Hellman Key Exchange data
//DHData_t dhdata;

DH *dh;

static void BitStream_ReadBigNum(bitstream_t *stream, BIGNUM *bn)
{
	unsigned char buffer[128];
	unsigned int len;
	len = BitStream_ReadByte(stream);
	if (len > 128) {
		return;
	}
	BitStream_ReadData(stream, buffer, len);

	BN_bin2bn(buffer, len, bn);
}

static void BitStream_WriteBigNum(bitstream_t *stream, BIGNUM *bn)
{
	unsigned char buffer[128];
	int len2 = BN_num_bytes(bn);

	if ( len2 > 128) {
		return;
	}

	memset(buffer, 0, len2);

	BN_bn2bin(bn, buffer);
	BitStream_WriteByte(stream, (unsigned char)len2);

	BitStream_WriteData(stream, buffer, len2);
}

const char *CH_ProcessChallengeRequest(challenge_t *challenge)
{
	static char msg[512];
	unsigned char buff[256];
	char base128buff[348];

	unsigned int ch;
	unsigned int key;
	unsigned char skey[32];
	int i;

	bitstream_t stream;

	//UINT64 g, m, p, p2;
	BIGNUM *p;

	Base128_Decode(Cmd_Argv(1), 0, buff, 256);

	BitStream_Init(&stream, buff, 256);
	BitStream_BeginReading(&stream);
	key = BitStream_ReadUInt(&stream);
	if (key != 0x84AB169F) { // Validate challenge
		// Old style challenge request
		// Send a rigged challenge key
		challenge->challenge = ( (rand() << 16) ^ rand() ) & 0xFFF7FFFF; // Ensure bit 19 is NOT set
		Com_sprintf(msg, sizeof(msg), "challengeResponse %i",challenge->challenge);
		
		return msg;
	}

	if (dh) {
		DH_free(dh);
	} 
	dh = DH_new();

	dh->p = BN_new();
	BN_dec2bn(&dh->g, "5");

	p = BN_new();

	BitStream_ReadBigNum(&stream, dh->p);	// Read prime
	BitStream_ReadBigNum(&stream, p);		// Read public key of client

	if (!DH_generate_key(dh)) {				// Generate public/private key for the server
		challenge->challenge = 0;
		BN_free(p);
		DH_free(dh);
		dh = 0;
		return "print\nInvalid challenge request\n";
	}
	if (!DH_compute_key(skey, p, dh)) {		// Compute the shared key
		challenge->challenge = 0;
		BN_free(p);
		DH_free(dh);
		dh = 0;
		return "print\nInvalid challenge request\n";
	}

	// Secret tag: Bit 19 must always be set
	// The following code ensures this is always the case
	ch = -1;	// Start with all bits set to 1
	for (i=0; i<32; i+=4) {
		ch ^= (*(unsigned int *)&skey[i] << 12 );	// Shift the 19th bit (and all below) to 31st and xor it with the 'challenge'
	}
	ch &= 0x80000000;
	ch >>= 12;	// Shift it back so the highest bit is now the 19th.
	// The 19th bit will be 1 if the amount of 1's on the 19th bit is even, otherwise it'll be 0

	for (i=0; i<32; i+=4) {
		ch ^= *(unsigned int *)&skey[i];
	}

	challenge->challenge = ch;

	//g = BitStream_ReadUInt64(&stream);
	//m = BitStream_ReadUInt64(&stream);
	//p = BitStream_ReadUInt64(&stream);

	//DHKE_Init(&dhdata);

	//p2 = DHKE_GenerateRecipientKeys(&dhdata, g, m);
	//challenge->challenge = (unsigned int)DHKE_CalculateRecipientSharedKey(&dhdata, p);

	BitStream_Init(&stream, buff, 256);
	BitStream_WriteUInt(&stream, 0x18ABD591);
	//BitStream_WriteUInt64(&stream, p2);
	BitStream_WriteBigNum(&stream, dh->pub_key);

	BN_free(p);
	DH_free(dh);
	dh = 0;

	Base128_Encode(stream.data, stream.cursize, base128buff, sizeof(base128buff));

	Com_sprintf(msg, sizeof(msg), "challengeResponse \"%s\"", base128buff);
	return msg;
	
}