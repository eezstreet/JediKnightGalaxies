/////////////////////////////////
// Engine alterations done by UI
/////////////////////////////////

// DISABLE THIS FOR PUBLIC RELEASES!
#define ALLOW_NOCD

#include <windows.h>
#include <libudis86/udis86.h>
#include <assert.h>

#define JKG_HOOK void __declspec(naked)

static void (*Com_Printf)( const char *msg, ... ) = (void(*)(const char*, ...))0x437080;
static void (*Cvar_Set2)(const char *cvarname, const char *newvalue, int force) = (void(*)(const char*, const char*, int))0x4396A0;

static void (*RE_SetColor)(float *color) = (void(*)(float*))0x491660;

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
// PatchByte (WIN32 & Linux compatible)
// --------------------------------------------------
// Modifies a single byte
// Returns 1 if successful, returns 0 on failure.
// ==================================================
static int PatchByte(int address, unsigned char newbyte) {
	if (!UnlockMemory(address, 1)) return 0;
	*(unsigned char *)address = newbyte;
	LockMemory(address, 1);
	return 1;
}



// ==================================================
// JKG_PlacePatch (WIN32 & Linux compatible)
// --------------------------------------------------
// Patches the code at address to make a go towards
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


/* _Hook_MapTrap
**
** This hook prevents the player from utilizing any of the map commands (devmap and spmap included)
** This is because players cant normally launch a server like that
** And if they do, the server wont start due to lack of resources
** So its blocked right off the bat
**
** Hook location: 0x4560B0 (SV_Map_f)
** Hook size: <xx> <xxxxxxxx> (5)
** Original code: A1 08CCB300 (MOV EAX, [00B3CC08])
** Hook Return point: a) 0x4560B5, b) ret
*/

static char MapMsg[] = "You are not allowed to start a Jedi Knight Galaxies server\n";
static PatchData_t *pMapTrap;
static JKG_HOOK _Hook_MapTrap() {
	__asm {
		pushad
		push	offset MapMsg
		call	Com_Printf
		add		esp,4
		popad
		ret
	}
}

/* _Hook_PrimitivesTrap
**
** Now, there are people that find it funny to trick others into doing r_pri 5 (or related bad values)
** As we all know, this kinda fucks up the renderer, causing it to render...nothing
** To avoid this, the value of r_primitives is now checked, and, if its invalid, its defaulted to 0
**
** Hook location: 0x4AC360 (R_DrawElements)
** Hook size: <xxxx> <xxxxxxxx> (6)
** Original code: 8B0D 2430FE00 (MOV ECX, [FE3024])
** Hook Return point: 0x4AC366
*/

static char PrimitivesMsg[] = "Invalid r_primitives setting detected, reverting to 0\n";
static char Pri_Cvar[] = "r_primitives";
static char Pri_Val[] = "0";
static PatchData_t *pPrimitivesTrap;
static JKG_HOOK _Hook_PrimitivesTrap() {
	__asm {
		pushad
		mov		ecx, ds:[0xFE3024]
		mov		ecx, [ecx+0x20]
		cmp		ecx, 3
		jna		allok
		// Bad value, revert to 0
		push	1					// Force
		push	offset Pri_Val		// Cvar Value
		push	offset Pri_Cvar	// Cvar Name
		call	Cvar_Set2
		add		esp,0x0c
		push	offset PrimitivesMsg
		call	Com_Printf
		add		esp,0x4
allok:
		popad
		mov		ecx, DS:[0xFE3024]
		push	0x4AC366
		ret
	}
}

/* _Hook_SetuTrap
**
** To avoid potential complications with ppl fuckin around with their userinfo
** the Setu command is disabled and cannot be used
**
** Hook location: 0x439D40 (Cvar_Setu_f)
** Hook Return point: ret
*/

static char SetuMsg[] = "You are not allowed to set userinfo cvars\n";
PatchData_t *pSetuTrap;
static JKG_HOOK _Hook_SetuTrap() {
	__asm {
		push	offset SetuMsg
		call	Com_Printf
		add		esp,4
		ret
	}
}

/* _Hook_JKGValidation
**
** The server expects the client so send a command right after connecting
** this is done as a final step in validating the client's integrity
** To do this, the command is queued right after receiving connectResponse
**
** Hook location: 0x41EC8E (CL_ConnectionlessPacket)
** Hook Return point: ret
*/

