// =================================================
// Client Auxiliary Library
// -------------------------------------------------
// This DLL is made to provide aid to the client-side
// modules, by performing tasks like cross-module
// communication, database queries, and hooks that
// need to be executed when the modules are unloaded
// =================================================


#include "gl_enginefuncs.h"
#include "jkg_threading.h"
#include "jkg_libcurl.h"
#include "gl_sound.h"

#include "exports.h"
#include "qcommon/game_version.h"

gl_ui_exports_t		gl_ui_exports;
gl_ui_imports_t		*gl_ui_imports;

gl_cg_exports_t		gl_cg_exports;
gl_cg_imports_t		*gl_cg_imports;

static void *gl_crossover_cg;	// Store the struct pointers
static void *gl_crossover_ui;	// Store the struct pointers

static int bEnginePatched = 0;
void JKG_PatchEngine();
void GL_SSE_Init();
void GL_DPE_Init();
void ActivateCrashHandler();

// To test the crash handler
void CrashMe(void)
{
	*(int *)0=0; // >=D!
}

void GL_PatchEngine(void)
{
	cvar_t *cvar;
	if (!bEnginePatched) {
		JKG_PatchEngine();
		GL_SSE_Init();
		GL_DPE_Init();
		ActivateCrashHandler();
		Cmd_AddCommand("#crash", CrashMe);
		bEnginePatched = 1;
	}
	// Unlock the dynamic glow cvars. That is, remove their cheat flag
	cvar = Cvar_FindVar("r_dynamicglowsoft");
	cvar->flags &= ~CVAR_CHEAT;
	cvar = Cvar_FindVar("r_dynamicglowintensity");
	cvar->flags &= ~CVAR_CHEAT;
	cvar = Cvar_FindVar("r_dynamicglowdelta");
	cvar->flags &= ~CVAR_CHEAT;
	cvar = Cvar_FindVar("r_dynamicglowpasses");
	cvar->flags &= ~CVAR_CHEAT;
	cvar = Cvar_FindVar("r_ambientScale");
	cvar->flags &= ~CVAR_CHEAT;

#ifdef DEBUG
	cvar = Cvar_FindVar("r_logFile");
	cvar->flags &= ~CVAR_CHEAT;
#endif

	// Register the JKG version
	Cvar_Get("clver", JKG_VERSION, CVAR_ROM|CVAR_USERINFO);
}

void GL_UI_BreakLinkup(void)
{
	gl_ui_imports = 0;
	gl_crossover_ui = 0;
}

void GL_CG_BreakLinkup(void)
{
	gl_cg_imports = 0;
	gl_crossover_cg = 0;
}

void GL_CG_RegisterCrossover(void *funcs)
{
	gl_crossover_cg = funcs;
}

void GL_UI_RegisterCrossover(void *funcs)
{
	gl_crossover_ui = funcs;
}

void *GL_CG_GetCrossover()
{
	return gl_crossover_ui;
}

void *GL_UI_GetCrossover()
{
	return gl_crossover_cg;
}

// ---------- Threading ---------------

static void (*quitFunc)( void ) = NULL;
void JKG_ShutdownWrapper( void )
{
	JKG_ShutdownThreading( 6000 );
	if (quitFunc) {
		quitFunc();
	}
}

void GL_UI_InitBackgroundWorker(void)
{
	if (!quitFunc) {
		quitFunc = Cmd_RedirectCommand("quit", JKG_ShutdownWrapper);
	}
	JKG_InitThreading();
}

void GL_UI_PurgeTasks(void)
{
	JKG_MainThreadPoller(1, 1);
}

void GL_UI_ProcessTasks(void)
{
	JKG_MainThreadPoller(1, 0);
}

void GL_CG_PurgeTasks(void)
{
	JKG_MainThreadPoller(0, 1);
}

void GL_CG_ProcessTasks(void)
{
	JKG_MainThreadPoller(0, 0);
}

// ---------- Tasks ---------------

void GL_UI_Task_Test(void (*callback)(void *))
{
	JKG_NewNetworkTask(LCMETHOD_TEST, callback, 1);
}

void GL_UI_Task_GetTermsOfUse(void (*callback)(void *))
{
	JKG_NewNetworkTask(LCMETHOD_GETTERMSOFUSE, callback, 1);
}

void GL_UI_Task_RegisterUser(const char *username, const char *password, const char *email, void (*callback)(void *))
{
	JKG_NewNetworkTask(LCMETHOD_REGISTERUSER, callback, 1, username, password, email);
}

void GL_UI_Task_Login(const char *username, const char *password, void (*callback)(void *))
{
	JKG_NewNetworkTask(LCMETHOD_LOGIN, callback, 1, username, password);
}

void GL_CG_Task_Test(void (*callback)(void *))
{
	JKG_NewNetworkTask(LCMETHOD_TEST, callback, 0);
}



// ---------- Linkup ---------------

gl_ui_exports_t *GL_UI_AuxLinkup(gl_ui_imports_t *imports)
{
	if (imports->version != UI_AUX_VERSION) {
		return 0;	// Bad version, don't do linkup
	}
	GL_UI_RegisterCrossover(0); // Clear the crossover data
	gl_ui_exports.version = GL_AUX_VERSION;
	gl_ui_exports.GL_PatchEngine = GL_PatchEngine;
	gl_ui_exports.GL_BreakLinkup = GL_UI_BreakLinkup;
	gl_ui_exports.GL_RegisterCrossover = GL_UI_RegisterCrossover;
	gl_ui_exports.GL_GetCrossover = GL_UI_GetCrossover;
	
	gl_ui_exports.GL_InitBackgroundWorker = GL_UI_InitBackgroundWorker;
	gl_ui_exports.GL_PurgeTasks = GL_UI_PurgeTasks;
	gl_ui_exports.GL_ProcessTasks = GL_UI_ProcessTasks;

	gl_ui_exports.GL_Task_Test = GL_UI_Task_Test;
	gl_ui_exports.GL_Task_GetTermsOfUse = GL_UI_Task_GetTermsOfUse;
	gl_ui_exports.GL_Task_RegisterUser = GL_UI_Task_RegisterUser;
	gl_ui_exports.GL_Task_Login = GL_UI_Task_Login;

	gl_ui_exports.GL_S_Init = JKG_S_Init;
	gl_ui_exports.GL_S_Update = JKG_S_Update;

	gl_ui_imports = imports;
	return &gl_ui_exports;
}

gl_cg_exports_t *GL_CG_AuxLinkup(gl_cg_imports_t *imports)
{
	if (imports->version != CG_AUX_VERSION) {
		return 0;	// Bad version, don't do linkup
	}
	GL_CG_RegisterCrossover(0); // Clear the crossover data
	gl_cg_exports.version = GL_AUX_VERSION;
	gl_cg_exports.GL_BreakLinkup = GL_CG_BreakLinkup;
	gl_cg_exports.GL_RegisterCrossover = GL_CG_RegisterCrossover;
	gl_cg_exports.GL_GetCrossover = GL_CG_GetCrossover;

	gl_cg_exports.GL_PurgeTasks = GL_CG_PurgeTasks;
	gl_cg_exports.GL_ProcessTasks = GL_CG_ProcessTasks;

	gl_cg_exports.GL_Task_Test = GL_CG_Task_Test;

	gl_cg_imports = imports;
	return &gl_cg_exports;
}