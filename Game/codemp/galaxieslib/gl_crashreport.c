////////////////////////////////
//							  
// Jedi Knight Galaxies crash handler 
//
// In the somewhat unlikely event of a crash
// this code will create a thorough crash log so the cause can be determined
//
////////////////////////////////

#include "qcommon/game_version.h"

int	bCrashing = 0;

#include <libudis86/udis86.h>
#include <time.h>
#include "qcommon/disablewarnings.h"

#include <string.h>
#include <stdio.h>

#ifndef _WIN32
#error	Crash reporter is win32-only
#endif

#include <windows.h>



static unsigned long DisasmBacktrace(unsigned char *block, unsigned long base, unsigned long size, unsigned long ip, int n) {
	int i;
	unsigned long abuf[128];
	unsigned long addr;
	unsigned long back;
	unsigned long cmdsize;
	unsigned char *pdata;

	ud_t ud;

	// Check if block is not NULL
	if (block == NULL)	
		return 0;

	// Clamp range to 0-127
	if (n < 0) {		
		n = 0;
	} else if (n > 127) {
		n = 127; 
	}

	// No need to process this one, just return the IP
	if (n == 0)
		return ip;

	// Ensure the IP is within range
	if (ip > base + size)		
		ip = base + size;

	// If the goal instruction is guaranteed going to go under the base address
	// don't bother searching and just return the base address
	if (ip <= base + n)
		return base;
  

	// Calculate how far back we should start scanning
	// assuming an instruction cannot be larger than 16 bytes.
	back = 16 * (n + 3);

	// If this goes too far back, clamp it to block instead
	if (ip < base + back)
		back = ip - base;

	addr = ip - back;
	pdata = block + (addr - base);

	// Prepare udis86
	ud_init(&ud);
	ud_set_mode(&ud, 32);
	ud_set_syntax(&ud, NULL);

	ud_set_input_buffer(&ud, pdata, back + 16);
	ud_set_pc(&ud, addr);

	for (i = 0; addr < ip; i++) {
		abuf[i % 128] = addr;

		cmdsize = ud_disassemble(&ud);

		if (!cmdsize) break;

		addr += cmdsize;
	}

	if (i < n) {
		return abuf[0];
	} else {
		return abuf[(i - n + 128) % 128];
	}
}

static unsigned int GetJumpTarget(ud_t *ud, int operand)
{
	switch (ud->operand[0].size) {
		case 8:
			return ud->pc + ud->operand[operand].lval.sbyte; 
			break;
		case 16:
			return ud->pc + ud->operand[operand].lval.sword; 
			break;
		case 32:
			return ud->pc + ud->operand[operand].lval.sdword; 
			break;
		default:
			return 0;
			break;
	}
}

#include "gl_enginefuncs.h"

static const char *JKG_Crash_GetCrashlogName() {
	static char Buff[1024];
	time_t rawtime;
	struct tm * timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(Buff, 1024,"JKG-CLCrashlog_%d-%m-%Y_%H-%M-%S.log",timeinfo);
	return Buff;
}

static void JKG_FS_WriteString(const char *msg, fileHandle_t f) {
	FS_Write(msg, strlen(msg), f);
}

static cvar_t *fs_basepath;
static cvar_t *fs_game;

static void UpdateCvars() {
	// Since we need these cvars, we'll just cast em right here
	// Saves us some calls ;)
#ifdef _WIN32
	fs_basepath = *(cvar_t **)0xB65EDC;
	fs_game = *(cvar_t **)0xB65ECC;
#endif
}

static const char *JKG_GetModPath()
{
	static char buff[260];
	sprintf_s(buff, 260, "%s\\%s", fs_basepath->string, fs_game->string);
	return buff;
}


#include <dbghelp.h>
#include <tlhelp32.h>
#include <psapi.h>
// Windows version of the crash handler
LPTOP_LEVEL_EXCEPTION_FILTER oldHandler = 0;
// Used in case of a stack overflow
char StackBackup[0x18000];
unsigned int StackBackupStart;


typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
typedef BOOL (WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, PDWORD);

