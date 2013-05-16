#include "cg_local.h"
#include "../ui/ui_shared.h"
#include "q_shared.h"

extern displayContextDef_t cgDC;
extern float *hudTintColor;
extern vec4_t bluehudtint;
extern vec4_t redhudtint;

int MenuFontToHandle(int iMenuFont);
void ChatBox_SetPaletteAlpha(float alpha);
void MiniMap_Render(menuDef_t *menu, float radiusScale);
void CG_DrawJetpackCloak(menuDef_t *menuHUD);

static __inline void AdjustOpacityLevels(void)
{
	//playerState_t *ps = &cg.predictedPlayerState;
	if ( (cg.predictedPlayerState.stats[STAT_HEALTH] < 1 || cg.deathcamTime) ||
		cg.predictedPlayerState.zoomMode ||
		cg.predictedPlayerState.pm_type == PM_INTERMISSION ||
		cg.spectatorTime)
	{
		cg.jkg_HUDOpacity -= ((float)cg.frameDelta/250.0f);
	} else {
		cg.jkg_HUDOpacity += ((float)cg.frameDelta/200.0f);	
	}

	if (cg.jkg_HUDOpacity >= 1.0f)
		cg.jkg_HUDOpacity = 1.0f;
	if (cg.jkg_HUDOpacity < 0.0f)
		cg.jkg_HUDOpacity = 0.0f;

	return;
}

/*
================
CG_DrawSaberStyle

If the weapon is a light saber (which needs no ammo) then draw a graphic showing
the saber style (fast, medium, strong)
================
*/
static void CG_DrawSaberStyle( centity_t *cent, menuDef_t *menuHUD)
{

	const weaponInfo_t *weaponInfo;
	itemDef_t		*focusItem;
	const char		*text;
	float			width;


	if (!cent->currentState.weapon ) // We don't have a weapon right now
	{
		return;
	}

	if ( cent->currentState.weapon != WP_SABER )
	{
		return;
	}

	// Can we find the menu?
	if (!menuHUD)
	{
		return;
	}

    weaponInfo = CG_WeaponInfo (WP_SABER, 0);

	// draw the current saber style in this window
	switch ( cg.predictedPlayerState.fd.saberDrawAnimLevel )
	{
	case 1: //FORCE_LEVEL_1: Fast
		text = "Style: Fast";
		break;
	case 2: //FORCE_LEVEL_2: Medium
		text = "Style: Medium";
		break;
	case 3: //FORCE_LEVEL_3: Strong
		text = "Style: Strong";
		break;
	case 4: //FORCE_LEVEL_4://Desann
		text = "Style: Desann";
		break;
	case 5: //FORCE_LEVEL_5://Tavion
		text = "Style: Tavion";
		break;
	case 6: //SS_DUAL
		text = "Style: Dual";
		break;
	case 7: //SS_STAFF
		text = "Style: Staff";
		break;
	default:	// ??? Should never happen
		text = "Style: Unknown";
		break;
	}

	// Now then, lets render this text ^_^

	width = (float)trap_R_Font_StrLenPixels(text, cgDC.Assets.qhSmallFont, 1) * 0.6f;
				
	focusItem = Menu_FindItemByName(menuHUD, "infobar");
	if (focusItem)
	{
		trap_R_Font_DrawString(focusItem->window.rect.x + ((focusItem->window.rect.w/2) - (width/2)), focusItem->window.rect.y, text, colorWhite, cgDC.Assets.qhSmallFont, -1, 0.6f);
	}

	focusItem = Menu_FindItemByName(menuHUD, "weapicon");
	if (focusItem)
	{
		trap_R_SetColor( NULL );	
		CG_DrawPic( 
			focusItem->window.rect.x, 
			focusItem->window.rect.y, 
			focusItem->window.rect.w, 
			focusItem->window.rect.h, 
			weaponInfo->hudIcon
			);			
	}
}

