/*================================================================
//
//  UI <--> CGame crossover module
//  Allows UI and CGame to directly interact with eachother
//
//  Used in Jedi Knight Galaxies to enable the UI to directly communicate with the server
//
//  IMPORTANT: If you plan to use syscalls, use the CO_SysCall_** functions before you do so!
//
//  By BobaFett
//
//================================================================*/

// System concept: (updated)
// At startup, both UI and Cgame as linked up the auxiliary library
// To make the crossover system work, the crossover structures are
// sent to the auxiliary library, where the other module can obtain them
//
// Once a module is unloaded, it simply breaks the link with the aux-lib,
// causing the crossover interface to be discarded

// CGame Part

#include "cg_local.h"
#include "cg_public.h"

#include "aux_cg_exports.h"
extern gl_cg_imports_t *gl_cg_imports;

struct {	
	// Filled by UI
	void (* DoAntiCheat)();
	qboolean (* HandleServerCommand)(const char *command);
	void (* SetEscapeTrap)(int activate);
	void (* PartyMngtNotify)(int msg);
} * ui_crossover;

struct {
	// Filled by Cgame
	void (* SendClientCommand)(const char *command);
	qboolean (* EscapeTrapped)();
	void *(* PartyMngtDataRequest)(int data);
} cg_crossover;

void CO_SysCall_UI() {
	// Enable UI syscalls
#ifdef WIN32
	// Engine hack
	*(int *)0x12A4F18 = *(int *)0xB28224; // CurrentVm = uivm;
#else
	#error "CO_SysCall_UI: LINUX/MAC"
#endif
}
void CO_SysCall_CG() {
	// Enable CGame syscalls
#ifdef WIN32
	// Engine hack
	*(int *)0x12A4F18 = *(int *)0x8AF0FC; // CurrentVm = cgvm;
#else
	#error "CO_SysCall_CG: LINUX/MAC"
#endif
}

void CO_Shutdown() {
	if (gl_cg_imports) {
		gl_cg_imports->GL_RegisterCrossover(NULL);
	}
}

void CO_SendClientCommand(const char *cmd) {
	CO_SysCall_CG();
	trap_SendClientCommand(cmd);
	CO_SysCall_UI();
}

qboolean CO_EscapeTrapped() {
	// Return 1 to block, 0 to pass through
	if (cg.trapEscape) {
		CO_SysCall_CG();
		trap_SendClientCommand("~esc");
		CO_SysCall_UI();
		return 1;
	}
	return 0;
}

void CO_EscapeTrap(int activate) {
	if (gl_cg_imports) {
		ui_crossover = gl_cg_imports->GL_GetCrossover();
		if (ui_crossover) {
			ui_crossover->SetEscapeTrap(activate);
		}
	}
}

void CO_DoAntiCheat() {
	if (gl_cg_imports) {
		ui_crossover = gl_cg_imports->GL_GetCrossover();
		if (ui_crossover) {
			ui_crossover->DoAntiCheat();
		}
	}
}

qboolean CO_ServerCommand(const char *cmd) {
	if (gl_cg_imports) {
		ui_crossover = gl_cg_imports->GL_GetCrossover();
		if (ui_crossover) {
			return ui_crossover->HandleServerCommand(cmd);
		}
	}
	return qfalse;
}

void CO_PartyMngtNotify(int msg) {
	if (gl_cg_imports) {
		ui_crossover = gl_cg_imports->GL_GetCrossover();
		if (ui_crossover) {
			ui_crossover->PartyMngtNotify(msg);
		}
	}
}

void *CO_PartyMngtDataRequest(int data) {
	// Team Management data request from UI
	// Data 0 = Current party/invitations
	// Data 1 = Seeking players
	// Data 2 = Last seeking players refresh time (needed for delta feed)
	if (data == 0) {
		return &cgs.party;
	} else if (data == 1) {
		return &cgs.partyList;
	} else if (data == 2) {
		return (void *)cgs.partyListTime;
	} else {
		return 0;
	}
}

void CO_InitCrossover() {
	cg_crossover.SendClientCommand = CO_SendClientCommand;
	cg_crossover.EscapeTrapped = CO_EscapeTrapped;
	cg_crossover.PartyMngtDataRequest = CO_PartyMngtDataRequest;
	// Transmit this structure to the auxlib so UI can use it
	if (gl_cg_imports) {	// This should never be NULL, but just in case
		gl_cg_imports->GL_RegisterCrossover(&cg_crossover);
	}
}