const char *JKG_GetOSDisplayString( )
{
	OSVERSIONINFOEX osvi;
	SYSTEM_INFO si;
	PGNSI pGNSI;
	PGPI pGPI;
	BOOL bOsVersionInfoEx;
	DWORD dwType;
	char buf[80];
	static char name[1024];

	memset(&name, 0, sizeof(name));
	memset(&si, 0, sizeof(SYSTEM_INFO));
	memset(&osvi, 0, sizeof(OSVERSIONINFOEX));

	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	if( !(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)) )
		return name;

   // Call GetNativeSystemInfo if supported or GetSystemInfo otherwise.

	pGNSI = (PGNSI) GetProcAddress(GetModuleHandleA("kernel32.dll"), "GetNativeSystemInfo");
	if( pGNSI != NULL ) {
		pGNSI(&si);
	} else {
		GetSystemInfo(&si);
	}

	strcpy(name, "Microsoft ");
	if ( osvi.dwPlatformId == VER_PLATFORM_WIN32_NT && osvi.dwMajorVersion > 4 )
	{
		// Test for the specific product.

		if ( osvi.dwMajorVersion == 6 )
		{
			if( osvi.dwMinorVersion == 0 )	{
				if( osvi.wProductType == VER_NT_WORKSTATION ) {
					strcat(name, "Windows Vista ");
				} else  {
					strcat(name, "Windows Server 2008 ");
				}
			}

			if ( osvi.dwMinorVersion == 1 ) {
				if( osvi.wProductType == VER_NT_WORKSTATION ) {
					strcat(name, "Windows 7 ");
				} else {
					strcat(name, "Windows Server 2008 R2 ");
				}
			}

			pGPI = (PGPI) GetProcAddress( GetModuleHandleA("kernel32.dll"), "GetProductInfo");

			pGPI( osvi.dwMajorVersion, osvi.dwMinorVersion, 0, 0, &dwType);

			switch( dwType )
			{
			case PRODUCT_ULTIMATE:
				strcat(name, "Ultimate Edition");
				break;
			/*case PRODUCT_PROFESSIONAL:
				strcat(name, "Professional");
				break;
			*/
			case PRODUCT_HOME_PREMIUM:
				strcat(name, "Home Premium Edition");
				break;
			case PRODUCT_HOME_BASIC:
				strcat(name, "Home Basic Edition");
				break;
			case PRODUCT_ENTERPRISE:
				strcat(name, "Enterprise Edition");
				break;
			case PRODUCT_BUSINESS:
				strcat(name, "Business Edition");
				break;
			case PRODUCT_STARTER:
				strcat(name, "Starter Edition");
				break;
			case PRODUCT_CLUSTER_SERVER:
				strcat(name, "Cluster Server Edition");
				break;
			case PRODUCT_DATACENTER_SERVER:
				strcat(name, "Datacenter Edition");
				break;
			case PRODUCT_DATACENTER_SERVER_CORE:
				strcat(name, "Datacenter Edition (core installation)");
				break;
			case PRODUCT_ENTERPRISE_SERVER:
				strcat(name, "Enterprise Edition");
				break;
			case PRODUCT_ENTERPRISE_SERVER_CORE:
				strcat(name, "Enterprise Edition (core installation)");
				break;
			case PRODUCT_ENTERPRISE_SERVER_IA64:
				strcat(name, "Enterprise Edition for Itanium-based Systems");
				break;
			case PRODUCT_SMALLBUSINESS_SERVER:
				strcat(name, "Small Business Server");
				break;
			case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
				strcat(name, "Small Business Server Premium Edition");
				break;
			case PRODUCT_STANDARD_SERVER:
				strcat(name, "Standard Edition");
				break;
			case PRODUCT_STANDARD_SERVER_CORE:
				strcat(name, "Standard Edition (core installation)");
				break;
			case PRODUCT_WEB_SERVER:
				strcat(name, "Web Server Edition");
				break;
			default:
				strcat(name, "Unknown Edition");
				break;
			}
		}

		if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 )
		{
			if( GetSystemMetrics(SM_SERVERR2) ) {
				strcat(name, "Windows Server 2003 R2, ");
			} else if ( osvi.wSuiteMask & VER_SUITE_STORAGE_SERVER ) {
				strcat(name, "Windows Storage Server 2003");
			/*} else if ( osvi.wSuiteMask & VER_SUITE_WH_SERVER ) {
				strcat(name, "Windows Home Server");
			*/
			} else if ( osvi.wProductType == VER_NT_WORKSTATION && si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ) {
				strcat(name, "Windows XP Professional x64 Edition");
			} else {
				strcat(name, "Windows Server 2003, ");
			}

			// Test for the server type.
			if ( osvi.wProductType != VER_NT_WORKSTATION ) {
				if ( si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64 ) {
					if( osvi.wSuiteMask & VER_SUITE_DATACENTER ) {
						strcat(name, "Datacenter Edition for Itanium-based Systems");
					} else if ( osvi.wSuiteMask & VER_SUITE_ENTERPRISE ) {
						strcat(name, "Enterprise Edition for Itanium-based Systems");
					} else {
						strcat(name, "Standard Edition for Itanium-based Systems");
					}
				} else if ( si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ) {
					if( osvi.wSuiteMask & VER_SUITE_DATACENTER ) {
						strcat(name, "Datacenter x64 Edition");
					} else if ( osvi.wSuiteMask & VER_SUITE_ENTERPRISE ) {
						strcat(name, "Enterprise x64 Edition");
					} else {
						strcat(name, "Standard x64 Edition");
					}
				}
			} else {
				if ( osvi.wSuiteMask & VER_SUITE_COMPUTE_SERVER ) {
					strcat(name, "Compute Cluster Edition");
				} else if ( osvi.wSuiteMask & VER_SUITE_DATACENTER ) {
					strcat(name, "Datacenter Edition");
				} else if ( osvi.wSuiteMask & VER_SUITE_ENTERPRISE ) {
					strcat(name, "Enterprise Edition");
				} else if ( osvi.wSuiteMask & VER_SUITE_BLADE ) {
					strcat(name, "Web Edition");
				} else {
					strcat(name, "Standard Edition");
				}
			}
		}

		if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 ) {
			strcat(name, "Windows XP ");
			if( osvi.wSuiteMask & VER_SUITE_PERSONAL ) {
				strcat(name, "Home Edition");
			} else {
				strcat(name, "Professional");
			}
		}

		if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 ) {
			strcat(name, "Windows 2000 ");
			if ( osvi.wProductType == VER_NT_WORKSTATION ) {
				strcat(name, "Professional");
			} else {
				if( osvi.wSuiteMask & VER_SUITE_DATACENTER ) {
					strcat(name, "Datacenter Server");
				} else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE ) {
					strcat(name, "Advanced Server");
				} else {
					strcat(name, "Server");
				}
			}
		}

		// Include service pack (if any) and build number.

		if( strlen(osvi.szCSDVersion) > 0 )
		{
			strcat(name, " " );
			strcat(name, osvi.szCSDVersion);
		}

		sprintf( buf, TEXT(" (build %d)"), osvi.dwBuildNumber);
		strcat(name, buf);

		if ( osvi.dwMajorVersion >= 6 ) {
			if ( si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64 ) {
				strcat(name, ", 64-bit");
			} else if (si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_INTEL ) {
				strcat(name, ", 32-bit");
			}
		}
	}
	else
	{  
		switch(osvi.dwPlatformId)
		{
		case VER_PLATFORM_WIN32s:
			strcat(name, "Windows 32s");	
			break;
		case VER_PLATFORM_WIN32_WINDOWS:
			if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0) {
				strcat(name, "Windows 95");
				if (osvi.szCSDVersion[0] == 'B' || osvi.szCSDVersion[0] == 'C') {
					strcat(name, " OSR2");
				}
			} else if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10) {
				strcat(name, "Windows 98");
				if (osvi.szCSDVersion[0] == 'A') {
					strcat(name, " SE");
				}
			} else if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90) {
				strcat(name, "Windows ME");
			} else {
				strcat(name, "Unknown");
			}
			break;
		case VER_PLATFORM_WIN32_NT:
			if (osvi.dwMajorVersion <=4) {
				strcat(name,  "Windows NT");
			}
		}
	}
	return name;
}



