/////////////////////////////////////////////
//
//  Jedi Knight Galaxies (Client-side Auxiliary library)
//  BSP Decoding library
//
//  This is used to make it possible to load
//  BSP's using JKG's BSP encryption
//
//  SECURITY WARNING:
//  THIS CODE IS CLASSIFIED
//  DO NOT LEAK THIS OUT TO *ANYONE*
//
////////////////////////////////////////////

#pragma warning( disable: 4305 )

// Standard BSP: "RBSP"
#define BSP_IDENT				(('P'<<24)+('S'<<16)+('B'<<8)+'R')
#define BSP_VERSION				1
// Encrypted BSP: "GBSP"
#define GBSP_IDENT				(('P'<<24)+('S'<<16)+('B'<<8)+'G')
#define GBSP_VERSION_CODE		0xc810		// Verification tag for GBSPs

typedef struct {
	int		fileofs, filelen;
} lump_t;

#define	LUMP_ENTITIES		0
#define	LUMP_SHADERS		1
#define	LUMP_PLANES			2
#define	LUMP_NODES			3
#define	LUMP_LEAFS			4
#define	LUMP_LEAFSURFACES	5
#define	LUMP_LEAFBRUSHES	6
#define	LUMP_MODELS			7
#define	LUMP_BRUSHES		8
#define	LUMP_BRUSHSIDES		9
#define	LUMP_DRAWVERTS		10
#define	LUMP_DRAWINDEXES	11
#define	LUMP_FOGS			12
#define	LUMP_SURFACES		13
#define	LUMP_LIGHTMAPS		14
#define	LUMP_LIGHTGRID		15
#define	LUMP_VISIBILITY		16
#define LUMP_LIGHTARRAY		17
#define	HEADER_LUMPS		18

typedef unsigned int uint;
typedef unsigned char uchar;
typedef unsigned short ushort;

typedef struct {
	int			ident;
	int			version;

	lump_t		lumps[HEADER_LUMPS];
} dheader_t;

typedef struct {
	int displace;
	int	offset;
	int mask;
} BSPEncryptInfo_t;

static uchar MasterKey[128] = { 0xB5, 0x89, 0x94, 0x4A, 0x4D, 0xC6, 0x4, 0xC3, 0xD1, 0xB6, 0xC, 0x6A, 0xDD, 0xCA, 0x60, 0xF6, 0xDF, 0xE, 0xF3, 0x5D, 0x86, 0xC4, 0xE, 0x98, 0x78, 0x4C, 0x9F, 0xA6, 0x44, 0x48, 0xD4, 0xD3, 0x97, 0xFC, 0xE9, 0x3A, 0xB2, 0xFB, 0x3E, 0x89, 0x1B, 0x100, 0xAD, 0x4, 0x93, 0x1A, 0x1A, 0xCD, 0x49, 0xC, 0x4C, 0x62, 0x4D, 0xF3, 0xFB, 0x67, 0x47, 0x29, 0x2A, 0xA6, 0x69, 0x6A, 0xB6, 0x54, 0xA2, 0x35, 0x30, 0x95, 0x15, 0x75, 0xE8, 0x43, 0xC9, 0x61, 0x4A, 0xEB, 0xA2, 0xA1, 0x6E, 0x19, 0x90, 0xB2, 0xEA, 0xD6, 0x6, 0x8B, 0xEB, 0x6E, 0xAE, 0x81, 0x84, 0x77, 0x5A, 0x68, 0x45, 0xE, 0x3E, 0xFB, 0x10, 0x64, 0x5D, 0x7D, 0x28, 0x79, 0x42, 0xA1, 0x8B, 0x28, 0xF0, 0xA8, 0x82, 0x64, 0x1B, 0xC9, 0x76, 0xC1, 0x99, 0xD5, 0x5, 0x36, 0x13, 0x1B, 0x55, 0x21, 0x0, 0x89, 0xA8, 0x8B};
static uint MaskKeys[16] = { 0x2223F8CF, 0x12887B2E, 0x31957E26, 0x30AFB3CB, 0x2D6148C1, 0x1A7F88CE, 0x3297A7EA, 0x3D90D3D0, 0x399D20C, 0x174C8E8B, 0x31189EAF, 0x25EADCD9, 0x1315607E, 0x297636B5, 0x11E0DF44, 0x34C6B237 };
static ushort RootKeys[8] = { 0x71EA, 0x6D9A, 0xB87A, 0x1939, 0x761E, 0xA18D, 0x8B20, 0xF930 };	


