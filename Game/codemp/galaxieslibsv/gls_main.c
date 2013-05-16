// =================================================
// Server Auxiliary Library
// -------------------------------------------------
// This DLL is made to provide aid to the server-side
// module,  by performing tasks database queries,
// and hooks that need to be executed when the
// modules are unloaded
// =================================================

#include "exports.h"

static gls_exports_t	gls_exports;
static gls_imports_t	*gls_imports;

static int bEnginePatched = 0;
void JKG_PatchEngine();
void ActivateCrashHandler();

void GLS_PatchEngine(void) {
	if (!bEnginePatched) {
		JKG_PatchEngine();
		ActivateCrashHandler();
		bEnginePatched = 1;
	}
}

void GLS_BreakLinkup(void) {
	gls_imports = 0;
}

int GLS_JampGameAvailable() {
	if (gls_imports) {
		return 1;
	}
	return 0;
}

void GLS_DisplayStressLevel();
double GLS_GetStressLevel();

void GLS_ExtCrashInfo(int fileHandle) {
	if (gls_imports) {
		gls_imports->JKG_ExtCrashInfo(fileHandle);
	}
}

gls_exports_t *GLS_AuxLinkup(gls_imports_t *imports) {
	if (imports->version != G_AUX_VERSION) {
		return 0;	// Bad version, don't do linkup
	}
	gls_exports.version = GLS_AUX_VERSION;
	gls_exports.GLS_PatchEngine = GLS_PatchEngine;
	gls_exports.GLS_BreakLinkup = GLS_BreakLinkup;
	gls_exports.GLS_StressLevelInfo = ( void ( * )( void )) GLS_DisplayStressLevel;
	gls_exports.GLS_GetStressLevel = ( double ( * )( void )) GLS_GetStressLevel;
	gls_imports = imports;
	return &gls_exports;
}
