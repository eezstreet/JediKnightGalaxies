//       ____ ___________________   ___           ____  __ _______   ___  ________  ___ ______________
//      |    |\_   _____/\______ \ |   |         |    |/ _|\      \ |   |/  _____/ /   |   \__    ___/
//      |    | |    __)_  |    |  \|   |         |      <  /   |   \|   /   \  ___/    ~    \|    |   
//  /\__|    | |        \ |    `   \   |         |    |  \/    |    \   \    \_\  \    Y    /|    |   
//  \________|/_______  //_______  /___|         |____|__ \____|__  /___|\______  /\___|_  / |____|   
//                    \/         \/                      \/       \/            \/       \/           
//                         ________    _____   ____       _____  ____  ___ ______________ _________   
//                        /  _____/   /  _  \ |    |     /  _  \ \   \/  /|   \_   _____//   _____/   
//                       /   \  ___  /  /_\  \|    |    /  /_\  \ \     / |   ||    __)_ \_____  \    
//                       \    \_\  \/    |    \    |___/    |    \/     \ |   ||        \/        \   
//                        \______  /\____|__  /_______ \____|__  /___/\  \|___/_______  /_______  /   
//                               \/         \/        \/	   \/	   \_/			  \/        \/ (c)
// jkg_libcurl.c
// JKG Libcurl Communications System
// (c) 2013 Jedi Knight Galaxies

//////////////////////////////////////////////////
//
//  JKG Libcurl Communications System
//  Version 1.0 (Multi-platform)
//  Client-side
//
//  $Id$
//
//////////////////////////////////////////////////

/*
	This system currently uses a single curl easy handle to facilitate all
	network tasks. This means that only one network task can run at a time.
	
	If the system is under-performing, we will need to switch to the curl
	multi handle method instead.

	- Didz
*/

#define _CRT_SECURE_NO_WARNINGS

#include "openssl/evp.h"
#include "openssl/dh.h"
#include "openssl/bn.h"
#include "openssl/rand.h"
#include "curl/curl.h"
#include "gl_enginefuncs.h"
#include "json/cJSON.h"
#include "jkg_libcurl.h"
#include "encoding/base64.h"
#include "qcommon/game_version.h"
#include <assert.h>
#include <stdlib.h>
#include <stdarg.h>

enum {
	LCSTATE_UNINIT = 0,	// Not initialized
	LCSTATE_AUTHING,	// Authenticating (doing the handshake)
	LCSTATE_READY,		// Ready
	LCSTATE_FAILURE = -1,
};

static int libcurl_state = LCSTATE_UNINIT;

// This struct holds all our authentication related data
static struct jkg_auth_s {
	unsigned char DH_shared_secret[128];

	// Cryptographic keys used (linked to curl handle, so if we get more than 1 handle, we need more than one of these)
	unsigned char crypto_key[32];
	unsigned char crypto_iv[16];

	// Authentication data
	char sessionID[33];
	int authenticated;	// If 0, we need to do a handshake first
} jkg_auth;


typedef struct jkg_socketState_s
{
	int active;
	int linked;
	int queued;
	int id;
	CURL *handle;
	asyncTask_t *task;
	unsigned char crypto_key[32];
	unsigned char crypto_iv[16];
	jkgMemoryChunk_t stream;
} jkg_socketState_t;

static struct jkg_connections_s {
	CURL *handshake;
	CURLM *multi;
	int activeSockets;

	jkg_socketState_t sockets[CURL_MAX_HANDLES];

} jkg_connections;

static char useragent[MAX_STRING_CHARS];

#define JKGTAG "JKG/"JKG_VERSION

void JKG_Libcurl_Reset ( CURL *curlh )
{
	curl_easy_reset( curlh );
	curl_easy_setopt( curlh, CURLOPT_USERAGENT, useragent );
	curl_easy_setopt( curlh, CURLOPT_TIMEOUT, 30);	// 30 second timeout

	// The URL will vary depending on the request, so dont set it here
	//curl_easy_setopt( curlh, CURLOPT_URL, jkg_masterServerUrl.string ); // The URL cvar string will be copied by Libcurl, so we can just pass this pointer
	return;
}

