///////////////////////////////////////////////////////////////////
//
//  Engine alteration
//
//  All engine modifications that are to be present at run-time
//  go in here
//
//  By BobaFett
//
///////////////////////////////////////////////////////////////////
//  Windows and linux compatible

#include <libudis86/udis86.h>
#include "g_engine.h"
#undef INFINITE

#ifdef _WIN32
	#include <windows.h>
#else
	#include <sys/mman.h>
	#include <unistd.h>
	#include <string.h>
    #include <stdlib.h>
    //typedef unsigned char byte;
	#define _stricmp strcasecmp
#endif

#ifdef _WIN32
// Windows defs here
static void (*NET_OutOfBandPrint)( int sock, netadr_t adr, const char *format, ... ) = (void (*)(int, netadr_t, const char*,...))0x41A230;
static void (*Com_Printf2)( const char *fmt, ... ) = (void (*)(const char *, ...))0x40FBE0;
static void (*Com_DPrintf)( const char *fmt, ... ) = (void (*)(const char *, ...))0x40FDB0;
static const char *(*Cmd_Argv)( int arg ) = (const char *(*)(int))0x40F490;
static void (*SV_SendServerCommand)(client_t *cl, const char *fmt, ...) = (void(*)(client_t *cl, const char *fmt, ...))0x4435D0;
#else
// Linux defs here
static void (*NET_OutOfBandPrint)( int sock, netadr_t adr, const char *format, ... ) = (void (*)(int, netadr_t, const char*,...))0x807B744;
static void (*Com_Printf2)( const char *fmt, ... ) = (void (*)(const char *, ...))0x8072CA4;
static void (*Com_DPrintf)( const char *fmt, ... ) = (void (*)(const char *, ...))0x8072ED4;
static const char *(*Cmd_Argv)( int arg ) = (const char *(*)(int))0x812C264;
static void (*SV_SendServerCommand)(client_t *cl, const char *fmt, ...) = (void(*)(client_t *cl, const char *fmt, ...))0x8056214;

#endif

void Com_sprintf (char *dest, int size, const char *fmt, ...);

/*
static short   BigShort (short l)
{
	byte    b1,b2;

	b1 = l&255;
	b2 = (l>>8)&255;

	return (b1<<8) + b2;
}*/

static const char	*NET_AdrToString (netadr_t a)
{
	static	char	s[64];

	if (a.type == NA_LOOPBACK) {
		Com_sprintf (s, sizeof(s), "loopback");
	} else if (a.type == NA_BOT) {
		Com_sprintf (s, sizeof(s), "bot");
	} else if (a.type == NA_IP) {
		Com_sprintf (s, sizeof(s), "%i.%i.%i.%i:%hu",
			a.ip[0], a.ip[1], a.ip[2], a.ip[3], BigShort(a.port));
	} else {
		Com_sprintf (s, sizeof(s), "%02x%02x%02x%02x.%02x%02x%02x%02x%02x%02x:%hu",
		a.ipx[0], a.ipx[1], a.ipx[2], a.ipx[3], a.ipx[4], a.ipx[5], a.ipx[6], a.ipx[7], a.ipx[8], a.ipx[9],
		BigShort(a.port));
	}

	return s;
}

typedef struct {
	int addr;
	int size;
	char origbytes[24];
} PatchData_t;

typedef enum {
	PATCH_JUMP,
	PATCH_CALL,
} PatchType_e;

extern vmCvar_t		jkg_antifakeplayer;

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
	ret = VirtualProtect((LPVOID)address, size, PAGE_EXECUTE_READWRITE, &dummy);
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
	free(*patch);
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
//		__JKG_StartHook(<ID>);		<-- Shell
//		{
//			// Hook code here		<-- Contents of shell
//		}
//		__JKG_EndHook(<ID>);		<-- Shell
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

#include "jkg_asmdefines.h"

// =================================================
// Hook 1:
// Extended protocol version check
// -------------------------------------------------
// To solve the issue with clients not having the
// clientsize (and q3fill attacks), the server uses
// a different protocol number: 89. And getinfo,
// shows protocol 27.
//
// So when a player connects, the actions are as follows:
//
// Protocol:	Action:
// 25			Update to JKA 1.01
// 26			Install Jedi Knight Galaxies
// 27			Permaban (only q3fill will do this)
// 89			Join
// Other		Wrong game detected
//
// The hook is placed in SV_DirectConnect
// The patch is inside SVC_Info
// =================================================
//
#ifdef __linux__
// Define linux symbols
#define _PVCHECK_INFOPROTOCOL 0x805681E
#define _PVCHECK_PATCHPOS 0x804C078
#define _PVCHECK_NETPOS [ebp+8]
#define _PVCHECK_RETOK 0x804C0DF
#define _PVCHECK_RETBAD 0x804C0D2
#define FUNC_USED __attribute__((used))
#else
// Define windows symbols
#define _PVCHECK_INFOPROTOCOL 0x443A8B
#define _PVCHECK_PATCHPOS 0x43C52F
#define _PVCHECK_NETPOS dword ptr [esp+0x51898]
#define _PVCHECK_RETOK 0x43C56E
#define _PVCHECK_RETBAD 0x43C563
#define FUNC_USED
#endif

