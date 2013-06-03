// Jedi Knight Galaxies UI Scripts

#include "ui_shared.h"
#include "ui_local.h"

#define KEYWORDHASH_SIZE	512

typedef struct JKGkeywordHashUI_s
{
	char *keyword;
	void (*func)( char **args );
	struct JKGkeywordHashUI_s *next;
} JKGkeywordHashUI_t;

typedef struct JKGkeywordHashSv_s
{
	char *keyword;
	void (*func)();
	struct JKGkeywordHashSv_s *next;
} JKGkeywordHashSv_t;

int JKGKeywordHash_Key(char *keyword) {
	int register hash, i;

	hash = 0;
	for (i = 0; keyword[i] != '\0'; i++) {
		if (keyword[i] >= 'A' && keyword[i] <= 'Z')
			hash += (keyword[i] + ('a' - 'A')) * (119 + i);
		else
			hash += keyword[i] * (119 + i);
	}
	hash = (hash ^ (hash >> 10) ^ (hash >> 20)) & (KEYWORDHASH_SIZE-1);
	return hash;
}

void JKGKeywordHashUI_Add(JKGkeywordHashUI_t *table[], JKGkeywordHashUI_t *key) {
	int hash;

	hash = JKGKeywordHash_Key(key->keyword);
/*
	if (table[hash]) {
		int collision = qtrue;
	}
*/
	key->next = table[hash];
	table[hash] = key;
}

JKGkeywordHashUI_t *JKGKeywordHashUI_Find(JKGkeywordHashUI_t *table[], char *keyword)
{
	JKGkeywordHashUI_t *key;
	int hash;

	hash = JKGKeywordHash_Key(keyword);
	for (key = table[hash]; key; key = key->next) {
		if (!Q_stricmp(key->keyword, keyword))
			return key;
	}
	return 0;
}

void JKGKeywordHashSv_Add(JKGkeywordHashSv_t *table[], JKGkeywordHashSv_t *key) {
	int hash;

	hash = JKGKeywordHash_Key(key->keyword);
/*
	if (table[hash]) {
		int collision = qtrue;
	}
*/
	key->next = table[hash];
	table[hash] = key;
}

JKGkeywordHashSv_t *JKGKeywordHashSv_Find(JKGkeywordHashSv_t *table[], char *keyword)
{
	JKGkeywordHashSv_t *key;
	int hash;

	hash = JKGKeywordHash_Key(keyword);
	for (key = table[hash]; key; key = key->next) {
		if (!Q_stricmp(key->keyword, keyword))
			return key;
	}
	return 0;
}

