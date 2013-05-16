/////////////////////////////////////
//
// Jedi Knight Galaxies anti-cheat system
//
// NOTE: Code is obfuscated and intentionally corrupted to make it more difficult to hack
//
// Written by BobaFett
// Inspired by Vortex
//
// WARNING: SECURITY RISK!
// THIS CODE IS TO STAY HIDDEN AT ALL COSTS, DO NOT LEAK THIS FILE!
// THIS INCLUDES TRUSTED FRIENDS, NO ONE MAY GET HIS HANDS ON THIS FILE
// THE ONLY ONE ALLOWED TO SEND THIS FILE OR PARTS OF THIS TO OTHERS IS BOBA HIMSELF!
//
////////////////////////////////////



#include <malloc.h>
#include <stdlib.h>

char	* __cdecl va( char *format, ... );

#define _db __asm _emit //(so we can insert data more easilly)

int UI_AC_DoAC(); // Used in AddyTbl, so specify prototype
void UI_AC_DoAC_bridge();
void UI_AC_GetErrorFunc();
void UI_AC_CrashJA();
void UI_AC_Bridge();
void UI_AC_PushMasterKey();
// Table used to obscure calls, that way we'll never have a direct function address in-code
// Offsets of functions in this table:
// UI_AC_DoAC: 0x20
// UI_AC_DoAC_bridge: 0x38
// UI_AC_GetErrorFunc 0x54
// Table is filled with random data (that look like pointers)
int AddyTbl[] = {0x40004353,0x40003453,0x40007568,0x40007AB2,0x40001324,0x400A9012,0x4001A034, 0x4002B31C,
/*0x20*/		(int)UI_AC_DoAC,0x4000BA12,0x4000CA82,0x40019231,0x4001BB29,0x400BBA12, 
/*0x38*/		(int)UI_AC_DoAC_bridge,0x40058456,0x40008353,0x4001AA84,0x400288B6,0x4009342B,0x40023423, 
/*0x54*/		(int)UI_AC_GetErrorFunc,0x400B2921,0x40033034,0x40003923,0x400183B2,0x4009B33C,0x4003B374,
/*0x70*/		(int)UI_AC_CrashJA, 0x40023123,0x400A3423,0x40049234,0x40052893,0x40033493,0x40093423,
/*0x8C*/		(int)UI_AC_Bridge, 0x40034234,0x40009487,0x40018549,0x40025394,0x40009348,
/*0xA4*/		(int)UI_AC_PushMasterKey, 0x40012312, 0x40012392, 0x40022942, 0x400A9123};

// 512 bit decryption key
//unsigned char __MasterKey[64] = "\x0F\x3C\x2E\x60\x5E\x96\x86\x5E\x90\xB7\x38\x48\x16\xA3\x47\xE3\x29\xF3\xF5\x37\x4B\x7B\xDD\x4B\xEF\x29\x3D\x30\x77\x2F\x90\x41\x60\x64\x65\xA8\xB9\xE7\x6B\x25\xD9\x07\xCB\x0B\x52\x5F\xBB\x8A\x21\x46\x51\x35\x28\x66\x95\xBB\xDD\xAA\xD9\xF8\x43\x5E\x4D\x32";
                       
// Error strings

// Jedi Knight Galaxies Anti-Cheat: Cheating detected (Code: %i)
//unsigned char __CheatingDetected[55] = "\x37\x70\xF1\x7A\xA9\x66\x6D\x78\xF0\x6A\x3B\xFB\xD6\xA2\xA1\xE0\x6A\xDD\x1B\x2B\xCC\xD6\x96\xDB\x9F\x72\xE2\x64\x6C\x06\xF4\xC9\xC0\xE0\x78\x99\x85\x21\x6A\x94\x84\xE6\x83\xD2\x9F\x5D\x96\x07\x8C\x44\x17\xDA\xCA\x77";

// ~~actr %i
//unsigned char __ACTR[10] = "\xF6\x3C\x11\x77\x6D\x1A\x3C\x34\x12\x91"

// String decryption algorithm
// Algorithm (CLASSIFIED INFORMATION)
//
// In[i] + key[(i) & 63] - Wraparound addition
// In[i] <<c (key[(i+1) & 63]&7) - Circular shift left
// Int[i] ^= key[(i+2) & 63] - Xor it
//
// Algorithm requires a 512 bit key (64 bytes)
// Algorithm, encrypt and decrypt functions made by BobaFett