static PatchData_t *pPVCheck;

static int JKG_CheckProtocol(const char *userinfo, netadr_t *addr);

static void *_Hook_ProtocolVersionCheck()
{
	__JKG_StartHook(0);
	{
		__asm2__(	lea		esi, _PVCHECK_NETPOS);	// Store netadr_t in esi, we'll need it later
		__asm1__(	pushad						);	// Secure registers
		__asm1__(	push	esi					);	// Push netadr ptr
		//__asm1__(	push	ebx					);	// Push provided protocol version
#ifdef _WIN32
		__asm2__(	lea	eax, [esp+0x40]			);	// Get unserinfo address
#else
		__asm2__(	lea	eax, [ebp+0x518A0]		);	// Get unserinfo address
#endif
		__asm1__(	push	eax					);	// Push userinfo
		__asm1__(	call	JKG_CheckProtocol	);	// Call JKG_CheckProtocol
		__asm2__(	add		esp, 8				);	// Clean up stack
		__asm2__(	test	eax, eax			);	// Check if the return value is 0
		__asm1__(	popad						);	// Restore registers
		__asm1__(	je		deny				);	// If return value is 0, go to deny
		__asm1__(	push	_PVCHECK_RETOK		);	// Push-ret jump to successful connection
		__asm1__(	ret							);	//  return point
		__asmL__( deny:							);
		__asm1__(	push	_PVCHECK_RETBAD		);	// Push-ret jump to denied connection
		__asm1__(	ret							);	//  return point
	}
	__JKG_EndHook(0);
}

static int FUNC_USED JKG_CheckProtocol(const char *userinfo, netadr_t *addr) {
	int version;
	int challenge;
	
	version = atoi(Info_ValueForKey(userinfo, "protocol"));
	challenge = atoi(Info_ValueForKey(userinfo, "challenge"));

	if (version == 25) {
		// v1.00 client
		NET_OutOfBandPrint(1,*addr,"print\nPlease update to JA v1.01.\n");
		Com_DPrintf("Rejecting client from %s: Uses v1.00\n", NET_AdrToString(*addr));
		return 0;
	} else if (version == 26) {
		// v1.01 non-jkg client
		NET_OutOfBandPrint(1,*addr,"print\nPlease download Jedi Knight Galaxies to play on this server\nVisit www.terrangaming.com/projects/jkg for more info.\n");
		Com_DPrintf("Rejecting client from %s: Does not have JKG installed\n", NET_AdrToString(*addr));
		return 0;
	} else if (version == 27) {
		// Hehehe, q3fill bait
		if (jkg_antifakeplayer.integer) {
			NET_OutOfBandPrint(1,*addr,"print\nFake clients are not allowed to join this server.\n");
			// This is rather important, so display regardless of developer mode
			Com_Printf2("Fake client detected from %s!\n", NET_AdrToString(*addr));
			// Do the actual banning here..
			return 0;
		} else {
			// :\....alright then.. let it in.., do give a debug warning though
			Com_DPrintf("Caution: Fake client joined - %s\n", NET_AdrToString(*addr));
			return 1;
		}
	} else if (version == 89) {
		// JKG client, check challenge and then allow entry
		if (challenge & 0x00080000) {
			// Valid key
			return 1;
		} else {
			NET_OutOfBandPrint(1,*addr,"print\nClient authentication failed.\n");
			Com_DPrintf("Rejecting client from %s: Invalid challenge\n", NET_AdrToString(*addr));
			return 0;
		}
	} else {
		// Wtf o.o different game
		NET_OutOfBandPrint(1,*addr,"print\nClient version unknown.\n");
		Com_DPrintf("Rejecting client from %s: Is not running Jedi Academy\n", NET_AdrToString(*addr));
		return 0;
	}
}

// =================================================
// Hook 2:
// Custom anti-q3infoboom patch
// -------------------------------------------------
// Because Luigi Auriemma's 'fix' has side-effects
// such as userinfo being cut off from the connect
// packet. JKG implements a custom protection
// against this issue.
//
// This will also undo the fix from Luigi
//
// The hook is placed in SV_ConnectionlessPacket
// The patch is inside MSG_ReadStringLine
// =================================================
//
#ifdef __linux__
// Define linux symbols
#define _IBFIX_MSGPATCH 0x807803D
#define _IBFIX_PATCHPOS 0x8056E23
#define _IBFIX_QSTRICMP 0x807F434
#else
// Define windows symbols
#define _IBFIX_MSGPATCH 0x418B2C
#define _IBFIX_PATCHPOS 0x443F7F
#define _IBFIX_QSTRICMP 0x41B8A0
#endif

static PatchData_t *pIBFix;