/*
================
CG_DrawAmmo
================
*/
static void CG_DrawAmmo( centity_t	*cent,menuDef_t *menuHUD)
{
	playerState_t	*ps;
	int				ammo;
	itemDef_t		*focusItem;
	float			width;
	const char		*text;
	vec4_t			opacity;
	const weaponInfo_t *weaponInfo;
	MAKERGBA(opacity, 1,1,1, cg.jkg_HUDOpacity);

	ps = &cg.snap->ps;

	// Can we find the menu?
	if (!menuHUD)
	{
		return;
	}

	if (!cent->currentState.weapon ) // We don't have a weapon right now
	{
		return;
	}
	
	weaponInfo = CG_WeaponInfo (cent->currentState.weapon, cent->currentState.weaponVariation);

	if ( GetWeaponData( cent->currentState.weapon, cent->currentState.weaponVariation )->firemodes[0].cost == 0 && GetWeaponData( cent->currentState.weapon, cent->currentState.weaponVariation )->firemodes[1].cost == 0)
	{ //just draw "infinite"
		text = "Ammo: Infinite";
	}
	else
	{
		if ( GetWeaponAmmoClip( cent->currentState.weapon, cent->currentState.weaponVariation ))
		{
			ammo = ps->stats[STAT_AMMO];
		}
		else
		{
			ammo = ps->ammo[GetWeaponAmmoIndex( cent->currentState.weapon, cent->currentState.weaponVariation )];
		}

		if ( GetWeaponAmmoClip( cent->currentState.weapon, cent->currentState.weaponVariation ))
		{
			// Display the amount of clips too
			float temp;
			temp = ceil(( float ) ps->ammo[GetWeaponAmmoIndex( cent->currentState.weapon, cent->currentState.weaponVariation )] / ( float ) GetWeaponAmmoClip( cent->currentState.weapon, cent->currentState.weaponVariation ));
			text = va( "Ammo: %i (%i)", ammo, ( int ) temp );
		}
		else
		{
			text = va( "Ammo: %i", ammo );
		}	
	}

	// Now then, lets render this text ^_^

	width = (float)trap_R_Font_StrLenPixels(text, cgDC.Assets.qhSmallFont, 1) * 0.6f;
				
	focusItem = Menu_FindItemByName(menuHUD, "infobar");
	if (focusItem)
	{
		trap_R_Font_DrawString(focusItem->window.rect.x + ((focusItem->window.rect.w/2) - (width/2)), focusItem->window.rect.y, text, opacity, cgDC.Assets.qhSmallFont, -1, 0.6f);
	}

	focusItem = Menu_FindItemByName(menuHUD, "weapicon");
	if (focusItem)
	{
		trap_R_SetColor( opacity );	
		CG_DrawPic( 
			focusItem->window.rect.x, 
			focusItem->window.rect.y, 
			focusItem->window.rect.w, 
			focusItem->window.rect.h, 
			weaponInfo->hudIcon
			);			
	}
}

/*
================
CG_DrawForcePower
================
*/
static void CG_DrawForcePower( menuDef_t *menuHUD )
{

	// This goes in the lower bar on the hud	
	const int		maxForcePower = 100;
	vec4_t			calcColor;
	float			percent;
	qboolean	flash=qfalse;
	itemDef_t	*focusItem;
	vec4_t	opacity;
	
	MAKERGBA( opacity, 1, 1, 1, 1*cg.jkg_HUDOpacity );

	if (!menuHUD)
	{
		return;
	}

	// Make the hud flash by setting forceHUDTotalFlashTime above cg.time
	if (cg.forceHUDTotalFlashTime > cg.time )
	{
		flash = qtrue;
		if (cg.forceHUDNextFlashTime < cg.time)	
		{
			cg.forceHUDNextFlashTime = cg.time + 400;
			trap_S_StartSound (NULL, 0, CHAN_LOCAL, cgs.media.noforceSound );

			if (cg.forceHUDActive)
			{
				cg.forceHUDActive = qfalse;
			}
			else
			{
				cg.forceHUDActive = qtrue;
			}

		}
	}
	else	// turn HUD back on if it had just finished flashing time.
	{
		cg.forceHUDNextFlashTime = 0;
		cg.forceHUDActive = qtrue;
	}

//	if (!cg.forceHUDActive)
//	{
//		return;
//	}


	if (flash)
	{
		MAKERGBA(calcColor, 1, 1, 1, 0.6f*cg.jkg_HUDOpacity);
	}
	else 
	{
		MAKERGBA(calcColor, 1, 1, 1, 1*cg.jkg_HUDOpacity);
	}

	focusItem = Menu_FindItemByName(menuHUD, "bar2ico");
	if (focusItem)
	{
		calcColor[3] *= cg.jkg_HUDOpacity;
		trap_R_SetColor( calcColor );	
		CG_DrawPic( 
			focusItem->window.rect.x, 
			focusItem->window.rect.y, 
			focusItem->window.rect.w, 
			focusItem->window.rect.h, 
			trap_R_RegisterShader("gfx/jkghud/ico_stamina.png") // TODO: Precache this
			);			
	}

	// Work out the bar now

	percent = (float)cg.snap->ps.fd.forcePower / (float)maxForcePower;
	percent *= 0.75f; // Range of the bar is 0 to 0.75f

	focusItem = Menu_FindItemByName(menuHUD, "bar2");
	if (focusItem)
	{
		trap_R_SetColor( opacity );
		trap_R_DrawStretchPic(
							focusItem->window.rect.x,
							focusItem->window.rect.y, 
							focusItem->window.rect.w * percent,
							focusItem->window.rect.h,
							0, 0, percent, 1,
							focusItem->window.background
							);		
	}
}

