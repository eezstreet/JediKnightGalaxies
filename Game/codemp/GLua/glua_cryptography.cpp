// Implements cryptographic functionality into GLua (Hashing and symmetric encryption)
// Using OpenSSL's EVP interface.

#include "../game/g_local.h"
#include "../game/q_shared.h"

#include <openssl/evp.h>
#include <stdlib.h>

#include "glua.h"


typedef struct {
	const EVP_MD *md;			// Loaded message digest algorithm. NULL if none is loaded.
	EVP_MD_CTX ctx;		// EVP context
} GLua_EVP_MD_CTX_t;


typedef struct {
	EVP_CIPHER *cipher;		// Loaded cipher algorithm. NULL if none is loaded.
	EVP_CIPHER_CTX ctx;		// EVP context
	/* Encrypted/Decrypted data buffer */
	unsigned char *data;	// Data (size of alloc is length + 1 blocksize)
	unsigned int blocksize;	// Cached
	unsigned int length;	// Length of the data
} GLua_EVP_CIPHER_CTX_t;


GLua_EVP_MD_CTX_t *GLua_CheckDigestContext(lua_State *L, int idx) {
	GLua_EVP_MD_CTX_t *data;
	if (!ValidateObject(L, idx, GO_DIGESTCONTEXT)) {
		luaL_typerror(L, idx, "DigestContext");
	}

	data = (GLua_EVP_MD_CTX_t *)lua_touserdata(L, idx);
	return data;
}

GLua_EVP_CIPHER_CTX_t *GLua_CheckCipherContext(lua_State *L, int idx) {
	GLua_EVP_CIPHER_CTX_t *data;
	if (!ValidateObject(L, idx, GO_CIPHERCONTEXT)) {
		luaL_typerror(L, idx, "CipherContext");
	}

	data = (GLua_EVP_CIPHER_CTX_t *)lua_touserdata(L, idx);
	return data;
}

/* Creates a new message digest context */
static int GLua_Cryptography_CreateDigestContext(lua_State *L) {
	GLua_EVP_MD_CTX_t *ctx;


	ctx = (GLua_EVP_MD_CTX_t *)lua_newuserdata(L, sizeof(GLua_EVP_MD_CTX_t));
	memset(ctx, 0, sizeof(GLua_EVP_MD_CTX_t));
	EVP_MD_CTX_init(&ctx->ctx);

	luaL_getmetatable(L,"DigestContext");
	lua_setmetatable(L,-2);
	return 1;
}

/* Creates a new cipher context */
static int GLua_Cryptography_CreateCipherContext(lua_State *L) {
	GLua_EVP_CIPHER_CTX_t *ctx;


	ctx = (GLua_EVP_CIPHER_CTX_t *)lua_newuserdata(L, sizeof(GLua_EVP_CIPHER_CTX_t));
	memset(ctx, 0, sizeof(GLua_EVP_CIPHER_CTX_t));
	EVP_CIPHER_CTX_init(&ctx->ctx);

	luaL_getmetatable(L,"CipherContext");
	lua_setmetatable(L,-2);
	return 1;
}

/* Creates a message digest */
// Sample usage: crypto.Digest("md5", "message to digest")
static int GLua_Cryptography_Digest(lua_State *L) {
	EVP_MD_CTX ctx;
	const EVP_MD *md;
	unsigned char digest[EVP_MAX_MD_SIZE];
	unsigned int digestlen;

	const char *digestname = luaL_checkstring(L,1);
	unsigned int length;
	const char *data = luaL_checklstring(L, 2, &length);

	md = EVP_get_digestbyname(digestname);
	if (!md) {
		return 0;
	}

	EVP_MD_CTX_init(&ctx);
	EVP_DigestInit_ex(&ctx, md, NULL);
	EVP_DigestUpdate(&ctx, data, length);
	EVP_DigestFinal_ex(&ctx, digest, &digestlen);
	EVP_MD_CTX_cleanup(&ctx);

	lua_pushlstring(L, (const char *)digest, digestlen);
	return 1;
}