static void FUNC_USED JKG_CheckConnectionlessPacket(const char *cmd) {
	char *s;
	if (!_stricmp(cmd,"getstatus") || !_stricmp(cmd,"getinfo")) {
		// We got a risky function here, get arg 1 and do a cutoff if needed
		s = (char *)Cmd_Argv(1);
		if (strlen(s) > 32) {
			// POSSIBLE TODO: Add a check for malicious use and take action (ie. ban)
			s[32] = 0;	// 32 chars should be more than enough for the challenge number
		}
	} else if (!_stricmp(cmd,"connect")) {
		s = (char *)Cmd_Argv(1);
		if (strlen(s) > 980) {
			s[980] = 0;
		}

	}
}

static void *_Hook_InfoBoomFix()
{
	__JKG_StartHook(1);
	{
		__asm1__(	pushad									);	// Secure registers
		__asm1__(	push	ebx								);	// Secure registers
		__asm1__(	call	JKG_CheckConnectionlessPacket	);
		__asm2__(	add		esp, 4							);
		__asm1__(	popad									);
		__asm1__(	push	_IBFIX_QSTRICMP					);
		__asm1__(	ret										);
	}
	__JKG_EndHook(1);
}



// =================================================
// Hook 3:
// Cinematic PVS correction
// -------------------------------------------------
// Because cinematics take control of the camera
// but leave the player alone, the pvs wont be
// correct.
// This means that if the camera goes too far away
// from the player, entities wont show up.
// To fix this, we'll create a hook that overrides
// the pvs info server-side.
// If a player is in a cinematic, the server will
// call a framework function which estimates the
// position of the camera (using linear interp.)
//
// The hook is placed in SV_BuildClientSnapshot
// =================================================
//
#ifdef __linux__
// Define linux symbols
#define _CPVSFIX_REG edx
#define _CPVSFIX_PATCHPOS 0x8058AA8
#define _CPVSFIX_RETFUNC 0x8058404
#else
// Define windows symbols
#define _CPVSFIX_REG ecx
#define _CPVSFIX_PATCHPOS 0x4454D1
#define _CPVSFIX_RETFUNC 0x444F00
#endif

static PatchData_t *pCPVSFix;

static void JKG_CheckPVSCorrection(int clientNum, float *x, float *y, float *z);

static void *_Hook_PVSCorrection()
{
	__JKG_StartHook(2);
	{
		__asm1__(	pushad									);	// Secure registers
		__asm2__(	lea		eax, [_CPVSFIX_REG + 8]			);	// Push org[2]
		__asm1__(	push	eax								);	//
		__asm2__(	lea		eax, [_CPVSFIX_REG + 4]			);	// Push org[1]
		__asm1__(	push	eax								);	//
		__asm2__(	lea		eax, [_CPVSFIX_REG]				);	// Push org[0]
		__asm1__(	push	eax								);	//
#ifdef __linux													// This code gets the client num
		__asm2__(	mov		eax, [ebp+0x8]					);	// But since it differs per OS
		__asm2__(	mov		ebx, [ebp-0x14]					);	// We'll just ifdef it
		__asm2__(	mov		esi, [ebx+eax+0x209f4]	);
#else
		__asm2__(	mov		esi, dword ptr [ebx+0xD0]		);
#endif
		__asm1__(	push	esi								);	// Push clientNum
		__asm1__(	call	JKG_CheckPVSCorrection			);	// Call
		__asm2__(	add		esp, 0x10						);	// Clean up stack
		__asm1__(	popad									);	// Restore registers
		__asm1__(	push	_CPVSFIX_RETFUNC				);	// Push-ret jump to SV_AddEntitiesVisibleFromPoint
		__asm1__(	ret										);
	}
	__JKG_EndHook(2);
}

void GLua_GetCinematicCamPosition(int clientNum, float *x, float *y, float *z);

static void FUNC_USED JKG_CheckPVSCorrection(int clientNum, float *x, float *y, float *z) {
	// x y and z are the current coords used for PVS
	// In case an override is needed, just change them
	GLua_GetCinematicCamPosition(clientNum, x, y, z);
}

// =================================================
// Hook 4:
// Connection Activity Check
// -------------------------------------------------
// To verify if a client is valid, the server will
// check if the connection is active or not.
// Q3fill will not send anything after the connect
// request. To detect this, we'll place a hook on
// the function that detects ingame packets
// The result is stored in an array
//
// At connect time, the entry for the client is set
// to 0. After the first non-text packet, its set
// to 1.
// If, after 1 second, its still 0, the client is
// considered fake, and is dropped and banned.
//
// The hook is placed in SV_ReadPackets
// The return point is a forwarded call to
// SV_ExecuteClientMessage
// =================================================
//
#ifdef __linux__
// Define linux symbols
#define _CACHFIX_REG esi
#define _CACHFIX_PATCHPOS 0x80571EF
#define _CACHFIX_SVSCLIENTSPOS 0x83121EC
#define _CACHFIX_RETFUNC 0x804E8C4
#else
// Define windows symbols
#define _CACHFIX_REG ebx
#define _CACHFIX_PATCHPOS 0x44420E
#define _CACHFIX_SVSCLIENTSPOS 0x606224
#define _CACHFIX_RETFUNC 0x43C3A0
#endif