void Menu_ShowItemByName(menuDef_t *menu, const char *p, qboolean bShow);
/*
void JKAG_LoginAcc(void) {
	menuDef_t *menu;
	itemDef_t *tmp;
	const char *username;
	const char *password;
	menu = Menus_FindByName("JKAG_login");
	if (!menu) return;
	tmp = Menu_FindItemByName(menu,"jkag_username");
	if (!tmp) return;
	username = ((editFieldDef_t *)tmp->typeData)->buffer;
	tmp = Menu_FindItemByName(menu,"jkag_password");
	if (!tmp) return;
	password = ((editFieldDef_t *)tmp->typeData)->buffer;
	// Alright we got it all, update the UI and send the login message
	Menu_ShowItemByName(menu, "main", qfalse);
	Menu_ShowItemByName(menu, "backdrops", qfalse);
	Menu_ShowItemByName(menu, "login_back", qtrue);
	Menu_ShowItemByName(menu, "login_msg", qtrue);
	cgImports->SendClientCommand(va("~clLogin \"%s\" \"%s\"", username, password));
	//trap_Cmd_ExecuteText(EXEC_NOW, va("~clLogin \"%s\" \"%s\"", username, password));
}

void JKAG_RegisterAcc() {
	menuDef_t *menu;
	itemDef_t *tmp;
	const char *username;
	const char *password;
	const char *email;
	menu = Menus_FindByName("JKAG_login");
	if (!menu) return;
	tmp = Menu_FindItemByName(menu,"jkag_regusername");
	if (!tmp) return;
	username = ((editFieldDef_t *)tmp->typeData)->buffer;
	tmp = Menu_FindItemByName(menu,"jkag_regpassword");
	if (!tmp) return;
	password = ((editFieldDef_t *)tmp->typeData)->buffer;
	tmp = Menu_FindItemByName(menu,"jkag_regemail");
	if (!tmp) return;
	email = ((editFieldDef_t *)tmp->typeData)->buffer;
	// Alright we got it all, update the UI and send the login message
	Menu_ShowItemByName(menu, "main", qfalse);
	Menu_ShowItemByName(menu, "reg2", qfalse);
	Menu_ShowItemByName(menu, "backdrops", qfalse);
	Menu_ShowItemByName(menu, "reg_msg", qfalse);
	cgImports->SendClientCommand(va("~clRegister \"%s\" \"%s\" \"%s\"", username, password, email));
	//trap_Cmd_ExecuteText(EXEC_NOW, va("~clRegister \"%s\" \"%s\" \"%s\"", username, password, email));
}

void JKAG_Login_Esc() {
	// Escape key handler for the login UI
	// We can be in 5 places, with 5 responses:
	// 1. Main menu -> go to 'leave'
	// 2. Reg/login screen, while processing -> ignore
	// 3. Reg/login screen, after processing -> mimic Go Back
	// 4. Registration screen -> go back to main
	// 5. Leave server screen -> go back to main
	menuDef_t *menu;
	itemDef_t *tmp;
	menu = Menus_FindByName("JKAG_login");
	if (!menu) return;
	// Check situation 1: if login_background is visible, main is shown
	tmp = Menu_FindItemByName(menu,"login_background");
	if (tmp && tmp->window.flags & WINDOW_VISIBLE) {
		// We got situation 1, switch to leave window
		Menu_ShowItemByName(menu, "main", qfalse);
		Menu_ShowItemByName(menu, "backdrops", qfalse);
		Menu_ShowItemByName(menu, "quit", qtrue);
		return;
	}
	// Check situation 2, if either reg_msg or login_msg is visible, we're busy logging in/registering
	tmp = Menu_FindItemByName(menu,"reg_msg");
	if (tmp && tmp->window.flags & WINDOW_VISIBLE) return;
	tmp = Menu_FindItemByName(menu,"login_msg");
	if (tmp && tmp->window.flags & WINDOW_VISIBLE) return;
	
	// Check situation 3, if login_fail or reg_success is visible, go to main menu, if reg_fail is visible, go to reg2
	tmp = Menu_FindItemByName(menu,"login_fail");
	if (tmp && tmp->window.flags & WINDOW_VISIBLE) {
		Menu_ShowItemByName(menu, "login_back", qfalse);
		Menu_ShowItemByName(menu, "login_fail", qfalse);
		Menu_ShowItemByName(menu, "backdrops", qfalse);
		Menu_ShowItemByName(menu, "main", qtrue);

		tmp = Menu_FindItemByName(menu,"jkag_username");
		((editFieldDef_t *)tmp->typeData)->buffer[0] = 0;
		tmp->cursorPos = 0;

		tmp = Menu_FindItemByName(menu,"jkag_password");
		((editFieldDef_t *)tmp->typeData)->buffer[0] = 0;
		tmp->cursorPos = 0;
		return;
	}
	tmp = Menu_FindItemByName(menu,"reg_success");
	if (tmp && tmp->window.flags & WINDOW_VISIBLE) {
		Menu_ShowItemByName(menu, "reg_success", qfalse);
		Menu_ShowItemByName(menu, "reg_general", qfalse);
		Menu_ShowItemByName(menu, "backdrops", qfalse);
		Menu_ShowItemByName(menu, "main", qtrue);

		tmp = Menu_FindItemByName(menu,"jkag_username");
		((editFieldDef_t *)tmp->typeData)->buffer[0] = 0;
		tmp->cursorPos = 0;

		tmp = Menu_FindItemByName(menu,"jkag_password");
		((editFieldDef_t *)tmp->typeData)->buffer[0] = 0;
		tmp->cursorPos = 0;
		return;
	}
	tmp = Menu_FindItemByName(menu,"reg_fail");
	if (tmp && tmp->window.flags & WINDOW_VISIBLE) {
		Menu_ShowItemByName(menu, "reg_fail", qfalse);
		Menu_ShowItemByName(menu, "backdrops", qfalse);
		Menu_ShowItemByName(menu, "reg2", qtrue);
		return;
	}
	// Check situation 4
	// if jkag_register_disagree or jkag_register_back is visible, we're in the registration screen
	tmp = Menu_FindItemByName(menu,"jkag_register_disagree");
	if (tmp && tmp->window.flags & WINDOW_VISIBLE) {
		Menu_ShowItemByName(menu, "reg_general", qfalse);
		Menu_ShowItemByName(menu, "reg", qfalse);
		Menu_ShowItemByName(menu, "backdrops", qfalse);
		Menu_ShowItemByName(menu, "main", qtrue);
		
		tmp = Menu_FindItemByName(menu,"jkag_username");
		((editFieldDef_t *)tmp->typeData)->buffer[0] = 0;
		tmp->cursorPos = 0;

		tmp = Menu_FindItemByName(menu,"jkag_password");
		((editFieldDef_t *)tmp->typeData)->buffer[0] = 0;
		tmp->cursorPos = 0;
		return;
	}
	
	tmp = Menu_FindItemByName(menu,"jkag_register_back");
	if (tmp && tmp->window.flags & WINDOW_VISIBLE) {
		Menu_ShowItemByName(menu, "reg_general", qfalse);
		Menu_ShowItemByName(menu, "reg2", qfalse);
		Menu_ShowItemByName(menu, "backdrops", qfalse);
		Menu_ShowItemByName(menu, "main", qtrue);
		
		tmp = Menu_FindItemByName(menu,"jkag_username");
		((editFieldDef_t *)tmp->typeData)->buffer[0] = 0;
		tmp->cursorPos = 0;

		tmp = Menu_FindItemByName(menu,"jkag_password");
		((editFieldDef_t *)tmp->typeData)->buffer[0] = 0;
		tmp->cursorPos = 0;
		return;
	}
	// Check situation 5
	// If jkag_quit_no is visible, we're in the 'leave the server' screen
	tmp = Menu_FindItemByName(menu,"jkag_quit_no");
	if (tmp && tmp->window.flags & WINDOW_VISIBLE) {
		Menu_ShowItemByName(menu, "quit", qfalse);
		Menu_ShowItemByName(menu, "backdrops", qfalse);
		Menu_ShowItemByName(menu, "main", qtrue);
		return;
	}
}


void JKAG_Cmd_clLoginResp() {
	char buff[512];
	menuDef_t *menu;
	itemDef_t *tmp;
	menu = Menus_FindByName("JKAG_login");
	if (!menu) return;

	trap_Argv(1,buff,sizeof(buff));
	if (buff[0] == 's') {
		// Success
		Menus_CloseByName("JKAG_login");
		trap_Syscall_UI();
		trap_Print("Login successful!\n");
		trap_Syscall_CG();
	} else {
		// Failed
		Menu_ShowItemByName(menu, "login_msg", qfalse);
		Menu_ShowItemByName(menu, "login_fail", qtrue);
		trap_Argv(2,buff,sizeof(buff));
		tmp = Menu_FindItemByName(menu, "login_fail_desc");
		if (tmp && tmp->typeData) {
			Q_strncpyz(((textDef_t *)tmp->typeData)->text,buff,sizeof(((textDef_t *)tmp->typeData)->text));
			((textDef_t *)tmp->typeData)->customText = 1;
		}
	}
}

void JKAG_Cmd_clRegisterResp() {
	char buff[512];
	menuDef_t *menu;
	itemDef_t *tmp;
	menu = Menus_FindByName("JKAG_login");
	if (!menu) return;

	trap_Argv(1,buff,sizeof(buff));
	if (buff[0] == 's') {
		// Success
		Menu_ShowItemByName(menu, "reg_msg", qfalse);
		Menu_ShowItemByName(menu, "reg_success", qtrue);
	} else {
		// Failed
		Menu_ShowItemByName(menu, "reg_msg", qfalse);
		Menu_ShowItemByName(menu, "reg_fail", qtrue);
		trap_Argv(2,buff,sizeof(buff));
		tmp = Menu_FindItemByName(menu, "reg_fail_desc");
		if (tmp && tmp->typeData) {
			Q_strncpyz(((textDef_t *)tmp->typeData)->text,buff,sizeof(((textDef_t *)tmp->typeData)->text));
			((textDef_t *)tmp->typeData)->customText = 1;
		}
	}

}*/