static char ValidationCmd[] = "~svrValidateClient\n";
PatchData_t *pJKGValidation;
static JKG_HOOK _Hook_JKGValidation() {
	__asm {
		pushad
		lea edi, [ValidationCmd]
		mov eax, 0x41C6F0				// CL_AddReliableCommand
		call eax
		popad
		add esp, 0x400
		ret
	}
}

/* _Hook_SuppressConnectPrints
**
** If the client is rejected from a server for any reason, the server will
** send a 'print' OOB command to the client to state the reject message.
** We want to use the messages as commands sometimes but the downside is that
** the engine prints the message to console before ui parses it.
** This hook suppresses printing to console when clc.state is CA_CHALLENGING
**
** Hook location: 0x41EE19 (CL_ConnectionlessPacket)
** Hook Return point: Com_Printf (if cls.state is not CA_CHALLENGING)
**                    ret        (if cls.state is CA_CHALLENGING)
*/

typedef enum {
	CA_UNINITIALIZED,
	CA_DISCONNECTED, 	// not talking to a server
	CA_AUTHORIZING,		// not used any more, was checking cd key 
	CA_CONNECTING,		// sending request packets to the server
	CA_CHALLENGING,		// sending challenge packets to the server
	CA_CONNECTED,		// netchan_t established, getting gamestate
	CA_LOADING,			// only during cgame initialization, never during main loop
	CA_PRIMED,			// got gamestate, waiting for first frame
	CA_ACTIVE,			// game views should be displayed
	CA_CINEMATIC		// playing a cinematic or a static pic, not connected to a server
} connstate_t;

PatchData_t *pSuppressConnectPrints;
static JKG_HOOK _Hook_SuppressConnectPrints() {
	__asm {
		// ecx isn't used before the call
		mov ecx, DS:[0x8AF100] // cls.state
		cmp ecx, CA_CHALLENGING
		jz hopover
		push Com_Printf // Push to Com_Printf here
hopover:
		ret
	}
}

/* _Hook_ScreenshotQuality
**
** This lets you set the screenshot quality (in percent)
** Value used is based on the cvar jkg_ssquality
** Values are clamped to the range [0 - 100]
**
** Hook location: 0x4A1335 (R_TakeScreenshot)
** Hook Return point: 0x4A133A
*/

#define	MAX_CVAR_VALUE_STRING	256

typedef int	cvarHandle_t;

typedef struct {
	cvarHandle_t	handle;
	int			modificationCount;
	float		value;
	int			integer;
	char		string[MAX_CVAR_VALUE_STRING];
} vmCvar_t;

extern vmCvar_t	jkg_ssquality;

static int Hook_ScreenshotQuality() {
	int quality = jkg_ssquality.integer;
	//if (quality < 0) quality = 0;
	//if (quality > 100) quality = 100;
	return quality;
}

PatchData_t *pSSQuality;
static JKG_HOOK _Hook_ScreenshotQuality() {
	__asm {
		push	edi
		push	ecx
		push	0		// <-- Quality
		push	ebx
		pushad
		call	Hook_ScreenshotQuality
		mov		[esp+0x24], eax		// <-- Change Quality
		popad
		push	0x4A133A
		ret
	}
}

///////////////////////////////////////////////////
// Console window command handling modifications 
//                                               
// These two hooks modify the behaviour of       
// commands typed into the console window itself 
// First off, they're no longer displayed in the
// console (ingame), and 'name;' is blocked
// since xfire spams that.

/* _Hook_SysConsole_XfireFix
**
** Blocks the command 'name;' from being executed if typed in the console window
** Done to stop xfire's 'name;' spam
**
** Hook location: 0x453DCA
** Hook return point (valid cmd): 0x453DD0
** Hook return point (bad cmd): _Hook_SysConsole_CmdBail
** Safe registers: edx, ecx
*/

static int Hook_SysConsole_XfireFix(const char *cmd) {
	if (strstr(cmd, "name;")) {
		// Its xfire, block it
		SetWindowTextA(*(HWND *)0xB8EBB4, "");
		return 0;
	}
	return 1;
}