// =================================================
// Jedi Knight Galaxies BSP encryption
// -------------------------------------------------
// To avoid people from stealing our maps, they
// have their header and index table encrypted.
// This will effectively prevent anything from
// successfully reading them
//
// JKG will, when a BSP is loaded, perform the
// decoding on the fly is that is needed.
//
// BSP Format:
//	int			ident;
//	int			version;
//	lump_t		lumps[HEADER_LUMPS];
//
// In normal BSP's the ident is 'RBSP' and version is 1
// In encrypted BSP's, the ident is 'GBSP' and the version
// is a packed version of all decryption data:
//
// The first two bytes are an ID marker, being 0x10c8
// The latter two bytes the decryption info, formatted
// as follows (x represents a bit)
//
//    2            3              4
//  _____     ___________   ___________
// /     \   /           \ /           \
// x x x x   x x x x   x x x x   x x x x
// \___/ 
//   1
// 
// 1: Root Key, used to decrypt the rest
// 2: Lump mask key (after decryption)
// 3: Displacement  (after decryption)
// 4: Offset        (after decryption)
//
// The lump mask key is used to unmask all lumps
// before final decryption, which uses a circular
// portion from the 1024-bit master key, starting at
// byte 'displacement', beginning its rotation at
// point 'offset'
//
// Illustation:
//
//                       2
//              1________v________
//              /                 \
// xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx <-- master 1024-bit key
//
// Where 1 is the displacement and 2 is the offset
// in this displacement
// When decrypting, bytes from the master key are used to do so
// The decryption uses data starting at point 2
// wrapping around in area 1
// Due to this, 4096 unique keys are possible.
// The decryption is run on the lumps table, so 
// that after decryption they contain their original
// values again and can be used by the engine
//
// =================================================

static void JKG_GetDecryptInfo(dheader_t *header, BSPEncryptInfo_t *encInfo);

static unsigned int jkg_rotl(unsigned char value, int shift) {
    shift &= 7;
    return (value << shift) | (value >> (8 - shift));
}
 
static unsigned int jkg_rotr(unsigned char value, int shift) {
    shift &= 7;
    return (value >> shift) | (value << (8 - shift));
}

static void JKG_BSP_Decrypt(unsigned char *masterkey, int offset, unsigned char *lumps, int len) {
	int i;
	for (i=0; i<len; i++) {
		*lumps += masterkey[(i+offset) & 63];
		*lumps = (unsigned char)jkg_rotl(*lumps, masterkey[(i+1+offset) & 63] & 7);
		*lumps ^= masterkey[(i+2+offset) & 63];
		lumps++;
	}
}

static void JKG_BSP_Encrypt(unsigned char *masterkey, int offset, unsigned char *lumps, int len) {
	int i;
	for (i=0; i<len; i++) {
		*lumps ^= masterkey[(i+offset+2) & 63];
		*lumps = (unsigned char)jkg_rotr(*lumps, masterkey[(i+1+offset) & 63] & 7);
		*lumps -= masterkey[(i+offset) & 63];
		lumps++;
	}
}

// =================================================
// JKG_BSPIsValid
// -------------------------------------------------
// Determines whether the BSP is valid or not
// Pass the dheader_t struct (as a void *)
//
// Return values:
// 0: Invalid
// 1: Standard BSP
// 2: Galaxies BSP
// =================================================

int JKG_BSPIsValid(void *bspheader) {
	dheader_t *header = (dheader_t *)bspheader;
	if (header->ident == BSP_IDENT && header->version == BSP_VERSION) {
		return 1;
	}
	if (header->ident == GBSP_IDENT && *(ushort *)&header->version == GBSP_VERSION_CODE) {
		return 2;
	}
	return 0;
}

// =================================================
// JKG_DecodeBSP
// -------------------------------------------------
// Decrypts the BSP (if required)
// Pass the dheader_t struct (as a void *)
//
// Return values:
// 0: Failed to decrypt
// 1: Decrypted successfully
// =================================================
int JKG_DecodeBSP(void *bspheader) {
	dheader_t *header = (dheader_t *)bspheader;
	BSPEncryptInfo_t encInfo;
	int i;
	int valid = JKG_BSPIsValid(bspheader);
	
	if (!valid) {
		return 0;
	} else if (valid == 1) {
		return 1;	// Its a base BSP, no need to decrypt xD
	}
	JKG_GetDecryptInfo(header, &encInfo);
	// Unmask all lumps first
	for (i=0; i<HEADER_LUMPS; i++) {
		header->lumps[i].filelen ^= MaskKeys[encInfo.mask];
		header->lumps[i].fileofs ^= MaskKeys[encInfo.mask];
	}
	JKG_BSP_Decrypt((uchar *)&MasterKey[encInfo.displace], encInfo.offset, (uchar *)&header->lumps, sizeof(header->lumps));
	header->ident = BSP_IDENT;
	header->version = BSP_VERSION;
	return 1;	
}

// =================================================
// JKG_GetDecryptInfo (INTERNAL)
// -------------------------------------------------
// Decrypts the BSP (if required)
// Pass the dheader_t struct (as a void *)
//
// CAUTION: This function assumes the header is a valid
//          encrypted BSP
//
// =================================================
static void JKG_GetDecryptInfo(dheader_t *header, BSPEncryptInfo_t *encInfo) {
	ushort data;
	int rootKey;
	data = *(ushort *)((int)&header->version + 2);
	rootKey = (data & 0xE000) >> 13;			// 1110 0000 0000 0000
	data ^= RootKeys[rootKey];
	encInfo->mask = (data & 0xF000) >> 12;		// 1111 0000 0000 0000
	encInfo->displace = (data & 0x0FC0) >> 6;	// 0000 1111 1100 0000
	encInfo->offset = data & 0x003F;			// 0000 0000 0011 1111
}