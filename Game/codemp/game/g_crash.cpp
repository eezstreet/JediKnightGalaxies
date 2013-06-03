////////////////////////////////
//							  
// Jedi Knight Galaxies crash handler 
//
// In the somewhat unlikely event of a crash
// this code will create a thorough crash log so the cause can be determined
//
////////////////////////////////


#include <libudis86/udis86.h>
#include "g_local.h"
#undef INFINITE		// Causes a define clash
#include <time.h>

int StrToDword(const char *str) {
	int len = strlen(str);
	int pos;
	int val;
	char ch;
	int result = 0;
	int hb; // Halfbyte (see below) to start with
	// Layout:
	//      7      6      5      4      3      2      1      0    <-- Halfbytes
	//    <xxxx> <xxxx> <xxxx> <xxxx> <xxxx> <xxxx> <xxxx> <xxxx> <-- Bits (x = bit)
	//
	// Each hex digit represents 4 bits
	// So 8 digits and 32 bits

	if (len < 3 || len > 10) { // Invalid length
		return 0;
	}
	// Determine starting halfbyte:
	hb = (len-3);
	for (pos = 2; pos < len; hb--, pos++) {
		// Alright parse it
		ch = str[pos];
		if (ch >= '0' && ch <= '9')
			val = ch - '0';
		else if (ch >= 'A' && ch <= 'F')
			val = ch - 'A' + 10;
		else if (ch >= 'a' && ch <= 'f')
			val = ch - 'a' + 10;
		else
			return 0; // Bad digit
		// Ok the char is valid, parsy time
		result |= ((val & 15 ) << (hb*4));
	}
	return result;
}

void Cmd_DisAsmDirect_f() {
	char Addrbuf[32];
	ud_t disasm;
	int Addr;
	if (trap_Argc() < 2) {
		G_Printf("Usage: /disasm <address>\n");
		return;
	}
	trap_Argv(1,Addrbuf,32);
	// Look for 0x notation
	if (Addrbuf[0] == '0' && Addrbuf[1] == 'x') {
		Addr = StrToDword(Addrbuf);
	} else {
		Addr = atoi(Addrbuf);
	}
	if (!Addr) {
		G_Printf("Bad pointer provided, aborting\n");
		return;
	}

	ud_init(&disasm);
	ud_set_input_buffer(&disasm, (uint8_t *)Addr, 16);
	ud_set_mode(&disasm, 32);
	ud_set_pc(&disasm, Addr);
	ud_set_syntax(&disasm, UD_SYN_INTEL);

	ud_disassemble(&disasm);

	G_Printf("%08X: %s (%s)\n", Addr, ud_insn_asm(&disasm), ud_insn_hex(&disasm));
}

static void JKG_FS_WriteString(const char *msg, fileHandle_t f) {
	trap_FS_Write(msg, strlen(msg), f);
}

void JKG_ExtCrashInfo(int fileHandle) {
	char cs[1024];
	// In case of a crash, the auxiliary library will write a report
	// If jampgame is still loaded when the crash occours (and this is usually the case)
	// this function will be called to provide additional information
	// Such as the map the server was on, the clients on it,etc

	fileHandle_t f = (fileHandle_t)fileHandle;
	JKG_FS_WriteString("----------------------------------------\n"
					   "          Server info / players\n"
					   "----------------------------------------\n", f);
	trap_GetServerinfo( cs, sizeof( cs ) );
	JKG_FS_WriteString(va("Map: %s\n\n", Info_ValueForKey( cs, "mapname" )), f);
	JKG_FS_WriteString(va("Players: %i/%i:\n\n", level.numConnectedClients, level.maxclients), f);
	if (level.numConnectedClients != 0) {
		int i;
		JKG_FS_WriteString("|ID|Name                                |Ping|IP                    |\n",f);
		JKG_FS_WriteString("+--+------------------------------------+----+----------------------+\n",f);
		for (i=0; i < level.maxclients; i++) {
			if (level.clients[i].pers.connected != CON_DISCONNECTED) {
				JKG_FS_WriteString(va("|%-2i|%-36s|%-4i|%-22s|\n",i,level.clients[i].pers.netname,level.clients[i].ps.ping, level.clients[i].sess.IP), f); 
			}
		}
		JKG_FS_WriteString("+--+------------------------------------+----+----------------------+\n",f);
	}
}