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
// gl_cin.c
// OGV/OGM Playback Support
// (c) 2013 Jedi Knight Galaxies

// This here is to enable the playback of ogg theora/vorbis video files
// While theora creates videos with quality comparable to Xvid and DivX
// (which means you can see artifacts), its still much better than
// the alternative: RoQ files.

// These adjustments are hooked directly into the engine's CIN system,
// which is responsible for RoQ playback (and soon ogv/ogm playback)

// NOTE: This system is heavilly based on Xreal's ogv/ogm implementation

// gl_cin.c is responsible for linking the new interface into JA's CIN
// system and processing all the required calls
// 
// gl_ogv.c is responsible for the actual processing of the video

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "gl_enginefuncs.h"

#pragma warning( disable: 4996 )

#pragma optimize( "g", off )

#define MAX_OSPATH 256
typedef unsigned char byte;
typedef int fileHandle_t;

// cinematic states
typedef enum {
	FMV_IDLE,
	FMV_PLAY,		// play
	FMV_EOF,		// all other conditions, i.e. stop/EOF/abort
	FMV_ID_BLT,
	FMV_ID_IDLE,
	FMV_LOOPED,
	FMV_ID_WAIT
} e_status;

// This struct differs from quake 3's version!
typedef struct {
	char				fileName[MAX_OSPATH];
	int					CIN_WIDTH, CIN_HEIGHT;
	int					xpos, ypos, width, height;
	qboolean			looping, holdAtEnd, dirty, alterGameState, silent, shader;
	fileHandle_t		iFile;
	e_status			status;
	unsigned int		startTime;
	unsigned int		lastTime;
	long				tfps;
	long				RoQPlayed;
	long				ROQSize;
	unsigned int		RoQFrameSize;
	long				onQuad;
	long				numQuads;
	long				samplesPerLine;
	unsigned int		roq_id;
	long				screenDelta;

	void ( *VQ0)(byte *status, void *qdata );
	void ( *VQ1)(byte *status, void *qdata );
	void ( *VQNormal)(byte *status, void *qdata );
	void ( *VQBuffer)(byte *status, void *qdata );

	//long				samplesPerPixel;				// defaults to 2
	byte*				gray;
	unsigned int		xsize, ysize, maxsize, minsize;

	qboolean			/*half, smootheddouble,*/inMemory;
	long				normalBuffer0;
	long				roq_flags;
	long				roqF0;
	long				roqF1;
	long				t[2];
	long				roqFPS;
	int					playonwalls;
	byte*				buf;
	long				drawX, drawY;
} cin_cache;

#define MAX_VIDEO_HANDLES 16

qboolean IsOGMHandle[MAX_VIDEO_HANDLES];

cin_cache *CIN_GetCinByHandle(int handle) {
	if (handle < 0 || handle >= MAX_VIDEO_HANDLES) {
		return 0;
	}
	return (cin_cache *)(0x577A68 + (440 * handle));
}

int *currentHandle = (int *)0x571FDC;
int *CL_handle = (int *)0x571FE0;
int *cls_state = (int *)0x8AF100;
cvar_t *cl_inGameVideo; // = *(cvar_t **)0xB25CB8;


char    *GL_FileExtension(const char *fni) {
	char           *fn = (char *)fni + strlen(fni) - 1;
	char           *eptr = NULL;

	while(*fn != '/' && fn != fni) {
		if(*fn == '.') {
			eptr = fn;
			break;
		}
		fn--;
	}

	return eptr;
}

// gl_ogv.c

int             Cin_OGM_Init(int handle, const char *filename, int systemFlags);
int             Cin_OGM_Run(int handle, int time);
unsigned char  *Cin_OGM_GetOutput(int handle, int *outWidth, int *outHeight);
double			Cin_OGM_GetAspect(int handle);
void            Cin_OGM_Shutdown(int handle);
int				Cin_OGM_GetFlags(int handle);

#define CIN_system	1
#define CIN_loop	2
#define	CIN_hold	4
#define CIN_silent	8
#define CIN_shader	16
#define CIN_aspect	32		// Not required if CIN_system is set