PatchData_t *pSysCon_XfireFix;
PatchData_t *pSysCon_ConsoleFix;

static JKG_HOOK _Hook_SysConsole_CmdBail() {
	__asm
	{
		pop		edi
		pop		esi
		add		esp, 0x400
		retn	0x10
	}
}

static JKG_HOOK _Hook_SysConsole_XfireFix() {
	__asm {
		lea		ecx, [esp+0x08]
		pushad
		push ecx
		call Hook_SysConsole_XfireFix
		add esp, 0x4
		test	eax,eax
		je block
		popad
		lea		edx, [eax+1]
		lea     ecx, [ecx+0]
		push	0x453DD0
		ret
block:
		popad
		lea		edx, [eax+1]
		lea     ecx, [ecx+0]
		push	_Hook_SysConsole_CmdBail
		ret

	}
}

PatchData_t *pRecordFix;

///////////////////////////////////////////////////
// Extended color code support
//                                               
// This hook implements the extended color codes
// with the format ^xRGB, where each is a single
// hex digit (0 to F) representing R G and B
// respectively.
//
// This is implemented directly into RE_Font_DrawFontText

/* _Hook_ExtColorCodes
**
** Implements extended color code support
**
** Hook location: 0x4959A3
** Hook return point (invalid colorcode): 0x4959A9
** Hook return point (valid colorcode): 0x495CCF
*/


static float ExtColor_GetLevel(char chr) {
	if (chr >= '0' && chr <= '9') {
		return ( (float)(chr-'0') / 15.0f );
	}
	if (chr >= 'A' && chr <= 'F') {
		return ( (float)(chr-'A'+10) / 15.0f );
	}
	if (chr >= 'a' && chr <= 'f') {
		return ( (float)(chr-'a'+10) / 15.0f );
	}
	return -1;
}

typedef float vec_t;
typedef vec_t vec4_t[4];

static int Hook_ExtColorCodes(const char *text, int drawingShadow) {
	const char *r, *g, *b;
	float red, green, blue;
	vec4_t	color;
	r = text+1;
	g = text+2;
	b = text+3;
	// Get the color levels (if the numbers are invalid, it'll return -1, which we can use to validate)
	red = ExtColor_GetLevel(*r);
	green = ExtColor_GetLevel(*g);
	blue = ExtColor_GetLevel(*b);
	// Determine if all 3 are valid
	if (red == -1 || green == -1 || blue == -1) {
		return 0;
	}

	// Colorcode is valid, lets see if we should apply it or not
	if (drawingShadow) {
		return 1;		// Valid, but dont apply regardless of that
	}
	// We're clear to go, lets construct our color

	color[0] = red;
	color[1] = green;
	color[2] = blue;

	// HACK: Since cgame will use a palette override to implement dynamic opacity (like the chatbox)
	// we must ensure we use that alpha as well.
	// So copy the alpha of colorcode 0 (^0) instead of assuming 1.0

	color[3] =*(float *)(0x56DF54 /*0x56DF48 + 12*/);

	RE_SetColor(color);		// And apply it
	return 1;				// All done
}

static JKG_HOOK _Hook_ExtColorCodes() {
	__asm
	{
		xor eax,eax
		mov al, [ebx]
		cmp al, 0x78	// 'x'
		jne bail
		// We might have one, so secure the registers and get going
		push ebx
		pushad
		mov eax, DS:[0x12DC0D0]
		push ebx
		push eax
		push ebx
		call Hook_ExtColorCodes
		add esp, 8
		pop ebx
		test eax,eax
		je bail2	// Not a valid colorcode after all
		add ebx, 4	// Add 4 to ebx, so the string advances 4 characters (xRGB)
		mov [esp+0x20], ebx
		popad		// Colorcode was valid
		pop ebx
		push 0x495CCF
		ret
bail2:
		popad
		pop ebx
bail:
		// Not a colorcode we seek, bail
		cmp al, 0x30
		push 0x4959A9
		ret
	}
}
PatchData_t *pExtColorCodes;

///////////////////////////////////////////////////
// No-cd patch
//                                               
// This will disable JA's cd-checks
//  making it possible to play the game without cd.
//
// Only available if ALLOW_NOCD is defined
//
// WARNING: This patch is *illegal*, do not include
//          in public releases!
//

