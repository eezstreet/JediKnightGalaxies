//       ____ ___________________   ___           ____  __ _______   ___  ________  ___ ______________
//      |    |\_   _____/\______ \ |   |         |    |/ _|\      \ |   |/  _____/ /   |   \__    ___/
//      |    | |    __)_  |    |  \|   |         |      <  /   |   \|   /   \  ___/    ~    \|    |   
//  /\__|    | |        \ |    `   \   |         |    |  \/    |    \   \    \_\  \    Y    /|    |   
//  \________|/_______  //_______  /___|         |____|__ \____|__  /___|\______  /\___|_  / |____|   
//                    \/         \/                      \/       \/            \/       \/           
//                         ________    _____   ____       _____  ____  ___ ______________ _________   
//                        /  _____/   /  _  \ |    |     /  _  \ \   \/  /|   \_   _____//   _____/   
//                       /   \  ___  /  /_\  \|    |    /  /_\  \ \     / |   ||    __)_ \_____  \    
//                       \    \_\  \/    |    \    |___/    |    \/     \ |   ||        \/        \   
//                        \______  /\____|__  /_______ \____|__  /___/\  \|___/_______  /_______  /   
//                               \/         \/        \/	   \/	   \_/			  \/        \/ (c)
// aux_cg_exports.h
// Auxiliary library imports and exports for the client-side
// Client-game version
// (c) 2013 Jedi Knight Galaxies

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
