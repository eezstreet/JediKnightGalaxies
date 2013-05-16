/////////////////////////////////
// Engine alterations done by UI
/////////////////////////////////

#include "cg_local.h"
#include <windows.h>
#include <gl/gl.h>
#include <libudis86/udis86.h>
#include "jkg_patcher.h"
#include "jkg_glcommon.h"
#include "jkg_eshader.h"

#define JKG_HOOK void __declspec(naked)

typedef struct {
	int addr;
	int size;
	char origbytes[24];
} PatchData_t;

typedef enum {
	PATCH_JUMP,
	PATCH_CALL,
} PatchType_e;

#define MAX_JKG_CVARS	1224

cvar_t cvarIndeces[MAX_JKG_CVARS];
unsigned int MAX_JKG_CVARS_I = MAX_JKG_CVARS;

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
	ret = VirtualProtect((LPVOID)address, size, PAGE_EXECUTE_READWRITE, &dummy);
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
// Patches the code at address to make a go towards
// destination.
// The instruction used is either JMP or CALL, 
// depending on the type specified.
//
// Before the code is modified, the code page is
// unlocked.
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
// JKG_PatchMemory (WIN32 & Linux compatible)
// --------------------------------------------------
// Patches the code at address to make a go towards
// destination.
//
// Before the code is modified, the code page is
// unlocked.
//
// This function returns a malloced PatchData_t.
// To remove the patch, call JKG_RemoveMemPatch. This
// will also free the PatchData_t.
// ==================================================

static PatchData_t *JKG_PatchMemory( unsigned int address, unsigned char *byteSequence, unsigned int numBytes ) {
	PatchData_t *patch = malloc(sizeof(PatchData_t));
	//int addr = address;

	ud_t ud;

	if(!byteSequence)
	{
		return NULL;
	}

	ud_init(&ud);
	ud_set_input_buffer(&ud, (uint8_t *)address, numBytes);
	ud_set_mode(&ud, 32);
	ud_set_pc(&ud, address);
	ud_set_syntax(&ud, NULL);

	/*while (ud_disassemble(&ud))
	{
		sz += ud_insn_len(&ud);
		if (sz >= numBytes) {
			break;
		}
	}*/

	//assert(sz >= numBytes);

	/*if (sz < 5 || sz > 24) {
		// This really shouldnt ever happen, in the worst case scenario,
		// the block is 20 bytes (4 + 16), so if we hit 24, something went wrong
		return NULL;
	}*/

	patch->addr = address;
	patch->size = numBytes;
	memcpy(patch->origbytes, (const void *)address, numBytes);
	UnlockMemory(address, numBytes); // Make the memory writable
	
	// Patch the bytes at the address
	memcpy((unsigned int *)address, byteSequence, numBytes);

	/**(unsigned char *)address = type == PATCH_JUMP ? 0xE9 : 0xE8;
	*(unsigned int *)(address+1) = destination - (address + 5);
	memset((void *)(address+5),0x90,sz-5);	// Nop the rest*/
	LockMemory(address, numBytes);
	return patch;
}

static void JKG_RemoveMemPatch(PatchData_t **patch) {
	if (!*patch)
		return;
	UnlockMemory((*patch)->addr, (*patch)->size);
	memcpy((void *)(*patch)->addr, (*patch)->origbytes, (*patch)->size);
	LockMemory((*patch)->addr, (*patch)->size);
	*patch = 0;
}

typedef struct {
	void *returnAddr;
	void *funcAddr;
	const char *state;
} mapLoadTbl_t;

static const mapLoadTbl_t mapLoadTbl [] = {
	{( void * ) 0x491126, ( void * ) 0x490380, "( Loading Shaders )"},
	{( void * ) 0x491135, ( void * ) 0x48CE40, "( Loading Lightmaps )"},
	{( void * ) 0x49113D, ( void * ) 0x490490, "( Loading Planes )"},
	{( void * ) 0x491153, ( void * ) 0x4905C0, "( Loading Fogs )"},
	{( void * ) 0x491166, ( void * ) 0x48FDF0, "( Loading Surfaces - 0.0% )"}, // This one takes the longest, so show progress here
	{( void * ) 0x491170, ( void * ) 0x490410, "( Loading Mark Surfaces )"},
	{( void * ) 0x49117E, ( void * ) 0x490190, "( Loading Nodes And Leafs )"},
	{( void * ) 0x491188, ( void * ) 0x48FF90, "( Loading Sub Models )"},
	{( void * ) 0x491194, ( void * ) 0x48D100, "( Loading Visibility )"},
	{( void * ) 0x4911B3, ( void * ) 0x490BB0, "( Loading Entities )"},
	{( void * ) 0x4911BF, ( void * ) 0x4909C0, "( Loading Light Grid )"},
	{( void * ) 0x4911CF, ( void * ) 0x490B30, "( Loading Light Array )"},
	{NULL, NULL, NULL},
};