// Free the returned string when you're done with it
// Before this can be used, a valid handshake is required
static const char *JKG_EncryptKey(unsigned char key[32], unsigned char iv[16])
{
	unsigned char data[48];	// Input size is 48 (key + iv)
	unsigned char enc[49];	// Output size is (always) 49, as we skip padding
	char *b64;
	int outlen;
	int tmplen;
	int blen;
	int offset;
	EVP_CIPHER_CTX aes;
	unsigned char masterkey[256];

	if (!jkg_auth.authenticated) {
		return 0;
	}

	offset = rand() % 204;

	memcpy(masterkey, jkg_auth.DH_shared_secret, 128);
	memcpy(masterkey + 128, jkg_auth.DH_shared_secret, 128);

	memcpy(data, key, 32);
	memcpy(data + 32, iv, 16);

	EVP_CIPHER_CTX_init(&aes);

	EVP_EncryptInit_ex(&aes, EVP_aes_256_cbc(), NULL, masterkey + offset, masterkey + offset + 32);
	EVP_CIPHER_CTX_set_padding(&aes, 0);
	EVP_EncryptUpdate(&aes, enc + 1, &outlen, data, 48);
	EVP_EncryptFinal_ex(&aes, enc + 1 + outlen, &tmplen);
	outlen += tmplen;

	assert(outlen == 48);

	EVP_CIPHER_CTX_cleanup(&aes);

	enc[0] = (unsigned char)offset;

	b64 = (char *)malloc( (blen = Base64_EncodeLength(outlen + 1)) );

	if (!b64) {
		return NULL;
	}

	Base64_EncodeURL( (const char *)enc, 49, b64, blen );

	return b64;
}

// Remember to free the returned string after using it!
static const char *JKG_EncryptJson ( cJSON *json, unsigned char key[32], unsigned char iv[16] )
{
	const char *sjson;
	unsigned char *encrypted;
	char *b64;
	int outlen;
	int tmplen;
	int blen;
	EVP_CIPHER_CTX aes;
	
	sjson = cJSON_Serialize(json, 0);
	if (!sjson)	{
		return NULL;
	}

	encrypted = (unsigned char *)malloc(strlen(sjson) + 16); // The length of the encrypted data will never exceed this
	if (!encrypted) {
		free((char *)sjson);
		return NULL;
	}

	EVP_CIPHER_CTX_init(&aes);

	EVP_EncryptInit_ex(&aes, EVP_aes_256_cbc(), NULL,  key, iv);
	EVP_EncryptUpdate(&aes, encrypted, &outlen, (const unsigned char *)sjson, strlen(sjson));
	EVP_EncryptFinal_ex(&aes, encrypted + outlen, &tmplen);
	outlen += tmplen;

	EVP_CIPHER_CTX_cleanup(&aes);

	free((char *)sjson);

	b64 = (char *)malloc( (blen = Base64_EncodeLength(outlen)) );

	if (!b64) {
		free(encrypted);
		return NULL;
	}

	Base64_EncodeURL( (const char *)encrypted, outlen, b64, blen );

	free(encrypted);

	return b64;
}

// Remember to free the returned string after using it!
static const char *JKG_EncryptSerializedJson ( const char *json, unsigned char key[32], unsigned char iv[16] )
{
	const char *sjson;
	unsigned char *encrypted;
	char *b64;
	int outlen;
	int tmplen;
	int blen;
	EVP_CIPHER_CTX aes;
	
	sjson = json;
	if (!sjson)	{
		return NULL;
	}

	encrypted = (unsigned char *)malloc(strlen(sjson) + 16); // The length of the encrypted data will never exceed this
	if (!encrypted) {
		free((char *)sjson);
		return NULL;
	}

	EVP_CIPHER_CTX_init(&aes);

	EVP_EncryptInit_ex(&aes, EVP_aes_256_cbc(), NULL,  key, iv);
	EVP_EncryptUpdate(&aes, encrypted, &outlen, (const unsigned char *)sjson, strlen(sjson));
	EVP_EncryptFinal_ex(&aes, encrypted + outlen, &tmplen);
	outlen += tmplen;

	EVP_CIPHER_CTX_cleanup(&aes);

	b64 = (char *)malloc( (blen = Base64_EncodeLength(outlen)) );

	if (!b64) {
		free(encrypted);
		return NULL;
	}

	Base64_EncodeURL( (const char *)encrypted, outlen, b64, blen );

	free(encrypted);

	return b64;
}