void JKG_SendEscape(char ** args) {
	cgImports->EscapeTrapped();
}

#include "jkg_conversations.h"
#include "jkg_pazaak.h"
#include "jkg_partymanager.h"
#include "jkg_slice.h"
#include "jkg_lootui.h"
#include "jkg_inventory.h"

extern void UI_BuildQ3Model_List( void );
void JKG_Hack_RefreshModelList(char **args)
{
	char info[MAX_INFO_STRING];
	info[0] = '\0';
	trap_GetConfigString(CS_SERVERINFO, info, sizeof(info));

	trap_Cvar_Set("ui_gameType", Info_ValueForKey(info, "g_gametype"));
	ui_gameType.integer = atoi( Info_ValueForKey(info, "g_gametype") );
	if(ui_gameType.integer >= GT_TEAM)
	{
		// Don't do this in FFA/non gang wars mode. Causes unnecessary hitches.
		UI_BuildQ3Model_List();
	}
}

// JKG scripts used in .menu files
JKGkeywordHashUI_t JKGScripts[] = {
	// Generic
	{"sendescape",			JKG_SendEscape,				    0		},
	{"refreshmodellist",	JKG_Hack_RefreshModelList,		0		},
	// Convo
	{"jkg_convoresponse",	Conv_Script_ProcessChoice,		0		},
	{"jkg_convoslider",		Conv_Script_ConvoSlider,		0		},
	{"jkg_teaction",		Conv_Script_ProcessTextEntry,	0		},
	// Pazaak
	{"pzk_handhover",		Pazaak_Script_HandSlotHover,	0		},
	{"pzk_usecard",			Pazaak_Script_UseCard,			0		},
	{"pzk_btnpress",		Pazaak_Script_ButtonPress,		0		},
	{"pzk_dlgbutton",		Pazaak_Script_DialogButton,		0		},
	{"pzk_flip",			Pazaak_Script_Flip,				0		},
	{"pzk_onesc",			Pazaak_Script_OnEsc,			0		},
	{"pzk_cardhover",		Pazaak_Script_CardHover,		0		},
	{"pzk_selectcard",		Pazaak_Script_SelectCard,		0		},
	{"pzk_sdhover",			Pazaak_Script_SDHover,			0		},
	{"pzk_removesd",		Pazaak_Script_RemoveSD,			0		},
	// Team Management
	{"pmngt_dlgbutton",		PartyMngt_Script_DialogButton,	0		},
	{"pmngt_button",		PartyMngt_Script_Button,		0		},
	{"pmngt_open",			PartyMngt_Script_OpenDlg,		0		},
	{"pmngt_close",			PartyMngt_Script_CloseDlg,		0		},
	// Slicing
	{"slc_runprog",			JKG_Slice_Script_RunProgram,	0		},
	{"slc_dlgbutton",		JKG_Slice_Script_DialogButton,	0		},
	{"slc_stopslice",		JKG_Slice_Script_StopSlicing,	0		},
	{"slc_onesc",			JKG_Slice_Script_OnEsc,			0		},
	// Inventory
	{"inv_open",            JKG_Inventory_OpenDialog,       0       },
	{"inv_close",           JKG_Inventory_CloseDialog,      0       },
	{"inv_acislot",         JKG_Inventory_ACI_Button,       0       },
	{"inv_button",          JKG_Inventory_Script_Button,    0       },
	{"inv_arrow",			JKG_Inventory_Arrow,			0		},
	{"inv_highlight",		JKG_Inventory_CheckTooltip,		0		},
	{"inv_closefromshop",	JKG_Inventory_CloseFromShop,	0		},
	// Looting
	{"loot_button",			JKG_LootScript_Button,			0		},
	{"loot_item",			JKG_LootScript_Item,			0		},
	// Shop
	{"shop_open",			JKG_Shop_OpenDialog,			0		},
	{"shop_close",			JKG_Shop_CloseDialog,			0		},
	{"shop_arrow_next",		JKG_Shop_ArrowNext,				0		},
	{"shop_arrow_prev",		JKG_Shop_ArrowPrev,				0		},
	{"shop_feederSel",		JKG_Shop_ItemSelect,			0		},
	{"shop_update",			JKG_Shop_Update,				0		},
	{"shop_clearfocus",		JKG_Shop_ClearFocus,			0		},
	{"shop_buyconfirm_yes",	JKG_Shop_BuyConfirm_Yes,		0		},
	{"shop_buyconfirm_no",	JKG_Shop_BuyConfirm_No,			0		},
	{"shop_buyconfirm",		JKG_Shop_BuyConfirm_Display,	0		},
	{"shop_openinventory",	JKG_Shop_OpenInventoryMenu,		0		},
	//Login System
	/*{"loginacc",			JKAG_LoginAcc,  				0		},
	{"registeracc",			JKAG_RegisterAcc,				0		},
	{"login_esc",			JKAG_Login_Esc,			    	0		},*/
	{0,						0,					    		0		},
};

