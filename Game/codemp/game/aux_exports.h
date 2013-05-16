#define		GLS_AUX_VERSION		1
#define		G_AUX_VERSION		1

typedef struct auxlib_exports_s {
	int version;						// G_AUX_VERSION
	void (* JKG_ExtCrashInfo)(int fileHandle);
} gls_exports_t;

typedef struct auxlib_imports_s {
	int version;						// GLS_AUX_VERSION
	void (* GLS_BreakLinkup)(void);		// Call this when jampgame is about to be unloaded
	void (* GLS_PatchEngine)(void);
	void (* GLS_StressLevelInfo)(void);
	double (* GLS_GetStressLevel)(void);
} gls_imports_t;