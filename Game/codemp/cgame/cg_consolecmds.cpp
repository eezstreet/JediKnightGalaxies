// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_consolecmds.c -- text commands typed in at the local console, or
// executed by a key binding

#include "cg_local.h"
#include "../ui/ui_shared.h"
#include "bg_saga.h"

// TEST
#include "jkg_cg_auxlib.h"
#include "json/cJSON.h"

extern menuDef_t *menuScoreboard;

void CG_CameraZoomIn( void )
{
	cg_thirdPersonRange.value -= 5;
	if ( cg_thirdPersonRange.value < 30 ) cg_thirdPersonRange.value = 30;
}

void CG_CameraZoomOut( void )
{
	cg_thirdPersonRange.value += 5;
	if ( cg_thirdPersonRange.value > 100 ) cg_thirdPersonRange.value = 100;
}

void CG_Start360Camera( void )
{
	if ( cgi.Key_GetCatcher() == 0 && cg.i360CameraTime == 0 )
	{
		usercmd_t cmd;
		cgi.GetUserCmd( cgi.GetCurrentCmdNumber(), &cmd );

		cg.i360CameraForce		= -1;
		cg.i360CameraTime		= cg.time + 250;
		cg.i360CameraOffset		= 0;
		cg.i360CameraOriginal	= 0;
		cg.i360CameraUserCmd	= cmd.angles[YAW];
	}
}

void CG_Stop360Camera( void )
{
	if ( cg.i360CameraTime )
	{
		/* This was a short click, so only change the third person camera! */
		if ( cg.i360CameraTime > cg.time )
		{
			cgi.SendConsoleCommand( "cg_thirdPerson !" );
		}
		/* It was a full rotate so reset the view angles to their original position */
		else
		{
			vec3_t angle;
			angle[YAW] = cg.i360CameraOriginal;
			cgi.SetClientForceAngle( cg.time + 10, angle );
		}
	}

	cg.i360CameraTime = 0;
}

void CG_TargetCommand_f( void ) {
	int		targetNum;
	char	test[4];

	targetNum = CG_CrosshairPlayer();
	if (!targetNum ) {
		return;
	}

	cgi.Argv( 1, test, 4 );
	cgi.SendConsoleCommand( va( "gc %i %i", targetNum, atoi( test ) ) );
}

void CG_OpenPartyManagement_f( void ) {
	uiImports->PartyMngtNotify( 10 );
}

/*
=================
CG_SizeUp_f

Keybinding command
=================
*/
static void CG_SizeUp_f (void) {
	cgi.Cvar_Set("cg_viewsize", va("%i",(int)(cg_viewsize.integer+10)));
}


/*
=================
CG_SizeDown_f

Keybinding command
=================
*/
static void CG_SizeDown_f (void) {
	cgi.Cvar_Set("cg_viewsize", va("%i",(int)(cg_viewsize.integer-10)));
}


/*
=============
CG_Viewpos_f

Debugging command to print the current position
=============
*/
static void CG_Viewpos_f (void) {
	CG_Printf ("%s (%i %i %i) : (%i %i %i)\n", cgs.mapname, (int)cg.refdef.vieworg[0],
		(int)cg.refdef.vieworg[1], (int)cg.refdef.vieworg[2], 
		(int)cg.refdef.viewangles[0], (int)cg.refdef.viewangles[1], (int)cg.refdef.viewangles[2]);
}


static void CG_ScoresDown_f( void ) {

	CG_BuildSpectatorString();
	if ( cg.scoresRequestTime + 2000 < cg.time ) {
		// the scores are more than two seconds out of data,
		// so request new ones
		cg.scoresRequestTime = cg.time;
		cgi.SendClientCommand( "score" );

		// leave the current scores up if they were already
		// displayed, but if this is the first hit, clear them out
		if ( !cg.showScores ) {
			cg.showScores = qtrue;
			cg.numScores = 0;
		}
	} else {
		// show the cached contents even if they just pressed if it
		// is within two seconds
		cg.showScores = qtrue;
	}
}