static PatchData_t *pCACHFix;

static void JKG_ConnActivityCheck(int clientNum);

static void *_Hook_ConnectionActivityCheck()
{
	__JKG_StartHook(3);
	{
		__asm1__(	pushad									);	// Secure registers
		__asm2__(	mov		eax, _CACHFIX_REG				);	// Try to figure out the clientNum
		__asm2__(	sub		eax, DS:[_CACHFIX_SVSCLIENTSPOS]);	// Get array offset (in svs.clients)
		__asm2__(	xor		edx, edx						);	// Clear upper 32-bits of dividend
		__asm2__(	mov		ecx, 0x51478					);	// Set divisor (sizeof(client_t))
		__asm1__(	div		ecx								);	// Divide (edx:eax / ecx -> eax (quotient) + edx (remainder)
		__asm2__(	cmp		eax, MAX_CLIENTS				);	// See if its above MAX_CLIENTS
		__asm1__(	jnb		bad								);	// If so, bail
		__asm1__(	push	eax								);  // Push client num
		__asm1__(	call	JKG_ConnActivityCheck			);	// Call
		__asm2__(	add		esp, 0x4						);	// Clean up stack
		__asmL__( bad:										);
		__asm1__(	popad									);	// Restore registers
		__asm1__(	push	_CACHFIX_RETFUNC				);	// Push-ret jump to SV_ExecuteClientMessage
		__asm1__(	ret										);
	}
	__JKG_EndHook(3);
}

int ClientConnectionActive[MAX_CLIENTS];

static void FUNC_USED JKG_ConnActivityCheck(int clientNum) {
    ClientConnectionActive[clientNum] = 1;
}


// =================================================
// Hook 5:
// Player Isolation Hook
// -------------------------------------------------
// In order to handle player isolation,
// players who are isolated from eachother will
// not be sent to eachother. This way they cannot
// see eachother, and at the same time, the client-side
// prediction issues are fixed aswell.
//
// The hook is placed in SV_AddEntitiesVisibleFromPoint
// =================================================
//

#ifdef __linux__
// Define linux symbols
#define _PLISO_PATCHPOS 0x80584D8
#define _PLISO_RETPOS1 0x80584DD
#define _PLISO_RETPOS2 0x80588E7
#define _PLISO_RETPOS3 0x80584F2
#else
// Define windows symbols
#define _PLISO_PATCHPOS 0x444FCC
#define _PLISO_RETPOS1 0x444FD1
#define _PLISO_RETPOS2 0x44537C
#define _PLISO_RETPOS3 0x444FE3
#endif

static PatchData_t *pPLISO;

static int JKG_PlayerIsolationCheck(int clientNum, int entID);
static void *_Hook_PlayerIsolation()
{
	__JKG_StartHook(4);
	{
		__asm1__(	pushad									);	// Secure registers
#ifdef _WIN32
		__asm1__(	push	ebp								);	// Push Entity ID
		__asm1__(	push	[edi + 0xD0]					);	// Push ClientNum
#else
		__asm1__(	push	esi								);	// Same two for linux
		__asm2__(       mov     ecx, [ebp+0x0C]                         )
                __asm1__(	push	[ecx + 0xD0]					);
#endif
		__asm1__(	call	JKG_PlayerIsolationCheck		);
		__asm2__(	add		esp,	8						);
		__asm2__(	test	eax,	eax						);
		__asm1__(	popad									);
		__asm1__(	jne		goback							);
#ifdef _WIN32
		__asm2__(	test	ah,		8						);
		__asm1__(	jz		ret3							);
#else
		__asm2__(	test	bh,		8						);
		__asm1__(	jz		ret3							);
#endif
		__asm1__(	push	_PLISO_RETPOS1					);
		__asm1__(	ret										);
		__asmL__( goback:									);
		__asm1__(	push	_PLISO_RETPOS2					);
		__asm1__(	ret										);
		__asmL__( ret3:										);
		__asm1__(	push	_PLISO_RETPOS3					);
		__asm1__(	ret										);
	}
	__JKG_EndHook(4);
}

int JKG_IsIsolated(int client1, int client2);
// Return 0 to pass through, 1 to block
static int FUNC_USED JKG_PlayerIsolationCheck(int clientNum, int entID) {
        if (entID < 0 || entID >= MAX_CLIENTS) {
		return 0;
	}
	return JKG_IsIsolated(clientNum, entID);
}

// =================================================
// Hook 6:
// Rcon Command Hook
// -------------------------------------------------
// RCON is now fitted with usage logging, ACL and
// hammer bans
// =================================================
//

// TODO: everything ^_^'

/*
static int JKG_RconCheck(int valid) {
	netadr_t from = *(netadr_t *)0x610238;
	cvar_t *sv_rconpassword = *(cvar_t *)0x606210;
	if (!valid) {
		// Hammer check
	}

}
*/

