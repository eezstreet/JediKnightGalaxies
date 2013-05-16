// Defines for the team management UI module

int JKG_PartyMngt_FeederCount(int feeder);
qboolean JKG_PartyMngt_FeederSelection(int feeder, int index, itemDef_t *item);
const char *JKG_PartyMngt_FeederItemText(int feeder, int index, int column, qhandle_t *handle1, qhandle_t *handle2, qhandle_t *handle3);

void JKG_PartyMngt_DrawDialog(int line, float x, float y, float w, float h);

void PartyMngt_Script_DialogButton(char **args);
void PartyMngt_Script_Button(char **args);
void PartyMngt_Script_OpenDlg(char **args);
void PartyMngt_Script_CloseDlg(char **args);

void PartyMngt_ShowMessage_f();