/*
================
CG_DrawHealth
================
*/
static void CG_DrawHealth( menuDef_t *menuHUD )
{
	vec4_t			glowColor;
	playerState_t	*ps;
	int				healthAmt, maxAmt;
	//int				i,currValue,inc;
	double			percentage, factor;
	itemDef_t		*focusItem;

	const char *text;
	int x;

	static double	fcurrent = 0;
	vec4_t	fadecolor = {1, 1, 1, 0.5f};
	static vec4_t	draincolor = {1, 0.4f, 0.4f, 1};
	static vec4_t	fillcolor = {0.4f, 1, 0.4f, 1};
	vec4_t	opacity;

	int state = 0;

	MAKERGBA(opacity, 1, 1, 1, cg.jkg_HUDOpacity);

	// Can we find the menu?
	if (!menuHUD)
	{
		return;
	}

	ps = &cg.snap->ps;

	// What's the health?
	healthAmt = ps->stats[STAT_HEALTH];
	maxAmt = ps->stats[STAT_MAX_HEALTH];
	//if (healthAmt > ps->stats[STAT_MAX_HEALTH])
	//{
	//	healthAmt = ps->stats[STAT_MAX_HEALTH];
	//}

	focusItem = Menu_FindItemByName(menuHUD, "healthbar");

	if (focusItem)
	{
		percentage = (double)healthAmt / (double)maxAmt;
		if (percentage > 1) {
			percentage = 1;
		} else if (percentage < 0) {
			percentage = 0;
		}
		factor = 0.62109375f * percentage + 0.330078125f;

		if (fcurrent < factor) {
			// Raise it
			fcurrent += (cg.frameDelta * .0003); // Go up 30% per second
			if (fcurrent > factor) {
				// We passed it
				fcurrent = factor;
			}
		} else if (fcurrent > factor) {
			// Lower it
			fcurrent -= (cg.frameDelta * .0003); // Go up 30% per second
			if (fcurrent < factor) {
				fcurrent = factor;
			}
		}

		if (fcurrent != 0) {
			if (fcurrent < factor) {
				state = 1;
				// We're filling up, draw up to fcurrent solid, and up to factor translucent
				trap_R_SetColor(opacity);
				trap_R_DrawStretchPic(focusItem->window.rect.x, focusItem->window.rect.y, focusItem->window.rect.w * fcurrent, focusItem->window.rect.h, 0, 0, fcurrent, 1, focusItem->window.background);
				fadecolor[3] *= cg.jkg_HUDOpacity;
				trap_R_SetColor(fadecolor);
				trap_R_DrawStretchPic(focusItem->window.rect.x + focusItem->window.rect.w * fcurrent, focusItem->window.rect.y, focusItem->window.rect.w * (factor - fcurrent), focusItem->window.rect.h, fcurrent, 0, factor, 1, focusItem->window.background);
				trap_R_SetColor(NULL);
			} else if (fcurrent > factor) {
				state = 2;
				// We're draining, draw up to factor solid, and up to fcurrent translucent
				trap_R_SetColor(opacity);
				trap_R_DrawStretchPic(focusItem->window.rect.x, focusItem->window.rect.y, focusItem->window.rect.w * factor, focusItem->window.rect.h, 0, 0, factor, 1, focusItem->window.background);
				fadecolor[3] *= cg.jkg_HUDOpacity;
				trap_R_SetColor(fadecolor);
				trap_R_DrawStretchPic(focusItem->window.rect.x + focusItem->window.rect.w * factor, focusItem->window.rect.y, focusItem->window.rect.w * (fcurrent - factor), focusItem->window.rect.h, factor, 0, fcurrent, 1, focusItem->window.background);
				trap_R_SetColor(NULL);
			} else {
				state = 0;
				// Just solid
				trap_R_SetColor(opacity);
				trap_R_DrawStretchPic(focusItem->window.rect.x, focusItem->window.rect.y, focusItem->window.rect.w * factor, focusItem->window.rect.h, 0, 0, factor, 1, focusItem->window.background);
			}
		}
	}

	focusItem = Menu_FindItemByName(menuHUD, "healthtext");

	if (focusItem)
	{
		// Center and draw the text, apply glow if needed
		if (state == 1) {
			VectorCopy4(fillcolor, glowColor);
		} else if (state == 2) {
			VectorCopy4(draincolor, glowColor);
		} else {
			VectorCopy4(colorWhite, glowColor);
			if (healthAmt < 25) {
				// Glow the text from red to white
				float fade = 0.5f + cos((float)cg.time/250) *.5;
				glowColor[1] = glowColor[2] = fade;
			}
		}
		glowColor[3] *= cg.jkg_HUDOpacity;
		text = va("%i / %i", healthAmt, maxAmt);
		x = ((focusItem->window.rect.w/2) - (trap_R_Font_StrLenPixels(text, cgs.media.hudfont1, 0.6f) / 2)) + focusItem->window.rect.x;
		trap_R_Font_DrawString(
			x,
			focusItem->window.rect.y,
			text,
			glowColor,
			cgs.media.hudfont1,
			-1,
			0.6f);
		trap_R_SetColor(NULL);
	}
}