static void JKG_Crash_AddOSData(fileHandle_t f) {
	JKG_FS_WriteString(va("Operating system: %s\n", JKG_GetOSDisplayString()), f);
}

static int GetModuleNamePtr(void* ptr, char *buffFile, char *buffName, void ** ModuleBase, int * ModuleSize) {
	MODULEENTRY32 M = {0};
	int i = 0;
	HANDLE	hSnapshot = NULL;
	
	// BUFFERS MUST BE (AT LEAST) 260 BYTES!!
	if (buffFile) buffFile[0]=0;
	if (buffName) buffName[0]=0;
	if (ModuleBase) *ModuleBase = 0;
	if (ModuleSize) *ModuleSize = 0;

	for (i = 0; i < 10; i++) {
		hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, 0);
		if (hSnapshot != INVALID_HANDLE_VALUE) {
			break;
		}
		if (GetLastError() != ERROR_BAD_LENGTH) {
			break;
		}
	}
	
	if ((hSnapshot != INVALID_HANDLE_VALUE) && Module32First(hSnapshot, &M)) {
		do {
			if ((unsigned int)ptr > (unsigned int)M.modBaseAddr && (unsigned int)ptr <= (unsigned int)((unsigned int)M.modBaseAddr+(unsigned int)M.modBaseSize)) {
				if (buffFile) strncpy(buffFile, M.szExePath, MAX_PATH);
				if (buffName) strncpy(buffName, M.szModule, 256);
				if (ModuleBase) *ModuleBase = M.modBaseAddr;
				if (ModuleSize) *ModuleSize = M.modBaseSize;
				CloseHandle(hSnapshot);
				return 1;
			}
		} while (Module32Next(hSnapshot, &M));
	}

	CloseHandle(hSnapshot);
	// No matches found
	return 0;
}