/*
#ifndef NDEBUG
unsigned int _rotl(unsigned char value, int shift) {
    shift &= 7;
    return (value << shift) | (value >> (8 - shift));
}
 
unsigned int _rotr(unsigned char value, int shift) {
    shift &= 7;
    return (value >> shift) | (value << (8 - shift));
}
#endif
*/

void DecryptString(unsigned char *masterkey, unsigned char *source, int len, unsigned char* dest) {
	// Dest must be the same size as Source!
	int i;
	for (i=0; i<len; i++) {
		*dest = *source + masterkey[i & 63];
		*dest = (unsigned char)_rotl(*dest, masterkey[(i+1) & 63] & 7);
		*dest ^= masterkey[(i+2) & 63];
		dest++;
		source++;
	}
}

void EncryptString(unsigned char *masterkey, unsigned char *source, int len, unsigned char* dest) {
	// Dest must be the same size as Source!
	int i;
	for (i=0; i<len; i++) {
		*dest = *source;
		*dest ^= masterkey[(i+2) & 63];
		*dest = (unsigned char)_rotr(*dest, masterkey[(i+1) & 63] & 7);
		*dest -= masterkey[i & 63];
		dest++;
		source++;
	}
}

void __declspec(naked) UI_AC_PushMasterKey() {
	// Most epic form of hiding data :P in-function ref xD
	__asm {
		// Reconstruct current function address
		call luls
luls:
		pop eax
		lea eax, [eax+0x6]		
		jno lols		// Since xor clears the overflow flag, this jump is always taken
		_db 0x0F _db 0x3C _db 0x2E _db 0x60 _db 0x5E _db 0x96 _db 0x86 _db 0x5E _db 0x90 _db 0xB7 _db 0x38 _db 0x48 _db 0x16 _db 0xA3 _db 0x47 _db 0xE3 _db 0x29 _db 0xF3 _db 0xF5 _db 0x37 _db 0x4B _db 0x7B _db 0xDD _db 0x4B _db 0xEF _db 0x29 _db 0x3D _db 0x30 _db 0x77 _db 0x2F _db 0x90 _db 0x41 _db 0x60 _db 0x64 _db 0x65 _db 0xA8 _db 0xB9 _db 0xE7 _db 0x6B _db 0x25 _db 0xD9 _db 0x07 _db 0xCB _db 0x0B _db 0x52 _db 0x5F _db 0xBB _db 0x8A _db 0x21 _db 0x46 _db 0x51 _db 0x35 _db 0x28 _db 0x66 _db 0x95 _db 0xBB _db 0xDD _db 0xAA _db 0xD9 _db 0xF8 _db 0x43 _db 0x5E _db 0x4D _db 0x32
lols:
		pop ebx
		push eax
		push ebx
		ret



	}
}

// Oki 2 lil thingies here:
// CL_SendClientCommand is located at 41C6F0, pass message through edi
// CL_WritePacket is located at 41A470 (no args)