static void CG_ScoresUp_f( void ) {
	if ( cg.showScores ) {
		cg.showScores = qfalse;
		cg.scoreFadeTime = cg.time;
	}
}

extern menuDef_t *menuScoreboard;
void Menu_Reset();			// FIXME: add to right include file

static void CG_scrollScoresDown_f( void) {
	if (menuScoreboard && cg.scoreBoardShowing) {
		Menu_ScrollFeeder(menuScoreboard, FEEDER_SCOREBOARD, qtrue);
		Menu_ScrollFeeder(menuScoreboard, FEEDER_REDTEAM_LIST, qtrue);
		Menu_ScrollFeeder(menuScoreboard, FEEDER_BLUETEAM_LIST, qtrue);
	}
}


static void CG_scrollScoresUp_f( void) {
	if (menuScoreboard && cg.scoreBoardShowing) {
		Menu_ScrollFeeder(menuScoreboard, FEEDER_SCOREBOARD, qfalse);
		Menu_ScrollFeeder(menuScoreboard, FEEDER_REDTEAM_LIST, qfalse);
		Menu_ScrollFeeder(menuScoreboard, FEEDER_BLUETEAM_LIST, qfalse);
	}
}


static void CG_spWin_f( void) {
	cgi.Cvar_Set("cg_cameraOrbit", "2");
	cgi.Cvar_Set("cg_cameraOrbitDelay", "35");
	cgi.Cvar_Set("cg_thirdPerson", "1");
	cgi.Cvar_Set("cg_thirdPersonAngle", "0");
	cgi.Cvar_Set("cg_thirdPersonRange", "100");
	CG_AddBufferedSound(cgs.media.winnerSound);
	//trap_S_StartLocalSound(cgs.media.winnerSound, CHAN_ANNOUNCER);
	CG_CenterPrint(CG_GetStringEdString("MP_INGAME", "YOU_WIN"), SCREEN_HEIGHT * .30, 0);
}

static void CG_spLose_f( void) {
	cgi.Cvar_Set("cg_cameraOrbit", "2");
	cgi.Cvar_Set("cg_cameraOrbitDelay", "35");
	cgi.Cvar_Set("cg_thirdPerson", "1");
	cgi.Cvar_Set("cg_thirdPersonAngle", "0");
	cgi.Cvar_Set("cg_thirdPersonRange", "100");
	CG_AddBufferedSound(cgs.media.loserSound);
	//trap_S_StartLocalSound(cgs.media.loserSound, CHAN_ANNOUNCER);
	CG_CenterPrint(CG_GetStringEdString("MP_INGAME", "YOU_LOSE"), SCREEN_HEIGHT * .30, 0);
}


static void CG_TellTarget_f( void ) {
	int		clientNum;
	char	command[128];
	char	message[128];

	clientNum = CG_CrosshairPlayer();
	if ( clientNum == -1 ) {
		return;
	}

	cgi.Args( message, 128 );
	Com_sprintf( command, 128, "tell %i %s", clientNum, message );
	cgi.SendClientCommand( command );
}

static void CG_TellAttacker_f( void ) {
	int		clientNum;
	char	command[128];
	char	message[128];

	clientNum = CG_LastAttacker();
	if ( clientNum == -1 ) {
		return;
	}

	cgi.Args( message, 128 );
	Com_sprintf( command, 128, "tell %i %s", clientNum, message );
	cgi.SendClientCommand( command );
}


/*
==================
CG_StartOrbit_f
==================
*/

