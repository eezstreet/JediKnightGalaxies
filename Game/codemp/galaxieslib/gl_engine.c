///////////////////////////////////////////////////////////////////
//
//  Engine alteration (client-side auxiliary)
//
//  All engine modifications that are to be present at run-time
//  go in here
//
//  By BobaFett
//
///////////////////////////////////////////////////////////////////
//  Windows only

#include <libudis86/udis86.h>
#include <assert.h>
#include <windows.h>
#include "qcommon/game_version.h"

// Windows defs here
static void (*Com_Printf)( const char *fmt, ... ) = (void (*)(const char *, ...))0x437080;
static void (*Com_DPrintf)( const char *fmt, ... ) = (void (*)(const char *, ...))0x437240;


const char *consoleversion = "JKG: v"JKG_VERSION JKG_VERSION_SUFFIX" / JAmp: v1.0.1.0";	// Change this to alter the console text

#define JKG_CHALLENGE_KEY 0x807124BA  /* Must match the one defined on the server side! */

typedef struct {
	int addr;
	int size;
	char origbytes[24];
} PatchData_t;

typedef enum {
	PATCH_JUMP,
	PATCH_CALL,
} PatchType_e;

// ==================================================
// UnlockMemory (WIN32 & Linux compatible)
// --------------------------------------------------
// Makes the memory at address writable for at least
// size bytes.
// Returns 1 if successful, returns 0 on failure.
// ==================================================
static int UnlockMemory(int address, int size) {
	int ret;
	int dummy;
	ret = VirtualProtect((LPVOID)address, size, PAGE_EXECUTE_READWRITE, (PDWORD)&dummy);
	return (ret != 0);
}

// ==================================================
// LockMemory (WIN32 & Linux compatible)
// --------------------------------------------------
// Makes the memory at address read-only for at least
// size bytes.
// Returns 1 if successful, returns 0 on failure.
// ==================================================
static int LockMemory(int address, int size) {
	int ret;
	ret = VirtualProtect((LPVOID)address, size, PAGE_EXECUTE_READ, NULL);
	return (ret != 0);
}

// ==================================================
// JKG_PlacePatch (WIN32 & Linux compatible)
// --------------------------------------------------
// Patches the code at address to make it go towards
// destination.
// The instruction used is either JMP or CALL, 
// depending on the type specified.
//
// Before the code is modified, the code page is
// unlocked. If you wish to stop it from being locked
// again, specify 1 for nolock
//
// This function returns a malloced PatchData_t.
// To remove the patch, call JKG_RemovePatch. This
// will also free the PatchData_t.
// ==================================================

static PatchData_t *JKG_PlacePatch( int type, unsigned int address, unsigned int destination ) {
	PatchData_t *patch = malloc(sizeof(PatchData_t));

	int addr = address;
	int sz = 0;
	// Disassemble the code and determine the size of the code we have to replace
	ud_t ud;

	ud_init(&ud);
	ud_set_input_buffer(&ud, (uint8_t *)address, 48);
	ud_set_mode(&ud, 32);
	ud_set_pc(&ud, address);
	ud_set_syntax(&ud, NULL);

	while (ud_disassemble(&ud))
	{
		sz += ud_insn_len(&ud);
		if (sz >= 5) {
			break;
		}
	}

	assert(sz >= 5);

	if (sz < 5 || sz > 24) {
		// This really shouldnt ever happen, in the worst case scenario,
		// the block is 20 bytes (4 + 16), so if we hit 24, something went wrong
		return NULL;
	}

	patch->addr = address;
	patch->size = sz;
	memcpy(patch->origbytes, (const void *)address, sz);
	UnlockMemory(address, sz); // Make the memory writable
	*(unsigned char *)address = type == PATCH_JUMP ? 0xE9 : 0xE8;
	*(unsigned int *)(address+1) = destination - (address + 5);
	memset((void *)(address+5),0x90,sz-5);	// Nop the rest
	LockMemory(address, sz);
	return patch;
}

