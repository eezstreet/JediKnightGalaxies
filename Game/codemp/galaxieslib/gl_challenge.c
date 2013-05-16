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

#include <assert.h>
#include <encoding/base128.h>
//#include "libs/dhkeyexchange.h"
#include <encoding/bitstream.h>
#include "gl_enginefuncs.h"

#include <string.h>

#include <openssl/dh.h>
#include <openssl/bn.h>

// Diffie-Hellman Key Exchange data
//DHData_t dhdata;
DH *dh;

void DH_getprime(DH *dh, int seed);

/* Called when starting a connection to a server */
void CH_InitConnection()
{
	// If we have a DH object, free it
	if (dh) {
		DH_free(dh);
	}
	dh = DH_new();	// Create a new DH object

	// Generate a 256-bit prime with 5 as generator
	//DH_generate_parameters_ex(dh, 256, 5, NULL);
	DH_getprime(dh, Sys_Milliseconds());

	// Generate public and private key
	DH_generate_key(dh);

	/*DHKE_Init(&dhdata);
	DHKE_GeneratePrimes(&dhdata, 0, 0);
	DHKE_GenerateSenderKeys(&dhdata);*/
}

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

const char *CH_DoChallenge()
{
	// Send a challenge to the server
	static char msg[512];
	unsigned char buff[256];
	char base128buff[348];

	bitstream_t stream;

	if (!dh) {
		// SHOULD NEVER HAPPEN!
		CH_InitConnection();
	}

	BitStream_Init(&stream, buff, 256);

	BitStream_WriteUInt(&stream, 0x84AB169F); // Verification code
	//BitStream_WriteUInt64(&stream, dhdata.Generator);
	//BitStream_WriteUInt64(&stream, dhdata.Modulus);
	//BitStream_WriteUInt64(&stream, dhdata.PublicA);
	BitStream_WriteBigNum(&stream, dh->p);
	BitStream_WriteBigNum(&stream, dh->pub_key);
	assert (dh->pub_key);

	Base128_Encode(stream.data, stream.cursize, base128buff, sizeof(base128buff));

	Com_sprintf(msg, sizeof(msg), "getchallenge \"%s\"", base128buff);
	
	return msg;
}

int CH_ProcessChallengeResponse(unsigned int *challenge)
{
	const char *data = Cmd_Argv(1);
	unsigned char buff[256];
	//UINT64 pb;
	BIGNUM *pb;
	unsigned int key;
	unsigned char skey[32];
	unsigned int ch;
	int i;
	bitstream_t stream;

	//if (!dhdata.Generator || !dhdata.Modulus) {
	if (!dh) {
		return 0;
	}

	Base128_Decode(data, 0, buff, 256);

	BitStream_Init(&stream, buff, 256);
	BitStream_BeginReading(&stream);
	key = BitStream_ReadUInt(&stream);
	if (key != 0x18ABD591) {	// Verify response key
		return 0;
	}

	pb = BN_new();
	//pb = BitStream_ReadUInt64(&stream);
	BitStream_ReadBigNum(&stream, pb);

	if (!DH_compute_key(skey, pb, dh)) {
		BN_free(pb);
		DH_free(dh);
		dh = 0;
		return 0;
	}

	BN_free(pb);

	// Secret tag: Bit 19 must always be set
	// The following code ensures this is always the case
	ch = (unsigned int)-1;	// Start with all bits set to 1
	for (i=0; i<32; i+=4) {
		ch ^= (*(unsigned int *)&skey[i] << 12 );	// Shift the 19th bit (and all below) to 31st and xor it with the 'challenge'
	}
	ch &= 0x80000000;
	ch >>= 12;	// Shift it back so the highest bit is now the 19th.
	// The 19th bit will be 1 if the amount of 1's on the 19th bit is even, otherwise it'll be 0
	// The remaining bits will add some randomness to hide the purpose of this code

	for (i=0; i<32; i+=4) {
		ch ^= *(unsigned int *)&skey[i];
	}

	*challenge = ch;

	DH_free(dh);
	dh = 0;
	
	return 1;
}