#define CA_DISCONNECTED	1
#define CA_CINEMATIC	9

char	*  va( char *format, ... ) ;


typedef enum {
	TC_NONE,
	TC_S3TC,
	TC_S3TC_DXT
} textureCompression_t;

typedef struct {
	const char				*renderer_string;
	const char				*vendor_string;
	const char				*version_string;
	const char				*extensions_string;

	int						maxTextureSize;			// queried from GL
	int						maxActiveTextures;		// multitexture ability
	float					maxTextureFilterAnisotropy;

	int						colorBits, depthBits, stencilBits;

	qboolean				deviceSupportsGamma;
	textureCompression_t	textureCompression;
	qboolean				textureEnvAddAvailable;
	qboolean				clampToEdgeAvailable;

	int						vidWidth, vidHeight;

	int						displayFrequency;

	// synonymous with "does rendering consume the entire screen?", therefore
	// a Voodoo or Voodoo2 will have this set to TRUE, as will a Win32 ICD that
	// used CDS.
	qboolean				isFullscreen;
	qboolean				stereoEnabled;
} glconfig_t;

// Hooked into the engine's version of this, return -2 to resume with the RoQ loader
// Anything else will be passed as the return value of the engine-function


void (*Con_Close)() = (void (*)())0x4183D0;

int __cdecl CIN_PlayCinematic(const char *filename, int x, int y, int w, int h, int systemBits) {
	const char *ext;
	cin_cache *cin;
	double aspect, targetaspect;
	glconfig_t *glconfig = (glconfig_t *)0x914070;

	// Set to false by default, and change to true if we successfully load it
	IsOGMHandle[*currentHandle] = qfalse; 

	ext = GL_FileExtension(filename);
	if (!ext) {
		return -2;
	}
	if (!stricmp(ext, ".ogm") || !stricmp(ext, ".ogv")) {
		// Alright, we got an ogm/ogv file, load it up :)

		cin = CIN_GetCinByHandle(*currentHandle);

		if(Cin_OGM_Init(*currentHandle, filename, systemBits)) {
			Com_DPrintf("Starting ogm playback failed (%s)\n", filename);
			cin->fileName[0] = 0;
			Cin_OGM_Shutdown(*currentHandle);
			return -1;
		}

		IsOGMHandle[*currentHandle] = qtrue;

		if ((systemBits & CIN_system) || (systemBits & CIN_aspect)) {

			// Handle aspect ratios here
			aspect = (double)glconfig->vidWidth / (double)glconfig->vidHeight;
			targetaspect = Cin_OGM_GetAspect(*currentHandle);
		
			if (targetaspect == 0 || aspect == targetaspect) {
				cin->xpos = x;
				cin->ypos = y;
				cin->width = w;
				cin->height = h;
			} else if (aspect < targetaspect) {
				cin->xpos = x;
				cin->width = w;
				cin->height = (int)((float)h / targetaspect * aspect);
				cin->ypos = y + ((h - cin->height) / 2);
			} else {
				cin->ypos = y;
				cin->height = h;
				cin->width = (int)((float)w / aspect * targetaspect);
				cin->xpos = x + ((w - cin->width) / 2);
			}
		} else {
			cin->xpos = x;
			cin->ypos = y;
			cin->width = w;
			cin->height = h;
		}
		cin->dirty = qtrue;
		
		cin->startTime = 0;

		cin->looping = (systemBits & CIN_loop);

		cin->holdAtEnd = (systemBits & CIN_hold) != 0;
		cin->alterGameState = (systemBits & CIN_system) != 0;
		cin->playonwalls = 1;
		cin->silent = (systemBits & CIN_silent) != 0;
		cin->shader = (systemBits & CIN_shader) != 0;

		if(cin->alterGameState) {
			// close the menu
			//if(uivm)
			//{
			//	VM_Call(uivm, UI_SET_ACTIVE_MENU, UIMENU_NONE);
			//}

			// VM_CallUI already checks if uivm is set, so dont bother checking twice
			VM_CallUI(7 /*UI_SET_ACTIVE_MENU*/ ,0 /*UIMENU_NONE*/);
		}
		else
		{
			cl_inGameVideo = *(cvar_t **)0xB25CB8;
			cin->playonwalls = cl_inGameVideo->integer;
		}

		if(cin->alterGameState)
		{
			*cls_state = CA_CINEMATIC;
		}

		cin->status = FMV_PLAY;

		Con_Close();

		return *currentHandle;
	} else {
		return -2; // Resume with the RoQ loader
	}
}