static void JKG_RemovePatch(PatchData_t **patch) {
	if (!*patch)
		return;
	UnlockMemory((*patch)->addr, (*patch)->size);
	memcpy((void *)(*patch)->addr, (*patch)->origbytes, (*patch)->size);
	LockMemory((*patch)->addr, (*patch)->size);
	*patch = 0;
}

// =================================================
// Hook 1:
// JKG Encrypted BSP support
// -------------------------------------------------
// This hook will facilitate the loading of 
// specially encrypted BSP's for JKG.
// This encryption is done to avoid ripping of the
// maps. The decryption is done right before the
// version check.
//
// NOTE: The client-side has 2 places where maps are
//       loaded! CM_LoadMap and RE_LoadWorldMap
//
// Hooks located in CM_LoadMap and RE_LoadWorldMap
// =================================================

// Define windows symbols
#define _BSPDH_PATCHPOS_CM			0x428666
#define _BSPDH_RETPOS_CM			0x42866D

#define _BSPDH_PATCHPOS_RE			0x4910E2
#define _BSPDH_RETPOS_RE			0x4910EA

static PatchData_t *pBSPDH_CM;
static PatchData_t *pBSPDH_RE;

int JKG_DecodeBSP(void *bspheader);

static void __declspec(naked) _Hook_BSPDecryptor_CM()
{
	__asm {
		pushad
		lea		eax, [esp+0x38]
		push	eax
		call	JKG_DecodeBSP
		add		esp, 4
		test	eax, eax
		jne		done
		mov		[esp+0x3C],	2
done:
		popad
		mov		esi, [esp+0x1C]
		cmp		esi, 1
		push	_BSPDH_RETPOS_CM
		ret
	}
}

static void __declspec(naked) _Hook_BSPDecryptor_RE()
{
	__asm {
		pushad
		push	esi
		call	JKG_DecodeBSP
		pop		esi
		test	eax, eax
		jne		done
		mov		[esp+4],	2
done:
		popad
		mov		eax, [esi+4]
		xor		edi, edi
		cmp		eax, 1 
		push	_BSPDH_RETPOS_RE
		ret
	}
}

void HRT_Start(int TimerID);
void HRT_Stop(int TimerID);
double HRT_GetTimingMS(int TimerID);
static PatchData_t *pSLT;

static void Hook_ShaderLoadInit() {
	HRT_Start(50);
}

static void Hook_ShaderLoadFinish(const char *name) {
	HRT_Stop(50);
	Com_DPrintf("Shader %s took %.3f ms to load\n", name, HRT_GetTimingMS(50));
}

static void __declspec(naked) _Hook_ShaderLoadTime() {
	__asm {
		pushad						// Secure registers
		call Hook_ShaderLoadInit	// Call Init to store the current time
		popad						// Restore registers
		add esp, 4					// Wipe return address
		push retaddr				// Put in a new one
		push 0x4B5F00				// Push-ret jump to R_FindShader
		ret
retaddr:							// <-- Forged return address
		pushad						// Secure registers
		push [esp+0x20]				// Since we 'returned', the shader name is at [esp] now
		call Hook_ShaderLoadFinish	// Call finish to display the name and time
		add esp, 4					// Clear our first arg
		popad						// Restore registers
		push 0x48D20E				// Push-ret to return address (the one we wiped earlier)
		ret
	}
}

// Hook 3: CL_CharEvent fix
// The engine doesn't forward char events to cgame, while it DOES send em to UI..
// Which makes no sense -.-.. so yea..fixin that :P

static PatchData_t *pCEF;