/*
================
CG_DrawArmor
================
*/
static void CG_DrawArmor( menuDef_t *menuHUD )
{
	//vec4_t			calcColor;
	vec4_t			glowColor;
	playerState_t	*ps;
	int				armor, maxArmor;
	itemDef_t		*focusItem;
	double			percentage, factor ; //quarterArmor;
	//int				i,currValue,inc;

	const char *text;
	int x;

	static double	fcurrent = 0;
	vec4_t	fadecolor = {1, 1, 1, 0.5f};
	static vec4_t	draincolor = {1, 0.4f, 0.4f, 1};
	static vec4_t	fillcolor = {0.4f, 1, 0.4f, 1};
	vec4_t	opacity;

	int state = 0;

	MAKERGBA(opacity, 1, 1, 1, cg.jkg_HUDOpacity);

	//ps = &cg.snap->ps;
	ps = &cg.predictedPlayerState;

	// Can we find the menu?
	if (!menuHUD)
	{
		return;
	}

	armor = ps->stats[STAT_ARMOR];
	maxArmor = ps->stats[STAT_MAX_ARMOR];


	// TEST: just render the whole thing for now, we'll fix it later
	focusItem = Menu_FindItemByName(menuHUD, "shieldbar");

	if (focusItem)
	{
		percentage = (double)armor / (double)maxArmor;
		if (percentage > 1) {
			percentage = 1;
		} else if (percentage < 0) {
			percentage = 0;
		}
		factor = 0.6171875f * percentage + 0.34375f;
		// Fade our fcurrent to this factor
		if (fcurrent < factor) {
			// Raise it
			fcurrent += (cg.frameDelta * .0003); // Go up 30% per second
			if (fcurrent > factor) {
				// We passed it
				fcurrent = factor;
			}
		} else if (fcurrent > factor) {
			// Lower it
			fcurrent -= (cg.frameDelta * .0003); // Go up 30% per second
			if (fcurrent < factor) {
				fcurrent = factor;
			}
		}

		if (fcurrent != 0) {
			if (fcurrent < factor) {
				state = 1;
				// We're filling up, draw up to fcurrent solid, and up to factor translucent
				trap_R_SetColor(opacity);
				trap_R_DrawStretchPic(focusItem->window.rect.x, focusItem->window.rect.y, focusItem->window.rect.w * fcurrent, focusItem->window.rect.h, 0, 0, fcurrent, 1, focusItem->window.background);
				fadecolor[3] *= cg.jkg_HUDOpacity;
				trap_R_SetColor(fadecolor);
				trap_R_DrawStretchPic(focusItem->window.rect.x + focusItem->window.rect.w * fcurrent, focusItem->window.rect.y, focusItem->window.rect.w * (factor - fcurrent), focusItem->window.rect.h, fcurrent, 0, factor, 1, focusItem->window.background);
				trap_R_SetColor(opacity);
			} else if (fcurrent > factor) {
				state = 2;
				// We're draining, draw up to factor solid, and up to fcurrent translucent
				trap_R_DrawStretchPic(focusItem->window.rect.x, focusItem->window.rect.y, focusItem->window.rect.w * factor, focusItem->window.rect.h, 0, 0, factor, 1, focusItem->window.background);
				fadecolor[3] *= cg.jkg_HUDOpacity;
				trap_R_SetColor(fadecolor);
				trap_R_DrawStretchPic(focusItem->window.rect.x + focusItem->window.rect.w * factor, focusItem->window.rect.y, focusItem->window.rect.w * (fcurrent - factor), focusItem->window.rect.h, factor, 0, fcurrent, 1, focusItem->window.background);
				trap_R_SetColor(opacity);
			} else {
				state = 0;
				// Just solid
				trap_R_SetColor(opacity);
				trap_R_DrawStretchPic(focusItem->window.rect.x, focusItem->window.rect.y, focusItem->window.rect.w * factor, focusItem->window.rect.h, 0, 0, factor, 1, focusItem->window.background);
			}
		}
	}


	if (!armor) {
		return;
	}

	focusItem = Menu_FindItemByName(menuHUD, "shieldtext");

	if (focusItem)
	{
		if (state == 1) {
			VectorCopy4(fillcolor, glowColor);
		} else if (state == 2) {
			VectorCopy4(draincolor, glowColor);
		} else {
			VectorCopy4(colorWhite, glowColor);
		}
		glowColor[3] *= cg.jkg_HUDOpacity;
		// Center and draw the text, positioning will be finetuned later on :P
		text = va("%i / %i", armor, maxArmor);
		x = ((focusItem->window.rect.w/2) - (trap_R_Font_StrLenPixels(text, cgs.media.hudfont1, 0.6f) / 2)) + focusItem->window.rect.x;
		trap_R_Font_DrawString(
			x,
			focusItem->window.rect.y,
			text,
			glowColor,
			cgs.media.hudfont1,
			-1,
			0.6f);
		trap_R_SetColor(NULL);
	}
}

