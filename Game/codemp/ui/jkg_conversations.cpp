////////////////////////////////////////////////
//
// Conversation controller code for UI
//
////////////////////////////////////////////////

#include "ui_shared.h"
#include "ui_local.h"


// Text entry bits
#define TE_ALLOWCANCEL 1	/* Enable the cancel button */
#define TE_NUMBERONLY 2		/* Allow only entry of numbers */
#define TE_ALLOWNEGATIVE 4	/* Allow the number to be negative, to be used in combination with TE_NUMBERONLY */
#define TE_ALLOWPERIOD 8	/* Allow decimal spaces, to be used in combination with TE_NUMBERONLY */
#define TE_PASSWORD 16		/* Hide the contents (show *** instead) */

typedef struct {
	int active;
	/* Lines data */
	char linesbuff[1024];
	char *lines[4];
	/* Choice data */
	int choicecount;
	char **choices;
	int showingChoices;
	int awaitingData;
	int processedChoices;
	int choiceScroll;
	/* Text Entry data */
	int showingTextEntry;
	char caption[1024];
	char defval[64];
	int textEntryBits;		/* TE bits, see TE_* defines */
} jkg_convdata_t;

static jkg_convdata_t ConvoData;

typedef struct {
	int arg;
	int argc;
	char buff[1024];
} parsebuff_t;

static void Conv_InitParseBuff(parsebuff_t *pb) {
	memset(pb,0,sizeof(parsebuff_t));
	pb->arg = 1;
	pb->argc = trap_Argc();
}

static const char *Conv_NextToken(parsebuff_t *pb) {
	if (pb->arg > pb->argc) return NULL;
	trap_Argv(pb->arg++,pb->buff, sizeof(pb->buff));
	return pb->buff;
}

static qboolean Conv_TokensAvailable(parsebuff_t *pb) {
	if (pb->arg >= pb->argc) return qfalse;
	return qtrue;
}

static int Conv_ParseVector(parsebuff_t *pb, vec3_t *vec) {
	const char *token;
	int i;
	for (i=0; i<3; i++) {
		token = Conv_NextToken(pb);
		if (!token) {
			Com_Printf("WARNING: ^3Error processing conversation info: Could not parse vector\n");
			return 1;
		}
		(*vec)[i] = atof(token);
	}
	return 0;
}

static int Conv_ParseVector2(parsebuff_t *pb, vec2_t *vec) {
	const char *token;
	int i;
	for (i=0; i<2; i++) {
		token = Conv_NextToken(pb);
		if (!token) {
			Com_Printf("WARNING: ^3Error processing conversation info: Could not parse vector2\n");
			return 1;
		}
		(*vec)[i] = atof(token);
	}
	return 0;
}

static int Conv_ParseInt(parsebuff_t *pb, int *num) {
	const char *token;
	token = Conv_NextToken(pb);
	if (!token) {
		Com_Printf("WARNING: ^3Error processing conversation info: Could not parse int\n");
		return 1;
	}
	*num = atoi(token);

	return 0;
}

static int Conv_ParseFloat(parsebuff_t *pb, float *num) {
	const char *token;
	token = Conv_NextToken(pb);
	
	if (!token) {
		Com_Printf("WARNING: ^3Error processing conversation info: Could not parse float\n");
		return 1;
	}
	*num = atof(token);

	return 0;
}


static void Conv_Reset() {
	int i;
	ConvoData.active = 0;
	for (i=0; i<4; i++) {
		ConvoData.lines[i] = 0;
	}
	if (ConvoData.choices) {
		for (i=0; i<ConvoData.choicecount; i++) {
			free(ConvoData.choices[i]);
		}
		free(ConvoData.choices);
		ConvoData.choices = 0;
	}
	ConvoData.choicecount = 0;
	ConvoData.awaitingData = 0;
	ConvoData.showingChoices = 0;
	ConvoData.processedChoices = 0;
	ConvoData.choiceScroll = 0;
	ConvoData.caption[0] = 0;
	ConvoData.defval[0] = 0;
	ConvoData.showingTextEntry = 0;
	ConvoData.textEntryBits = 0;
}