// =================================================
// Hook 7:
// Download exploit fix
// -------------------------------------------------
// This hook here fixes the download exploit
// If a download is requested while ingame, it
// gets denied using a nice lil print, otherwise
// the download is redirected to a placeholder file
// that'll be sent instead.
//
// Hook located in SV_BeginDownload_f
// =================================================
//


#ifdef __linux__
// Define linux symbols
#define _DHFIX_PATCHPOS			0x804D723
#define _DHFIX_RETPOS1			0x804D544
#define _DHFIX_RETPOS2			0x804D751
#define _DHFIX_CLREG			ebx
#define _DHFIX_SVSCLIENTSPOS	0x83121EC


#else
// Define windows symbols
#define _DHFIX_PATCHPOS			0x43B3C7
#define _DHFIX_RETPOS1			0x43B2E0
#define _DHFIX_RETPOS2			0x43B3E8
#define _DHFIX_CLREG			esi
#define _DHFIX_SVSCLIENTSPOS	0x606224

#endif

static PatchData_t *pDHFIX;
static int JKG_CheckDownloadRequest(int clientNum, client_t *cl, const char *filename);

static void *_Hook_DownloadHackFix()
{
	__JKG_StartHook(5);
	{
		__asm1__(	pushad									);	// Secure registers
		__asm2__(	mov		eax, _DHFIX_CLREG				);	// Get location of client_t
		__asm2__(	sub		eax, DS:[_DHFIX_SVSCLIENTSPOS]	);  // Work out the clientnum
		__asm2__(	xor		edx, edx						);  // by doing cl - svs.clients
		__asm2__(	mov		ecx, 0x51478					);  //
		__asm1__(	div		ecx								);  // Do division to get clientNum
		__asm1__(	push	0								);  // Push filename (placeholder)
		__asm1__(	push	_DHFIX_CLREG					);  // Push client_t *cl
		__asm1__(	push	eax								);  // Push clientNum
		__asm1__(	push	1								);  // Push 1 (for Cmd_Argv)
		__asm1__(	call	Cmd_Argv						);  // Call Cmd_Argv to get filename
		__asm2__(	add		esp, 4							);  // Clean up stack
		__asm2__(	mov		[esp+8], eax					);  // Replace the 0 we pushed
		__asm1__(	call	JKG_CheckDownloadRequest		);  // Call JKG_CheckDownloadRequest
		__asm2__(	add		esp, 0xC						);  // Clean up stack
		__asm2__(	test	eax, eax						);  // Check return value
		__asm1__(	popad									);  // Restore registers
		__asm1__(	je		bail							);  // If return value = 0, goto bail
		__asm1__(	push	_DHFIX_RETPOS1					);  // Push-ret call forward to
		__asm1__(	ret										);  // SV_CloseDownload
		__asmL__( bail:										);	//
		__asm2__(	add		esp, 4							);  // Remove return address (we redirected a call)
		__asm1__(	push	_DHFIX_RETPOS2					);  // Push-ret jump to
		__asm1__(	ret										);  // the end of SV_BeginDownload_f
	}
	__JKG_EndHook(5);
}

static int FUNC_USED JKG_CheckDownloadRequest(int clientNum, client_t *cl, const char *filename) {
	int illegal = 0;
	if (!filename) {
		illegal = 1;
	} else if (strlen(filename) < 4) {
		illegal = 1;
	} else if (Q_stricmpn(filename + strlen(filename) - 4, ".pk3", 4)) {
		illegal = 1;
	} else if (strstr(filename, "..")) {
		illegal = 1;
	}
	if (cl->state == CS_ACTIVE) {
		// These are 100% guaranteed to be fake
		if (illegal) {
			SV_SendServerCommand(cl, "print \"Download request for %s rejected: Illegal download request detected\n\"", filename);
			return 0;
		} else {
			SV_SendServerCommand(cl, "print \"Download request for %s rejected: Download requested while in-game\n\"", filename);
			return 0;
		}
	} else {
		if (illegal) {
			// Get a substitute file and send that instead
			// TODO: use a cvar for this
			Q_strncpyz(cl->downloadName, "baddownload.txt", 64);
			return 0;
		}
		// Legal download (or substituted one ;P)
		return 1;
	}
}

// =================================================
// Hook 8:
// Disconnection message override
// -------------------------------------------------
// This hook allows the default disconnection message 
// to be replaced by another
//
// Hook located in SV_Disconnect_f
// =================================================


#ifdef __linux__
// Define linux symbols
#define _DMO_PATCHPOS 0x804EA8B
#define _DMO_SVSCLIENTSPOS 0x83121EC
#define _DMO_RETFUNC 0x804CB84
#else
// Define windows symbols
#define _DMO_PATCHPOS 0x43BE75
#define _DMO_SVSCLIENTSPOS 0x606224
#define _DMO_RETFUNC 0x43BBF0
#endif

static PatchData_t *pDMO;

static void FUNC_USED Hook_DiscoMsgOverride(int clientNum, client_t *client, const char **message) {
	switch (level.clients[clientNum].customDisconnectMsg)
	{
	case 1:	// Server transfer
		*message = "has travelled to another location";
		break;	
	default:
		break;	// Dont change, so keep @@@DISCONNECT
	}
}

