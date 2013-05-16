//////////////////////
//
// Jedi Knight Galaxies
// Anti-debug
//
//////////////////////

#include <windows.h>
#pragma warning( disable: 4733 )

void __declspec(naked) JKG_AntiDebug() {
	// EVIL! :P
	// We simply jump to 0x01
	// If we got a debugger here, it goes boom
	// Otherwise, SEH is executed, and makes it exit properly
	__asm {
		// not now..
		ret
	}
	__asm {
		push offset handler
		push fs:[0]
		mov fs:[0], esp
		sub esp, 0x3520
		xor eax,eax
		inc eax
		push eax
		ret
		mov eax, 0xD0A0C0D0
		and eax, 0x0E0D000E
		push eax
		mov eax, ExitProcess
		call eax
		ret
		__emit 0xE9
handler:
		mov ecx, [esp+0xc]		// Get CONTEXT
		mov eax, [ecx+0xC4]		// Get ESP
		mov ebx, [eax+0x3524]	// Get Handler
		lea ebx, [ebx-2]		// Quick way to get Handler-2 :P (the ret)
		lea eax, [eax+0x3520]
		mov fs:[0], eax			// Revert to old handler
		add dword ptr [ecx+0xC4], 0x3528 // Clean up stack
		mov dword ptr [ecx+0xB8], ebx	// Update EIP
		xor eax,eax				// Tell the kernel to resume execution at our updated EIP
		ret						// Boom :P
	}
}