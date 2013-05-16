// Defines for the conversations UI module

void Conv_ProcessCommand_f();

void Conv_OwnerDraw_Text(rectDef_t *rect, float scale, vec4_t color, int iMenuFont);
void Conv_OwnerDraw_LastText(rectDef_t *rect, float scale, vec4_t color, int iMenuFont);
void Conv_OwnerDraw_Choices(int choice, rectDef_t *rect, float scale, vec4_t color, int iMenuFont);
void Conv_OwnerDraw_TECaption(rectDef_t *rect, float scale, vec4_t color, int iMenuFont);
qboolean Convo_ChoiceVisible(int choice);
void Conv_Script_ProcessChoice(char **args);
void Conv_Script_ProcessTextEntry(char **args);
void Conv_Script_ConvoSlider(char **args);
qboolean Conv_HandleKey_Options(int choice, int flags, float *special, int key);
void Conv_OwnerDraw_ScrollButtons(int dir, rectDef_t *rect, vec4_t color);
qboolean Conv_HandleKey_KeyHandler(int flags, float *special, int key);