static void *BSPLoadProgress(void *retaddr) {

	mapLoadTbl_t *entry;
	for (entry = ( mapLoadTbl_t * ) &mapLoadTbl; entry->returnAddr; entry++) {
		if (entry->returnAddr == retaddr) {
			// Found it :D
			if (cg.showMapLoadProgress) {
				CG_LoadingString(va("%s %s", cgs.mapname, entry->state));
			}
			return entry->funcAddr;
		}
	}
	CG_Printf("CRITICAL ERROR: Unknown BSP loading stage encountered! (%08X)\n", retaddr);
	return NULL;
}

static JKG_HOOK _Hook_BSPLoadProgress() {
	__asm
	{
		sub esp, 4			// Alloc space for the return pointer
		pushad				// Secure registers
		mov eax, [esp+0x24]	// Get return address
		push eax			// Push return address
		call BSPLoadProgress	// Call BSPLoadProgress
		add esp,4
		test eax,eax		// Check if the return value is NULL
		je error			// If so, go to error
		mov [esp+0x20], eax	// Put the function address in our alloc
		popad				// Restore registers
		ret					// Jump to function (our alloc with ptr is on top of the stack)
error:
		popad				// Restore registers
		add esp, 4			// Remove alloc
		ret					// Return to BSP loader (will skip the current stage!)
	}
}

PatchData_t *BspLoaderPatch[12];
PatchData_t *BspSurfaceLoaderPatch[3];


static int ldrSurfaceCount;			// Total amount of surfaces
static int ldrSurfaceLast;			// Amount of surfaces at last update (DECREMENTS!)
static int ldrSurfaceThreshold;		// Amount of surfaces to load between progress updates

static JKG_HOOK _Hook_BSPSurfaceLoadProgressInit() {
	// Simple hook, just store the surface count 'n get outta here
	__asm
	{
		mov [ldrSurfaceCount], eax
		mov [ldrSurfaceLast], eax
		mov [ldrSurfaceThreshold], 100
		jle skip
		push 0x48FEC4
		ret
skip:
		push 0x48FF57
		ret
	}
}

static void BSPSurfaceLoadProgressUpdate() {
	// Time to show an update
	if (cg.showMapLoadProgress) {
		float pct = 100.0f - (((float)ldrSurfaceLast / (float)ldrSurfaceCount) * 100.0f);
		CG_LoadingString(va("%s ( Loading Surfaces - %.1f%% )", cgs.mapname, pct));
	}
}

static JKG_HOOK _Hook_BSPSurfaceLoadProgress() {
	// This hook is called every cycle of the surface loader
	// As such this function has to be as efficient as possible
	// No calls to C are made until it is required
	__asm
	{
		push ebx		// Secure ebx only
		mov ebx, [ldrSurfaceLast]
		sub ebx, eax
		cmp ebx, [ldrSurfaceThreshold]
		jge progressupdate
done:		
		pop ebx
		add edi, 0x94
		push 0x48FF49
		ret
progressupdate:
		pushad
		mov [ldrSurfaceLast], eax
		call BSPSurfaceLoadProgressUpdate
		popad
		jmp done
	}
}

static void BSPSurfaceLoadProgressFinalize() {
	// Time to show an update
	if (cg.showMapLoadProgress) {
		CG_LoadingString(va("%s ( Finalizing Surfaces... )", cgs.mapname));
	}
}

static JKG_HOOK _Hook_BSPSurfaceLoadFinished() {
	// We finished loading, so now we deal with stitching 'n whatnot,
	// so update the progress accordingly
	__asm
	{
		pushad
		call BSPSurfaceLoadProgressFinalize
		popad
		push 0x48FC90
		ret
	}
}

