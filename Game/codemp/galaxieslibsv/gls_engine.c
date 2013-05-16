///////////////////////////////////////////////////////////////////
//
//  Engine alteration (server-side auxiliary)
//
//  All engine modifications that are to be present at run-time
//  go in here
//
//  By BobaFett
//
///////////////////////////////////////////////////////////////////
//  Windows and linux compatible

#include <libudis86/udis86.h>
#include <assert.h>

#ifdef _WIN32
	#include <windows.h>
	#define NOINLINE
#else
	#include <sys/mman.h>
	#include <unistd.h>
	#include <string.h>
    #include <stdlib.h>
    typedef unsigned char byte;
	#define _stricmp strcasecmp
	#define NOINLINE __attribute__((noinline))
#endif

#include "gls_enginefuncs.h"

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
#ifdef _WIN32
	ret = VirtualProtect((LPVOID)address, size, PAGE_EXECUTE_READWRITE, (PDWORD)&dummy);
	return (ret != 0);
#else
	// Linux is a bit more tricky
	int page1, page2;
	page1 = address & ~( getpagesize() - 1);
	page2 = (address+size) & ~( getpagesize() - 1);
	if( page1 == page2 ) {
		ret = mprotect((char *)page1, getpagesize(), PROT_READ | PROT_WRITE | PROT_EXEC);
		return (ret == 0);
	} else {
		ret = mprotect((char *)page1, getpagesize(), PROT_READ | PROT_WRITE | PROT_EXEC);
		if (ret) return 0;
		ret = mprotect((char *)page2, getpagesize(), PROT_READ | PROT_WRITE | PROT_EXEC);
		return (ret == 0);
	}
#endif
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
#ifdef _WIN32
	ret = VirtualProtect((LPVOID)address, size, PAGE_EXECUTE_READ, NULL);
	return (ret != 0);
#else
	// Linux is a bit more tricky
	int page1, page2;
	page1 = address & ~( getpagesize() - 1);
	page2 = (address+size) & ~( getpagesize() - 1);
	if( page1 == page2 ) {
		ret = mprotect((char *)page1, getpagesize(), PROT_READ | PROT_EXEC);
		return (ret == 0);
	} else {
		ret = mprotect((char *)page1, getpagesize(), PROT_READ | PROT_EXEC);
		if (ret) return 0;
		ret = mprotect((char *)page2, getpagesize(), PROT_READ | PROT_EXEC);
		return (ret == 0);
	}
#endif
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
	//int addr = address;
	int sz = 0;

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

// ==================================================
// Shell System for Hooks (By BobaFett & Deathspike)
// --------------------------------------------------
// This system, that I (BobaFett) call:
// The Shell System, allows for hooks to be created
// on both windows and linux with exactly the same code
// (OS specific asm changes not included)
//
// The system works as follows:
// Since compilers have the tendancy to add prologue
// and epilogue code to functions, we put our asm
// inside a safe 'shell'
// The shell is defined by:
//
// void *MyHook()
// {
//		__JKG_StartHook;			<-- Shell
//		{
//			// Hook code here		<-- Contents of shell
//		}
//		__JKG_EndHook;				<-- Shell
// }
//
// This code should be placed in a function returning
// a void *, as shown above.
// When called, it will return the pointer to the
// shell's contents, which can then be used to place
// hooks (ie. jumps).
//
// Note that the shell's contents (the hook in question)
// are not executed!
//
// For the actual asm, 3 defines are available:
// __asm1__ for zero/single operand opcodes	(push 10	)
// __asm2__ for dual operand opcodes		(mov eax, 1	)
// __asmL__ for labels						(mylabel:	)
//
// To compile this code on linux, you require the
// following in the gcc command line:
//  -masm=intel
// 
// NOTE: The hook's execution flow must NEVER get to
//       the shell layer! Always ensure the code is
//       ended with a jump or a return!
//
// ==================================================

#include "asmdefines.h"
	

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
// Hook located in CM_LoadMap
// =================================================

#ifdef __linux__
// Define linux symbols
#define _BSPDH_PATCHPOS			0x805BAC9
#define _BSPDH_RETPOS			0x805BAD2