static void CG_DrawTopLeftHUD ( menuDef_t *menuHUD, vec4_t opacity )
{
    itemDef_t *focusItem;

    if (!menuHUD)
	{
	    return;
	}

	// Print background of the bars
	focusItem = Menu_FindItemByName(menuHUD, "barsbackground");
	if (focusItem)
	{
		trap_R_SetColor(opacity);	
		CG_DrawPic( 
			focusItem->window.rect.x, 
			focusItem->window.rect.y, 
			focusItem->window.rect.w, 
			focusItem->window.rect.h, 
			focusItem->window.background 
			);			
	}


	if (cg.predictedPlayerState.pm_type != PM_SPECTATOR)
	{
		CG_DrawArmor(menuHUD);
		CG_DrawHealth(menuHUD);
	}

	// Put in the avatar before we put on the frame
	focusItem = Menu_FindItemByName(menuHUD, "avatar");
	if (focusItem)
	{
		trap_R_SetColor(opacity);	
		CG_DrawPic( 
			focusItem->window.rect.x, 
			focusItem->window.rect.y, 
			focusItem->window.rect.w, 
			focusItem->window.rect.h, 
			cgs.media.avatar_placeholder
			);	
	}		
	// Print frame
	focusItem = Menu_FindItemByName(menuHUD, "frame");
	if (focusItem)
	{
		trap_R_SetColor(opacity);	
		CG_DrawPic( 
			focusItem->window.rect.x, 
			focusItem->window.rect.y, 
			focusItem->window.rect.w, 
			focusItem->window.rect.h, 
			focusItem->window.background 
			);			
	}

	// Print level
	focusItem = Menu_FindItemByName(menuHUD, "leveltext");
	if (focusItem)
	{
		const char *temp = va("%i", 0);
		int x = ((focusItem->window.rect.w/2) - (trap_R_Font_StrLenPixels(temp, MenuFontToHandle(1), 0.6f) / 2)) + focusItem->window.rect.x;
		//CG_DrawRect(focusItem->window.rect.x, focusItem->window.rect.y, focusItem->window.rect.w, focusItem->window.rect.h, 1, colorWhite);
		trap_R_Font_DrawString(
			x,
			focusItem->window.rect.y,
			temp,
			opacity,//colorWhite,
			MenuFontToHandle(1),
			-1,
			0.6f);
	}

	focusItem = Menu_FindItemByName(menuHUD, "nametext");
	if (focusItem)
	{
		const char *temp = va("%s", Info_ValueForKey(CG_ConfigString(CS_PLAYERS + cg.snap->ps.clientNum), "n"));
		//CG_DrawRect(focusItem->window.rect.x,focusItem->window.rect.y,focusItem->window.rect.w,focusItem->window.rect.h,1,colorWhite);
		// To avoid rounding errors, we get the length at scale 1, then adjust it
		int x = ((focusItem->window.rect.w/2) - (((float)trap_R_Font_StrLenPixels(temp, MenuFontToHandle(1), 1) * 0.5f) / 2)) + focusItem->window.rect.x;
		//CG_DrawRect(focusItem->window.rect.x, focusItem->window.rect.y, focusItem->window.rect.w, focusItem->window.rect.h, 1, colorWhite);
		ChatBox_SetPaletteAlpha(opacity[3]);
		trap_R_Font_DrawString(
			x,
			focusItem->window.rect.y,
			temp,
			opacity,//colorWhite,
			MenuFontToHandle(1),
			-1,
			0.5f);
		ChatBox_SetPaletteAlpha(1);
	}
}

