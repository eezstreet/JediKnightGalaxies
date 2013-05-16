/***********************************************
 *
 *	JKG Administration System Header
 *
 *  This system assumes that the args are
 *  tokenized and available through Cmd_Argc 
 *  and Cmd_Argv
 *
 ***********************************************/

typedef enum {
	ADMRANK_NONE,
	ADMRANK_VIP,
	ADMRANK_DEVELOPER,
	ADMRANK_LITEADMIN,
	ADMRANK_ADMIN,
	ADMRANK_OPERATOR,
} admrank_e;

void JKG_Admin_Init();
qboolean JKG_Admin_Execute(const char *command, gentity_t *ent);
qboolean JKG_Admin_ExecuteRcon(const char *command);
void JKG_AdminNotify( admrank_e rank, const char *fmt, ... );