void __declspec(naked) UI_AC_CrashJA() {
	// Ok if we get here its time to kill JA
	__asm {
		// Decrypt the message and do a forged call to com_error (hidden return point)
		// This function will never return.
		// In case Com_Error is somehow rigged up, 
		//  the return address is set to Sys_Quit
		// So no matter what happens, JA will go down.
		push edi		// Secure return code
		push 10
		call malloc
		add esp,4
		push eax		// Secure ptr
		push eax		// Arg: dest
		push 10			// Arg: len
		jno cont
// First, inform the server of this
actr: 
		
		// In-function string
		// '~~actr %i' in encrypted form
		_db 0xF6 _db 0x3C _db 0x11 _db 0x77 _db 0x6D _db 0x1A _db 0x3C _db 0x34 _db 0x12 _db 0x91
cont:
		push actr
		lea eax, [AddyTbl]
		mov eax, [eax+0xA4]		// Get addy of UI_AC_PushMasterKey
		push MKR
		push eax			// Call it, will push the master key on the stack
		ret
MKR:						// MKR: Master Key Returnpoint
		call DecryptString // Decrypt our string
		add esp, 0x10	// Free args to DecryptString()
		// Args are now: edi (pushed on top) and the formatted string
		call va			// Call va, eax now contains ptr to our (formatted) string
		pop ebx			// Pop malloced ptr

		push eax		// Push formatted ptr
		push ebx		// Push malloced ptr
		call free		// Free the allocation
		add esp, 4		// Free args to free()
		pop eax			// Pop formatted string, next pop is edi btw :P
		mov edi,eax		// edi = message
		mov eax, 0x40C0F0	// Construct the ptr to avoid traces
		mov ebx, 0x010600
		or eax,ebx
		call eax
		xor eax,eax
		push eax
doagain:
		pop eax
		inc eax
		cmp eax,3
		jg docrash
		push eax
		mov eax, 0x40A070	// Construct the ptr to avoid traces
		mov ebx, 0x010400
		or eax,ebx
		push doagain
		jmp eax
docrash:
// Crash the game
		push 54
		call malloc		// Alloc space for the message
		add esp, 4
		push eax		// Secure ptr
		push eax		// Arg: dest
		push 54			// Arg: len
		jno cont2
cheatingdetected:			
		// In-function string
		// 'Jedi Knight Galaxies Anti-Cheat: Cheating detected (Code: %i)' in encrypted form
		_db 0x37 _db 0x70 _db 0xF1 _db 0x7A _db 0xA9 _db 0x66 _db 0x6D _db 0x78 _db 0xF0 _db 0x6A _db 0x3B _db 0xFB _db 0xD6 _db 0xA2 _db 0xA1 _db 0xE0 _db 0x6A _db 0xDD _db 0x1B _db 0x2B _db 0xCC _db 0xD6 _db 0x96 _db 0xDB _db 0x9F _db 0x72 _db 0xE2 _db 0x64 _db 0x6C _db 0x06 _db 0xF4 _db 0xC9 _db 0xC0 _db 0xE0 _db 0x78 _db 0x99 _db 0x85 _db 0x21 _db 0x6A _db 0x94 _db 0x84 _db 0xE6 _db 0x83 _db 0xD2 _db 0x9F _db 0x5D _db 0x96 _db 0x07 _db 0x8C _db 0x44 _db 0x17 _db 0xDA _db 0xCA _db 0x77
cont2:
		push cheatingdetected
		lea eax, [AddyTbl]
		mov eax, [eax+0xA4]		// Get addy of UI_AC_PushMasterKey
		push MKR2
		push eax			// Call it, will push the master key on the stack
		ret
MKR2:						// MKR: Master Key Returnpoint
		call DecryptString // Decrypt our string
		add esp, 0x10	// Free args to DecryptString()
		call va			// Call va, eax now contains ptr to our (formatted) string
		pop ebx			// Pop malloced ptr
		add esp,4		// Destroy error code (no longer needed :P)
		push eax		// Push formatted ptr
		push ebx		// Push malloced ptr
		call free		// Free the allocation
		add esp, 4		// Free args to free()
		pop eax
		//add esp,0x1000	// Do a bigass stack wipe, to screw up any stacktraces
		//					// And lets not forget that ebp is gone too
		// Fuck, crashes..
		push eax		// Error message
		push 0			// Com_Error code (ERR_FATAL)
		push 0x450180	// Fake return address (Sys_Quit)
		push 0x437290	// Com_Error (for push-ret jump)
		ret				// Good bye JA
	}
}

void __declspec(naked) UI_AC_GetErrorFunc() {
		__asm {
		lea eax, [AddyTbl]
		mov eax, [eax+0x70]
		lea eax, [eax+0xFFFF] // Get the address to UI_AC_CrashJA with mask
		mov ebx, eax
		and ebx, 0x0F0F0F0F0   // Cut it into pieces
		mov ecx, eax
		and ecx, 0x0F0F0F0F
		pop eax  // Pop return addy
		push ebx // Push both parts
		push ecx
		add eax,3 // Offset of return addy to jump over the bogus code
		push eax // Push return addy
		mov edx,1 // Set edx to 1 (used by return point)
		ret     // Go back
	}
}

int UI_AC_DoAC() {
	// Do the actual work here
	// The naked functions are just the decoy bridges, the real stuff happens here
	// Return values:
	// 0 = all ok, resume
	// Anything else: Error code (will be displayed on crash)


	return 0;
}

static int tmp;