#else
// Define windows symbols
#define _BSPDH_PATCHPOS			0x4041E6
#define _BSPDH_RETPOS			0x4041ED

#endif


static PatchData_t *pBSPDH;

int JKG_DecodeBSP(void *bspheader);

static NOINLINE void *_Hook_BSPDecryptor()
{
	__JKG_StartHook(BSPDecryptor);
	{
		__asm1__(	pushad						);
#ifdef _WIN32
		__asm2__(	lea		eax, [esp+0x34]		);
#else
		__asm2__(	lea		eax, [ebp-0xA8]		);
#endif
		__asm1__(	push	eax					);
		__asm1__(	call	JKG_DecodeBSP		);
		__asm2__(	add		esp, 4				);
		__asm2__(	test	eax, eax			);
		__asm1__(	jne		done				);
		__asm2__(	mov	DWORD PTR [esp+0x38], 2		);
		__asmL__( done:							);
		__asm1__(	popad						);
#ifdef _WIN32
		__asm2__(	mov		esi, [esp+0x18]		);
		__asm2__(	cmp		esi, 1				);
#else
		__asm2__(	mov		eax, [ebp-0xA4]		);
		__asm2__(	cmp		eax, 1				);
#endif
		__asm1__(	push	_BSPDH_RETPOS		);
		__asm1__(	ret							);
	}
	__JKG_EndHook(BSPDecryptor);
}

// =================================================
// Hook 2:
// Extended shutdown information
// -------------------------------------------------
// Normally, when the server shuts down, the client
// only receives 'Server disconnected'.
// This hook will include additional information
// as to why the server disconnected
// =================================================

#ifdef __linux__
// Define linux symbols
#define _SDM_PATCHPOS		0x8055E8B
#define _SDM_RETPOS			0x8055E93

#else
// Define windows symbols
#define _SDM_PATCHPOS		0x4433CA
#define _SDM_RETPOS			0x4433CF

#endif

extern int bCrashing;

static PatchData_t *pSDM;

const char* JKG_ShutdownMessage(void);

static NOINLINE void *_Hook_QuitMsg()
{
	__JKG_StartHook(QuitMsg);
	{
#ifdef _WIN32
		__asm2__(	sub		esp, 4				);
#endif
		__asm1__(	pushad						);
		__asm1__(	call	JKG_ShutdownMessage	);
#ifdef _WIN32		
		__asm2__(	mov		[esp+0x20], eax		);
#else
		__asm2__(	mov		[esp+0x24], eax		);
#endif
		__asm1__(	popad						);
		__asm1__(	push	_SDM_RETPOS			);
		__asm1__(	ret							);
	}
	__JKG_EndHook(QuitMsg);
}

const char* JKG_ShutdownMessage(void) {
	if (bCrashing) {
		return "disconnect \"\nThe server has encountered a technical problem and has been shut down\n\nOur apologies for the inconvenience\n\"";
	} else {
		return "disconnect \"\nThe server has been manually shut down\n\"";
	}
}


// =================================================
// Hook 3:
// Stress level indicator
// -------------------------------------------------
// These 2 hooks are placed inside SV_Frame to
// get time measurements used for the stress level
// indicator
// =================================================

#include "gls_hrtimer.h"

#ifdef __linux__
// Define linux symbols
#define _SLIs_PATCHPOS		0x8057924
#define _SLIs_RETPOS		0x805792A

#define _SLIe_PATCHPOS		0x8057A45
#define _SLIe_RETPOS		0x8057A4B

#else
// Define windows symbols
#define _SLIs_PATCHPOS		0x444663
#define _SLIs_RETPOS		0x444669

#define _SLIe_PATCHPOS		0x4447E2
//#define _SLIe_RETPOS		<no return pos>

#endif


static PatchData_t *pSLIs;
static PatchData_t *pSLIe;

void JKG_StressLvlStart(void);
void JKG_StressLvlEnd(void);

static NOINLINE void *_Hook_StressLevel_Start()
{
	__JKG_StartHook(StressLevel_Start);
	{
		__asm1__(	pushad						);
		__asm1__(	call	JKG_StressLvlStart	);
		__asm1__(	popad						);
#ifdef _WIN32
		__asm2__(	mov ecx, DS:[0x60621C]		);
#else
		__asm2__(	mov ecx, [0x83121E4]		);
#endif

		__asm1__(	push	_SLIs_RETPOS		);
		__asm1__(	ret							);
	}
	__JKG_EndHook(StressLevel_Start);
}