static void CG_StartOrbit_f( void ) {
	char var[MAX_TOKEN_CHARS];

	cgi.Cvar_VariableStringBuffer( "developer", var, sizeof( var ) );
	if ( !atoi(var) ) {
		return;
	}
	if (cg_cameraOrbit.value != 0) {
		cgi.Cvar_Set ("cg_cameraOrbit", "0");
		cgi.Cvar_Set("cg_thirdPerson", "0");
	} else {
		cgi.Cvar_Set("cg_cameraOrbit", "5");
		cgi.Cvar_Set("cg_thirdPerson", "1");
		cgi.Cvar_Set("cg_thirdPersonAngle", "0");
		cgi.Cvar_Set("cg_thirdPersonRange", "100");
	}
}

static void CG_StartCinematic(void) {
	if (cg.cinematicState < 1 || cg.cinematicState > 2) {
		cg.cinematicState = 1;
		cg.cinematicTime = cg.time;
	}
}

static void CG_StopCinematic(void) {
	if (cg.cinematicState > 0 && cg.cinematicState < 3) {
		cg.cinematicState = 3;
		cg.cinematicTime = cg.time;
	}
}

void CG_LoadHudMenu();

static void CG_ReloadHUD(void) {
	String_Init();
	CG_LoadHudMenu();
}

static void CG_PrintWeaponMuzzleOffset_f ( void )
{
    centity_t *cent = &cg_entities[cg.snap->ps.clientNum];
    void *g2Weapon = cent->ghoul2;
    
    if ( !cgi.G2_HaveWeGhoul2Models (g2Weapon) )
    {
        CG_Printf ("Current weapon does not use a GHOUL2 model.\n");
    }
    else if ( !cgi.G2API_HasGhoul2ModelOnIndex (&g2Weapon, 1) )
    {
        CG_Printf ("Current weapon has no model on index 1.\n");
    }
    else
    {
        static const vec3_t worldForward = { 0.0f, 1.0f, 0.0f };
        mdxaBone_t muzzleBone;
        vec3_t muzzleOffset;
        
        if ( !cgi.G2API_GetBoltMatrix (g2Weapon, 1, 0, &muzzleBone, worldForward, vec3_origin, cg.time, cgs.gameModels, cent->modelScale) )
        {
            CG_Printf ("Unable to get muzzle bolt matrix for the current weapon.\n");
            return;
        }
        
        BG_GiveMeVectorFromMatrix (&muzzleBone, ORIGIN, muzzleOffset);
        VectorSubtract (muzzleOffset, muzzleOffset, cent->lerpOrigin);
        
        CG_Printf ("Muzzle offset at (%f %f %f).\n", muzzleOffset[0], muzzleOffset[1], muzzleOffset[2]);
    }
}


// TEST
int testMasterFinalFunc (asyncTask_t *task) {
	cJSON *data = (cJSON *)task->finalData;
	
	if (task->errorCode == 0) {
		Com_Printf("Test successful! (bounce: %i - %i)\n", cJSON_ToInteger(cJSON_GetObjectItem(data, "bounce")), cgi.Milliseconds());
	} else {
		Com_Printf("Test failed!\n");
	}
	return 0;
}

static void JKG_OpenInventoryMenu_f ( void )
{
    uiImports->InventoryNotify (0);
}

void JKG_OpenShopMenu_f ( void )
{
	uiImports->ShopNotify( 0 );
}

static void JKG_UseACI_f ( void )
{
    char buf[3];
    int slot;
    
    if ( cgi.Argc() != 2 )
    {
        CG_Printf ("Usage: /useACI <slot number>\n");
        return;
    }
    
    cgi.Argv (1, buf, sizeof (buf));
    if ( buf[0] < '0' || buf[0] > '9' )
    {
        return;
    }
    
    slot = atoi (buf);
    
    if ( slot < 0 || slot >= MAX_ACI_SLOTS )
    {
        return;
    }
    
    if ( !cg.playerACI[slot] )
    {
        return;
    }
    
    cg.weaponSelect = slot;
}