static void *_Hook_DiscoMsgOverride()
{
	__JKG_StartHook(6);
	{
		__asm1__(	pushad									);	// Secure registers
		__asm2__(	mov		eax, [esp+0x24]					);	// Get location of client_t
		__asm2__(	sub		eax, DS:[_DHFIX_SVSCLIENTSPOS]	);  // Work out the clientnum
		__asm2__(	xor		edx, edx						);  // by doing cl - svs.clients
		__asm2__(	mov		ecx, 0x51478					);  //
		__asm1__(	div		ecx								);  // Do division to get clientNum
		__asm2__(	lea		ebx, [esp+0x28]					);
		__asm1__(	push	ebx								);  // Push message pointer
		__asm1__(	push	_DHFIX_CLREG					);  // Push client_t *cl
		__asm1__(	push	eax								);  // Push clientNum
		__asm1__(	call	Hook_DiscoMsgOverride			);  // Call Hook_DiscoMsgOverride
		__asm2__(	add		esp, 0xC						);  // Clean up stack
		__asm1__(	popad									);	// Restore registers
		__asm1__(	push	_DMO_RETFUNC					);  // Push-ret call forward to
		__asm1__(	ret										);  // SV_DropClient
	}
	__JKG_EndHook(6);
}

/*
// Moved to auxiliary library

// =================================================
// Hook 8:
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

static void *_Hook_BSPDecryptor()
{
	__JKG_StartHook(7);
	{
		__asm1__(	pushad						);
#ifdef _WIN32
		__asm2__(	lea		eax, [esp+0x34]		);
#else
		__asm2__(	lea		eax, [ebp-0xA4]		);
#endif
		__asm1__(	push	eax					);
		__asm1__(	call	JKG_DecodeBSP		);
		__asm2__(	add		esp, 4				);
		__asm2__(	test	eax, eax			);
		__asm1__(	jne		done				);
		__asm2__(	mov		[esp+0x38], 2		);
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
	__JKG_EndHook(7);
}
*/
/*
#ifdef _WIN32	// Console event handler
void G_ShutdownGame( int restart );
void G_Printf( const char *fmt, ... );
static void (* Sys_Quit)(void) = (void (*) (void))0x410CB0;

BOOL CtrlHandler( DWORD fdwCtrlType ) {
	// We don't care what signal we get, just shut it down
	switch( fdwCtrlType )
	{
	// Handle the CTRL-C signal.
	case CTRL_C_EVENT:
		G_Printf("Don't use Ctrl-C to close the server\n");
		return 1;
	// CTRL-CLOSE: confirm that the user wants to exit.
	case CTRL_CLOSE_EVENT:
		G_Printf("Server window was closed, shutting down server...\n");
		break;
	// Pass other signals to the next handler.
	case CTRL_BREAK_EVENT:
		G_Printf("Don't use Ctrl-Break to close the server\n");
		return 1;

	case CTRL_LOGOFF_EVENT:
		G_Printf("User signing off, shutting down server...\n");
		break;
	case CTRL_SHUTDOWN_EVENT:
		G_Printf("Operating system shutting down, shutting down server...\n");
		break;
	default:
		break;
	}
	Sys_Quit();
	return 0;
}

#endif
*/

// =================================================
// Hook 10:
// Protected PK3
// -------------------------------------------------
// This allows the game to use secure PK3s
// =================================================

static PatchData_t *pPAKSS1;
static PatchData_t *pPAKSS2;
static PatchData_t *pPAKSS3;
static unsigned int retAddy;
#define _PAK_PATCHPOS_SERVERSIDE1 0x00422164
#define _PAK_PATCHPOS_SERVERSIDE2 0x00421F76
#define _PAK_PATCHPOS_SERVERSIDE3 0x0042237F
void  *_Hook_PAKHook1()
{
	__JKG_StartHook(8);
	//LOC: 0x00422164
	{
		__asm2__( mov eax, [esi+0x58] );		// Normal PK3?
		__asm2__( test eax, eax );
		__asm1__( pushfd );
		__asm1__( je label_success );
		__asm2__( cmp eax, 0x0800 );			// Magically encrypted?
		__asm1__( je label_success  );
		__asm1__( popfd );
		__asm1__( push 0x004221C5 ); //0x0042216B
		__asm1__( ret );
		__asm label_success:
		__asm1__( popfd );
		__asm1__( push 0x0042216B ); // 0x004221C5
		__asm1__( ret );
	}
	__JKG_EndHook(8);
}

void *_Hook_PAKHook2()
{
	__JKG_StartHook(9);
	//LOC: 0x00421F76
	{
		__asm2__( test eax, eax );
		__asm1__( je label_success );
		__asm2__( cmp eax, 8 );
		__asm1__( je label_success );
		__asm2__( test eax, 0x0800 );
		__asm1__( jne label_success );
		__asm2__( cmp eax, 0x808 );
		__asm1__( je label_success );
		__asm2__( mov edx, 0x00421F7F );
		__asm1__( jmp edx );
		__asmL__(  label_success: );
		__asm2__( mov edx, 0x00421F84 );
		__asm1__( jmp edx );
	}
	__JKG_EndHook(9);
}