static void __declspec(naked) _Hook_CharFix() {
	__asm {
		// Kinda rare, but this time we dont need to secure registers :o
		test al, 0x8
		jz cont
		mov eax, DWORD PTR DS:[0x8AF0FC]		// cgvm
		push 1
		or ebx, 0x400
		push ebx
		push 6
		mov ebx, 0x44AE90			// VM_Call
		call ebx
		add esp, 0x0c
		pop esi
		pop ebx
		ret
cont:
		cmp DWORD PTR DS:[0x8AF100], 1			// cls.state
		push 0x41C68E
		ret
	}
}

int CIN_PlayCinematic(const char *filename, int x, int y, int w, int h, int systemBits);
// CIN Play hook
static PatchData_t *pCIN_P;
static void __declspec(naked) _Hook_CIN_Play() {
	__asm {
		push esp			// Store the current esp (also used for the return value later on)
		pushad
		mov ebp, [esp+0x20]	// Obtain the esp we pushed before
		// Push all args so we can forward the call
		push [ebp+0x12C]	// Push systemBits
		push [ebp+0x128]	// Push H
		push [ebp+0x124]	// Push W
		push [ebp+0x120]	// Push Y
		push [ebp+0x11c]	// Push X
		lea eax, [ebp+0x14]
		push eax			// Push filename (processed)
		call CIN_PlayCinematic
		add esp, 0x18
		cmp eax, -2
		je resumeload
		mov [esp+0x20], eax // Store our return value
		popad				// Restore our registers
		pop eax				// Get our return value back
		// Clean up and exit the function
		pop edi
		pop esi
		pop ebp
		pop ebx
		add esp, 0x104
		ret
resumeload:
		popad				// Restore registers
		add esp, 4			// Wipe the push esp we did before
		mov DWORD PTR DS:[EAX + 0x577BB0], 0
		push 0x416814		// Push-ret return to loader
		ret
	}
}

int CIN_OGMShutdown( void );
// CIN Shutdown hook
static PatchData_t *pCIN_S;
static void __declspec(naked) _Hook_CIN_Shutdown() {
	__asm {
		// Just forward the function to our one, and resume if it says so
		pushad
		call CIN_OGMShutdown
		test eax,eax
		je resumefunc
		popad
		ret
resumefunc:
		popad
		mov eax, DWORD PTR DS:[0x571FDC]
		push 0x416255
		ret
	}
}


int CIN_OGMRunCinematic( void );
// CIN Run hook
static PatchData_t *pCIN_R;
static void __declspec(naked) _Hook_CIN_Run() {
	__asm {
		pushad
		call CIN_OGMRunCinematic
		cmp eax, -1
		je resumefunc
		popad
		pop edi
		pop esi
		add esp, 8
		ret
resumefunc:
		popad
		test BYTE PTR DS:[0x12DD9D4], 1
		push 0x416463
		ret
	}
}

typedef void * unzFile;
void ProcessFileRead(unzFile unz);

// Zip file read hook
static PatchData_t *pZFR;
static void __declspec(naked) _Hook_unz_read() {
	__asm {
		pushad
		mov eax, [esp+0x38]
		push eax
		call ProcessFileRead
		add esp,4
		popad
		mov ecx, [edi+0x30]
		mov eax, [edi+0x4C]
		push 0x44A0D2
		ret
	}
}

void (* ScanAndLoadShaderFiles)(const char *shaderdir) = (void (*)(const char*))0x4B64C0;

void Z_Free(void *mptr) {
	__asm {
		mov eax, mptr
		mov ebx, 0x44e7b0
		call ebx
	}
}


static const char *cvarname001 = "cl_avidemo";
// Demo Playback Extension hooks
static PatchData_t *pSSPH1;
static PatchData_t *pSSPH2;
int Cvar_GetValueInt(const char *cvarname);

static void __declspec(naked) _Hook_ScreenshotPrint() {
	__asm {
		pushad
		push cvarname001
		call Cvar_GetValueInt
		add esp,4
		test eax,eax
		popad
		jne skip
		push 0x437080		// Com_Printf
skip:
		ret
	}
}