void Menu_ShowItemByName(menuDef_t *menu, const char *p, qboolean bShow);
itemDef_t *Menu_ClearFocus(menuDef_t *menu);

void Conv_ProcessCommand_f() {
	// Big one, this processes ALL conversation commands
	parsebuff_t pb;
	const char *token;
	int i;
	Conv_InitParseBuff(&pb);
	
	while (1) {
		token = Conv_NextToken(&pb);
		if (!token || !token[0]) break;

		if (!Q_stricmp(token,"start")) {
			if (!ConvoData.active) {
				Conv_Reset();
				// Start conversation mode
				ConvoData.active = 1;
			}
			Menus_CloseAll();
			if (Menus_ActivateByName("jkg_conversation"))
			{
				trap_Key_SetCatcher( KEYCATCH_UI );
			}
			Menu_ClearFocus(Menus_FindByName("jkg_conversation"));
			continue;
		}
		if (!Q_stricmp(token,"stop")) {
			// Stop cinematic mode
			Menus_CloseAll();
			trap_Key_SetCatcher(0);
			uiInfo.hideCursor = 0;
			Conv_Reset();
			continue;
		}
		if (!Q_stricmp(token,"halt")) {
			// Temporarily halt cinematic mode
			Menus_CloseByName("jkg_conversation");
			uiInfo.hideCursor = 1;
			continue;
		}
		if (!Q_stricmp(token,"haltm")) {
			// Temporarily halt cinematic mode, but retain the mouse
			Menus_CloseByName("jkg_conversation");
			uiInfo.hideCursor = 0;
			continue;
		}
		if (!Q_stricmp(token,"line")) {
			// We're receiving lines :)
			// We have to process the lines first though :o
			const char *temp;
			for (i=0; i<4; i++) {
				ConvoData.lines[i] = 0;
			}
			temp = Conv_NextToken(&pb);
			if (!temp) {
				Com_Printf("WARNING: ^3Error processing conversation info: Could not parse lines\n");
				return;
			}
			Q_strncpyz(ConvoData.linesbuff, temp, sizeof(ConvoData.linesbuff));
			temp = &ConvoData.linesbuff[0];
			for (i=0; i<4; i++) {
				ConvoData.lines[i] = ( char * ) temp;
				temp = strstr(temp, "\n");
				if (!temp) {
					break;
				}
				*(char *)temp = 0;
				temp++;
			}
			// Alrighty, we got that set up, now handle the menu calls so we disable the choices 'n show the lines
			{
				menuDef_t *menu;
				itemDef_t *item;

				menu = Menus_FindByName("jkg_conversation");
				Menu_ShowItemByName(menu, "choices", qfalse);
				Menu_ShowItemByName(menu, "textentry", qfalse);
				
				
				item = Menu_FindItemByName(menu, "keyhandler");
				if (item) {
					Menu_ClearFocus(menu);
					item->window.flags |= WINDOW_HASFOCUS;
				}
			}
			uiInfo.hideCursor = 1;
			ConvoData.showingChoices = 0;
			ConvoData.showingTextEntry = 0;
		}
		if (!Q_stricmp(token,"te")) {
			// Text entry mode
			const char *temp;
			temp = Conv_NextToken(&pb); // Get caption
			if (!temp) {
				Com_Printf("WARNING: ^3Error processing conversation info: Could not parse text entry caption\n");
				return;
			}
			Q_strncpyz(ConvoData.caption, temp, sizeof(ConvoData.caption));

			temp = Conv_NextToken(&pb); // Get default value
			if (!temp) {
				Com_Printf("WARNING: ^3Error processing conversation info: Could not parse text entry default value\n");
				return;
			}
			Q_strncpyz(ConvoData.defval, temp, sizeof(ConvoData.defval));

			if (Conv_ParseInt(&pb, &ConvoData.textEntryBits)) return;
			
			{
				menuDef_t *menu;
				itemDef_t *item;
				editFieldDef_t *editPtr;

				menu = Menus_FindByName("jkg_conversation");
				Menu_ShowItemByName(menu, "choices", qfalse);
				Menu_ShowItemByName(menu, "textentry", qfalse);

				item = Menu_FindItemByName(menu, "te_text");
				if (!item) {
					Com_Printf("CRITICAL ERROR: UI component te_text in jkg_conversation is missing! Ensure JKG is properly installed!\n");
					return;	// If we get here, we got corrupted assets
				}
				editPtr = (editFieldDef_t *)item->typeData;
				
				if (ConvoData.textEntryBits & TE_NUMBERONLY) {
					item->type = ITEM_TYPE_NUMERICFIELD;
					item->flags = 0;
					if (ConvoData.textEntryBits & TE_ALLOWNEGATIVE) item->flags |= ITF_TEXTFIELD_ALLOWNEGATIVE;
					if (ConvoData.textEntryBits & TE_ALLOWPERIOD) item->flags |= ITF_TEXTFIELD_ALLOWPERIOD;
				} else {
					item->type = ITEM_TYPE_EDITFIELD;
					item->flags = 0;
				}
				if (ConvoData.textEntryBits & TE_PASSWORD) item->flags |= ITF_TEXTFIELD_PASSWORD;
				Q_strncpyz(editPtr->buffer, ConvoData.defval, sizeof(editPtr->buffer));

				Menu_ShowItemByName(menu, "te_text", qtrue);
				Menu_ShowItemByName(menu, "te_message", qtrue);
				Menu_ShowItemByName(menu, "te_ok", qtrue);
				if (ConvoData.textEntryBits & TE_ALLOWCANCEL) {
					Menu_ShowItemByName(menu, "te_cancel", qtrue);
				}

				Menu_ClearFocus(menu);
				item->window.flags |= WINDOW_HASFOCUS;
				item->cursorPos = 0;
				editPtr->paintOffset = 0;
				Enter_EditField(item);
			}
			uiInfo.hideCursor = 0;
			ConvoData.showingChoices = 0;
			ConvoData.showingTextEntry = 1;
			continue;
		}
		if (!Q_stricmp(token,"choices")) {
			// We're receiving choices :)
			const char *temp;
			if (ConvoData.choices) {
				for (i=0; i<ConvoData.choicecount; i++) {
					free(ConvoData.choices[i]);
				}
				free(ConvoData.choices);
				ConvoData.choices = 0;
			}
			if (Conv_ParseInt(&pb, &ConvoData.choicecount)) return;
			ConvoData.choices = (char **)malloc(4*ConvoData.choicecount);
			memset(ConvoData.choices, 0, 4*ConvoData.choicecount);
			for (i=0; i < ConvoData.choicecount; i++) {
				temp = Conv_NextToken(&pb);
				if (!temp) {
					// Ok, we got a truncated packet, mark this one as incomplete and finish it later
					ConvoData.awaitingData = 1;
					ConvoData.processedChoices = i;
					return;
				}
				ConvoData.choices[i] = (char *)malloc(strlen(temp) + 1);
				Q_strncpyz(ConvoData.choices[i], temp, strlen(temp) + 1);
			}
			// Alrighty, we got that set up, now handle the menu calls so we disable the choices 'n show the lines
			{
				menuDef_t *menu;
				itemDef_t *item;

				menu = Menus_FindByName("jkg_conversation");
				Menu_ShowItemByName(menu, "choices", qtrue);
				Menu_ShowItemByName(menu, "textentry", qfalse);
				// Check the amount of items we have, and hide options if needed
				if (ConvoData.choicecount < 5) {
					// No need for the scroll buttons				
					Menu_ShowItemByName(menu, "moveup", qfalse);
					Menu_ShowItemByName(menu, "movedown", qfalse);
				}
				if (ConvoData.choicecount < 4) {
					Menu_ShowItemByName(menu, "choice4", qfalse);
				}
				if (ConvoData.choicecount < 3) {
					Menu_ShowItemByName(menu, "choice3", qfalse);
				}
				if (ConvoData.choicecount < 2) {
					Menu_ShowItemByName(menu, "choice2", qfalse);
				}
				
				item = Menu_FindItemByName(menu, "choice1");
				if (item) {
					Menu_ClearFocus(menu);
					item->window.flags |= WINDOW_HASFOCUS;
				}
				// We should always have at least 1 option, so dont hide that one

			}
			ConvoData.awaitingData = 0;
			ConvoData.choiceScroll = 0;
			uiInfo.hideCursor = 0;
			ConvoData.showingChoices = 1;
			ConvoData.showingTextEntry = 0;
		}
		if (!Q_stricmp(token,"cont")) {
			// We're receiving choices (continuation) :)
			const char *temp;
			if (!ConvoData.awaitingData) {
				Com_Printf("WARNING: ^3Error processing conversation info: Cont packet without incomplete choices packet received\n");
				return;
			}
			for (i=ConvoData.processedChoices; i < ConvoData.choicecount; i++) {
				temp = Conv_NextToken(&pb);
				if (!temp) {
					// Ok, we got a truncated packet, mark this one as incomplete and finish it later
					ConvoData.awaitingData = 1;
					ConvoData.processedChoices = i;
					return;
				}
				ConvoData.choices[i] = (char *)malloc(strlen(temp) + 1);
				Q_strncpyz(ConvoData.choices[i], temp, strlen(temp) + 1);
			}
			// Alrighty, we got that set up, now handle the menu calls so we disable the choices 'n show the lines
			{
				menuDef_t *menu;
				itemDef_t *item;

				menu = Menus_FindByName("jkg_conversation");
				Menu_ShowItemByName(menu, "choices", qtrue);
				// Check the amount of items we have, and hide options if needed
				if (ConvoData.choicecount < 4) {
					Menu_ShowItemByName(menu, "choice4", qfalse);
				}
				if (ConvoData.choicecount < 3) {
					Menu_ShowItemByName(menu, "choice3", qfalse);
				}
				if (ConvoData.choicecount < 2) {
					Menu_ShowItemByName(menu, "choice2", qfalse);
				}

				item = Menu_FindItemByName(menu, "choice1");
				if (item) {
					Menu_ClearFocus((menuDef_t *) item->parent);
					item->window.flags |= WINDOW_HASFOCUS;
				}
				// We should always have at least 1 option, so dont hide that one
			}
			ConvoData.awaitingData = 0;
			ConvoData.choiceScroll = 0;
			uiInfo.hideCursor = 0;
			ConvoData.showingChoices = 1;
			ConvoData.showingTextEntry = 0;
		}
	}
}