usercmd_t ( *pCL_CreateCmd )( void );
usercmd_t CL_CreateCmd( void )
{
	static usercmd_t	pPrevCmd;
	usercmd_t			pCmd		= pCL_CreateCmd();
	int					iCalculate	= 0;

	/* See if we are in 360 camera and act accordingly */
	if ( cg.i360CameraTime && cg.i360CameraTime < cg.time )
	{
		/* Calculate the difference between this command and the previous */
		iCalculate				 = ( pCmd.angles[YAW] - cg.i360CameraUserCmd );
		cg.i360CameraUserCmd	 = pCmd.angles[YAW];

		/* Sanity check on the absolute value of the new user command */
		if( abs( iCalculate ) > ( 65536 / 2 ))
		{
			iCalculate = 65536 - abs( iCalculate );
		}

		/* Smoothen the camera control and make sure it does not exceed 360! */
		cg.i360CameraOffset += iCalculate / 100;
		cg.i360CameraOffset %= 360;

		/* Normalize the angle, it shouldn't be negative */
		if( cg.i360CameraOffset < 0 )
		{
			cg.i360CameraOffset += 360;
		}

		/* Copy the previous frame angles on this command to prevent changing it, but allows me to change camera! */
		pCmd.angles[YAW] = pPrevCmd.angles[YAW];
	}

	/* Store the previous frame command for 360 camera purposes */
	pPrevCmd = pCmd;

	/* Return the generated command with possible alterations */
	return pCmd;
}

/* Toogle or retreive the state of the Sabotage function of the engine
 * This is the function responsible for causing the random CL_ParsePacketEntities error when the exe has been tampered with
 *
 * NOTE: Tampering with it is technically illegal, so dont leak that feature is available
 * Still though, it's handy to 'magically' crash people that are causing trouble
 */
void JKG_SetSabotageState(int state) {
	*(int *)0xB8D544 = state;
}

int JKG_GetSabotageState() {
	return *(int *)0xB8D544;
}

extern vmCvar_t jkg_normalMapping;

void JKG_BeginGenericShader ( void );
void JKG_EndGenericShader ( void );

#if 1
static PatchData_t *genericShader;
trRefEntity_t **backend_currentEntity = (trRefEntity_t **)0xFE2A64;
static trRefEntity_t **tr_worldEntity = (trRefEntity_t **)0xFE3688;
static int *tr_currentEntityNum = (int *)0xFE3794;
static void **tr_currentModel = (void **)0xFE379C;
static void **tess_shader = (void **)0x1072B60;
static void **tr_refractionShader = (void **)0xFE3274;

static qboolean hasNormals;
static void JKG_Normals ( void )
{
#define TESS_NORMALS (0x105D3A0)
    if ( !jkg_normalMapping.integer )
    {
        hasNormals = qfalse;
        return;
    }
    
    if ( *tess_shader == *tr_refractionShader && cgs.glconfig.stencilBits >= 4 )
    {
        hasNormals = qfalse;
        return;
    }

    if ( *backend_currentEntity && ((*backend_currentEntity)->e.hModel || (*backend_currentEntity)->e.ghoul2) )
    {
        glNormalPointer (GL_FLOAT, 16, (const GLvoid *)TESS_NORMALS);
        glEnableClientState (GL_NORMAL_ARRAY);
        hasNormals = qtrue;
    }
    else
    {
        glDisableClientState (GL_NORMAL_ARRAY);
        hasNormals = qfalse;
    }
}

//#define R_DRAWELEMENTS_ADDRESS 0x4AC360
//#define DRAW_MULTITEXTURED_RET 0x4AC860
static JKG_HOOK _Hook_GenericShader()
{
    __asm
    {
        pushad
        mov eax, hasNormals
        test eax, eax
        jnz applyshader
        jmp plainrender
/*        mov eax, [backend_currentEntity]
        test eax, eax
        jnz nexttest
        jmp plainrender
        
nexttest:
        cmp eax, [tr_worldEntity]
        jne applyshader
        jmp plainrender*/
        
applyshader:
        call JKG_BeginGenericShader
        popad
        
        mov edi, 0x4AC360
        call edi
        
        pushad
        call JKG_EndGenericShader
        popad
        
end:
        push 0x4AF1B9/*0x4AC860*/
        ret
        
plainrender:
        popad
        
        mov edi, 0x4AC360
        call edi
        
        jmp end
    }
}

