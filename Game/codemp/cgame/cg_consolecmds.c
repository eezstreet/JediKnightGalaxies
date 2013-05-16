// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_consolecmds.c -- text commands typed in at the local console, or
// executed by a key binding

#include "cg_local.h"
#include "../ui/ui_shared.h"
#include "bg_saga.h"

#include "cg_crossover.h"

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
	if ( trap_Key_GetCatcher() == 0 && cg.i360CameraTime == 0 )
	{
		usercmd_t cmd;
		trap_GetUserCmd( trap_GetCurrentCmdNumber(), &cmd );

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
			trap_SendConsoleCommand( "cg_thirdPerson !" );
		}
		/* It was a full rotate so reset the view angles to their original position */
		else
		{
			vec3_t angle;
			angle[YAW] = cg.i360CameraOriginal;
			trap_SetClientForceAngle( cg.time + 10, angle );
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

	trap_Argv( 1, test, 4 );
	trap_SendConsoleCommand( va( "gc %i %i", targetNum, atoi( test ) ) );
}

void CG_OpenPartyManagement_f( void ) {
	CO_PartyMngtNotify(10);
}

/*
=================
CG_SizeUp_f

Keybinding command
=================
*/
static void CG_SizeUp_f (void) {
	trap_Cvar_Set("cg_viewsize", va("%i",(int)(cg_viewsize.integer+10)));
}


/*
=================
CG_SizeDown_f

Keybinding command
=================
*/
static void CG_SizeDown_f (void) {
	trap_Cvar_Set("cg_viewsize", va("%i",(int)(cg_viewsize.integer-10)));
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
		trap_SendClientCommand( "score" );

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
	trap_Cvar_Set("cg_cameraOrbit", "2");
	trap_Cvar_Set("cg_cameraOrbitDelay", "35");
	trap_Cvar_Set("cg_thirdPerson", "1");
	trap_Cvar_Set("cg_thirdPersonAngle", "0");
	trap_Cvar_Set("cg_thirdPersonRange", "100");
	CG_AddBufferedSound(cgs.media.winnerSound);
	//trap_S_StartLocalSound(cgs.media.winnerSound, CHAN_ANNOUNCER);
	CG_CenterPrint(CG_GetStringEdString("MP_INGAME", "YOU_WIN"), SCREEN_HEIGHT * .30, 0);
}

static void CG_spLose_f( void) {
	trap_Cvar_Set("cg_cameraOrbit", "2");
	trap_Cvar_Set("cg_cameraOrbitDelay", "35");
	trap_Cvar_Set("cg_thirdPerson", "1");
	trap_Cvar_Set("cg_thirdPersonAngle", "0");
	trap_Cvar_Set("cg_thirdPersonRange", "100");
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

	trap_Args( message, 128 );
	Com_sprintf( command, 128, "tell %i %s", clientNum, message );
	trap_SendClientCommand( command );
}

static void CG_TellAttacker_f( void ) {
	int		clientNum;
	char	command[128];
	char	message[128];

	clientNum = CG_LastAttacker();
	if ( clientNum == -1 ) {
		return;
	}

	trap_Args( message, 128 );
	Com_sprintf( command, 128, "tell %i %s", clientNum, message );
	trap_SendClientCommand( command );
}


/*
==================
CG_StartOrbit_f
==================
*/

static void CG_StartOrbit_f( void ) {
	char var[MAX_TOKEN_CHARS];

	trap_Cvar_VariableStringBuffer( "developer", var, sizeof( var ) );
	if ( !atoi(var) ) {
		return;
	}
	if (cg_cameraOrbit.value != 0) {
		trap_Cvar_Set ("cg_cameraOrbit", "0");
		trap_Cvar_Set("cg_thirdPerson", "0");
	} else {
		trap_Cvar_Set("cg_cameraOrbit", "5");
		trap_Cvar_Set("cg_thirdPerson", "1");
		trap_Cvar_Set("cg_thirdPersonAngle", "0");
		trap_Cvar_Set("cg_thirdPersonRange", "100");
	}
}

void CG_SiegeBriefingDisplay(int team, int dontshow);
static void CG_SiegeBriefing_f(void)
{
	int team;

	if (cgs.gametype != GT_SIEGE)
	{ //Cannot be displayed unless in this gametype
		return;
	}

	team = cg.predictedPlayerState.persistant[PERS_TEAM];

	if (team != SIEGETEAM_TEAM1 &&
		team != SIEGETEAM_TEAM2)
	{ //cannot be displayed if not on a valid team
		return;
	}

	CG_SiegeBriefingDisplay(team, 0);
}