static void CG_DrawMiniMap ( menuDef_t *menuHUD, vec4_t opacity )
{
    itemDef_t *focusItem;

    if (!menuHUD)
	{
	    return;
	}	

	// Render the minimap
	// Use a default fixed radius of 500 units for now
	MiniMap_Render(menuHUD, 1500.0f);

	focusItem = Menu_FindItemByName(menuHUD, "frame");
	if (focusItem)
	{
		trap_R_SetColor(opacity);
		CG_DrawPic( 
			focusItem->window.rect.x, 
			focusItem->window.rect.y, 
			focusItem->window.rect.w, 
			focusItem->window.rect.h, 
			focusItem->window.background 
			);			
	}
}

static void CG_DrawHotkeyBar ( menuDef_t *menuHUD, vec4_t opacity )
{
    itemDef_t *focusItem;
	int i;

    if (!menuHUD)
	{
	    return;
	}

	// Print background of the bars
	for (i=0; i<11; i++) {
		focusItem = Menu_FindItemByName(menuHUD, va("slot%i", i));
		if (focusItem)
		{
			vec4_t col = {0.11f, 0.11f, 0.11f, 1.0f};
			qhandle_t shader = cgs.media.whiteShader;
			col[3] *= cg.jkg_HUDOpacity;
			if ( i < MAX_ACI_SLOTS && cg.playerACI[i] > 0 )
			{
			    int weapon, variation;
			    
			    if ( BG_GetWeaponByIndex (cg.playerACI[i], &weapon, &variation) )
			    {
			        const weaponInfo_t *weaponInfo = CG_WeaponInfo (weapon, variation);
			        shader = weaponInfo->hudIcon;
			        col[0] = 1.0f;
			        col[1] = 1.0f;
			        col[2] = 1.0f;
			        
			        trap_R_SetColor (colorTable[CT_MDGREY]);
			        trap_R_DrawStretchPic(focusItem->window.rect.x, focusItem->window.rect.y, focusItem->window.rect.w, focusItem->window.rect.h, 0, 0, 1, 1, cgs.media.whiteShader);
			    }
			}
			trap_R_SetColor( col );
			trap_R_DrawStretchPic(focusItem->window.rect.x, focusItem->window.rect.y, focusItem->window.rect.w, focusItem->window.rect.h, 0, 0, 1, 1, shader);
			//CG_DrawRect(focusItem->window.rect.x, focusItem->window.rect.y, focusItem->window.rect.w, focusItem->window.rect.h, 1, colorWhite);
		}

		focusItem = Menu_FindItemByName(menuHUD, va("slotl%i", i));
		if (focusItem)
		{
			trap_R_Font_DrawString(focusItem->window.rect.x, focusItem->window.rect.y, va("%i", i), opacity, cgDC.Assets.qhSmallFont, -1, 0.4f);
		}
	}
	focusItem = Menu_FindItemByName(menuHUD, "frame");
	if (focusItem)
	{
		trap_R_SetColor(opacity);
		CG_DrawPic( 
			focusItem->window.rect.x, 
			focusItem->window.rect.y, 
			focusItem->window.rect.w, 
			focusItem->window.rect.h, 
			focusItem->window.background 
			);			
	}
}

