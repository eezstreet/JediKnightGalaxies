// Auxiliary library imports and exports for the client-side
// Client-game version

#define		GL_AUX_VERSION		3
#define		CG_AUX_VERSION		2


typedef struct auxlib_cg_exports_s {
	int version;						// CG_AUX_VERSION
} gl_cg_exports_t;

typedef struct auxlib_cg_imports_s {
	int version;						// GL_AUX_VERSION
	void (* GL_BreakLinkup)(void);		// Call this when cgame is about to be unloaded
	void (* GL_RegisterCrossover)(void *funcs);
	void *(* GL_GetCrossover)();
	// Threading system
	void (* GL_PurgeTasks)(void);
	void (* GL_ProcessTasks)(void);
	// Tasks
	void (* GL_Task_Test)( void (*callback)(void *) );
} gl_cg_imports_t;