static const char *GetExceptionCodeDescription(int ExceptionCode) {
	switch (ExceptionCode) {
		case EXCEPTION_ACCESS_VIOLATION:
			return " (Access Violation)";
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
			return " (Array Bounds Exceeded)";
		case EXCEPTION_BREAKPOINT:
			return " (Breakpoint Encountered)";
		case EXCEPTION_DATATYPE_MISALIGNMENT:
			return " (Datatype Misallignment)";
		case EXCEPTION_FLT_DENORMAL_OPERAND:
			return " (Denormal Operand (Floating point operation))";
		case EXCEPTION_FLT_DIVIDE_BY_ZERO:
			return " (Division By Zero (Floating point operation))";
		case EXCEPTION_FLT_INEXACT_RESULT:
			return " (Inexact Result (Floating point operation))";
		case EXCEPTION_FLT_INVALID_OPERATION:
			return " (Invalid Operation (Floating point operation))";
		case EXCEPTION_FLT_OVERFLOW:
			return " (Overflow (Floating point operation))";
		case EXCEPTION_FLT_STACK_CHECK:
			return " (Stack Overflow/Underflow (Floating point operation))";
		case EXCEPTION_FLT_UNDERFLOW:
			return " (Underflow (Floating point operation))";
		case EXCEPTION_ILLEGAL_INSTRUCTION:
			return " (Illegal Instruction)";
		case EXCEPTION_IN_PAGE_ERROR:
			return " (In Page Error)";
		case EXCEPTION_INT_DIVIDE_BY_ZERO:
			return " (Division By Zero (Integer operation))";
		case EXCEPTION_INT_OVERFLOW:
			return " (Overflow (Integer operation))";
		case EXCEPTION_INVALID_DISPOSITION:
			return " (Invalid Disposition)";
		case EXCEPTION_NONCONTINUABLE_EXCEPTION:
			return " (Non-Continuable Exception)";
		case EXCEPTION_PRIV_INSTRUCTION:
			return " (Privileged Instruction)";
		case EXCEPTION_SINGLE_STEP:
			return " (Debugger Single Step)";
		case EXCEPTION_STACK_OVERFLOW:
			return " (Stack Overflow)";
		default:
			return "";
	}
}

static void JKG_Crash_AddCrashInfo(struct _EXCEPTION_POINTERS *EI, fileHandle_t f) {
	static char buffFile[MAX_PATH] = {0};
	static char buffName[MAX_PATH] = {0};
	unsigned int ModuleBase;

	PEXCEPTION_RECORD ER = EI->ExceptionRecord;
	GetModuleFileNameA(NULL, buffFile, MAX_PATH);
	JKG_FS_WriteString(va("Process: %s\n", buffFile),f);
	
	ModuleBase = SymGetModuleBase(GetCurrentProcess(), (DWORD)EI->ExceptionRecord->ExceptionAddress);
	if (ModuleBase) {
		GetModuleBaseName(GetCurrentProcess(), (HMODULE)ModuleBase, buffName, 260);
		JKG_FS_WriteString(va("Exception in module: %s\n", buffName), f);
	} else {
		JKG_FS_WriteString("Exception in module: Unknown\n", f);
	}
	
	/*if (GetModuleNamePtr(ER->ExceptionAddress,buffFile,buffName,(void **)&ModuleBase,NULL)) {
		JKG_FS_WriteString(va("Exception in module: %s\n", buffFile), f);
	} else {
		JKG_FS_WriteString("Exception in module: Unknown\n", f);
	}*/
	JKG_FS_WriteString(va("Exception Address: 0x%08X (%s+0x%X)\n", ER->ExceptionAddress, buffName ? buffName : "?", (unsigned int)ER->ExceptionAddress - ModuleBase), f);
	JKG_FS_WriteString(va("Exception Code: 0x%08X%s\n", ER->ExceptionCode, GetExceptionCodeDescription(ER->ExceptionCode)), f);
	if (ER->ExceptionCode == EXCEPTION_ACCESS_VIOLATION || ER->ExceptionCode == EXCEPTION_IN_PAGE_ERROR) { // Access violation, show read/write address
		switch (ER->ExceptionInformation[0]) {
			case 0:
				JKG_FS_WriteString(va("Attempted to read data at: 0x%08X\n", ER->ExceptionInformation[1]),f);
				break;
			case 1:
				JKG_FS_WriteString(va("Attempted to write data to: 0x%08X\n", ER->ExceptionInformation[1]),f);
				break;
			case 2:
				JKG_FS_WriteString(va("DEP exception caused attempting to execute: 0x%08X\n", ER->ExceptionInformation[1]),f);
				break;
			default:
				break;
		}
	}

}