static PatchData_t *normalPointerPatch;
static JKG_HOOK _Hook_NormalPointer()
{
    __asm
    {
        pushad
        call JKG_Normals
        popad
        
        mov edi, 0x1
        
        push 0x4AF2BB
        ret
    }
}
#endif

/*
 * CVAR limit hack
 * This makes use of a little patch to ensure we get above 1224 cvars
 *
 *
 */
// Unused/doesn't work atm
static PatchData_t *cvarLimitHack[4];



// Controller h4x
// God. I hate JA's controller stuff. HATE IT TO DEATH
// This will properly fix the controller stuff, in due time
static PatchData_t *controllerHook;
static PatchData_t *movementControl;

// Allow for rendering of SWFs with an "swfMap" keyword in shader files
// --eez
static PatchData_t *swf1;
static PatchData_t *swf2;
char *swfMapText = "swfMap";

DWORD ebxStore;
DWORD ediStore;

image_t test;

// Hook ShaderParse to allow for a "swfMap" keyword

extern void	gameswf_startswf(char *filename);
extern void	gameswf_continueswf();
qboolean __cdecl ShaderParse_SWFMap()
{
	char *token;
	shaderStage_t *stage = (shaderStage_t *)ediStore;
	size_t sizeCheck = sizeof(shaderStage_t);
	token = COM_ParseExt( (const char **)ebxStore, qfalse );
	if(!token || !token[0])
	{
		Com_Printf("^3WARNING: missing parameter for keyword 'swfMap' in shader %s\n", (char *)0x1074198);
		return qfalse;
	}
	gameswf_startswf(token);
	// eezstreet NOTE:
	// Before I go any farther on this, I need to assign an actual function which loads all the essential
	// SWF data on it.
	// Basically what I need is a SWF render function which renders similar to how CIN_UploadCinematic works.
	// The SWF would be parsed and all that in _this_ func.

	*(char *)(ediStore+0x1B) = 2;
	//stage->bundle[0].isVideoMap = 2;		// this will be the method of detection.
											// In R_BindAnimatedImage, it is hooked at the
											// beginning to check if bundle->isVideoMap is 2.
											// If so, render a SWF.



	*(int *)(ediStore+0x1C) = /*YOUR_SWF_HANDLE_HERE*/ 5;		// this serves as the "index" of the SWF that is referenced
	// NEEDS AN IMAGE ARRAY:
	//memcpy(&stage->bundle[0].image[0], (void *)0x00FE3200, sizeof(image_t));				// make the system shut up about stage having no image.

	// just give it a dummy image so that the engine shuts the fuck up
	strcpy(test.name, token);
	stage->bundle[0].image = (image_t *)0x00FE324C;

	return qtrue;
}

// TODO: Finish
void __declspec(naked) _Hook_ParseShaders() {
	__asm {
		pushad
		mov ebxStore, ebx
		mov ediStore, edi
		mov ecx, 0x1869F
		mov eax, swfMapText
		mov edx, esi
		mov ebx, 0x004446C0
		call ebx
		test eax, eax
		jz swfMap
		mov ecx, 0x1869F
		mov eax, 0x555034						// "videoMap"
		mov edx, esi
		mov ebx, 0x004446C0						// Q_stricmp
		call ebx
		test eax, eax
		jnz notVideoMap
// You're a videoMap, Harry
		mov ebx, ebxStore
		popad
		push 0x004B38AB
		ret
notVideoMap:
		popad
		push 0x004B38F7
		ret
swfMap:
		pushfd
		call ShaderParse_SWFMap
		test cl,cl
		jz swfMapParseFailure
		popfd
		popad
		mov edi, ediStore
		mov ebx, ebxStore
		push 0x004B4443
		ret
swfMapParseFailure:
		popfd
		popad
		push 0x004B4480
		ret
	}
}

void GL_Bind ( unsigned int textureId );
void JKG_BindSWFTexture(int SWFHandle)
{
	// Bind your SWF frame here, a la CIN_UploadCinematic and CIN_RunCinematic
#ifdef __SWF__
	gameswf_continueswf();
#endif
	// qglTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, bitmapData );
}

DWORD addressThing;