void GL_DPE_TakeDemoShot();
// Demo screenshotting hook
static PatchData_t *pDSSH;
static void __declspec(naked) _Hook_DemoScreenshot() {
	__asm {
		call GL_DPE_TakeDemoShot
		push 0x41F1C3		// Return point
		ret
	}
}



// Challenge xormask hook
// Applied in CL_ConnectionlessPacket (0x41EB0B)
// Redirects an atoi call

/* [ DEPRECATED ]
static PatchData_t *pCXMH;
static void __declspec(naked) _Hook_ChallengeXormask() {
	__asm {
		push [esp+4]
		lea eax, [fakeret]
		push eax
		push 0x52A209
		ret
fakeret:
		add esp, 4
		xor eax, JKG_CHALLENGE_KEY
		ret
	}
}
*/

// Challenge modification hooks (3 of em)
static PatchData_t *pCHINIT;
static PatchData_t *pCHSEND;
static PatchData_t *pCHRECV;

void CH_InitConnection();
static void __declspec(naked) _Hook_ChallengeInit() {
	__asm {
		pushad
		call CH_InitConnection
		popad
		mov dword ptr DS:[0x8AF104], esi
		push 0x41DB4F
		ret
	}
}

const char *CH_DoChallenge();
static void __declspec(naked) _Hook_ChallengeSend() {
	__asm {
		pushad
		call CH_DoChallenge
		test eax,eax
		jz skip				// Failsafe, in case the returned pointer is NULL
		mov [esp+0x3C], eax	// Overwrite the fmt argument
skip:
		popad
		push 0x443100		// Push-ret jump to NET_OutOfBandPrint
		ret
	}
}

int CH_ProcessChallengeResponse(unsigned int *challenge);
static void __declspec(naked) _Hook_ChallengeReceive() {
	__asm {
		sub esp,4
		pushad
		lea eax, [esp+0x20]
		push eax
		call CH_ProcessChallengeResponse
		add esp, 4
		test eax,eax
		popad
		pop eax
		jz bail
		sub esp, 4
		push 0x41EB10		// Push-ret jump back
		ret
bail:
		push 0x41EADF		// Jump to error com_printf
		ret
	}
}

// RE_StretchRaw backend sync fix

static PatchData_t *pSRSF;
static void __declspec(naked) _Hook_StretchRawSyncFix() {
	__asm {
		mov eax, 0x4AF530		// Call RB_EndSurface
		call eax
		mov eax, 0x48C0F0		// Call RB_ExecuteRenderCommands
		call eax
		ret
	}
}

// Clear the backbuffer if gamestate != CA_ACTIVE (r_clear handles that)

static PatchData_t *pCB;
static void __declspec(naked) _Hook_ClearBuffer() {
	__asm {

		push 0x3F800000
		push 0
		push 0
		push 0
		call DS:[0x010BCAF4]	// qglClearColor(0,0,0,1)

		push    0x4100
		call DS:[0x010BCAFC]	// qglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

		push 0x422EED
		ret

	}
}