static void JKG_Crash_AddRegisterDump(struct _EXCEPTION_POINTERS *EI, fileHandle_t f) {
	PCONTEXT CR = EI->ContextRecord;
	//PEXCEPTION_RECORD ER = EI->ExceptionRecord;
	JKG_FS_WriteString("General Purpose & Control Registers:\n", f);
	JKG_FS_WriteString(va("EAX: 0x%08X, EBX: 0x%08X, ECX: 0x%08X, EDX: 0x%08X\n", CR->Eax, CR->Ebx, CR->Ecx, CR->Edx),f);
	JKG_FS_WriteString(va("EDI: 0x%08X, ESI: 0x%08X, ESP: 0x%08X, EBP: 0x%08X\n", CR->Edi, CR->Esi, CR->Esp, CR->Ebp),f);
	JKG_FS_WriteString(va("EIP: 0x%08X\n\n", CR->Eip),f);
	JKG_FS_WriteString("Segment Registers:\n", f);
	JKG_FS_WriteString(va("CS: 0x%08X, DS: 0x%08X, ES: 0x%08X\n", CR->SegCs, CR->SegDs, CR->SegEs), f);
	JKG_FS_WriteString(va("FS: 0x%08X, GS: 0x%08X, SS: 0x%08X\n\n", CR->SegFs, CR->SegGs, CR->SegSs), f);
}

static BOOL CALLBACK JKG_Crash_EnumModules( LPSTR ModuleName, DWORD BaseOfDll, PVOID UserContext ) {
	char Path[MAX_PATH] = {0};
	GetModuleFileName((HMODULE)BaseOfDll, Path, MAX_PATH);
	JKG_FS_WriteString(va("0x%08X - %s - %s\n", BaseOfDll, ModuleName, Path), (fileHandle_t)UserContext);
	return TRUE;
}

static void JKG_Crash_ListModules(fileHandle_t f) {
	SymEnumerateModules(GetCurrentProcess(), (PSYM_ENUMMODULES_CALLBACK)JKG_Crash_EnumModules, (PVOID)f );
}

