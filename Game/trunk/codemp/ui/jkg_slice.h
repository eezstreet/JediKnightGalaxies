// Defines for the slicing UI module

void JKG_Slice_DrawGridSlot(int slot, float x, float y, float w, float h);
void JKG_Slice_DrawGridSummary(int slot, float x, float y, float w, float h);
void JKG_Slice_DrawSecurityClearance(int slot, float x, float y, float w, float h);
void JKG_Slice_DrawWarningLevel(float x, float y, float w, float h);
void JKG_Slice_DrawIntrusion(int field, float x, float y, float w, float h);
void JKG_Slice_DrawDialog(int line, float x, float y, float w, float h);

qboolean JKG_Slice_Grid_HandleKey(int slot, int flags, float *special, int key);
qboolean JKG_Slice_Summary_HandleKey(int slot, int flags, float *special, int key);

int JKG_Slice_ProgramCount();
qboolean JKG_Slice_ProgramSelection(int index);
const char *JKG_Slice_ProgramItemText(int index, int column, qhandle_t *handle1, qhandle_t *handle2, qhandle_t *handle3);

void JKG_Slice_ProcessCommand_f(void);

void JKG_Slice_Script_RunProgram(char **args);
void JKG_Slice_Script_DialogButton(char **args);
void JKG_Slice_Script_StopSlicing(char **args);
void JKG_Slice_Script_OnEsc(char **args);