// Remember to cJSON_Delete the returned cJSON structure after using it!
static cJSON *JKG_DecryptJson ( const char *b64, unsigned char key[32], unsigned char iv[16] )
{
	char *sjson;
	unsigned char *encrypted;
	int outlen;
	int tmplen;
	int len;
	cJSON *item;

	EVP_CIPHER_CTX aes;

	len = Base64_DecodeLength(0, b64, 0);

	encrypted = (unsigned char *)malloc(len);
	if (!encrypted) {
		return NULL;
	}

	sjson = (char *)malloc(len);
	if (!sjson) {
		return NULL;
	}

	Base64_Decode(b64, 0, (char *)encrypted, len);

	EVP_CIPHER_CTX_init(&aes);

	EVP_DecryptInit_ex(&aes, EVP_aes_256_cbc(), NULL,  key, iv);
	EVP_DecryptUpdate(&aes, (unsigned char *)sjson, &outlen, encrypted, len);
	if (!EVP_DecryptFinal_ex(&aes, (unsigned char *)sjson + outlen, &tmplen)) {
		// Decryption failed
		free(encrypted);
		free(sjson);
		return 0;
	}
	outlen += tmplen;

	EVP_CIPHER_CTX_cleanup(&aes);

	sjson[outlen] = '\0';
	
	free(encrypted);

	item = cJSON_ParsePooled(sjson, NULL, 0);

	free(sjson);

	return item;
}


// Remember to cJSON_Delete the returned cJSON structure after using it!
static cJSON *JKG_DecryptJsonBinary ( const unsigned char *data, int length, unsigned char key[32], unsigned char iv[16] )
{
	char *sjson;
	int outlen;
	int tmplen;
	cJSON *item;

	EVP_CIPHER_CTX aes;

	if (!data) {
		return NULL;
	}

	sjson = (char *)malloc(length);
	if (!sjson) {
		return NULL;
	}

	EVP_CIPHER_CTX_init(&aes);

	EVP_DecryptInit_ex(&aes, EVP_aes_256_cbc(), NULL,  key, iv);
	EVP_DecryptUpdate(&aes, (unsigned char *)sjson, &outlen, data, length);
	if (!EVP_DecryptFinal_ex(&aes, (unsigned char *)sjson + outlen, &tmplen)) {
		// Decryption failed
		free(sjson);
		return 0;
	}
	outlen += tmplen;

	EVP_CIPHER_CTX_cleanup(&aes);

	sjson[outlen] = '\0';
	
	item = cJSON_ParsePooled(sjson, NULL, 0);

	free(sjson);

	return item;
}

static void JKG_NetworkSetupCrypto(jkg_socketState_t *socket)
{
	// Seed the RNG with the server time to mix it up
	//RAND_add(&level.time, 4, 2);

	if (!socket) {
		// TEMP
		// Set up new key and iv
		RAND_bytes(jkg_auth.crypto_key, 32);
		RAND_bytes(jkg_auth.crypto_iv, 16);
	} else {
		RAND_bytes(socket->crypto_key, 32);
		RAND_bytes(socket->crypto_iv, 16);
	}
}

size_t JKG_NetworkWriteChunk ( void *ptr, size_t size, size_t nmemb, jkgMemoryChunk_t *stream )
{
	size_t realsize = size * nmemb;
	
	stream->writeBuffer = stream->writeBuffer ? realloc(stream->writeBuffer, stream->bufSize + realsize + 1) : malloc(stream->bufSize + realsize + 1);
	
	if (stream->writeBuffer)
	{
		memcpy( &(stream->writeBuffer[stream->bufSize]), ptr, realsize );
		stream->bufSize += realsize;
		*(stream->writeBuffer + stream->bufSize) = '\0';
	}

	return realsize;
}