// Encrypt using a symmetric cipher
// Syntax: cryptography.Encrypt(Cipher, Data, Key, [ IV ], [ Pad ])
// Sample usage: crypto.Encrypt("aes-128-cbc", "The data to encrypt", "The secret key!!" , "Init. Vector....", true)
static int GLua_Cryptography_Encrypt(lua_State *L) {
	EVP_CIPHER_CTX ctx;
	const EVP_CIPHER *cipher;
	
	unsigned int tmplen;

	unsigned int enclen = 0;
	unsigned char *encrypted;

	const char *ciphername = luaL_checkstring(L,1);
	
	unsigned int length;
	const unsigned char *data = (const unsigned char *)luaL_checklstring(L, 2, &length);

	unsigned int keylen;
	const char *key = luaL_checklstring(L, 3, &keylen);

	unsigned int ivlen;
	const char *iv = luaL_optlstring(L, 4, NULL, &ivlen);

	unsigned int blocklen;

	int usepadding = 1;

	if (!lua_isnoneornil(L, 5)) {
		usepadding = lua_toboolean(L, 5);
	}

	cipher = EVP_get_cipherbyname(ciphername);
	if (!cipher) {
		return 0;
	}

	// If an IV is provided, check if it's the correct length
	if (iv) {
		if (ivlen != (unsigned)EVP_CIPHER_iv_length(cipher)) {
			return 0;
		}
	}

	EVP_CIPHER_CTX_init(&ctx);
	EVP_EncryptInit_ex(&ctx, cipher, NULL, NULL, NULL);
	// Set up key size
	EVP_CIPHER_CTX_set_padding(&ctx, usepadding);
	if (!EVP_CIPHER_CTX_set_key_length(&ctx, keylen)) {
		// Bad key length
		EVP_CIPHER_CTX_cleanup(&ctx);
		return 0;
	}
	EVP_EncryptInit_ex(&ctx, NULL, NULL, (const unsigned char *)key, (const unsigned char *)iv);
	// Prepare output buffer (output length is (at most) the size of the input data + 1 full block)
	blocklen = EVP_CIPHER_CTX_block_size(&ctx);
	encrypted = (unsigned char *)malloc(length + blocklen);
	if (!encrypted) {
		EVP_CIPHER_CTX_cleanup(&ctx);
		return 0;
	}

	EVP_EncryptUpdate(&ctx, encrypted, (int *)&tmplen, data, length);
	enclen += tmplen;

	EVP_EncryptFinal_ex(&ctx, encrypted + enclen, (int *)&tmplen);
	enclen += tmplen;

	EVP_CIPHER_CTX_cleanup(&ctx);

	lua_pushlstring(L, (const char *)encrypted, enclen);
	free((void *)encrypted);
	return 1;
}

