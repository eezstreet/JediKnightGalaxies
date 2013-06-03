///////////////////////////////////
//
// Jedi Knight Galaxies Utility functions
//
//

#include "g_local.h"
#include <assert.h>

typedef struct
{
	char    id[4];
	int     ver;
	char    fileName[68];
	int     numBoneFrames;
	int     numTags;
	int     numMeshes;
	int     numMaxSkins;
	int     headerLength;
	int     tagStart;
	int     tagEnd;
	int     fileSize;
} md3header_t;

typedef struct
{
	vec3_t  mins;
	vec3_t  maxs;
	vec3_t  pos;
	vec_t   scale;
	char    creator[16];
} md3boneFrame_t;

void JKG_RotateBBox(vec3_t mins,vec3_t maxs, vec3_t angles){
	vec3_t sides[6];
	int i,j;
	vec3_t corners[8];
	if (VectorLength(angles) == 0) {
		return;
	}

	AngleVectors(angles, sides[0],sides[1],sides[2]);
	for (i = 0; i < 3;i++) {
		VectorCopy(sides[i],sides[i+3]);
		VectorScale(sides[i],maxs[i],sides[i]);
		VectorScale(sides[i+3],mins[i],sides[i+3]);
	}

	for (i = 0; i < 8;i++) {
		VectorAdd(sides[i % 6], sides[(i+(i>5?2:1)) % 6], corners[i]);
		VectorAdd(corners[i], sides[(i + (i>5?4:2)) % 6], corners[i]);
	}


	VectorCopy(corners[0],mins);
	VectorCopy(corners[0],maxs);
	for (i = 0;i < 8;i++) {
		for (j = 0;j < 3;j++) {
			if (maxs[j] < corners[i][j]) {
				maxs[j] = corners[i][j];
			}
			if (mins[j] > corners[i][j]) {
				mins[j] = corners[i][j];
			}
		}
	}
}

void JKG_GetAutoBoxForModel(const char *model, vec3_t angles, float scale, vec3_t mins, vec3_t maxs) {
	fileHandle_t f;
	md3header_t header;
	md3boneFrame_t boneframe;
	vec3_t imins, imaxs;
	trap_FS_FOpenFile(model, &f, FS_READ);
	trap_FS_Read((void *)&header, sizeof(md3header_t), f);
	trap_FS_Read((void *)&boneframe, sizeof(md3boneFrame_t), f);
	trap_FS_FCloseFile(f);
	imins[0] = boneframe.mins[0] * scale;
	imins[1] = boneframe.mins[1] * scale;
	imins[2] = boneframe.mins[2] * scale;
	imaxs[0] = boneframe.maxs[0] * scale; 
	imaxs[1] = boneframe.maxs[1] * scale;
	imaxs[2] = boneframe.maxs[2] * scale;
	JKG_RotateBBox(imins, imaxs, angles);
	VectorCopy(imins, mins);
	VectorCopy(imaxs, maxs);
}

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dbghelp.h>
#include <psapi.h>

#ifdef _DEBUG
#define MAX_JKG_ASSERT_STACK_CRAWL	64

#define JKG_Assert(_Expression) if(!_Expression){ JKG_AssertFunction(__FILE__, __LINE__, #_Expression); assert(_Expression); }


static const char *JKG_Asset_GetAssertlogName() {
	static char Buff[1024];
	time_t rawtime;
	struct tm * timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(Buff, 1024,"JKG-SVAssertlog_%Y-%m-%d_%H-%M-%S.log",timeinfo);
	return Buff;
}

static void JKG_WriteToAssertLogEasy( char *message, fileHandle_t *f )
{
	trap_FS_Write(message, strlen(message), *f);
}

static void InitSymbolPath( char * SymbolPath, const char* ModPath )
{
	static char Path[1024];

	SymbolPath[0] = 0;	// Clear the buffer
	// Creating the default path
	// ".;%_NT_SYMBOL_PATH%;%_NT_ALTERNATE_SYMBOL_PATH%;%SYSTEMROOT%;%SYSTEMROOT%\System32;"
	strcpy( SymbolPath, "." );

	// environment variable _NT_SYMBOL_PATH
	if ( GetEnvironmentVariableA( "_NT_SYMBOL_PATH", Path, 1024 ) )
	{
		strcat( SymbolPath, ";" );
		strcat( SymbolPath, Path );
	}

	// environment variable _NT_ALTERNATE_SYMBOL_PATH
	if ( GetEnvironmentVariableA( "_NT_ALTERNATE_SYMBOL_PATH", Path, 1024 ) )
	{
		strcat( SymbolPath, ";" );
		strcat( SymbolPath, Path );
	}

	// environment variable SYSTEMROOT
	if ( GetEnvironmentVariableA( "SYSTEMROOT", Path, 1024 ) )
	{
		strcat( SymbolPath, ";" );
		strcat( SymbolPath, Path );
		strcat( SymbolPath, ";" );

		// SYSTEMROOT\System32
		strcat( SymbolPath, Path );
		strcat( SymbolPath, "\\System32" );
	}

   // Add path of gamedata/JKG
	if ( ModPath != NULL )
		if ( ModPath[0] != '\0' )
		{
			strcat( SymbolPath, ";" );
			strcat( SymbolPath, ModPath );
		}
}