static void JKG_DumpWeaponList_f ( void )
{
    char filename[MAX_QPATH];
    if ( cgi.Argc() > 1 )
    {
        cgi.Argv (1, filename, sizeof (filename));
    }
    else
    {
        Q_strncpyz (filename, "weaponlist.txt", sizeof (filename));
    }
    
    if ( BG_DumpWeaponList (filename) )
    {
        CG_Printf ("Weapon list was written to %s.\n", filename);
    }
    else
    {
        CG_Printf ("Failed to write weapon list to %s.\n", filename);
    }
}

static void JKG_PrintWeaponList_f ( void )
{
	BG_PrintWeaponList();
}

static void JKG_ToggleCrouch ( void )
{
	if((cg.time - cg.crouchToggleTime) <= 400)
	{
		// You can now no longer "teabag at maximum velocity" --eez
		return;
	}
	cg.crouchToggled = !cg.crouchToggled;
	cg.crouchToggleTime = cg.time;
}

#ifdef __AUTOWAYPOINT__
extern void AIMod_AutoWaypoint ( void );
extern void AIMod_AutoWaypoint_Clean ( void );
extern void AIMod_MarkBadHeight ( void );
extern void AIMod_AddRemovalPoint ( void );
extern void AIMod_AWC_MarkBadHeight ( void );
extern void CG_ShowSurface ( void );
extern void CG_ShowSlope ( void );
#endif //__AUTOWAYPOINT__

typedef struct {
	char	*cmd;
	void	(*function)(void);
} consoleCommand_t;

static consoleCommand_t	commands[] = {
	{ "testgun", CG_TestGun_f },
	{ "testmodel", CG_TestModel_f },
	{ "nextframe", CG_TestModelNextFrame_f },
	{ "prevframe", CG_TestModelPrevFrame_f },
	{ "nextskin", CG_TestModelNextSkin_f },
	{ "prevskin", CG_TestModelPrevSkin_f },
	{ "viewpos", CG_Viewpos_f },
	{ "+scores", CG_ScoresDown_f },
	{ "-scores", CG_ScoresUp_f },
	{ "sizeup", CG_SizeUp_f },
	{ "sizedown", CG_SizeDown_f },
	{ "weapnext", CG_NextWeapon_f },
	{ "weapprev", CG_PrevWeapon_f },
	{ "weapon", CG_Weapon_f },
	{ "weaponclean", CG_WeaponClean_f },
	{ "tell_target", CG_TellTarget_f },
	{ "tell_attacker", CG_TellAttacker_f },
	{ "tcmd", CG_TargetCommand_f },
	{ "spWin", CG_spWin_f },
	{ "spLose", CG_spLose_f },
	{ "scoresDown", CG_scrollScoresDown_f },
	{ "scoresUp", CG_scrollScoresUp_f },
	{ "startOrbit", CG_StartOrbit_f },
	//{ "camera", CG_Camera_f },
	{ "loaddeferred", CG_LoadDeferredPlayers },
	{ "invnext", CG_NextInventory_f },
	{ "invprev", CG_PrevInventory_f },
	{ "forcenext", CG_NextForcePower_f },
	{ "forceprev", CG_PrevForcePower_f },
	// Jedi Knight Galaxies
	{ "startcin", CG_StartCinematic },
	{ "stopcin", CG_StopCinematic },
	{ "reloadhud", CG_ReloadHUD },
	{ "+camera", CG_Start360Camera },
	{ "-camera", CG_Stop360Camera },
	{ "cameraZoomIn", CG_CameraZoomIn },
	{ "cameraZoomOut", CG_CameraZoomOut },
	//{ "party", CG_OpenPartyManagement_f },
	{ "printWeaponMuzzle", CG_PrintWeaponMuzzleOffset_f },
	{ "useACI", JKG_UseACI_f },
	{ "inventory", JKG_OpenInventoryMenu_f },
	{ "shop", JKG_OpenShopMenu_f },
	{ "dumpWeaponList", JKG_DumpWeaponList_f },
	{ "printWeaponList", JKG_PrintWeaponList_f },

#ifdef __AUTOWAYPOINT__
	{ "awp", AIMod_AutoWaypoint },
	{ "autowaypoint", AIMod_AutoWaypoint },
	{ "awc", AIMod_AutoWaypoint_Clean },
	{ "autowaypointclean", AIMod_AutoWaypoint_Clean },
	{ "showsurface", CG_ShowSurface },
	{ "showslope", CG_ShowSlope },
	{ "aw_badheight", AIMod_MarkBadHeight },
	{ "awc_addremovalspot", AIMod_AddRemovalPoint },
	{ "awc_addbadheight", AIMod_AWC_MarkBadHeight },
#endif //__AUTOWAYPOINT__

	{ "togglecrouch", JKG_ToggleCrouch },
};