// Decrypt using a symmetric cipher
// Syntax: cryptography.Decrypt(Cipher, Data, Key, [ IV ], [ Pad ])
// Sample usage: crypto.Decrypt("aes-128-cbc", "<encrypted message>", "The secret key!!" , "Init. Vector....", true)
static int GLua_Cryptography_Decrypt(lua_State *L) {
	EVP_CIPHER_CTX ctx;
	const EVP_CIPHER *cipher;
	
	unsigned int tmplen;

	unsigned int declen = 0;
	unsigned char *decrypted;

	const char *ciphername = luaL_checkstring(L,1);
	
	unsigned int length;
	const unsigned char *data = (const unsigned char *)luaL_checklstring(L, 2, &length);

	unsigned int keylen;
	const char *key = luaL_checklstring(L, 3, &keylen);

	unsigned int ivlen;
	const char *iv = luaL_optlstring(L, 4, NULL, &ivlen);

	unsigned int blocklen;

	int usepadding = 1;

	if (!lua_isnoneornil(L, 5)) {
		usepadding = lua_toboolean(L, 5);
	}

	cipher = EVP_get_cipherbyname(ciphername);
	if (!cipher) {
		return 0;
	}

	// If an IV is provided, check if it's the correct length
	if (iv) {
		if (ivlen != (unsigned)EVP_CIPHER_iv_length(cipher)) {
			return 0;
		}
	}

	EVP_CIPHER_CTX_init(&ctx);
	EVP_DecryptInit_ex(&ctx, cipher, NULL, NULL, NULL);
	// Set up key size
	EVP_CIPHER_CTX_set_padding(&ctx, usepadding);
	if (!EVP_CIPHER_CTX_set_key_length(&ctx, keylen)) {
		// Bad key length
		EVP_CIPHER_CTX_cleanup(&ctx);
		return 0;
	}
	EVP_DecryptInit_ex(&ctx, NULL, NULL, (const unsigned char *)key, (const unsigned char *)iv);
	// Prepare output buffer (output length is (at most) the size of the input data)
	blocklen = EVP_CIPHER_CTX_block_size(&ctx);
	decrypted = (unsigned char *)malloc(length);
	if (!decrypted) {
		EVP_CIPHER_CTX_cleanup(&ctx);
		return 0;
	}

	EVP_DecryptUpdate(&ctx, decrypted, (int *)&tmplen, data, length);
	declen += tmplen;

	if (!EVP_DecryptFinal_ex(&ctx, decrypted + declen, (int *)&tmplen)) {
		EVP_CIPHER_CTX_cleanup(&ctx);
		free((void *)decrypted);
		return 0;
	}
	declen += tmplen;

	EVP_CIPHER_CTX_cleanup(&ctx);

	lua_pushlstring(L, (const char *)decrypted, declen);
	free((void *)decrypted);
	return 1;
}


static const struct luaL_reg cryptography_f [] = {
	{"CreateDigestContext", GLua_Cryptography_CreateDigestContext},
	//{"CreateCipherContext", GLua_Cryptography_CreateCipherContext},
	{"Digest", GLua_Cryptography_Digest},
	{"Encrypt", GLua_Cryptography_Encrypt},
	{"Decrypt", GLua_Cryptography_Decrypt},
	{NULL, NULL},
};


static int GLua_DigestContext_Index(lua_State *L) {
	GLua_EVP_MD_CTX_t *ctx = GLua_CheckDigestContext(L, 1);
	const char *key = lua_tostring(L,2);
	GLUA_UNUSED(ctx);
	lua_getmetatable(L,1);
	lua_getfield(L,-1,key);
	return 1;
}

static int GLua_DigestContext_NewIndex(lua_State *L) {
	return 0;
}

static int GLua_DigestContext_ToString(lua_State *L) {
	GLua_EVP_MD_CTX_t *ctx = GLua_CheckDigestContext(L, 1);
	if (ctx->md) {
		lua_pushstring(L,va("DigestContext [ %s ]",  OBJ_nid2sn(ctx->md->type)));
	} else {
		lua_pushstring(L,"DigestContext");
	}
	return 1;
}

static int GLua_DigestContext_GarbageCollect(lua_State *L) {
	GLua_EVP_MD_CTX_t *ctx = GLua_CheckDigestContext(L, 1);
	ctx->md = NULL;
	EVP_MD_CTX_cleanup(&ctx->ctx);
	return 0;
}


static int GLua_DigestContext_Init(lua_State *L) {
	GLua_EVP_MD_CTX_t *ctx = GLua_CheckDigestContext(L, 1);
	const char *digestname = luaL_checkstring(L, 2);

	ctx->md = EVP_get_digestbyname(digestname);

	EVP_DigestInit_ex(&ctx->ctx, ctx->md ? ctx->md : EVP_md_null(), NULL);		// Always init, no matter if md is valid or not
	lua_pushboolean(L, !!ctx->md);
	return 1;
}

static int GLua_DigestContext_Update(lua_State *L) {
	GLua_EVP_MD_CTX_t *ctx = GLua_CheckDigestContext(L, 1);
	unsigned int len;
	const char *data = luaL_checklstring(L, 2, &len);

	if (!ctx->md) {
		return 0;	// Calling Update without Init will cause a crash, so bail if we're not initialized (or loaded a bad algo).
	}
	EVP_DigestUpdate(&ctx->ctx, data, len);
	return 0;
}