// Only call this from the background thread!
static void JKG_Libcurl_LinkSocket(jkg_socketState_t *socket)
{
	if (!socket->linked && socket->active) {
		curl_multi_add_handle(jkg_connections.multi, socket->handle);
		socket->linked = 1;
	}
	socket->queued = 0;
}

// Only call this from the main thread
static void JKG_Libcurl_QueueSocket(jkg_socketState_t *socket)
{
	if (!socket->linked && socket->active) {
		socket->queued = 1;
	}
}

// Only call this from the background thread!
static void JKG_Libcurl_UnlinkSocket(jkg_socketState_t *socket)
{
	if (socket->linked) {
		curl_multi_remove_handle(jkg_connections.multi, socket->handle);
		socket->linked = 0;
	}
	socket->queued = 0;
}

static jkg_socketState_t *JKG_Libcurl_GetFreeSocket()
{
	int i;
	if (jkg_connections.activeSockets == CURL_MAX_HANDLES) {
		return NULL;
	}
	for (i = 0; i < CURL_MAX_HANDLES; i++) {
		if (!jkg_connections.sockets[i].active) {
			jkg_connections.sockets[i].active = 1;
			jkg_connections.activeSockets++;
			return &jkg_connections.sockets[i];
		}
	}
	return NULL;
}

static void JKG_Libcurl_FreeSocket(jkg_socketState_t *socket)
{
	CURL *temp;
	int id;
	if (socket->active) {
		JKG_Libcurl_UnlinkSocket(socket);
		if (socket->stream.writeBuffer) {
			free(socket->stream.writeBuffer);
		}
		temp = socket->handle;	// Preserve the handle
		id = socket->id;
		memset(socket, 0, sizeof(jkg_socketState_t));
		socket->id = id;
		socket->handle = temp;

		curl_easy_cleanup(socket->handle);		// Close the connection, so we dont overload the master server
		socket->handle = curl_easy_init();

		jkg_connections.activeSockets--;
	}
	
}