int MenuFontToHandle(int iMenuFont);

void Conv_OwnerDraw_Text(rectDef_t *rect, float scale, vec4_t color, int iMenuFont) {
	int x;
	int y;
	int i;
	int iFontIndex = MenuFontToHandle(iMenuFont);
	if (ConvoData.showingChoices || ConvoData.showingTextEntry) {
		return;		// Dont display this when choices are shown
	}
	y = rect->y;
	// Run through each line, center it and draw it
	for (i=0; i<4; i++) {
		if (ConvoData.lines[i]) {
			// Calculate the center position
			x = rect->x + ((rect->w / 2) - (trap_R_Font_StrLenPixels(ConvoData.lines[i], iFontIndex, scale) / 2));
			// And draw it
			trap_R_Font_DrawString(	x, y, ConvoData.lines[i],	color, iFontIndex, -1, scale	);
			// Shift up to the next line
			y += trap_R_Font_HeightPixels(iFontIndex, scale);
		}
	}
}

void Conv_OwnerDraw_LastText(rectDef_t *rect, float scale, vec4_t color, int iMenuFont) {
	// Identical to Conv_OwnerDraw_Text, except that this one draws when choices are shown
	int x;
	int y;
	int i;
	int iFontIndex = MenuFontToHandle(iMenuFont);
	if (!ConvoData.showingChoices) {
		return;		// Dont display this when choices are shown
	}
	y = rect->y;
	// Run through each line, center it and draw it
	// But this time, go in reverse!
	for (i=3; i>=0; i--) {
		if (ConvoData.lines[i]) {
			// Calculate the center position
			x = rect->x + ((rect->w / 2) - (trap_R_Font_StrLenPixels(ConvoData.lines[i], iFontIndex, scale) / 2));
			// And draw it
			trap_R_Font_DrawString(	x, y, ConvoData.lines[i], color, iFontIndex, -1, scale);
			// Shift up to the next line
			y -= trap_R_Font_HeightPixels(iFontIndex, scale);
		}
	}
}