// Hook into R_BindAnimatedImage.
// If a shader has a isVideoMap of -1, it does the SWF texture bind.
void __declspec(naked) _Hook_BindAnimatedImage()
{
	__asm
	{
		pushad
		mov al, [esi+0x1B]
		cmp al, 2
		jne notSWF
		mov eax, [esi+0x1C]
		push eax
		call JKG_BindSWFTexture
		add esp, 4
		popad
		//mov eax, 0x004B9690			// technically calling right into the middle of a func. Best we know what we're up against...
		//push 0x00416F20
		push 0x00416FC9
		ret
notSWF:
		popad
		// right, we need to recheck the ROQ crap since we kinda trashed that back there...
		push ecx
		push esi
		mov esi, eax
		mov al, [esi+0x1B]
		test al, al
		jz notCinematic
		mov eax, [esi+0x1C]
		mov addressThing, 0x004163D0
		call addressThing
		mov edx, [esi+0x1C]
		pop esi
		add esp, 4
		push 0x00416F20
		ret
notCinematic:
		push 0x004AC3EF
		ret
	}
}

// end

void JKG_PatchEngine() {
	int i;
	for (i=0; i<12; i++) {
		BspLoaderPatch[i] = JKG_PlacePatch(PATCH_CALL, (unsigned int)mapLoadTbl[i].returnAddr - 5, (unsigned int)_Hook_BSPLoadProgress);
	}
	BspSurfaceLoaderPatch[0] = JKG_PlacePatch(PATCH_JUMP, 0x48FEBE, ( unsigned int ) _Hook_BSPSurfaceLoadProgressInit);
	BspSurfaceLoaderPatch[1] = JKG_PlacePatch(PATCH_JUMP, 0x48FF43, ( unsigned int ) _Hook_BSPSurfaceLoadProgress);
	BspSurfaceLoaderPatch[2] = JKG_PlacePatch(PATCH_CALL, 0x48FF59, ( unsigned int ) _Hook_BSPSurfaceLoadFinished);
	
	#if 1
    genericShader = JKG_PlacePatch (PATCH_JUMP, 0x4AF1B4/*0x4AC85B*/, (unsigned int)_Hook_GenericShader);
    normalPointerPatch = JKG_PlacePatch (PATCH_JUMP, 0x4AF2B6, (unsigned int)_Hook_NormalPointer);
    #endif

	// TODO: Fix this -.-
	AttachClean( 0x41A170, ( unsigned long ) CL_CreateCmd, ( unsigned long * ) &pCL_CreateCmd );

	// Disable the notify system, so we wont get messages on top of the screen
	UnlockMemory(0x4178E4,2);
	*(short *)0x4178E4 = 0xE990;
	LockMemory(0x4178E4,2);

	controllerHook = JKG_PlacePatch( PATCH_CALL, 0x451566, ( unsigned int ) JKG_ControllerUpdate );
	movementControl = JKG_PlacePatch( PATCH_CALL, 0x41A1C2, ( unsigned int ) _Hook_CL_JoystickMovement );

	swf1 = JKG_PlacePatch( PATCH_JUMP, 0x004B3896, ( unsigned int ) _Hook_ParseShaders );
	swf2 = JKG_PlacePatch( PATCH_JUMP, 0x004AC3D0, ( unsigned int ) _Hook_BindAnimatedImage );
}

void JKG_UnpatchEngine() {
	int i;
	for (i=0; i<12; i++) {
		JKG_RemovePatch(&BspLoaderPatch[i]);
	}
	JKG_RemovePatch(&BspSurfaceLoaderPatch[0]);
	JKG_RemovePatch(&BspSurfaceLoaderPatch[1]);
	JKG_RemovePatch(&BspSurfaceLoaderPatch[2]);
	
	Detach( 0x41A170, ( unsigned long ) pCL_CreateCmd );
	#if 1
    JKG_RemovePatch (&genericShader);
    JKG_RemovePatch (&normalPointerPatch);
    #endif

	JKG_RemovePatch( &controllerHook );

	// Restore the notify system
	UnlockMemory(0x4178E4,2);
	*(short *)0x4178E4 = 0x8F0F;
	LockMemory(0x4178E4,2);

	JKG_RemovePatch( &swf1 );
	JKG_RemovePatch( &swf2 );
}