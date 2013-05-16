// Defines for the pazaak UI module

// Owner draw functions
void JKG_Pazaak_DrawCardSlot(int slot, float x, float y, float w, float h);
void JKG_Pazaak_DrawHandSlot(int slot, float x, float y, float w, float h);
void JKG_Pazaak_DrawNames(int player, float x, float y, float w, float h);
void JKG_Pazaak_DrawPoints(int player, float x, float y, float w, float h);
void JKG_Pazaak_DrawDialog(int line, float x, float y, float w, float h);
void JKG_Pazaak_DrawTimeout(float x, float y, float w, float h);
void JKG_Pazaak_DrawSelCardSlot(int slot, float x, float y, float w, float h);
void JKG_Pazaak_DrawSideDeckSlot(int slot, float x, float y, float w, float h);
void JKG_Pazaak_DrawWaiting(float x, float y, float w, float h, qhandle_t shader);

// jkgscript functions
void Pazaak_Script_HandSlotHover(char **args);
void Pazaak_Script_UseCard(char **args);
void Pazaak_Script_ButtonPress(char **args);
void Pazaak_Script_DialogButton(char **args);
void Pazaak_Script_Flip(char **args);
void Pazaak_Script_OnEsc(char **args);
void Pazaak_Script_CardHover(char **args);
void Pazaak_Script_SelectCard(char **args);
void Pazaak_Script_SDHover(char **args);
void Pazaak_Script_RemoveSD(char **args);

// Command processing
void JKG_ProcessPazaak_f();