void __declspec(naked) UI_AC_DoAC_bridge() {
	__asm {
		push edx		// Secure main return point
		push retpos		// forge return address
		lea eax, [AddyTbl]
		mov eax, [eax+0x20]	 // <-- contains pointer to UI_AC_DoAC ;)
		push eax
		ret				// 'call' UI_AC_DoAC
		_emit 0xE8		// Disassembler decoy
retpos:
		pop edx			// Restore main return point
		xor ecx,ecx		// ecx = 0
		test eax,eax	// check if UI_AC_DoAC returned 0
		setne cl		// if so, ecx = 0, else ecx = 1
		mov edi,eax		// put the return value in edi
		pop ebx			// Masked and forged return address
		lea ebx, [ebx-0xDEADF00D]	// Unmask
		push ebx		// Push-ret 
		ret				// Go
	}
}


void __declspec(naked) UI_AC_Exit() {
	__asm {
		pop eax			// Remove both parts of the crash function location
		pop eax
		push edx		// Do a push-ret to the main return point
		ret
	}
}

void __declspec(naked) UI_AC_Fail() {
	__asm {
		pop eax			// Obtain both parts of the crash function location
		pop ebx
		or eax,ebx		// Join them together
		lea eax, [eax-0xFFFF]	// Unmask it
		push eax		// Push-ret to it
		ret
	}
}


int ExitLoc[] = {(int)UI_AC_Exit, (int)UI_AC_Fail};

void __declspec(naked) UI_AC_Bridge() {
	__asm {
		jmp [ExitLoc + ecx*4]	// ecx = 0 if UI_AC_DoAC returned 0, otherwise ecx is 1
								// See ExitLoc array for meaning :P
	}
}

void __declspec(naked) UI_AC_AntiCheat() {
	__asm{
		xor ebp,0x0FFFFFFFF
		xor esp,0x0FFFFFFFF			 // Annoy debuggers and disassemblers
		xor esp,0x0FFFFFFFF

		xor edx,edx					// edx = 0
		call tmplbl					// Call the label (so we push the addy)
		ret
		_emit 0xE8					// Bogus code (also requires GetErrorFunc to add 2 to the return addy)
		_emit 0xC1
tmplbl:
		test edx,edx				// First time edx is 0, after GetErrorFunc returns, its 1
		jne cont					// If GetErrorFunc returned, go to cont
		lea eax, [AddyTbl]
		mov eax,[eax+0x54]
		push eax					// Jump to geterrorfunc (return addy is tmplbl)
		ret
		_emit 0x00					// Bogus code
cont:
		xor edx,edx					// Clear edx again
		call getret					// go to getret (push return addy)
		_emit 0xCC					// EOF decoy (int 3)
		_emit 0xCC  
		ret							// Execution never gets here
		_emit 0xCC
getback:
		pop edx						// Get return address
		mov eax, [edx-4]			// Read call operand
		add edx, eax				// Reconstruct address of getret
		lea eax, [AddyTbl]
		mov eax, [eax+0x8C]
		lea eax, [eax + 0xDEADF00D] // Get addy + mask
		push eax
		lea eax, [AddyTbl]			// Push addy + mask (from table)
		mov eax, [eax+0x38]
		push eax
		ret							// 'call' UI_AC_DoAC_bridge, returns in at edx (so getret)
getret:
		test edx,edx				// After returning from UI_AC_DoAC_bridge, its no longer 0
		je getback
		pop eax						// Raise return address by 1, to jump over the emit
		lea eax, [eax+1]
		push eax
		xor ebp,0x0FFFFFFFF			// Undo the ebp inversion we did before :P
		ret							// And we return
	}
}

// Used for the obfuscated bridge to the function above
void __declspec(naked) UI_AC_DoOverflow() {
	__asm {
		mov eax,0x18231922		// Purposely overflow
		imul eax,eax,0x23192	
		jo lol
		_emit 0xE8 // Direct byte injections to corrupt the code
		_emit 0xCC // Execution will never get here, disassemblers will :P
		_emit 0x00
lol:
		push lolwow	// Disassembler decoy push ret jump
		ret
		_emit 0x92 // Decoy for disassemblers
		_emit 0xb8
		_emit 0xCC
		_emit 0xe8
lolwow:
		pop eax		// Remove return address
		push ecx	// Push provided ecx return address
		ret			// And return
	}
}