static void JKG_Crash_DisAsm(struct _EXCEPTION_POINTERS *EI, fileHandle_t f) {
	unsigned long addr;
	int sz;
	int disp;
	ud_t da;
	ud_t dasym;
	int dmod;
	int i;
	int showsource = 0;
	unsigned int lastsourceaddr = 0;
	char modname[260];
	PIMAGEHLP_SYMBOL sym = malloc(1024);
	IMAGEHLP_LINE line;
	MEMORY_BASIC_INFORMATION mem = {0};
	memset(sym, 0, 1024);
	memset(&line, 0, sizeof(IMAGEHLP_LINE));
	sym->MaxNameLength = 800;
	sym->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
	line.SizeOfStruct = sizeof(IMAGEHLP_LINE);

	if (IsBadReadPtr(EI->ExceptionRecord->ExceptionAddress,16)) {
		JKG_FS_WriteString("ERROR: Exception address invalid, cannot create assembly dump\n\n",f);
		return;
	}
	dmod = SymGetModuleBase(GetCurrentProcess(), (DWORD)EI->ExceptionRecord->ExceptionAddress);
	if (dmod) {
		GetModuleBaseName(GetCurrentProcess(), (HMODULE)dmod, modname, 260);
	} else {
		strcpy(modname,"Unknown");
	}

	if (SymGetSymFromAddr(GetCurrentProcess(), (DWORD)EI->ExceptionRecord->ExceptionAddress, &disp, sym)) {
		// We got a symbol, display info
		JKG_FS_WriteString(va("Crash location located at 0x%08X: %s::%s(+0x%X) [Func at 0x%08X]\n",EI->ExceptionRecord->ExceptionAddress, modname,sym->Name, disp, sym->Address), f);
		// Try to find a source file
		if (SymGetLineFromAddr(GetCurrentProcess(), (DWORD)EI->ExceptionRecord->ExceptionAddress, &disp, &line)) {
			if (disp) {
				JKG_FS_WriteString(va("Source code: %s:%i(+0x%X)\n\n", line.FileName, line.LineNumber, disp), f);
			} else {
				JKG_FS_WriteString(va("Source code: %s:%i\n\n", line.FileName, line.LineNumber), f);
			}
			showsource = 1;
		} else {
			JKG_FS_WriteString("No source code information available\n\n", f);
			showsource = 0;
		}
	} else {
		// We don't have a symbol..
		JKG_FS_WriteString(va("Crash location located at 0x%08X: No symbol information available\n\n", EI->ExceptionRecord->ExceptionAddress), f);
	}
	VirtualQuery(EI->ExceptionRecord->ExceptionAddress, &mem, sizeof(MEMORY_BASIC_INFORMATION));
	// Do a 21 instruction disasm, 10 back and 10 forward

	addr = DisasmBacktrace((unsigned char *)mem.BaseAddress, (unsigned long)mem.BaseAddress, mem.RegionSize, (unsigned long)EI->ExceptionRecord->ExceptionAddress, 10);

	// Initialize udis
	ud_init(&da);
	ud_set_input_buffer(&da, (uint8_t *)addr, 21*16);
	ud_set_mode(&da, 32);
	ud_set_pc(&da, addr);
	ud_set_syntax(&da, UD_SYN_INTEL);

	// Initialize disassembler for symbol resolving
	ud_init(&dasym);
	ud_set_mode(&dasym, 32);
	ud_set_syntax(&dasym, NULL);

	JKG_FS_WriteString("^^^^^^^^^^\n", f);
	for(i=0; i<21; i++) {
		sz = ud_disassemble(&da);
		addr = ud_insn_off(&da);
		if (sz < 1) {
			JKG_FS_WriteString(va("ERROR: Could not disassemble code at 0x%08X, aborting...\n", addr), f);
			return;
		}
		if (addr == (int)EI->ExceptionRecord->ExceptionAddress) {
			JKG_FS_WriteString("\n=============================================\n", f);
		}
		// Check if this is a new sourcecode line
		if (showsource) {
			if (SymGetLineFromAddr(GetCurrentProcess(), (DWORD)addr, &disp, &line)) {
				if (line.Address != lastsourceaddr) {
					lastsourceaddr = line.Address;
					if (disp) {
						JKG_FS_WriteString(va("\n--- %s:%i(+0x%X) ---\n\n", line.FileName, line.LineNumber, disp), f);
					} else {
						JKG_FS_WriteString(va("\n--- %s:%i ---\n\n", line.FileName, line.LineNumber), f);
					}
				}
			}
		}
		
		JKG_FS_WriteString(va("0x%08X - %-30s", (unsigned int)ud_insn_off(&da), ud_insn_asm(&da)), f);
		if ( ((da.mnemonic >= UD_Ija && da.mnemonic <= UD_Ijz ) || da.mnemonic == UD_Icall ) && da.operand[0].type == UD_OP_JIMM) {
			// Its a call or jump, see if we got a symbol for it
			// BUT FIRST ;P
			// Since debug compiles employ a call table, we'll disassemble it first
			// if its a jump, we use that address, otherwise, we'll use this one
			unsigned int addr2 = GetJumpTarget(&da, 0);

			if (addr2 != 0) {
				ud_set_input_buffer(&dasym, (uint8_t *)addr2, 21*16);
				ud_set_pc(&dasym, addr2);

				if (ud_disassemble(&dasym)) {
					if (dasym.mnemonic == UD_Ijmp && da.operand[0].type == UD_OP_JIMM) {
						// Its a call table
						if (SymGetSymFromAddr(GetCurrentProcess(), GetJumpTarget(&dasym, 0), &disp, sym)) {
							// We got a symbol for it!
							if (disp) {
								JKG_FS_WriteString(va(" (%s+0x%X)", sym->Name, disp), f);
							} else {
								JKG_FS_WriteString(va(" (%s)", sym->Name), f);
							}
						}
					} else {
						// Its not a call table
						if (SymGetSymFromAddr(GetCurrentProcess(), addr2, &disp, sym)) {
							// We got a symbol for it!
							if (disp) {
								JKG_FS_WriteString(va(" (%s+0x%X)", sym->Name, disp), f);
							} else {
								JKG_FS_WriteString(va(" (%s)", sym->Name), f);
							}
						}
					}
				}
			}
		}
		if (addr == (int)EI->ExceptionRecord->ExceptionAddress) {
			JKG_FS_WriteString(" <-- Exception\n=============================================\n", f);
		}

		JKG_FS_WriteString("\n", f);
	}
	JKG_FS_WriteString("vvvvvvvvvv\n\n", f);
	free(sym);
	
}

