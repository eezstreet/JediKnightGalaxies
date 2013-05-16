////////////////////////////////////////
//
// JKG Auxiliary Library injector
// Server-side
//
// By BobaFett
//

#include "g_local.h"
#include "aux_exports.h"

gls_exports_t gls_exports;
gls_imports_t *gls_imports;
#undef INFINITE

#ifdef _WIN32
	#include <windows.h>
	static char *(* FS_BuildOSPath)( const char *base, const char *game, const char *qpath ) = (char *(*)(const char*,const char*, const char*))0x412E60;
	static int (* FS_CopyFile)(const char *filename) = (int (*)(const char*))0x44C080;
	static char *(* Cvar_VariableString)(const char *cvarname) = (char *(*)(const char*))0x411EF0;

#else
	#include <dlfcn.h>
	static char *(* FS_BuildOSPath)( const char *base, const char *game, const char *qpath ) = (char *(*)(const char*,const char*, const char*))0x812B8F4;
	static char *(* Cvar_VariableString)(const char *cvarname) = (char *(*)(const char*))0x80756F4;

	#define FS_CopyFile

	#define LoadLibrary(a)			dlopen(a, RTLD_LAZY);
	#define GetProcAddress(a, b)	dlsym(a, b)
	#define HMODULE		int

#endif

// Since linux lacks this function, we'll define our own which always returns 0
#ifdef __linux__
	int GetModuleHandle(const char *module) {
		return 0;
	}
#endif

void JKG_ExtCrashInfo(int fileHandle);

void JKG_LoadAuxiliaryLibrary() {
	// Load the JKG server-side auxiliary library
	HMODULE handle;
	gls_imports_t *(*GLS_AuxLinkup)(gls_exports_t *exports);
	if ((handle = GetModuleHandle("galaxiessvx86.dll")) == 0) {
		char *basepath = Cvar_VariableString("fs_basepath");
		char *fspath = Cvar_VariableString("fs_game");
#ifdef __linux__
                char *dllpath = FS_BuildOSPath(basepath, fspath, "galaxiessvi386.so");
#else
                char *dllpath = FS_BuildOSPath(basepath, fspath, "galaxiessvx86.dll");
                FS_CopyFile("galaxiessvx86.dll");
#endif
		handle = LoadLibrary(dllpath);
		if (!handle) {
                    #ifdef __linux__
                        G_Error("JKG_LoadAuxiliaryLibrary failed: Could not load library (%s) - %s\n", dllpath, dlerror());
                    #else
                        G_Error("JKG_LoadAuxiliaryLibrary failed: Could not load library (%s)\n", dllpath);
                    #endif
			return;
		}
	}

	GLS_AuxLinkup = (gls_imports_t *(*)(gls_exports_t*))GetProcAddress(handle, "GLS_AuxLinkup");
	if (!GLS_AuxLinkup) {
                G_Error("JKG_LoadAuxiliaryLibrary failed: GLS_AuxLinkup not located\n");
                return;
	}
	gls_exports.version = G_AUX_VERSION;
	gls_exports.JKG_ExtCrashInfo = JKG_ExtCrashInfo;
	gls_imports = GLS_AuxLinkup(&gls_exports);
	if (!gls_imports) {
		G_Error("JKG_LoadAuxiliaryLibrary failed: GLS_AuxLinkup failed\n");
		return;
	}
}

void JKG_GLS_PatchEngine() {
	if (!gls_imports) {
		return;
	}
	gls_imports->GLS_PatchEngine();
}

void JKG_GLS_BreakLinkup() {
	if (!gls_imports) {
		return;
	}
	gls_imports->GLS_BreakLinkup();
}

void JKG_GLS_StressLevelInfo() {
	if (!gls_imports) {
		return;
	}
	gls_imports->GLS_StressLevelInfo();
}

double JKG_GLS_GetStressLevel() {
	if (!gls_imports) {
		return 0;
	}
	return gls_imports->GLS_GetStressLevel();
}