// RoQShutdown gets redirected here
// Return 1 to stop RoQShutdown (if the file is OGM)
// Return 0 to resume with RoQShutdown
int __cdecl CIN_OGMShutdown( void ) {
	const char *s;
	cvar_t *cvar;
	cin_cache *cin;

	
	if (!IsOGMHandle[*currentHandle]) {
		return 0;	// Run the normal RoQShutdown
	}

	cin = CIN_GetCinByHandle(*currentHandle);
	Cin_OGM_Shutdown(*currentHandle);
	cin->buf = 0;

	if ( cin->status == FMV_IDLE ) {
		return 1;
	}
	Com_DPrintf("finished cinematic\n");
	cin->status = FMV_IDLE;

	if (cin->iFile) {
		FS_FCloseFile( cin->iFile );
		cin->iFile = 0;
	}

	if (cin->alterGameState) {
		*cls_state = CA_DISCONNECTED;
		// we can't just do a vstr nextmap, because
		// if we are aborting the intro cinematic with
		// a devmap command, nextmap would be valid by
		// the time it was referenced
		cvar = Cvar_FindVar("nextmap");
		s = cvar->string;
		if ( s[0] ) {
			CBuf_AddText( va("%s\n", s) );
			Cvar_Set2( "nextmap", "", 1);
		}
		*CL_handle = -1;
	}
	cin->fileName[0] = 0;
	*currentHandle = -1;
	return 1;
}

// Run a frame of the cinematic (Hooked into CIN_RunCinematic)
// Return -1 to resume with CIN_RunCinematic (in case of a RoQ)
// Anything else will be passed as return value of CIN_RunCinematic
int __cdecl CIN_OGMRunCinematic() {
	cin_cache *cin;

	
	if (!IsOGMHandle[*currentHandle]) {
		return -1;	// Run the normal CIN_RunCinematic
	}

	cin = CIN_GetCinByHandle(*currentHandle);


	if(Cin_OGM_Run(*currentHandle, cin->startTime == 0 ? 0 : CL_ScaledMilliseconds() - cin->startTime)) {
		cin->status = FMV_EOF;
	} else {
		int             newW, newH;
		qboolean        resolutionChange = qfalse;

		cin->buf = Cin_OGM_GetOutput(*currentHandle, &newW, &newH);

		if(newW != cin->CIN_WIDTH)
		{
			cin->CIN_WIDTH = newW;
			resolutionChange = qtrue;
		}
		if(newH != cin->CIN_HEIGHT)
		{
			cin->CIN_HEIGHT = newH;
			resolutionChange = qtrue;
		}

		if(resolutionChange)
		{
			cin->drawX = cin->CIN_WIDTH;
			cin->drawY = cin->CIN_HEIGHT;
		}
		
		cin->status = FMV_PLAY;
		cin->dirty = qtrue;
	}

	if(!cin->startTime)
		cin->startTime = CL_ScaledMilliseconds();

	if(cin->status == FMV_EOF)
	{
		if(cin->holdAtEnd)
		{
			cin->status = FMV_IDLE;
		}
		else if(cin->looping)
		{
			int flags = Cin_OGM_GetFlags(*currentHandle);
			Cin_OGM_Shutdown(*currentHandle);
			Cin_OGM_Init(*currentHandle, cin->fileName, flags);
			cin->buf = NULL;
			cin->startTime = 0;
			cin->status = FMV_PLAY;
		}
		else
		{
			RoQShutdown();
		}
	}
	return cin->status;
}