static void CG_SiegeCvarUpdate_f(void)
{
	int team;

	if (cgs.gametype != GT_SIEGE)
	{ //Cannot be displayed unless in this gametype
		return;
	}

	team = cg.predictedPlayerState.persistant[PERS_TEAM];

	if (team != SIEGETEAM_TEAM1 &&
		team != SIEGETEAM_TEAM2)
	{ //cannot be displayed if not on a valid team
		return;
	}

	CG_SiegeBriefingDisplay(team, 1);
}
static void CG_SiegeCompleteCvarUpdate_f(void)
{

	if (cgs.gametype != GT_SIEGE)
	{ //Cannot be displayed unless in this gametype
		return;
	}

	// Set up cvars for both teams
	CG_SiegeBriefingDisplay(SIEGETEAM_TEAM1, 1);
	CG_SiegeBriefingDisplay(SIEGETEAM_TEAM2, 1);
}
/*
static void CG_Camera_f( void ) {
	char name[1024];
	trap_Argv( 1, name, sizeof(name));
	if (trap_loadCamera(name)) {
		cg.cameraMode = qtrue;
		trap_startCamera(cg.time);
	} else {
		CG_Printf ("Unable to load camera %s\n",name);
	}
}
*/


static void PP_MotionBlurCmd(void) {
	char buff[10];
	if (trap_Argc() < 2) return;
	trap_Argv(1, buff, sizeof(buff));
	cg.motionBlurTime = atoi(buff);
}

static void PP_Blur(void) {
	char buff[10];
	if (trap_Argc() < 3) return;
	trap_Argv(1, buff, sizeof(buff));
	cg.blurPasses = atoi(buff);
	trap_Argv(2, buff, sizeof(buff));
	cg.blurLevel = atof(buff);

}

static void PP_Noise(void) {
	char buff[16];
	if (trap_Argc() < 3) return;
	trap_Argv(1, buff, sizeof(buff));
	cg.noise_cintensity = atof(buff);
	trap_Argv(2, buff, sizeof(buff));
	cg.noise_dintensity = atof(buff);
}

static void PP_ColorModCmd(void) {
	char buff[10];
	char buff2[10];
	float newval;
	if (trap_Argc() < 2) {
		Com_Printf("Usage: colormod <off/set> [r/g/b][s/r] or [fx/fxi/fxb] or [b/c/i] <new value>\n");
		return;
	}
	trap_Argv(1, buff, sizeof(buff));
	if (!Q_stricmp(buff,"off")) {
		cg.colorMod.active=0;
		return;
	}
	if (!Q_stricmp(buff,"set")) {
		if (!cg.colorMod.active) {
			// Init defaults
			cg.colorMod.red_scale = cg.colorMod.green_scale = cg.colorMod.blue_scale = 1;
			cg.colorMod.red_bias = cg.colorMod.green_bias = cg.colorMod.blue_bias = 0;
			cg.colorMod.fx = 0;
			cg.colorMod.fxintensity = 1;
			cg.colorMod.fxbrightness = 1;
			cg.colorMod.brightness = 0;
			cg.colorMod.inversion = 0;
			cg.colorMod.contrast = 1;
			cg.colorMod.active=1;
		}
		trap_Argv(2, buff, sizeof(buff));
		trap_Argv(3, buff2, sizeof(buff2));
		newval = atof(buff2);
		if (buff[0] == 'r') {
			if (buff[1] == 's') {
				cg.colorMod.red_scale = newval;
			} else {
				cg.colorMod.red_bias = newval;
			}
			return;
		}
		if (buff[0] == 'g') {
			if (buff[1] == 's') {
				cg.colorMod.green_scale = newval;
			} else {
				cg.colorMod.green_bias = newval;
			}
			return;
			
		}
		if (buff[0] == 'b') {
			if (buff[1] == 's') {
				cg.colorMod.blue_scale = newval;
			} else if (!buff[1]) {
				cg.colorMod.brightness = newval;
			} else {
				cg.colorMod.blue_bias = newval;
			}
			return;
		}
		if (buff[0] == 'c') {
			cg.colorMod.contrast = newval;
			return;
		}
		if (buff[0] == 'i') {
			cg.colorMod.inversion = newval;
			return;
		}
		if (!Q_stricmp(buff, "fx")) {
			cg.colorMod.fx = newval;
			return;
		}
		if (!Q_stricmp(buff, "fxi")) {
			cg.colorMod.fxintensity = newval;
			return;
		}
		if (!Q_stricmp(buff, "fxb")) {
			cg.colorMod.fxbrightness = newval;
			return;
		}
		Com_Printf("Invalid target\n");
		return;
	}
	Com_Printf("Invalid instruction\n");
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
    
    if ( !trap_G2_HaveWeGhoul2Models (g2Weapon) )
    {
        CG_Printf ("Current weapon does not use a GHOUL2 model.\n");
    }
    else if ( !trap_G2API_HasGhoul2ModelOnIndex (&g2Weapon, 1) )
    {
        CG_Printf ("Current weapon has no model on index 1.\n");
    }
    else
    {
        static const vec3_t worldForward = { 0.0f, 1.0f, 0.0f };
        mdxaBone_t muzzleBone;
        vec3_t muzzleOffset;
        
        if ( !trap_G2API_GetBoltMatrix (g2Weapon, 1, 0, &muzzleBone, worldForward, vec3_origin, cg.time, cgs.gameModels, cent->modelScale) )
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
		Com_Printf("Test successful! (bounce: %i - %i)\n", cJSON_ToInteger(cJSON_GetObjectItem(data, "bounce")), trap_Milliseconds());
	} else {
		Com_Printf("Test failed!\n");
	}
	return 0;
}

