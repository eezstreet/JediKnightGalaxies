////////////////////////////////////////
//
// JKG Auxiliary Library injector
// Client-side	/ CGame module
//
// By BobaFett
//

#include "aux_cg_exports.h"
#include "jkg_cg_auxlib.h"

gl_cg_exports_t gl_cg_exports;
gl_cg_imports_t *gl_cg_imports;
#undef INFINITE

#include <windows.h>

void CG_Error( const char *msg, ... );
void trap_Error( const char *fmt );

void JKG_CG_LoadAuxiliaryLibrary()
{
	// Load the JKG client-side auxiliary library
	HMODULE handle;
	gl_cg_imports_t *(*GL_CG_AuxLinkup)(gl_cg_exports_t *exports);

	if ((handle = GetModuleHandle("galaxiesx86.dll")) == 0) {
		// UI is responsible for loading this, if it aint loaded, error out
		CG_Error("JKG_CG_LoadAuxiliaryLibrary failed: Could not load library\n");
	}
	
	GL_CG_AuxLinkup = (gl_cg_imports_t *(*)(gl_cg_exports_t*))GetProcAddress(handle, "GL_CG_AuxLinkup");
	if (!GL_CG_AuxLinkup) {
		trap_Error("JKG_CG_LoadAuxiliaryLibrary failed: GL_CG_AuxLinkup not located\n");
		return;
	}
	gl_cg_exports.version = CG_AUX_VERSION;
	gl_cg_imports = GL_CG_AuxLinkup(&gl_cg_exports);
	if (!gl_cg_imports) {
		trap_Error("JKG_CG_LoadAuxiliaryLibrary failed: GL_CG_AuxLinkup failed\n");
		return;
	}

	gl_cg_imports->GL_PurgeTasks();
}

void JKG_GLCG_BreakLinkup()
{
	if (!gl_cg_imports) {
		return;
	}
	gl_cg_imports->GL_BreakLinkup();
	gl_cg_imports = 0;
}

void JKG_GLCG_ProcessTasks()
{
	if (!gl_cg_imports) {
		return;
	}
	gl_cg_imports->GL_ProcessTasks();
}

void JKG_GLCG_Task_Test(void (*callback)(asyncTask_t *task))
{
	if (!gl_cg_imports) {
		return;
	}
	gl_cg_imports->GL_Task_Test(callback);

}