static void JKG_WriteStackCrawl( fileHandle_t *f, DWORD *stackCrawl )
{
	int i, dmod, gotsource, sourcedisp;
	HANDLE proc, thread;
	IMAGEHLP_LINE line;
	DWORD disp;
	char ModName[260];
	static char SymPath[4096];
	static char basepath[260];
	static char fspath[260];
	PIMAGEHLP_SYMBOL sym = (PIMAGEHLP_SYMBOL)malloc(1024);

	proc = GetCurrentProcess();
	thread = GetCurrentThread();

	SymPath[0] = 0;
	basepath[0] = 0;
	fspath[0] = 0;

	InitSymbolPath(SymPath, NULL);
	SymInitialize(proc, SymPath, TRUE);

	memset(sym, 0, 1024);
	sym->MaxNameLength = 800;
	sym->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);

	for(i = 0; i < MAX_JKG_ASSERT_STACK_CRAWL; i++)
	{
		if(stackCrawl[i] == 0)
			continue;
		
		// Grab the base address of the module that's the problem atm
		dmod = SymGetModuleBase(proc, stackCrawl[i]);
		if (!dmod) {
			strcpy(ModName,"Unknown");
		} else {
			GetModuleBaseName(proc,(HMODULE)dmod, ModName, 260);
		}
		
		if (SymGetLineFromAddr(proc, stackCrawl[i], (PDWORD)&sourcedisp, &line)) {
			gotsource = 1;
		} else {
			gotsource = 0;
		}

		if (SymGetSymFromAddr(proc, stackCrawl[i], &disp, sym)) {
			if (gotsource) {
				JKG_WriteToAssertLogEasy(va("%s::%s(+0x%X) [0x%08X] - (%s:%i)\r\n", ModName, sym->Name, disp, stackCrawl[i], line.FileName, line.LineNumber), f);
			} else {
				JKG_WriteToAssertLogEasy(va("%s::%s(+0x%X) [0x%08X]\r\n", ModName, sym->Name, disp, stackCrawl[i]), f);
			}
		} else {
			if (gotsource) {
				// Not likely...
				JKG_WriteToAssertLogEasy(va("%s [0x%08X] - (%s:%i)\r\n", ModName, stackCrawl[i], line.FileName, line.LineNumber), f);
			} else {
				JKG_WriteToAssertLogEasy(va("%s [0x%08X]\r\n", ModName, stackCrawl[i]), f);
			}
		}

	}
	free(sym);

	JKG_WriteToAssertLogEasy("\r\n", f);
}

static BOOL CALLBACK JKG_Assert_EnumModules( LPSTR ModuleName, DWORD BaseOfDll, PVOID UserContext ) {
	char Path[MAX_PATH] = {0};
	GetModuleFileName((HMODULE)BaseOfDll, Path, MAX_PATH);
	JKG_WriteToAssertLogEasy(va("0x%08X - %s - %s\r\n", BaseOfDll, ModuleName, Path), (fileHandle_t *)UserContext);
	return TRUE;
}

static void JKG_Assert_ListModules(fileHandle_t *f) {
	SymEnumerateModules(GetCurrentProcess(), (PSYM_ENUMMODULES_CALLBACK)JKG_Assert_EnumModules, (PVOID)f );
}

void JKG_AssertFunction(char *file, int linenum, const char *expression)
{
	// This is different from the Win32 version of assert in that it dumps a stack crawl. Highly useful.
	DWORD stack[MAX_JKG_ASSERT_STACK_CRAWL];
	fileHandle_t f;


	//MessageBeep(MB_ICONERROR);	// BEEP

	memset(stack, 0, sizeof(DWORD)*MAX_JKG_ASSERT_STACK_CRAWL);

	CaptureStackBackTrace(0, MAX_JKG_ASSERT_STACK_CRAWL, (PVOID *)stack, NULL);

	// K, next we create an assert log
	trap_FS_FOpenFile(JKG_Asset_GetAssertlogName(), &f, FS_WRITE);
	JKG_WriteToAssertLogEasy(	"========================================\r\n"
								" Jedi Knight Galaxies Assertion Failure \r\n"
								"========================================\r\n\r\n", &f);
	JKG_WriteToAssertLogEasy(va("Version: %s (Windows)\r\n", GAMEVERSION), &f);
	JKG_WriteToAssertLogEasy(va("Side: Server-side\r\n"), &f);
	JKG_WriteToAssertLogEasy(va("Build Date/Time: %s %s\r\n", __DATE__, __TIME__), &f);

	JKG_WriteToAssertLogEasy("\r\n\r\n", &f);
	JKG_WriteToAssertLogEasy("Assertion Failure!\r\n", &f);
	JKG_WriteToAssertLogEasy(va("File: %s\r\n", file), &f);
	JKG_WriteToAssertLogEasy(va("Line: %i\r\n", linenum), &f);
	JKG_WriteToAssertLogEasy(va("Expression: %s\r\n\r\n", expression), &f);



	JKG_WriteToAssertLogEasy(	"========================================\r\n"
								"              Stack Crawl \r\n"
								"========================================\r\n", &f);

	JKG_WriteStackCrawl(&f, stack);

	JKG_WriteToAssertLogEasy(	"========================================\r\n"
								"           Module Information \r\n"
								"========================================\r\n", &f);

	JKG_Assert_ListModules(&f);

	trap_FS_FCloseFile(f);
	SymCleanup(GetCurrentProcess());
}
#endif
#endif