static void CG_TestMaster(void) {
	JKG_GLCG_Task_Test(testMasterFinalFunc);
}

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
	{ "briefing", CG_SiegeBriefing_f },
	{ "siegeCvarUpdate", CG_SiegeCvarUpdate_f },
	{ "siegeCompleteCvarUpdate", CG_SiegeCompleteCvarUpdate_f },
	// Jedi Knight Galaxies
	{ "motionblur", PP_MotionBlurCmd },
	{ "colormod", PP_ColorModCmd },
	{ "blur", PP_Blur },
	{ "noise", PP_Noise },
	{ "startcin", CG_StartCinematic },
	{ "stopcin", CG_StopCinematic },
	{ "reloadhud", CG_ReloadHUD },
	{ "+camera", CG_Start360Camera },
	{ "-camera", CG_Stop360Camera },
	{ "cameraZoomIn", CG_CameraZoomIn },
	{ "cameraZoomOut", CG_CameraZoomOut },
	{ "party", CG_OpenPartyManagement_f },
	{ "printWeaponMuzzle", CG_PrintWeaponMuzzleOffset_f },
	
	// TEST
	{ "cg_testmaster", CG_TestMaster },
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
		trap_AddCommand( commands[i].cmd );
	}

	//
	// the game server will interpret these commands, which will be automatically
	// forwarded to the server after they are not recognized locally
	//
	trap_AddCommand ("forcechanged");
	trap_AddCommand ("sv_invnext");
	trap_AddCommand ("sv_invprev");
	trap_AddCommand ("sv_forcenext");
	trap_AddCommand ("sv_forceprev");
	trap_AddCommand ("sv_saberswitch");
	trap_AddCommand ("engage_duel");
	trap_AddCommand ("force_heal");
	trap_AddCommand ("force_speed");
	trap_AddCommand ("force_throw");
	trap_AddCommand ("force_pull");
	trap_AddCommand ("force_distract");
	trap_AddCommand ("force_rage");
	trap_AddCommand ("force_protect");
	trap_AddCommand ("force_absorb");
	trap_AddCommand ("force_healother");
	trap_AddCommand ("force_forcepowerother");
	trap_AddCommand ("force_seeing");
	trap_AddCommand ("use_seeker");
	trap_AddCommand ("use_field");
	trap_AddCommand ("use_bacta");
	trap_AddCommand ("use_electrobinoculars");
	trap_AddCommand ("zoom");
	trap_AddCommand ("use_sentry");
	trap_AddCommand ("bot_order");
	trap_AddCommand ("saberAttackCycle");
	trap_AddCommand ("kill");
	trap_AddCommand ("say");
	trap_AddCommand ("say_team");
	trap_AddCommand ("tell");
	trap_AddCommand ("give");
	trap_AddCommand ("god");
	trap_AddCommand ("notarget");
	trap_AddCommand ("noclip");
	trap_AddCommand ("team");
	trap_AddCommand ("follow");
	trap_AddCommand ("levelshot");
	trap_AddCommand ("addbot");
	trap_AddCommand ("setviewpos");
	trap_AddCommand ("callvote");
	trap_AddCommand ("vote");
	trap_AddCommand ("callteamvote");
	trap_AddCommand ("teamvote");
	trap_AddCommand ("stats");
	trap_AddCommand ("teamtask");
	trap_AddCommand ("loaddefered");	// spelled wrong, but not changing for demo
}