void Conv_OwnerDraw_Choices(int choice, rectDef_t *rect, float scale, vec4_t color, int iMenuFont) {
	int option = choice + ConvoData.choiceScroll;
	int iFontIndex = MenuFontToHandle(iMenuFont);
	if (option < 0 || option >= ConvoData.choicecount) {
		return;
	}
	if (!ConvoData.choices || !ConvoData.choices[option]) {
		return;
	}

	trap_R_Font_DrawString(	rect->x, rect->y, va("%i: %s", option+1, ConvoData.choices[option]), color, iFontIndex, -1, scale);
}

void Conv_OwnerDraw_ScrollButtons(int dir, rectDef_t *rect, vec4_t color) {
	static vec4_t disabled = {0.0f, 0.645f, 0.957f, 0.4f};

	if (dir == 0) { // Up
		// If we're at the top, dont enable this one
		if (ConvoData.choiceScroll == 0) {
			trap_R_SetColor(disabled);
		} else {
			trap_R_SetColor(color);
		}
		trap_R_DrawStretchPic(rect->x, rect->y, rect->w, rect->h, 0, 0, 1, 1, uiInfo.uiDC.Assets.arrow);
	} else {
		if (ConvoData.choiceScroll >= ConvoData.choicecount - 4) {
			trap_R_SetColor(disabled);
		} else {
			trap_R_SetColor(color);
		}
		trap_R_DrawStretchPic(rect->x, rect->y, rect->w, rect->h, 0, 1, 1, 0, uiInfo.uiDC.Assets.arrow);
	}
	trap_R_SetColor(NULL);
}