/*
=================
CG_ConsoleCommand

The string has been tokenized and can be retrieved with
Cmd_Argc() / Cmd_Argv()
=================
*/
qboolean CG_ConsoleCommand( void ) {
	const char	*cmd;
	int		i;

	cmd = CG_Argv(0);

	for ( i = 0 ; i < sizeof( commands ) / sizeof( commands[0] ) ; i++ ) {
		if ( !Q_stricmp( cmd, commands[i].cmd ) ) {
			commands[i].function();
			return qtrue;
		}
	}

	return qfalse;
}


/*
=================
CG_InitConsoleCommands

Let the client system know about all of our commands
so it can perform tab completion
=================
*/
void CG_InitConsoleCommands( void ) {
	int		i;

	for ( i = 0 ; i < sizeof( commands ) / sizeof( commands[0] ) ; i++ ) {
		cgi.AddCommand( commands[i].cmd );
	}

	//
	// the game server will interpret these commands, which will be automatically
	// forwarded to the server after they are not recognized locally
	//
	cgi.AddCommand ("forcechanged");
	cgi.AddCommand ("sv_invnext");
	cgi.AddCommand ("sv_invprev");
	cgi.AddCommand ("sv_forcenext");
	cgi.AddCommand ("sv_forceprev");
	cgi.AddCommand ("sv_saberswitch");
	cgi.AddCommand ("engage_duel");
	cgi.AddCommand ("force_heal");
	cgi.AddCommand ("force_speed");
	cgi.AddCommand ("force_throw");
	cgi.AddCommand ("force_pull");
	cgi.AddCommand ("force_distract");
	cgi.AddCommand ("force_rage");
	cgi.AddCommand ("force_protect");
	cgi.AddCommand ("force_absorb");
	cgi.AddCommand ("force_healother");
	cgi.AddCommand ("force_forcepowerother");
	cgi.AddCommand ("force_seeing");
	cgi.AddCommand ("use_seeker");
	cgi.AddCommand ("use_field");
	cgi.AddCommand ("use_bacta");
	cgi.AddCommand ("use_electrobinoculars");
	cgi.AddCommand ("zoom");
	cgi.AddCommand ("use_sentry");
	cgi.AddCommand ("bot_order");
	cgi.AddCommand ("saberAttackCycle");
	cgi.AddCommand ("kill");
	cgi.AddCommand ("say");
	cgi.AddCommand ("say_team");
	cgi.AddCommand ("tell");
	cgi.AddCommand ("give");
	cgi.AddCommand ("god");
	cgi.AddCommand ("notarget");
	cgi.AddCommand ("noclip");
	cgi.AddCommand ("team");
	cgi.AddCommand ("follow");
	cgi.AddCommand ("levelshot");
	cgi.AddCommand ("addbot");
	cgi.AddCommand ("setviewpos");
	cgi.AddCommand ("callvote");
	cgi.AddCommand ("vote");
	cgi.AddCommand ("callteamvote");
	cgi.AddCommand ("teamvote");
	cgi.AddCommand ("stats");
	cgi.AddCommand ("teamtask");
	cgi.AddCommand ("loaddefered");	// spelled wrong, but not changing for demo
}