void ApplyNoCD()
{
#ifdef ALLOW_NOCD
	PatchByte( 0x41D9B8, 0xB8 );
	PatchByte( 0x45DE61, 0xB8 );
	PatchByte( 0x52C329, 0xB8 );
	Com_Printf( "^3No-cd patch enabled!\n" );
#endif
}

void UI_PatchEngine() {
	// Change the JA window title XD
	HWND *JAWnd = (HWND *)0xB8F038;
	SetWindowTextA(*JAWnd, "Jedi Knight Galaxies - Jedi Knight®: Jedi Academy (MP)");
	// Change transmitted protocol to 89
	UnlockMemory(0x41E3E3, 1);
	*(unsigned char *)0x41E3E3 = 89;
	LockMemory(0x41E3E3, 1);

	// Raise delta compressor's bitcount for ps.stats[STAT_WEAPONS] to 32
	UnlockMemory(0x44238B, 1);
	*(unsigned char *)0x44238B = 32;
	LockMemory(0x44238B, 1);

	// Raise delta compressor's settings for ammo (16 slots of 16 bits to 19 slots of 32 bits)
	//UnlockMemory(0x4424AB, 106);
	//*(unsigned char *)0x4424AC = 19;
	//*(unsigned char *)0x4424F2 = 32;
	//*(unsigned char *)0x442516 = 19;
	//LockMemory(0x4424AB, 106);

	// Fix Cmd_TokenizeString's ANSI support
	UnlockMemory(0x436C83, 1);
	*(char *)0x436C83 = 0x77; // (replaces JG (0x7F) by JA (0x77))
	LockMemory(0x436C83, 1);

	pMapTrap = JKG_PlacePatch(PATCH_JUMP, 0x4560B0,(unsigned int)_Hook_MapTrap);
	//pPrimitivesTrap = JKG_PlacePatch(PATCH_JUMP, 0x4AC360,(unsigned int)_Hook_PrimitivesTrap);
	pSetuTrap = JKG_PlacePatch(PATCH_JUMP, 0x439D40, (unsigned int)_Hook_SetuTrap);
	//pJKGValidation = JKG_PlacePatch(PATCH_JUMP, 0x41EC8E, (unsigned int)_Hook_JKGValidation);
	pSuppressConnectPrints = JKG_PlacePatch(PATCH_CALL, 0x41EE19, (unsigned int)_Hook_SuppressConnectPrints);
	pSSQuality = JKG_PlacePatch(PATCH_JUMP, 0x4A1335, (unsigned int)_Hook_ScreenshotQuality);
	
	pSysCon_XfireFix = JKG_PlacePatch(PATCH_JUMP, 0x453DCA, (unsigned int)_Hook_SysConsole_XfireFix);
	// Internal jump, used to skip the 'copy to console' code
	pSysCon_ConsoleFix = JKG_PlacePatch(PATCH_JUMP, 0x453E3D, (unsigned int)_Hook_SysConsole_CmdBail);

	// Remove requirement for g_sync for demo recording
	pRecordFix = JKG_PlacePatch(PATCH_JUMP, 0x41C9F9, (unsigned int)0x41CA36);

	// Extended colorcodes
	pExtColorCodes = JKG_PlacePatch(PATCH_JUMP, 0x4959A3, (unsigned int)_Hook_ExtColorCodes);

}

void UI_UnpatchEngine() {
	// Change transmitted protocol to 26
	UnlockMemory(0x41E3E3, 1);
	*(char *)0x41E3E3 = 26;
	LockMemory(0x41E3E3, 1);

	JKG_RemovePatch(&pMapTrap);
	//JKG_RemovePatch(&pPrimitivesTrap);
	JKG_RemovePatch(&pSetuTrap);
	//JKG_RemovePatch(&pJKGValidation);
	JKG_RemovePatch(&pSuppressConnectPrints);
	JKG_RemovePatch(&pSSQuality);

	JKG_RemovePatch(&pSysCon_XfireFix);
	JKG_RemovePatch(&pSysCon_ConsoleFix);

	// Reactivate the requirement for g_sync for demo recording
	JKG_RemovePatch(&pRecordFix);

	JKG_RemovePatch(&pExtColorCodes);
}