void Conv_OwnerDraw_TECaption(rectDef_t *rect, float scale, vec4_t color, int iMenuFont) {
	int x;
	int iFontIndex = MenuFontToHandle(iMenuFont);
	if (!ConvoData.showingTextEntry) {
		return;		// Dont display this when choices are shown
	}

	// Calculate the center position
	x = rect->x + ((rect->w / 2) - (trap_R_Font_StrLenPixels(ConvoData.caption, iFontIndex, scale) / 2));
	// And draw it
	trap_R_Font_DrawString(	x, rect->y, ConvoData.caption, color, iFontIndex, -1, scale);
}

qboolean Convo_ChoiceVisible(int choice) {
	int option = choice + ConvoData.choiceScroll;
	if (option < 0 || option >= ConvoData.choicecount)
		return qfalse;
	return qtrue;
}

void Conv_Script_ProcessChoice(char **args) {
	int choice;
	if (Int_Parse(args, &choice)) {
		choice += ConvoData.choiceScroll;
		cgImports->SendClientCommand(va("~convresp %i", choice));
	}
}

void Conv_Script_ProcessTextEntry(char **args) {
	int choice;	// 1 = OK, 2 = Cancel
	menuDef_t *menu;
	itemDef_t *item;
	editFieldDef_t *edit;

	if (Int_Parse(args, &choice)) {
		if (choice == 2 && !(ConvoData.textEntryBits & TE_ALLOWCANCEL)) {
			return;	// Cancel was chosen, but not available (esc pressed in textbox)
		}

		menu = Menus_FindByName("jkg_conversation");
		item = Menu_FindItemByName(menu, "te_text");
		edit = (editFieldDef_t *)item->typeData;

		cgImports->SendClientCommand(va("~convteresp %i \"%s\"", choice, &edit->buffer[0]));
	}
}