// Server commands we should process
JKGkeywordHashSv_t JKGCmds[] = {
	{"conv",			Conv_ProcessCommand_f,		    0		},
	{"pzk",				JKG_ProcessPazaak_f,		    0		},
	{"pmr",				PartyMngt_ShowMessage_f,	    0		},
	{"slc",				JKG_Slice_ProcessCommand_f,		0		},
	{"loot",			JKG_ProcessLoot,				0		},
	/*{"~clLoginResp",	JKAG_Cmd_clLoginResp		    0		},
	{"~clRegisterResp",	JKAG_Cmd_clRegisterResp,	    0		},*/
	{0,					0,							    0		},
};

JKGkeywordHashUI_t *JKGScriptHash[KEYWORDHASH_SIZE];
JKGkeywordHashSv_t *JKGCmdsHash[KEYWORDHASH_SIZE];

/*
===============
Item_SetupKeywordHash
===============
*/
void JKGScript_SetupKeywordHash(void) {
	int i;

	memset(JKGScriptHash, 0, sizeof(JKGScriptHash));
	for (i = 0; JKGScripts[i].keyword; i++) {
		JKGKeywordHashUI_Add(JKGScriptHash, &JKGScripts[i]);
	}
	memset(JKGCmdsHash, 0, sizeof(JKGCmdsHash));
	for (i = 0; JKGCmds[i].keyword; i++) {
		JKGKeywordHashSv_Add(JKGCmdsHash, &JKGCmds[i]);
	}
}

void UI_RunJKGScript(const char *scriptname, char **args) {
	JKGkeywordHashUI_t *script = JKGKeywordHashUI_Find(JKGScriptHash, ( char * ) scriptname);
	if (!script || !script->func) return;
	// HACK-HACK-HACK-HACK-HACK-HACK!! Yes i know this is extremely ugly, but this way we can cast a void * to a function pointer
	// without causing any warnings :D
	// RIP Boba's hack!
	//((void (*)(char **))( *((void (**)())&script->func) ))(args);
	script->func (args);
}

qboolean UI_RunSvCommand(const char *command) {
	// Only called by cgame, dont use trap calls inside this or nested functions!
	JKGkeywordHashSv_t *cmd = JKGKeywordHashSv_Find(JKGCmdsHash, ( char * ) command);
	if (!cmd || !cmd->func) return qfalse;
	trap_Syscall_UI();
	// Inside this function trap calls are safe to be used
	// RIP Boba's hack!
	//((void (*)(void)) ( *((void (**)())&cmd->func) ))();
	cmd->func();
	trap_Syscall_CG();
	return qtrue;
}