static void CG_DrawBottomRightHUD ( menuDef_t *menuHUD, centity_t *cent, vec4_t opacity )
{
	itemDef_t *focusItem;
	if (!menuHUD)
	{
	    return;
	}

	focusItem = Menu_FindItemByName(menuHUD, "framebg");
	if (focusItem)
	{
		vec4_t tmp;
		MAKERGBA(tmp,0,0,0, cg.jkg_HUDOpacity);
		trap_R_SetColor( tmp );	
		CG_DrawPic( 
			focusItem->window.rect.x, 
			focusItem->window.rect.y, 
			focusItem->window.rect.w, 
			focusItem->window.rect.h, 
			cgs.media.whiteShader 
			);			
	}

	// Draw the bars (temp)
	CG_DrawForcePower(menuHUD);
	
	CG_DrawJetpackCloak(menuHUD);
	
	focusItem = Menu_FindItemByName(menuHUD, "frame");
	if (focusItem)
	{
		trap_R_SetColor(opacity);
		CG_DrawPic( 
			focusItem->window.rect.x, 
			focusItem->window.rect.y, 
			focusItem->window.rect.w, 
			focusItem->window.rect.h, 
			focusItem->window.background 
			);			
	}

	// Draw ammo tics or saber style
	if ( cent->currentState.weapon == WP_SABER )
	{
		CG_DrawSaberStyle(cent,menuHUD);
	}
	else
	{
		CG_DrawAmmo(cent,menuHUD);
	}
}

/*
================
CG_DrawHUD
================
*/
void CG_DrawHUD(centity_t *cent)
{
	menuDef_t	*menuHUD = NULL;
	vec4_t	opacity		=	{1, 1, 1, 1};
	AdjustOpacityLevels();
	MAKERGBA(opacity, 1, 1, 1, cg.jkg_HUDOpacity);

	if (cgs.gametype >= GT_TEAM && cgs.gametype != GT_SIEGE)
	{	// tint the hud items based on team
		if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED )
			hudTintColor = redhudtint;
		else if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE )
			hudTintColor = bluehudtint;
		else // If we're not on a team for whatever reason, leave things as they are.
			hudTintColor = colorTable[CT_WHITE];
	}
	else
	{	// tint the hud items white (dont' tint)
		hudTintColor = colorTable[CT_WHITE];
	}

	// Jedi Knight Galaxies -- Put in our own HUD components here

	// Render the top-left stuff
	menuHUD = Menus_FindByName("hud_topleft");
	CG_DrawTopLeftHUD (menuHUD, opacity);

	// Render the minimap
	menuHUD = Menus_FindByName("hud_minimap");
	CG_DrawMiniMap (menuHUD, opacity);

	// Render the hotkey bar
	menuHUD = Menus_FindByName("hud_hotkey");
	CG_DrawHotkeyBar (menuHUD, opacity);

	menuHUD = Menus_FindByName("hud_bottomright");
	CG_DrawBottomRightHUD (menuHUD, cent, opacity);
}