static int GLua_DigestContext_Final(lua_State *L) {
	GLua_EVP_MD_CTX_t *ctx = GLua_CheckDigestContext(L, 1);
	unsigned char hash[EVP_MAX_MD_SIZE];
	unsigned int len;

	if (!ctx->md) {
		return 0;
	}
	EVP_DigestFinal_ex(&ctx->ctx, hash, &len);

	lua_pushlstring(L, (const char *)hash, len);
	return 1;
}

static int GLua_DigestContext_GetSize(lua_State *L) {
	GLua_EVP_MD_CTX_t *ctx = GLua_CheckDigestContext(L, 1);

	if (!ctx->md) {
		lua_pushinteger(L, 0);
		return 1;
	}
	lua_pushinteger(L, EVP_MD_size(ctx->md));
	return 1;
}

static const struct luaL_reg digestcontext_m [] = {
	// Meta functions
	{"__index", GLua_DigestContext_Index},
	{"__newindex", GLua_DigestContext_NewIndex},
	{"__tostring", GLua_DigestContext_ToString},
	{"__gc", GLua_DigestContext_GarbageCollect},
	// Methods
	{"Init", GLua_DigestContext_Init},
	{"Update", GLua_DigestContext_Update},
	{"Final", GLua_DigestContext_Final},
	// Info Functions
	{"GetSize", GLua_DigestContext_GetSize},
	{NULL, NULL},
};



static int GLua_CipherContext_Index(lua_State *L) {
	GLua_EVP_CIPHER_CTX_t *ctx = GLua_CheckCipherContext(L, 1);
	const char *key = lua_tostring(L,2);
	GLUA_UNUSED(ctx);
	lua_getmetatable(L,1);
	lua_getfield(L,-1,key);
	return 1;
}

static int GLua_CipherContext_NewIndex(lua_State *L) {
	return 0;
}

static int GLua_CipherContext_ToString(lua_State *L) {
	GLua_EVP_CIPHER_CTX_t *ctx = GLua_CheckCipherContext(L, 1);
	if (ctx->cipher) {
		lua_pushstring(L,va("CipherContext [ %s ]",  OBJ_nid2sn(ctx->cipher->nid)));
	} else {
		lua_pushstring(L,"CipherContext");
	}
	return 1;
}

static int GLua_CipherContext_GarbageCollect(lua_State *L) {
	GLua_EVP_CIPHER_CTX_t *ctx = GLua_CheckCipherContext(L, 1);
	ctx->cipher = NULL;
	EVP_CIPHER_CTX_cleanup(&ctx->ctx);
	return 0;
}

static const struct luaL_reg ciphercontext_m [] = {
	// Meta functions
	{"__index", GLua_DigestContext_Index},
	{"__newindex", GLua_DigestContext_NewIndex},
	{"__tostring", GLua_DigestContext_ToString},
	{"__gc", GLua_DigestContext_GarbageCollect},
	{NULL, NULL},
};

void GLua_Define_Cryptography(lua_State *L) {
	STACKGUARD_INIT(L)

	luaL_register(L, "crypto", cryptography_f);
	lua_pop(L,1);

	
	luaL_newmetatable(L,"DigestContext");
	luaL_register(L, NULL, digestcontext_m);
	
	lua_pushstring(L,"ObjID");
	lua_pushinteger(L, GO_DIGESTCONTEXT);
	lua_settable(L,-3);

	lua_pushstring(L,"ObjName");
	lua_pushstring(L,"DigestContext");
	lua_settable(L,-3);

	lua_pop(L,1);

	luaL_newmetatable(L,"CipherContext");
	luaL_register(L, NULL, ciphercontext_m);
	
	lua_pushstring(L,"ObjID");
	lua_pushinteger(L, GO_CIPHERCONTEXT);
	lua_settable(L,-3);

	lua_pushstring(L,"ObjName");
	lua_pushstring(L,"CipherContext");
	lua_settable(L,-3);

	lua_pop(L,1);

	STACKGUARD_CHECK(L)
}