void JKG_PatchEngine() {
	int temp;

	Com_Printf(" ------- Installing Engine Patches (Auxiliary library) -------- \n");
	///////////////////////////////
	// Hook 1: BSP Decryption
	///////////////////////////////
	pBSPDH_CM = JKG_PlacePatch(PATCH_JUMP, _BSPDH_PATCHPOS_CM, (unsigned int)_Hook_BSPDecryptor_CM);
	pBSPDH_RE = JKG_PlacePatch(PATCH_JUMP, _BSPDH_PATCHPOS_RE, (unsigned int)_Hook_BSPDecryptor_RE);
#ifndef NDEBUG
	if (!pBSPDH_CM || !pBSPDH_RE) {
		Com_Printf("Warning: Failed to place hook 1: BSP Decryption\n");
    }
#endif
	pSLT = JKG_PlacePatch(PATCH_CALL, 0x48D209, (unsigned int)_Hook_ShaderLoadTime);
#ifndef NDEBUG
	if (!pSLT ) {
		Com_Printf("Warning: Failed to place hook 2: Shader load timing info\n");
    }
#endif
	///////////////////////////////
	// Hook 3: CL_CharEvent fix
	///////////////////////////////
	pCEF = JKG_PlacePatch(PATCH_JUMP, 0x41C687, (unsigned int)_Hook_CharFix);
#ifndef NDEBUG
	if (!pCEF ) {
		Com_Printf("Warning: Failed to place hook 3: CL_CharEvent fix\n");
    }
#endif
	///////////////////////////////
	// Hook 4: CIN_PlayCinematic hook
	///////////////////////////////
	pCIN_P = JKG_PlacePatch(PATCH_JUMP, 0x41680A, (unsigned int)_Hook_CIN_Play);
#ifndef NDEBUG
	if (!pCIN_P ) {
		Com_Printf("Warning: Failed to place hook 4: CIN_PlayCinematic hook\n");
    }
#endif
	///////////////////////////////
	// Hook 5: RoQShutdown hook
	///////////////////////////////
	pCIN_S = JKG_PlacePatch(PATCH_JUMP, 0x416250, (unsigned int)_Hook_CIN_Shutdown);
#ifndef NDEBUG
	if (!pCIN_S ) {
		Com_Printf("Warning: Failed to place hook 5: RoQShutdown hook\n");
    }
#endif
	///////////////////////////////
	// Hook 6: CIN_RunCinematic hook
	///////////////////////////////
	pCIN_R = JKG_PlacePatch(PATCH_JUMP, 0x41645C, (unsigned int)_Hook_CIN_Run);
#ifndef NDEBUG
	if (!pCIN_R ) {
		Com_Printf("Warning: Failed to place hook 6: CIN_RunCinematic hook\n");
    }
#endif
	///////////////////////////////
	// Hook 7: unzReadCurrentFile hook
	///////////////////////////////
	pZFR = JKG_PlacePatch(PATCH_JUMP, 0x44A0CC, (unsigned int)_Hook_unz_read);
#ifndef NDEBUG
	if (!pZFR ) {
		Com_Printf("Warning: Failed to place hook 7: unzReadCurrentFile hook\n");
    }
#endif
	// To ensure the shaders are properly loaded, we'll call the load function again
	// NOTE: Free the old alloc's first or we're gonna be leaking memory
	if (*(unsigned int *)0x10746B8) {
		Z_Free(*(void **)0x10746B8);
	}
	if (*(unsigned int *)0x10DDD20) {
		Z_Free(*(void **)0x10DDD20);
	}
	ScanAndLoadShaderFiles("shaders");
	
	///////////////////////////////
	// Patch 7: Console version number
	///////////////////////////////
	UnlockMemory(0x417E48, 0x20);
	*(unsigned int *)0x417E59 = (unsigned int)consoleversion;
	temp = strlen(consoleversion) * -8;
	*(int *)0x417E49 = temp;
	LockMemory(0x417E48, 0x20);

	///////////////////////////////
	// Hook 8: Demo playback extention: Screenshot Echo Supression Hook
	///////////////////////////////
	pSSPH1 = JKG_PlacePatch(PATCH_CALL, 0x4A18CE, (unsigned int)_Hook_ScreenshotPrint);  // tga
	pSSPH2 = JKG_PlacePatch(PATCH_CALL, 0x4A1A2F, (unsigned int)_Hook_ScreenshotPrint);  // jpg
#ifndef NDEBUG
	if (!pSSPH1 || !pSSPH2 ) {
		Com_Printf("Warning: Failed to place hook 8: Screenshot Echo Supression Hook\n");
    }
#endif


	///////////////////////////////
	// Hook 9: Demo screenshotting hook
	///////////////////////////////
	pDSSH = JKG_PlacePatch(PATCH_JUMP, 0x41F169, (unsigned int)_Hook_DemoScreenshot); 
#ifndef NDEBUG
	if (!pDSSH ) {
		Com_Printf("Warning: Failed to place hook 9: Demo screenshotting hook\n");
    }
#endif

	///////////////////////////////
	// Hook 10: Challenge xormask hook  [ DEPRECATED ]
	///////////////////////////////
/*	pCXMH = JKG_PlacePatch(PATCH_CALL, 0x41EB0B, (unsigned int)_Hook_ChallengeXormask); 
#ifndef NDEBUG
	if (!pCXMH ) {
		Com_Printf("Warning: Failed to place hook 10: Challenge xormask hook\n");
    }
#endif*/

	///////////////////////////////
	// Hooks 10-12: Challenge modifications
	///////////////////////////////

	pCHINIT = JKG_PlacePatch(PATCH_JUMP, 0x41DB49, (unsigned int)_Hook_ChallengeInit);
	pCHSEND = JKG_PlacePatch(PATCH_CALL, 0x41E4BE, (unsigned int)_Hook_ChallengeSend);
	pCHRECV = JKG_PlacePatch(PATCH_JUMP, 0x41EAF7, (unsigned int)_Hook_ChallengeReceive);
#ifndef NDEBUG
	if (!pCHINIT || !pCHSEND || !pCHRECV) {
		Com_Printf("Warning: Failed to place hooks 10-12: Challenge modifications\n");
    }
#endif

	pSRSF = JKG_PlacePatch(PATCH_CALL, 0x48B1C7, (unsigned int)_Hook_StretchRawSyncFix);
#ifndef NDEBUG
	if (!pSRSF) {
		Com_Printf("Warning: Failed to place hook 13: StretchRaw Sync fix\n");
    }
#endif

#if 0
	// Old Alt+Enter patch, allows fullscreen toggle if cls.state == CA_DISCONNECTED || cls.state == CA_CONNECTED
	//OutputDebugString( "Patching alt-enter support\n" );
	UnlockMemory( 0x454B59, 0x10 );
	//*(unsigned short *)0x454B61 = 0x9090;
	*(unsigned char *)0x454B59 = 0x08;
	LockMemory( 0x454B59, 0x10 );
#else
	// New Alt+Enter patch, allows fullscreen toggle if cls.state >= CA_DISCONNECTED && cls.state <= CA_ACTIVE
	// This allows toggling fullscreen during server connecting, but it won't affect the loading since the
	// vid_restart call will be delayed until the client command buffer can be used.
	UnlockMemory( 0x454B55, 0x07 );
	*(unsigned char *)0x454B55 = 0x7C; // jl short 0x454B95 (no toggle if cls.state < CA_DISCONNECTED)
	*(unsigned char *)0x454B56 = 0x3E;
	*(unsigned char *)0x454B57 = 0x83; // cmp ecx, CA_ACTIVE (ecx = cls.state)
	*(unsigned char *)0x454B58 = 0xF9;
	*(unsigned char *)0x454B59 = 0x08;
	*(unsigned char *)0x454B5A = 0x7F; // jg short 0x454B95 (no toggle if cls.state > CA_ACTIVE)
	*(unsigned char *)0x454B5B = 0x39;
	// The player will be disallowed from toggling fullscreen when viewing a cinematic (CA_CINEMATIC:0x09)
	// or if the client is not initialized (CA_UNINITIALIZED:0x00)
	LockMemory( 0x454B55, 0x07 );
#endif

	
	pCB = JKG_PlacePatch(PATCH_JUMP, 0x422E85, (unsigned int)_Hook_ClearBuffer);
#ifndef NDEBUG
	if (!pCB) {
		Com_Printf("Warning: Failed to place hook 14: Buffer clear fix\n");
    }
#endif

	Com_Printf("Finished\n");
	
}