void *_Hook_PAKHook3()
{
	__JKG_StartHook(10);
	//LOC: 0x0042237F
	{
		__asm2__( cmp edx, ebx );
		__asm2__( mov edx, [esi+3Ch] );
		__asm2__( mov [edi+48h], edx );
		__asm2__( mov [edi+44h], ebx );
		__asm2__( mov ecx, [esi+34h] );
		// Hook start
		__asm1__( je label_success );
		__asm2__( mov edx, [esi+34h] );
		__asm2__( cmp edx, 0x800 );
		__asm1__( je label_success );
		// Failed all checks
		__asm2__( mov eax, 0 );
		__asm1__( jmp label_exit );
		__asmL__( label_success: );
		__asm2__( mov eax, 1 );
		__asmL__( label_exit: );
		__asm2__( mov edx, 0x00422390 );
		__asm1__( jmp edx );

	}
	__JKG_EndHook(10);
}

#ifdef _WIN32
#define _CLIENTCAPPOS1	0x4426B6
#define _CLIENTCAPPOS2	0x4426BA
#define _CLIENTCAPPOS3	0x4427F6
#define _CLIENTCAPPOS4	0x4427FA
#define _CLIENTCAPPOS5  0x4464F2
#else
#define _CLIENTCAPPOS1	0x8054D91
// LINUX TODO: Byte patch for Xyc's collision bug find
// LINUX TODO: Add a byte patch here to stop limiting sv_maxclients down to 32 if it's set over 64
#endif

static char * svsaytxt = "chat 100 \"%s\"";
#ifdef _WIN32
#define _SVSAYTXT 0x43A73C
#define _SVSAYTXTORIG 0x4ACA58
#else
#define _SVSAYTXT 0x804FB21
#define _SVSAYTXTORIG 0x819BB40
#endif

#ifdef _WIN32
#define _TOKFIX 0x40F5B3
#else
#define _TOKFIX 0x812C4A7
#endif