void DH_getbigprime(DH *dh, int seed);
static int JKG_NetworkHandShake( )
{
	CURLcode res;

	jkgMemoryChunk_t stream;
	cJSON *reply = NULL;
	DH *dh;
	BIGNUM *svpub = 0;
	long httpcode;
	int i;

	unsigned char pkey[128];

	union {
		struct {
			unsigned char prime[128];
			unsigned char key[128];
			unsigned char iv[16];
		} raw;
		unsigned char buffer[272];
	} keys;

	char request[365];

	libcurl_state = LCSTATE_AUTHING;

	memset( &stream, 0, sizeof(jkgMemoryChunk_t) );
#ifdef _DEBUG
	Com_Printf("DEBUG: Performing handshake\n");
#endif

	// Terminate all pending sockets
	for (i = 0; i < CURL_MAX_HANDLES; i++) {
		if (jkg_connections.sockets[i].linked) {	// If it is linked to a task, we can safely assume the task is not in a queue
			if (jkg_connections.sockets[i].task) {
				jkg_connections.sockets[i].task->state = TASKSTATE_QUEUED; // Reset the task
				JKG_Task_Queue(jkg_connections.sockets[i].task);
			}
			JKG_Libcurl_FreeSocket(&jkg_connections.sockets[i]);
			curl_easy_cleanup(jkg_connections.sockets[i].handle);
			jkg_connections.sockets[i].handle = curl_easy_init();
		}
	}

	jkg_connections.handshake = curl_easy_init();	// Handshakes use a fresh handle

	JKG_Libcurl_Reset( jkg_connections.handshake );

	curl_easy_setopt( jkg_connections.handshake, CURLOPT_URL, va("%s/authenticate.php", Cvar_String("jkg_masterserverurl")) );

	curl_easy_setopt( jkg_connections.handshake, CURLOPT_WRITEFUNCTION, JKG_NetworkWriteChunk );
	curl_easy_setopt( jkg_connections.handshake, CURLOPT_WRITEDATA, (void *)&stream );

	// Create the DH Context
	dh = DH_new();
	//DH_generate_parameters_ex( dh, 256, DH_GENERATOR_5, NULL );
	DH_getbigprime(dh, Sys_Milliseconds());
	DH_generate_key(dh);

	// Set up decryption keys
	RAND_bytes(keys.raw.iv, 16);
	BN_bn2bin(dh->pub_key, keys.raw.key);
	BN_bn2bin(dh->p, keys.raw.prime);

	// Encrypt them to send as request
	Base64_EncodeURL(keys.buffer, 272, request, sizeof(request)); 

	curl_easy_setopt( jkg_connections.handshake, CURLOPT_COPYPOSTFIELDS, va("r=%s", request));


	res = curl_easy_perform( jkg_connections.handshake );

	if (res != CURLE_OK) {
		Com_Printf( "Libcurl Error while doing handshake: %s\n", curl_easy_strerror( res ) );
		// Free the DH context
		DH_free(dh);
		libcurl_state = LCSTATE_READY;
		return res;
	}

	curl_easy_getinfo(jkg_connections.handshake, CURLINFO_RESPONSE_CODE, &httpcode);

	curl_easy_cleanup(jkg_connections.handshake);	// Clean up the handle, we no longer need it (for now)


	if (httpcode == 400) {
		return -1;
	} else if (httpcode == 301) {
		Com_Error(ERR_FATAL, "Fatal error: Master server authentication failure!");
		return -3;
	}

	if ( stream.writeBuffer ) {
#ifndef FINAL_BUILD
		Com_Printf( "^5%i bytes retrieved from the master server\n", stream.bufSize );
#endif
		//reply = JKG_DecryptJson( stream.writeBuffer, key, iv );
		reply = JKG_DecryptJsonBinary( stream.writeBuffer, stream.bufSize, keys.raw.key, keys.raw.iv );
		free( stream.writeBuffer );
	} else {
#ifndef FINAL_BUILD
		Com_Printf( "^3JKG_NetworkTaskFunc: stream.writeBuffer is NULL but Libcurl returned CURLE_OK\n" );
#endif
		reply = NULL;
	}

	if ( !reply ) {
		libcurl_state = LCSTATE_READY;
		return (CURL_LAST+2);
	}

	// We got a proper response, process it

	strncpy(jkg_auth.sessionID, cJSON_ToString(cJSON_GetObjectItem(reply, "s")), 32);
	jkg_auth.sessionID[32] = 0;

	Base64_Decode(cJSON_ToString(cJSON_GetObjectItem(reply, "pk")), 0, pkey, sizeof(pkey));
	svpub = BN_new();
	BN_bin2bn(pkey, 128, svpub);

	DH_compute_key(jkg_auth.DH_shared_secret, svpub, dh);
	// Free the DH context
	BN_free(svpub);
	DH_free(dh);

#ifdef _DEBUG
	Com_Printf("DEBUG: Handshake successful\n");
	Com_Printf("DEBUG: Session ID: %s\n", jkg_auth.sessionID);
#endif

	jkg_auth.authenticated = 1;

	libcurl_state = LCSTATE_READY;
	return 0;
}

static int JKG_NetworkTaskFunc ( struct asyncTask_s *task )
{
	jkg_socketState_t *socket;

	const char *enck, *encr;

	// Ensure this request is valid
	if (!(task->flags & (TASKFLAG_JSON_INITDATA | TASKFLAG_MALLOCED_INITDATA))) {
		return -1;	// JKG_NetworkTaskFunc requires the initData to be a json structure, serialized or otherwise
	}

	if (!jkg_auth.authenticated) {
		JKG_NetworkHandShake();	// TODO: Handle error codes
	}
	
	socket = JKG_Libcurl_GetFreeSocket();
	if (!socket) {
		task->flags |= TASKFLAG_RESET;	// Try later
		return -1;
	}

	JKG_NetworkSetupCrypto(socket);

	// Set up socket
	socket->task = task;

	memset( &socket->stream, 0, sizeof(jkgMemoryChunk_t) );

	JKG_Libcurl_Reset( socket->handle );
	curl_easy_setopt( socket->handle, CURLOPT_PRIVATE, socket);

	curl_easy_setopt( socket->handle, CURLOPT_URL, va("%s/requestcl.php", Cvar_String("jkg_masterserverurl")) );

	curl_easy_setopt( socket->handle, CURLOPT_WRITEFUNCTION, JKG_NetworkWriteChunk );
	curl_easy_setopt( socket->handle, CURLOPT_WRITEDATA, (void *)&socket->stream );

	// NOTE: Use CURLOPT_POSTFIELDS, not CURLOPT_HTTPPOST!
	// CURLOPT_HTTPPOST sends a binary POST message, which is transmitted in blocks, which dramatically
	// raises the query time (from .5 sec average to 2.5 sec average)

	enck = JKG_EncryptKey(socket->crypto_key, socket->crypto_iv);

	if (task->flags & TASKFLAG_JSON_INITDATA) {
		encr = JKG_EncryptJson((cJSON *)task->initData, socket->crypto_key, socket->crypto_iv);
	} else {
		encr = JKG_EncryptSerializedJson((const char *)task->initData, socket->crypto_key, socket->crypto_iv);
	}

	curl_easy_setopt( socket->handle, CURLOPT_COPYPOSTFIELDS, va("s=%s&k=%s&r=%s", jkg_auth.sessionID, enck, encr));
	free((void *)enck);
	free((void *)encr);

	JKG_Libcurl_QueueSocket(socket);	// Queue the socket

	task->flags |= TASKFLAG_PENDING;
	return 0;
}