static void *_Hook_StressLevel_End()
{
	__JKG_StartHook(StressLevel_End);
	{
		__asm1__(	pushad						);
#ifdef _WIN32
		__asm2__(	mov eax, 0x4436A0			);
		__asm1__(	call eax					);
#endif
		__asm1__(	call	JKG_StressLvlEnd	);
		__asm1__(	popad						);
#ifdef __linux__
		__asm2__(	mov	esi, [ebp-8]			);
		__asm1__(	push	_SLIe_RETPOS		);
#else
		__asm1__(	ret							);
#endif
	}
	__JKG_EndHook(StressLevel_End);
}

void JKG_StressLvlStart(void) {
	HRT_Start(1);
}

void GLS_StressAddTime(double frametimeMs);

void JKG_StressLvlEnd(void) {
	HRT_Stop(1);
	GLS_StressAddTime(HRT_GetTimingMS(1));
}

/*  [[ DEPRECATED ]]
// =================================================
// Hook 4:
// Challenge xormask hook
// -------------------------------------------------
// For increased protection, the challenge keys
// are encoded using a xormask
//
// Patch is located in SV_GetChallenge, overwriting
// 1 or 2 calls to NET_OutOfBandPrint
// =================================================


#ifdef __linux__
// Define linux symbols
#define _CXM_PATCHPOS1		0x804BB4A
#define _CXM_PATCHPOS2		0x804BBA1
#define _CXM_RETPOS			0x807B744

#else
// Define windows symbols
#define _CXM_PATCHPOS	0x43AB8C
#define _CXM_RETPOS		0x41A230

#endif


static PatchData_t *pCXMP1;
#ifdef __linux__
static PatchData_t *pCXMP2;
#endif

static void *_Hook_ChallengeHook()
{
	__JKG_StartHook(4);
	{
		__asm2__(	xor dword ptr  [esp+0x20] , JKG_CHALLENGE_KEY		);
		__asm1__(	push _CXM_RETPOS									);
		__asm1__(	ret													);
		
	}
	__JKG_EndHook(4);
}*/


// =================================================
// Hook 4:
// Challenge modifications
// -------------------------------------------------
// For increased protection, the challenge keys
// are replaced by the Diffie-Hellman Key Exchange algo
//
// Patch is located in SV_GetChallenge, overwriting
// 1 or 2 calls to NET_OutOfBandPrint
// =================================================


#ifdef __linux__
// Define linux symbols
#define _CXM_PATCHPOS1		0x804BB4A
#define _CXM_PATCHPOS2		0x804BBA1
#define _CXM_RETPOS			0x807B744

#else
// Define windows symbols
#define _CXM_PATCHPOS	0x43AB8C
#define _CXM_RETPOS		0x41A230

#endif


static PatchData_t *pCXMP1;
#ifdef __linux__
static PatchData_t *pCXMP2;
#endif

const char *CH_ProcessChallengeRequest(void *challenge);
static NOINLINE void *_Hook_ChallengeHook()
{
	__JKG_StartHook(ChallengeHook);
	{
		__asm1__(	pushad												);
#ifdef __linux__
		__asm1__(	push edi											);
#else
		__asm1__(	push ebx											);
#endif
		__asm1__(	call CH_ProcessChallengeRequest						);
		__asm2__(	add esp, 4											);
		__asm2__(	test eax,eax										);
		__asm1__(	je chskip												);
		__asm2__(	mov [esp+0x3C], eax									);
		__asmL__(chskip:													);
		__asm1__(	popad												);
		__asm1__(	push _CXM_RETPOS									);
		__asm1__(	ret													);
		
	}
	__JKG_EndHook(ChallengeHook);
}

#ifdef _WIN32	// Console event handler
static void (* Sys_Quit)(void) = (void (*) (void))0x410CB0;
static int CtrlAck = 0;
static void (* CBuf_AddText)(const char *cmd) = (void (*)(const char *))0x40F2B0;