void JKG_PatchEngine() {
	Com_Printf(" ------- Installing Engine Patches -------- \n");
	///////////////////////////////
	// Hook 1: Version check
	///////////////////////////////
	pPVCheck = JKG_PlacePatch(PATCH_JUMP, _PVCHECK_PATCHPOS, (unsigned int)_Hook_ProtocolVersionCheck());
    if (!pPVCheck) {
		Com_Printf("Warning: Failed to place hook 1: Protocol check\n");
    }
	///////////////////////////////
	// Patch 1: Change protocol version shown by getinfo to 27 (q3fill bait)
	///////////////////////////////
	UnlockMemory(_PVCHECK_INFOPROTOCOL,1);
	*(unsigned char *)_PVCHECK_INFOPROTOCOL = 27;
	LockMemory(_PVCHECK_INFOPROTOCOL,1);

	pIBFix = JKG_PlacePatch(PATCH_CALL, _IBFIX_PATCHPOS, (unsigned int)_Hook_InfoBoomFix()); // We'll be overwriting a call here
    if (!pIBFix) {
		Com_Printf("Warning: Failed to place hook 2: Q3infoboom fix\n");
    }
	///////////////////////////////
	// Patch 2: Revert the patch of Luigi (in case its patched)
	///////////////////////////////
	UnlockMemory(_IBFIX_MSGPATCH,1);
	*(unsigned int *)_IBFIX_MSGPATCH = (unsigned int)0x3FF;
	LockMemory(_IBFIX_MSGPATCH,1);

	///////////////////////////////
	// Hook 3: PVS correction
	///////////////////////////////

	pCPVSFix = JKG_PlacePatch(PATCH_CALL, _CPVSFIX_PATCHPOS, (unsigned int)_Hook_PVSCorrection()); // We'll be overwriting a call here
    if (!pCPVSFix) {
		Com_Printf("Warning: Failed to place hook 3: PVS Correction\n");
    }
	///////////////////////////////
	// Hook 4: Connection Activity Check
	///////////////////////////////

	pCACHFix = JKG_PlacePatch(PATCH_CALL, _CACHFIX_PATCHPOS, (unsigned int)_Hook_ConnectionActivityCheck()); // We'll be overwriting a call here
    if (!pCACHFix) {
		Com_Printf("Warning: Failed to place hook 4: Connection Activity Check\n");
    }
	///////////////////////////////
	// Hook 5: Player Isolation Hook
	///////////////////////////////

    pPLISO = JKG_PlacePatch(PATCH_JUMP, _PLISO_PATCHPOS, (unsigned int)_Hook_PlayerIsolation());
    if (!pPLISO) {
		Com_Printf("Warning: Failed to place hook 5: Player Isolation Hook\n");
    }

	///////////////////////////////
	// Hook 7: Download Hack Fix
	///////////////////////////////
	pDHFIX = JKG_PlacePatch(PATCH_CALL, _DHFIX_PATCHPOS, (unsigned int)_Hook_DownloadHackFix()); // We'll be overwriting a call here
    if (!pDHFIX) {
		Com_Printf("Warning: Failed to place hook 7: Download Hack Fix\n");
    }

	///////////////////////////////
	// Hook 8: Disconnect message override
	///////////////////////////////
	pDMO = JKG_PlacePatch(PATCH_CALL, _DMO_PATCHPOS, (unsigned int)_Hook_DiscoMsgOverride()); // We'll be overwriting a call here
    if (!pDMO) {
		Com_Printf("Warning: Failed to place hook 8: Disconnect message override\n");
    }

	///////////////////////////////
	// Patch 3: Client cap elevation
	///////////////////////////////

	UnlockMemory(_CLIENTCAPPOS1,1);
	*(unsigned char *)_CLIENTCAPPOS1 = (unsigned char)MAX_CLIENTS;
	LockMemory(_CLIENTCAPPOS1,1);
#ifdef _CLIENTCAPPOS2
	UnlockMemory(_CLIENTCAPPOS2,1);
	*(unsigned char *)_CLIENTCAPPOS2 = (unsigned char)MAX_CLIENTS;
	LockMemory(_CLIENTCAPPOS2,1);
#endif
#ifdef _CLIENTCAPPOS3
    UnlockMemory(_CLIENTCAPPOS3,1);
	*(unsigned char *)_CLIENTCAPPOS3 = (unsigned char)MAX_CLIENTS;
	LockMemory(_CLIENTCAPPOS3,1);
#endif
#ifdef _CLIENTCAPPOS4
    UnlockMemory(_CLIENTCAPPOS4,1);
	*(unsigned char *)_CLIENTCAPPOS4 = (unsigned char)MAX_CLIENTS;
	LockMemory(_CLIENTCAPPOS4,1);
#endif
#ifdef _CLIENTCAPPOS5
    UnlockMemory(_CLIENTCAPPOS5,1);
	*(unsigned char *)_CLIENTCAPPOS5 = (unsigned char)MAX_CLIENTS;
	LockMemory(_CLIENTCAPPOS5,1);
#endif

	///////////////////////////////
	// Patch 4: Fix svsay to handle fading text properly
	///////////////////////////////
	
	UnlockMemory(_SVSAYTXT, 4);
	*(int *)_SVSAYTXT = ( int ) svsaytxt;
	LockMemory(_SVSAYTXT, 4);

	///////////////////////////////
	// Patch 5: Fix ANSI support in Cmd_TokenizeString
	///////////////////////////////
	
	UnlockMemory(_TOKFIX, 1);
	*(char *)_TOKFIX = 0x77; // (replaces JG (0x7F) by JA (0x77))
	LockMemory(_TOKFIX, 1);	

	pPAKSS1 = JKG_PlacePatch(PATCH_JUMP, _PAK_PATCHPOS_SERVERSIDE1, (unsigned int)_Hook_PAKHook1());
	pPAKSS2 = JKG_PlacePatch(PATCH_JUMP, _PAK_PATCHPOS_SERVERSIDE2, (unsigned int)_Hook_PAKHook2());
	pPAKSS3 = JKG_PlacePatch(PATCH_JUMP, _PAK_PATCHPOS_SERVERSIDE3, (unsigned int)_Hook_PAKHook3());
	if(!pPAKSS1)
	{
		Com_Printf("Warning: Failed to place PK3 Protection Patch\n");
	}

	Com_Printf("Finished\n");
}

void JKG_UnpatchEngine() {
	JKG_RemovePatch(&pPVCheck);
	JKG_RemovePatch(&pIBFix);
	JKG_RemovePatch(&pCPVSFix);
	JKG_RemovePatch(&pCACHFix);
	JKG_RemovePatch(&pPLISO);
	JKG_RemovePatch(&pDHFIX);
	JKG_RemovePatch(&pPAKSS1);
	JKG_RemovePatch(&pPAKSS2);
	JKG_RemovePatch(&pPAKSS3);
	UnlockMemory(_PVCHECK_INFOPROTOCOL,1);
	*(unsigned char *)_PVCHECK_INFOPROTOCOL = 26;
	LockMemory(_PVCHECK_INFOPROTOCOL,1);

	UnlockMemory(_SVSAYTXT, 4);
	*(int *)_SVSAYTXT = _SVSAYTXTORIG;
	LockMemory(_SVSAYTXT, 4);
}

void SetWindowTitle(const char *newtitle) {
#ifdef _WIN32
	SetConsoleTitle(newtitle);
#endif
}

void UpdateWindowTitle() {
	char cs[2048];
	trap_GetServerinfo( cs, sizeof( cs ) );
	SetWindowTitle(va("Jedi Knight Galaxies Server - %s (%i/%i)", Info_ValueForKey( cs, "mapname" ), level.numConnectedClients, level.maxclients));
}