#include <openssl/sha.h>
const char *JKG_SHA1Encode(const char *str)	// Used to hash passwords
{
	static char hash[41];
	unsigned char md[20];
	int i;

	SHA1(str, strlen(str), md);

	for (i=0; i<20; i++) {
		Com_sprintf(hash + (2*i), 3,"%02X", md[i]);
	}
	return hash;
}

asyncTask_t *JKG_NewNetworkTask ( lcMethod_t method, void (*finalCallback)(asyncTask_t *taskPointer), int ui, ... )
{
	cJSONStream *stream = cJSON_Stream_New(8, 0, 1024, 512);

	va_list arg;
	va_start(arg, ui);

	cJSON_Stream_BeginObject(stream, NULL);

	switch ( method )
	{
		case LCMETHOD_TEST:
			// Test query for debugging, must pass a random value as 'bounce'
			cJSON_Stream_WriteString( stream, "do", "test");
			cJSON_Stream_WriteString( stream, "bounce", va("%i", Sys_Milliseconds()));
			break;
		case LCMETHOD_GETTERMSOFUSE:
			cJSON_Stream_WriteString( stream, "do", "getterms");
			break;
		case LCMETHOD_REGISTERUSER:
			cJSON_Stream_WriteString( stream, "do", "register" );
			cJSON_Stream_WriteString( stream, "username", va_arg(arg, const char *));
			cJSON_Stream_WriteString( stream, "password", va_arg(arg, const char *));
			cJSON_Stream_WriteString( stream, "email", va_arg(arg, const char *) );
			break;
		case LCMETHOD_LOGIN:
			cJSON_Stream_WriteString( stream, "do", "login" );
			cJSON_Stream_WriteString( stream, "username",  va_arg(arg, const char *));
			cJSON_Stream_WriteString( stream, "password",  va_arg(arg, const char *));
			break;
		default:
			// Unknown method, abort
			free((void *)cJSON_Stream_Finalize(stream));
			return 0;
	}

	va_end(arg);

	cJSON_Stream_EndBlock(stream);

	return JKG_NewAsyncTask( JKG_NetworkTaskFunc, (void *)cJSON_Stream_Finalize(stream), TASKFLAG_MALLOCED_INITDATA, finalCallback, ui );
}

