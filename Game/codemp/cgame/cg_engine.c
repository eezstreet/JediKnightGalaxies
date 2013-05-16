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
//  Caution: Windows only

#include <windows.h>
#include <Psapi.h>
#include "q_shared.h"
#include "cg_local.h"
#include "cg_public.h"

int OldCodeProtect;
void __inline OpenJAMPMemory() {
	// Unlocks the JA Codepage
	// MUST BE CALLED PRIOR TO ANY CODEPAGE ALTERATION!!!
	// DO NOT CALL MORE THAN ONCE!! (until CloseJAMPMemory is called that is)
	VirtualProtect((LPVOID)0x00400000,0x00180000,PAGE_EXECUTE_READWRITE,(PDWORD)&OldCodeProtect);
}

void __inline CloseJAMPMemory() {
	// Locks the JA Codepage again
	// ALWAYS CALL AFTER YOU'RE FINISHED WITH CODEPAGE ALTERATIONS!
	// DO NOT CALL PRIOR TO CALLING OPENJAMPMEMORY!!!
	VirtualProtect((LPVOID)0x00400000,0x139E85,OldCodeProtect, NULL);
}

int tmpval; // Temporary storage

// _i prefix stands for Internal, to avoid naming conflicts

int __declspec(naked) _iCom_Printf(const char *fmt, ...) {
	// Call redirect (calls the Com_Printf function in the engine)
	__asm
	{
		pop eax
		mov tmpval ,eax
		mov eax, 0x437080
		call eax
		mov ebx, tmpval
		push ebx
		ret
	}
}


int Hook_CmdFilter() {
	const char *cmd = *(const char **)0xB39808;
	if (cmd[0] == '~') {
		// Internal networking command, block
		_iCom_Printf("Illegal command: %s\n", cmd);
		return 1;
	}
	return 0;
}

void __declspec(naked) _Hook_CmdFilter() {
	__asm {
		pushad
		call Hook_CmdFilter
		test eax,eax
		jne block
		popad
		mov ecx, DS:[0x954B6C]
		push 0x41D612
		ret
block:
		popad
		push 0x41D642
		ret
	}

}

void PatchEngine() {
	int tmp;
	unsigned char hook[16];
	OpenJAMPMemory();
	
	// Entities per snapshot modification (CL_GetSnapshot)
	// 00411885 - 7E 18 - JLE SHORT jamp.0041189F
	// Replaced by:
	// 00411885 - EB 18 - JMP SHORT jamp.0041189F
	*(unsigned char *)0x411885 = 0xEB;
	// 00411834 - 8995 54200200 - MOV DWORD PTR SS:[EBP+22054],EDX
	// Replaced by:
	// 00411834 - 8995 54C50800 - MOV DWORD PTR SS:[EBP+85C54],EDX
	*(unsigned int *)0x411836 = 0x85C54;

	// Command filter hook
	// 0041D60C - 8B0D 6C4B9500 - MOV ECX, DWORD PTR DS:[954B6C]
	// Replaced By:
	// 0041D60C - E9 xxxxxxxx 90 - JMP _Hook_CmdFilter; NOP
	tmp = (int)_Hook_CmdFilter;
	hook[0] = (unsigned char)0xE9; // JMP
	hook[5] = (unsigned char)0x90; // NOP
	*(unsigned int *)(&hook[1]) = tmp - (0x41D60C+5);
	memcpy((void *)0x41D60C, &hook, 6);

	CloseJAMPMemory();
}

void UnpatchEngine() {
	// Reverting everything back to its original code
	OpenJAMPMemory();
	// Entities per snapshot modification (CL_GetSnapshot)
	// 00411885 - 7E 18 - JLE SHORT jamp.0041189F
	*(unsigned char *)0x411885 = 0x7E;
	*(unsigned int *)0x411836 = 0x22054;


	// Revert Command filter hook
	memcpy((void *)0x41D60C, "\x8B\x0D\x6C\x4B\x95\x00", 6);

	CloseJAMPMemory();
}


typedef struct cmd_function_s {
	struct cmd_function_s	*next;
	char					*name;
	void					*function;
} cmd_function_t;

void *Cmd_GetCommand(const char *cmdname) {
	// This function returns the function a command points to (if any)
	// And returns the original function (if you want to restore it later)
	cmd_function_t *cmd;
	for(cmd = (cmd_function_t *)0xB3CC18; cmd; cmd = cmd->next) {
		if (!Q_stricmp(cmd->name, cmdname)) {
			// Found it
			return cmd->function;
		}
	}
	return NULL;
}

void *Cmd_EditCommand(const char *cmdname, void *newfunction) {
	// This function redirects an already registered command to a new function
	// And returns the original function (if you want to restore it later)
	cmd_function_t *cmd;
	void *ret = NULL;
	for(cmd = (cmd_function_t *)0xB3CC18; cmd; cmd = cmd->next) {
		if (!Q_stricmp(cmd->name, cmdname)) {
			// Found it
			ret = cmd->function;
			cmd->function = newfunction;
			break;
		}
	}
	return ret;
}