BOOL CtrlHandler( DWORD fdwCtrlType ) {
	if (CtrlAck) {	// Avoid recursive use
		return 1;
	}
	CtrlAck = 1;
	switch( fdwCtrlType ) 
	{ 
	case CTRL_C_EVENT: 
		Com_Printf("Ctrl-C was used, shutting down server\n");
		break;
	case CTRL_CLOSE_EVENT: 
		Com_Printf("Server window was closed, shutting down server...\n");
		break;
	case CTRL_BREAK_EVENT: 
		Com_Printf("Ctrl-Break was used, shutting down server\n");
		break;
	case CTRL_LOGOFF_EVENT: 
		Com_Printf("User signing off, shutting down server...\n");
		break;
	case CTRL_SHUTDOWN_EVENT: 
		Com_Printf("Operating system shutting down, shutting down server...\n");
		break;
	default: 
		break;
	} 
	
	CBuf_AddText("quit");

	//Sys_Quit();
	return 1;
}

#endif


void JKG_PatchEngine() {
	Com_Printf(" ------- Installing Engine Patches (Auxiliary library) -------- \n");

	///////////////////////////////
	// Hook 1: BSP Decryption
	///////////////////////////////
	pBSPDH = JKG_PlacePatch(PATCH_JUMP, _BSPDH_PATCHPOS, (unsigned int)_Hook_BSPDecryptor());
    if (!pBSPDH) {
		Com_Printf("Warning: Failed to place hook 1: BSP Decryption\n");
    }

	pSDM = JKG_PlacePatch(PATCH_JUMP, _SDM_PATCHPOS, (unsigned int)_Hook_QuitMsg());
    if (!pSDM) {
		Com_Printf("Warning: Failed to place hook 2: Shutdown Messages\n");
    }

	pSLIs = JKG_PlacePatch(PATCH_JUMP, _SLIs_PATCHPOS, (unsigned int)_Hook_StressLevel_Start());
	pSLIe = JKG_PlacePatch(PATCH_JUMP, _SLIe_PATCHPOS, (unsigned int)_Hook_StressLevel_End());
	if (!pSLIs || !pSLIe) {
		Com_Printf("Warning: Failed to place hook 3: Server stress indicator\n");
    }

#ifdef __linux__
	pCXMP1 = JKG_PlacePatch(PATCH_CALL, _CXM_PATCHPOS1, (unsigned int)_Hook_ChallengeHook());
	pCXMP2 = JKG_PlacePatch(PATCH_CALL, _CXM_PATCHPOS2, (unsigned int)_Hook_ChallengeHook());
	if (!pCXMP1 || !pCXMP2) {
#else
	pCXMP1 = JKG_PlacePatch(PATCH_CALL, _CXM_PATCHPOS, (unsigned int)_Hook_ChallengeHook());
	if (!pCXMP1) {
#endif
		Com_Printf("Warning: Failed to place hook 4: Challenge hook\n");
    }

	// If we're on windows, enable the console event handler

	// Raise delta compressor's bit size of ps.stats[STAT_WEAPON] to 32
#ifdef __linux__
	UnlockMemory(0x807A439, 4);
	*(unsigned char *)0x807A439 = 32;
	LockMemory(0x807A439, 4);
#else
	UnlockMemory(0x419AB0, 1);
	*(unsigned char *)0x419AB0 = 32;
	LockMemory(0x419AB0, 1);
#endif

	// Raise delta compressor's settings for ammo transmission (from 16 slots of 16 bits, to 19 slots of 32 bits)
#ifdef __linux__
//#error Ammo transmission patch not yet compatible with linux
#else
	//UnlockMemory(0x41992F, 610);
	//*(unsigned char *)0x41992F = 19;
	//*(unsigned char *)0x419B80 = 32;
	//*(unsigned char *)0x419B91 = 19;
	//LockMemory(0x419AB0, 610);
#endif

#ifdef _WIN32
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, 1);


	{
		HWND hwnd = GetConsoleWindow();
		HMENU hmenu = GetSystemMenu(hwnd, FALSE);
		DeleteMenu(hmenu, SC_CLOSE, MF_BYCOMMAND);
		//ModifyMenu(hmenu, SC_CLOSE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED, SC_CLOSE, NULL);
	}

#endif

	Com_Printf("Finished\n");
}