void JKG_Crash_HandleStackFrame(STACKFRAME *sf) {
	//char StackBackup[0x18000];
	if (StackBackupStart) {
		if (sf->AddrFrame.Offset >= StackBackupStart && sf->AddrFrame.Offset <= (StackBackupStart+0x18000)) {
			sf->AddrFrame.Offset = (sf->AddrFrame.Offset - StackBackupStart) + (int)&StackBackup[0];
		}
	}
}

static void JKG_Crash_BackTrace(struct _EXCEPTION_POINTERS *EI, fileHandle_t f) {
	HANDLE proc, thread;
	int frameok;
	PIMAGEHLP_SYMBOL sym = malloc(1024);
	IMAGEHLP_LINE line;
	STACKFRAME sf;
	char ModName[260];
	int dmod;
	int disp;
	int gotsource;
	int sourcedisp;
	CONTEXT ctx = *EI->ContextRecord;	// Copy of the context, since it may be changed
	
	memset(&sf, 0, sizeof(STACKFRAME));
	memset(&line, 0, sizeof(IMAGEHLP_LINE));
	line.SizeOfStruct=sizeof(IMAGEHLP_LINE);
	sf.AddrPC.Offset = EI->ContextRecord->Eip;
	sf.AddrPC.Mode = AddrModeFlat;
	sf.AddrStack.Offset = EI->ContextRecord->Esp;
	sf.AddrStack.Mode = AddrModeFlat;
	sf.AddrFrame.Offset = EI->ContextRecord->Ebp;
	sf.AddrFrame.Mode = AddrModeFlat;
	memset(sym, 0, 1024);
	sym->MaxNameLength = 800;
	sym->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
	
	proc = GetCurrentProcess();
	thread = GetCurrentThread();
	if (StackBackupStart) {
		JKG_FS_WriteString("WARNING: Program crashed by a stack overflow, the backtrace will be inconsistent\n", f);
	}
	while(1) {
		frameok = StackWalk(IMAGE_FILE_MACHINE_I386, proc, thread,&sf,&ctx,NULL,SymFunctionTableAccess,SymGetModuleBase,NULL);
		if (!frameok || !sf.AddrFrame.Offset) {
			break;
		}
		dmod = SymGetModuleBase(proc,sf.AddrPC.Offset);
		if (!dmod) {
			strcpy(ModName,"Unknown");
		} else {
			GetModuleBaseName(proc,(HMODULE)dmod, ModName, 260);
		}
		
		if (SymGetLineFromAddr(GetCurrentProcess(), sf.AddrPC.Offset, &sourcedisp, &line)) {
			gotsource = 1;
		} else {
			gotsource = 0;
		}

		if (SymGetSymFromAddr(proc,sf.AddrPC.Offset, &disp, sym)) {
			if (gotsource) {
				JKG_FS_WriteString(va("%s::%s(+0x%X) [0x%08X] - (%s:%i)\n", ModName, sym->Name, disp, sf.AddrPC.Offset, line.FileName, line.LineNumber), f);
			} else {
				JKG_FS_WriteString(va("%s::%s(+0x%X) [0x%08X]\n", ModName, sym->Name, disp, sf.AddrPC.Offset), f);
			}
		} else {
			if (gotsource) {
				// Not likely...
				JKG_FS_WriteString(va("%s [0x%08X] - (%s:%i)\n", ModName, sf.AddrPC.Offset, line.FileName, line.LineNumber), f);
			} else {
				JKG_FS_WriteString(va("%s [0x%08X]\n", ModName, sf.AddrPC.Offset), f);
			}
		}
	}
	free(sym);
	JKG_FS_WriteString("\n",f);
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


static void (__cdecl * iCom_Error)(int level, const char *fmt, ...) = (void (*) (int, const char *, ...))0x437290;

// Execution is moved in here after the exception handler is finished
static void JKG_TerminateGame(void) 
{
	iCom_Error(0, "Jedi Knight Galaxies has encountered a fatal error and was forced to shut down.\nOur apologies for the inconvenience.");
	// This line should never be reached, but just in case.
	exit(1);
}

static void (__cdecl * Sys_Quit)(void) = (void (*) (void))0x450180;

static LONG WINAPI UnhandledExceptionHandler (struct _EXCEPTION_POINTERS *EI /*ExceptionInfo*/) {
	// Alright, we got an exception here, create a crash log and let the program grind to a halt :P
	int bTerminate = 0;
	static char SymPath[4096];
	static char basepath[260];
	static char fspath[260];
	const char *filename = JKG_Crash_GetCrashlogName();
	fileHandle_t f;

	SymPath[0] = 0;
	basepath[0] = 0;
	fspath[0] = 0;

	if (bCrashing) {
		bTerminate = 1;
	}
	bCrashing = 1;
	UpdateCvars();
	InitSymbolPath(SymPath, JKG_GetModPath());
	SymInitialize(GetCurrentProcess(), SymPath, TRUE);
	Com_Printf("------------------------------------------------------------\n\nUnhandled exception encountered!\nExecuting crash handler...\n\nCreating crash log %s...\n", filename);
	FS_FOpenFileByMode(filename, &f, FS_WRITE);
	JKG_FS_WriteString("========================================\n"
		               "     Jedi Knight Galaxies Crash Log\n"
					   "========================================\n", f);
	JKG_FS_WriteString(va("Version: %s (Windows)\n", GAMEVERSION), f);
	JKG_FS_WriteString(va("Side: Client-side\n"), f);
	JKG_FS_WriteString(va("Build Date/Time: %s %s\n", __DATE__, __TIME__), f);
	
	JKG_Crash_AddOSData(f);
	JKG_FS_WriteString("Crash type: Exception\n\n"
					   "----------------------------------------\n"
					   "          Exception Information\n"
					   "----------------------------------------\n", f);
	JKG_Crash_AddCrashInfo(EI, f);
	JKG_FS_WriteString("\n"
					   "----------------------------------------\n"
					   "              Register Dump\n"
					   "----------------------------------------\n", f);
	JKG_Crash_AddRegisterDump(EI, f);
	JKG_FS_WriteString("----------------------------------------\n"
					   "               Module List\n"
					   "----------------------------------------\n", f);
	JKG_Crash_ListModules(f);
	JKG_FS_WriteString("\n----------------------------------------\n"
					   "          Disassembly/Source code\n"
					   "----------------------------------------\n", f);
	JKG_Crash_DisAsm(EI, f);

	JKG_FS_WriteString("----------------------------------------\n"
					   "                Backtrace\n"
					   "----------------------------------------\n", f);
	JKG_Crash_BackTrace(EI, f);

	JKG_FS_WriteString("========================================\n"
					   "             End of crash log\n"
					   "========================================\n", f);
	FS_FCloseFile(f);
	SymCleanup(GetCurrentProcess());
	Com_Printf("Crash report finished\n");
	Com_Printf("Please contact the JKG development team with this crash log and specify the circumstances in which this crash occurred.\n");

	Com_Printf("\nAttempting to shut down gracefully...\n");
	if (bTerminate) {
		Sys_Quit();
		return EXCEPTION_EXECUTE_HANDLER;
	} else {
		EI->ContextRecord->Eip = (int)JKG_TerminateGame;
		return EXCEPTION_CONTINUE_EXECUTION;
	}
	return EXCEPTION_EXECUTE_HANDLER;
}

static LONG WINAPI UnhandledExceptionHandler_Failsafe(struct _EXCEPTION_POINTERS *EI) {
	if (EI->ExceptionRecord->ExceptionCode == EXCEPTION_STACK_OVERFLOW) {
		// Alright, we got a VERY serious issue here..
		// In this state the exception handler itself will run outta stack too
		// So we'll just use a nice hack here to roll up esp by 16k
		__asm
		{
			mov eax, EI
			mov StackBackupStart, esp
			mov esi, esp
			mov edi, offset StackBackup
			mov ecx, 0x6000
			rep stosd
			add esp, 0x18000
			push eax
			call UnhandledExceptionHandler
			jmp skip
		}
	}
	StackBackupStart=0;
	return UnhandledExceptionHandler(EI);
skip:
	;
}


void ActivateCrashHandler() {
	oldHandler = SetUnhandledExceptionFilter(UnhandledExceptionHandler_Failsafe);
}

void DeactivateCrashHandler() {
	if (!oldHandler) return;
	SetUnhandledExceptionFilter(oldHandler);
}