// NOTE: This function is executed on the worker thread!
void JKG_Libcurl_Poller( void )
{
	int tasks;
	CURLMsg *msg;
	int msg_remaining;
	int httpcode;
	jkg_socketState_t *socket;
	cJSON *reply;
	int i;

	for (i = 0; i < CURL_MAX_HANDLES; i++) {
		if (jkg_connections.sockets[i].queued ) {
			JKG_Libcurl_LinkSocket(&jkg_connections.sockets[i]);
		}
	}

	while (curl_multi_perform(jkg_connections.multi, &tasks) == CURLM_CALL_MULTI_PERFORM );

	if (tasks != jkg_connections.activeSockets) {
		while(msg = curl_multi_info_read( jkg_connections.multi, &msg_remaining) )
		{

			// Check return code
			curl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE, &httpcode);
			curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &socket);

			if (msg->data.result != CURLE_OK) {
#ifndef FINAL_BUILD
				Com_Printf( "Libcurl Error: %s\n", curl_easy_strerror( msg->data.result ) );
#endif
				socket->task->finalData = 0;
				socket->task->errorCode = msg->data.result;
			} else {
				if (httpcode == 403) {
					// We lost contact with the server, re-establish it
					// Flush all remaining messages
					while(curl_multi_info_read(jkg_connections.multi, &msg_remaining));
					jkg_auth.authenticated = 0;
					JKG_NetworkHandShake();	// Calling this will reset all pending queries, including this one
					break;
				} else if (httpcode == 400) {
					socket->task->errorCode = -1;
					socket->task->finalData = 0;
				} else if (httpcode == 301) {
					Com_Error(ERR_FATAL, "Fatal error: Master server authentication failure!");
					socket->task->errorCode = -2;
					socket->task->finalData = 0;
				} else {
					if ( socket->stream.writeBuffer ) {
#ifndef FINAL_BUILD
						Com_Printf( "^5%i bytes retrieved from the master server\n", socket->stream.bufSize );
#endif
						//reply = JKG_DecryptJson( socket->stream.writeBuffer, socket->crypto_key, socket->crypto_iv );
						reply = JKG_DecryptJsonBinary( socket->stream.writeBuffer, socket->stream.bufSize, socket->crypto_key, socket->crypto_iv );
					} else {
#ifndef FINAL_BUILD
						Com_Printf( "JKG_Libcurl_Pooler: stream.writeBuffer is NULL but Libcurl returned CURLE_OK\n" );
#endif
						reply = NULL;
					}

					if ( !reply ) {
						socket->task->errorCode = (CURL_LAST+2);
						socket->task->finalData = 0;
					} else {
						socket->task->finalData = reply;
						socket->task->flags |= TASKFLAG_JSON_FINALDATA;
					}
				}
			}

			socket->task->flags &= ~TASKFLAG_PENDING;
			socket->task->state = TASKSTATE_DATACOLLECT;

			JKG_Task_Finished(socket->task);

			JKG_Libcurl_FreeSocket(socket);
		} 
	}
}

int JKG_Libcurl_Init ( void )
{
	int i;
	Cvar_Get("jkg_masterserverurl", "http://jkg-master.terrangaming.com", CVAR_ROM);

	Com_sprintf( useragent, sizeof(useragent), "%s (%s; http://jkgalaxies.com/) %s", JKGTAG, Cvar_String("version"), curl_version() );

	// Initialize Libcurl
	memset(&jkg_connections, 0, sizeof(jkg_connections));

	curl_global_init( CURL_GLOBAL_WIN32 ); // Initiate win32 sockets, no need for SSL
	for (i = 0; i < CURL_MAX_HANDLES; i++) {
		jkg_connections.sockets[i].id = i;
		jkg_connections.sockets[i].handle = curl_easy_init();
	}
	
	jkg_connections.multi = curl_multi_init();

	// Enable pipelining to reduce server strain
	curl_multi_setopt(jkg_connections.multi, CURLMOPT_PIPELINING, 1);

#ifndef FINAL_BUILD
	Com_Printf( "Initialized Libcurl: %s\n", useragent );
	Com_Printf( "Master Server URL = %s\n", Cvar_String("jkg_masterserverurl"));
#endif

	libcurl_state = LCSTATE_READY;
	return 0;
}

void JKG_Libcurl_Shutdown ( void )
{
	int i;
	libcurl_state = LCSTATE_UNINIT;


	for (i = 0; i < CURL_MAX_HANDLES; i++) {
		if (jkg_connections.sockets[i].active) {
			curl_multi_remove_handle(jkg_connections.multi, jkg_connections.sockets[i].handle);
		}
		curl_easy_cleanup(jkg_connections.sockets[i].handle);
	}
		
	curl_multi_cleanup( jkg_connections.multi );

	curl_global_cleanup();
}
