////////////////////////////////////////
//
// JKG Auxiliary Library injector
// Client-side	/ UI module
//
// By BobaFett
//
/*
#include "aux_ui_exports.h"
#include "jkg_ui_auxlib.h"

gl_ui_exports_t gl_ui_exports;
gl_ui_imports_t *gl_ui_imports;

#undef INFINITE

#include <windows.h>

static void (*Com_Error)(int level, const char *fmt, ...) = (void (*)(int,const char*,...))0x437290;

void JKG_UI_LoadAuxiliaryLibrary()
{
	// Load the JKG client-side auxiliary library
	HMODULE handle;
	gl_ui_imports_t *(*GL_UI_AuxLinkup)(gl_ui_exports_t *exports);
	if ((handle = GetModuleHandle("galaxiesx86.dll")) == 0) {
		char *basepath = Cvar_VariableString("fs_basepath");
		char *fspath = Cvar_VariableString("fs_game");
		char *dllpath = FS_BuildOSPath(basepath, fspath, "galaxiesx86.dll");
		
		FS_CopyFile("galaxiesx86.dll");
		handle = LoadLibrary(dllpath);
		if (!handle) {
			Com_Error(0, "JKG_UI_LoadAuxiliaryLibrary failed: Could not load library\n");
			return;
		}
	}
	
	GL_UI_AuxLinkup = (gl_ui_imports_t *(*)(gl_ui_exports_t*))GetProcAddress(handle, "GL_UI_AuxLinkup");
	if (!GL_UI_AuxLinkup) {
		Com_Error(0, "JKG_UI_LoadAuxiliaryLibrary failed: GL_UI_AuxLinkup not located\n");
		return;
	}
	gl_ui_exports.version = UI_AUX_VERSION;
	gl_ui_imports = GL_UI_AuxLinkup(&gl_ui_exports);
	if (!gl_ui_imports) {
		Com_Error(0, "JKG_UI_LoadAuxiliaryLibrary failed: GL_UI_AuxLinkup failed\n");
		return;
	}
	// Initialize the threading system
	gl_ui_imports->GL_InitBackgroundWorker();
	// Purge pending tasks
	gl_ui_imports->GL_PurgeTasks();
}

void JKG_GLUI_PatchEngine()
{
	if (!gl_ui_imports) {
		return;
	}
	gl_ui_imports->GL_PatchEngine();
}

void JKG_GLUI_BreakLinkup()
{
	if (!gl_ui_imports) {
		return;
	}
	gl_ui_imports->GL_BreakLinkup();
	gl_ui_imports = 0;
}

void JKG_GLUI_ProcessTasks()
{
	if (!gl_ui_imports) {
		return;
	}
	gl_ui_imports->GL_ProcessTasks();

}

void JKG_GLUI_Task_Test(void (*callback)(asyncTask_t *task))
{
	if (!gl_ui_imports) {
		return;
	}
	gl_ui_imports->GL_Task_Test(callback);
}

void JKG_GLUI_Task_GetTermsOfUse(void (*callback)(asyncTask_t *task))
{
	if (!gl_ui_imports) {
		return;
	}
	gl_ui_imports->GL_Task_GetTermsOfUse(callback);
}


void JKG_GLUI_Task_RegisterUser(const char *username, const char *password, const char *email, void (*callback)(asyncTask_t *task))
{
	if (!gl_ui_imports) {
		return;
	}
	gl_ui_imports->GL_Task_RegisterUser(username, password, email, callback);
}

void JKG_GLUI_Task_Login(const char *username, const char *password, void (*callback)(asyncTask_t *task))
{
	if (!gl_ui_imports) {
		return;
	}
	gl_ui_imports->GL_Task_Login(username, password, callback);
}
*/