void Conv_Script_ConvoSlider(char **args) {
	int direction;
	if (Int_Parse(args, &direction)) {
		ConvoData.choiceScroll += (direction-1);
		if (ConvoData.choiceScroll > ConvoData.choicecount - 4) {
			ConvoData.choiceScroll = ConvoData.choicecount - 4;
		}
		if (ConvoData.choiceScroll < 0) {
			ConvoData.choiceScroll = 0;
		}
	}
}

qboolean Item_SetFocus(itemDef_t *item, float x, float y);
void Menu_NoDefaultKeyBehaviour();
void Item_RunScript(itemDef_t *item, const char *s);

qboolean Conv_HandleKey_KeyHandler(int flags, float *special, int key) {
	if (key == A_MOUSE1 || key == A_MOUSE2 || key == A_ENTER || key == A_KP_ENTER || key == A_SPACE ) {
		cgImports->EscapeTrapped();
		Menu_NoDefaultKeyBehaviour();
	}
	return qfalse;
}

qboolean Conv_HandleKey_Options(int choice, int flags, float *special, int key) {
	menuDef_t *menu;
	itemDef_t *item;

	menu = Menu_GetFocused();	

	if (!menu)
	{
		return (qfalse);
	}

	if (key == A_ENTER || key == A_KP_ENTER || key == A_SPACE ) {
	
		item = Menu_FindItemByName(menu, va("choice%i", choice+1));		
		if (item)
		{
			Item_RunScript(item, item->action);
		}
		Menu_NoDefaultKeyBehaviour();
	} else if (key == A_CURSOR_DOWN || key == A_MWHEELDOWN) {
		if (choice == 3) {
			ConvoData.choiceScroll++;
			if (ConvoData.choiceScroll > ConvoData.choicecount - 4) {
				ConvoData.choiceScroll = ConvoData.choicecount - 4;
			}
			if (ConvoData.choiceScroll < 0) {
				ConvoData.choiceScroll = 0;
			}
		} else {
			int itm = choice+2;
			if (itm > ConvoData.choicecount) {
				itm = ConvoData.choicecount;
			}
			item = Menu_FindItemByName(menu, va("choice%i", itm));
			if (item) {
				Menu_ClearFocus((menuDef_t *) item->parent);
				item->window.flags |= WINDOW_HASFOCUS;
			}
		}
		Menu_NoDefaultKeyBehaviour();
	} else if (key == A_CURSOR_UP || key == A_MWHEELUP) {
		if (choice == 0) {
			ConvoData.choiceScroll--;
			if (ConvoData.choiceScroll > ConvoData.choicecount - 4) {
				ConvoData.choiceScroll = ConvoData.choicecount - 4;
			}
			if (ConvoData.choiceScroll < 0) {
				ConvoData.choiceScroll = 0;
			}
		} else {
			item = Menu_FindItemByName(menu, va("choice%i", choice));
			if (item) {
				Menu_ClearFocus((menuDef_t *) item->parent);
				item->window.flags |= WINDOW_HASFOCUS;
			}
		}
		Menu_NoDefaultKeyBehaviour();
	} else if (key >= A_1 && key <= A_9) {
		// Manually picked an option there, see if the item exists, and if so, select it
		if (ConvoData.choicecount >= (key - A_1 + 1)) {
			cgImports->SendClientCommand(va("~convresp %i", (key - A_1 + 1)));
		}
		Menu_NoDefaultKeyBehaviour();
	}
	return qfalse;
}