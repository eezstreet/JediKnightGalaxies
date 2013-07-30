// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_main.c -- initialization and primary entry point for cgame
#include "cg_local.h"
#include "jkg_navmesh_visualiser.h"

#include "../ui/ui_shared.h"
// display context for new ui stuff
displayContextDef_t cgDC;

#if !defined(CL_LIGHT_H_INC)
	#include "cg_lights.h"
#endif

// Jedi Knight Galaxies
#include "cg_weapons.h"
#include "jkg_gangwars.h"
#include "bg_items.h"

/*
Ghoul2 Insert Start
*/
void CG_InitItems(void);
/*
Ghoul2 Insert End
*/

void CG_InitJetpackGhoul2(void);
void CG_CleanJetpackGhoul2(void);

vec4_t colorTable[CT_MAX] = 
{
{0, 0, 0, 0},			// CT_NONE
{0, 0, 0, 1},			// CT_BLACK
{1, 0, 0, 1},			// CT_RED
{0, 1, 0, 1},			// CT_GREEN
{0, 0, 1, 1},			// CT_BLUE
{1, 1, 0, 1},			// CT_YELLOW
{1, 0, 1, 1},			// CT_MAGENTA
{0, 1, 1, 1},			// CT_CYAN
{1, 1, 1, 1},			// CT_WHITE
{0.75f, 0.75f, 0.75f, 1},	// CT_LTGREY
{0.50f, 0.50f, 0.50f, 1},	// CT_MDGREY
{0.25f, 0.25f, 0.25f, 1},	// CT_DKGREY
{0.15f, 0.15f, 0.15f, 1},	// CT_DKGREY2

{0.810f, 0.530f, 0.0f,  1},	// CT_VLTORANGE -- needs values
{0.810f, 0.530f, 0.0f,  1},	// CT_LTORANGE
{0.610f, 0.330f, 0.0f,  1},	// CT_DKORANGE
{0.402f, 0.265f, 0.0f,  1},	// CT_VDKORANGE

{0.503f, 0.375f, 0.996f, 1},	// CT_VLTBLUE1
{0.367f, 0.261f, 0.722f, 1},	// CT_LTBLUE1
{0.199f, 0.0f,   0.398f, 1},	// CT_DKBLUE1
{0.160f, 0.117f, 0.324f, 1},	// CT_VDKBLUE1

{0.300f, 0.628f, 0.816f, 1},	// CT_VLTBLUE2 -- needs values
{0.300f, 0.628f, 0.816f, 1},	// CT_LTBLUE2
{0.191f, 0.289f, 0.457f, 1},	// CT_DKBLUE2
{0.125f, 0.250f, 0.324f, 1},	// CT_VDKBLUE2

{0.796f, 0.398f, 0.199f, 1},	// CT_VLTBROWN1 -- needs values
{0.796f, 0.398f, 0.199f, 1},	// CT_LTBROWN1
{0.558f, 0.207f, 0.027f, 1},	// CT_DKBROWN1
{0.328f, 0.125f, 0.035f, 1},	// CT_VDKBROWN1

{0.996f, 0.796f, 0.398f, 1},	// CT_VLTGOLD1 -- needs values
{0.996f, 0.796f, 0.398f, 1},	// CT_LTGOLD1
{0.605f, 0.441f, 0.113f, 1},	// CT_DKGOLD1
{0.386f, 0.308f, 0.148f, 1},	// CT_VDKGOLD1

{0.648f, 0.562f, 0.784f, 1},	// CT_VLTPURPLE1 -- needs values
{0.648f, 0.562f, 0.784f, 1},	// CT_LTPURPLE1
{0.437f, 0.335f, 0.597f, 1},	// CT_DKPURPLE1
{0.308f, 0.269f, 0.375f, 1},	// CT_VDKPURPLE1

{0.816f, 0.531f, 0.710f, 1},	// CT_VLTPURPLE2 -- needs values
{0.816f, 0.531f, 0.710f, 1},	// CT_LTPURPLE2
{0.566f, 0.269f, 0.457f, 1},	// CT_DKPURPLE2
{0.343f, 0.226f, 0.316f, 1},	// CT_VDKPURPLE2

{0.929f, 0.597f, 0.929f, 1},	// CT_VLTPURPLE3
{0.570f, 0.371f, 0.570f, 1},	// CT_LTPURPLE3
{0.355f, 0.199f, 0.355f, 1},	// CT_DKPURPLE3
{0.285f, 0.136f, 0.230f, 1},	// CT_VDKPURPLE3

{0.953f, 0.378f, 0.250f, 1},	// CT_VLTRED1
{0.953f, 0.378f, 0.250f, 1},	// CT_LTRED1
{0.593f, 0.121f, 0.109f, 1},	// CT_DKRED1
{0.429f, 0.171f, 0.113f, 1},	// CT_VDKRED1
{.25f, 0, 0, 1},					// CT_VDKRED
{.70f, 0, 0, 1},					// CT_DKRED
	
{0.717f, 0.902f, 1.0f,   1},		// CT_VLTAQUA
{0.574f, 0.722f, 0.804f, 1},		// CT_LTAQUA
{0.287f, 0.361f, 0.402f, 1},		// CT_DKAQUA
{0.143f, 0.180f, 0.201f, 1},		// CT_VDKAQUA

{0.871f, 0.386f, 0.375f, 1},		// CT_LTPINK
{0.435f, 0.193f, 0.187f, 1},		// CT_DKPINK
{	  0,    .5f,    .5f, 1},		// CT_LTCYAN
{	  0,   .25f,   .25f, 1},		// CT_DKCYAN
{   .179f, .51f,   .92f, 1},		// CT_LTBLUE3
{   .199f, .71f,   .92f, 1},		// CT_LTBLUE3
{   .5f,   .05f,    .4f, 1},		// CT_DKBLUE3

{   0.0f,   .613f,  .097f, 1},		// CT_HUD_GREEN
{   0.835f, .015f,  .015f, 1},		// CT_HUD_RED
{	.567f,	.685f,	1.0f,	.75f},	// CT_ICON_BLUE
{	.515f,	.406f,	.507f,	1},		// CT_NO_AMMO_RED
{   1.0f,   .658f,  .062f, 1},		// CT_HUD_ORANGE

};

#include "holocronicons.h"

int cgWeatherOverride = 0;

int forceModelModificationCount = -1;

void CG_Init( int serverMessageNum, int serverCommandSequence, int clientNum );
void CG_Shutdown( void );

void CG_CalcEntityLerpPositions( centity_t *cent );
void CG_ROFF_NotetrackCallback( centity_t *cent, const char *notetrack);

void UI_CleanupGhoul2(void);

static int	C_PointContents(void);
static void C_GetLerpOrigin(void);
static void C_GetLerpData(void);
static void C_Trace(void);
static void C_G2Trace(void);
static void C_G2Mark(void);
static int	CG_RagCallback(int callType);
static void C_GetBoltPos(void);
static void C_ImpactMark(void);

#define MAX_MISC_ENTS	4000

//static refEntity_t	*MiscEnts = 0;
//static float		*Radius = 0;
static refEntity_t	MiscEnts[MAX_MISC_ENTS]; //statically allocated for now.
static float		Radius[MAX_MISC_ENTS];
static float		zOffset[MAX_MISC_ENTS]; //some models need a z offset for culling, because of stupid wrong model origins

static int			NumMiscEnts = 0;

extern autoMapInput_t cg_autoMapInput; //cg_view.c
extern int cg_autoMapInputTime;
extern vec3_t cg_autoMapAngle;

void CG_MiscEnt(void);
void CG_DoCameraShake( vec3_t origin, float intensity, int radius, int time );

qboolean cgame_initializing = qtrue;

//do we have any force powers that we would normally need to cycle to?
qboolean CG_NoUseableForce(void)
{
	int i = FP_HEAL;
	while (i < NUM_FORCE_POWERS)
	{
		if (i != FP_SABERTHROW &&
			i != FP_SABER_OFFENSE &&
			i != FP_SABER_DEFENSE &&
			i != FP_LEVITATION)
		{ //valid selectable power
			if (cg.predictedPlayerState.fd.forcePowersKnown & (1 << i))
			{ //we have it
				return qfalse;
			}
		}
		i++;
	}

	//no useable force powers, I guess.
	return qtrue;
}

extern void ChatBox_UseMessageMode(int whichOne);

static int C_PointContents(void)
{
	TCGPointContents	*data = (TCGPointContents *)cg.sharedBuffer;

	return CG_PointContents( data->mPoint, data->mPassEntityNum );
}

static void C_GetLerpOrigin(void)
{
	TCGVectorData		*data = (TCGVectorData *)cg.sharedBuffer;

	VectorCopy(cg_entities[data->mEntityNum].lerpOrigin, data->mPoint);
}

static void C_GetLerpData(void)
{//only used by FX system to pass to getboltmat
	TCGGetBoltData		*data = (TCGGetBoltData *)cg.sharedBuffer;

	VectorCopy(cg_entities[data->mEntityNum].lerpOrigin, data->mOrigin);
	VectorCopy(cg_entities[data->mEntityNum].modelScale, data->mScale);
	VectorCopy(cg_entities[data->mEntityNum].lerpAngles, data->mAngles);
	if (cg_entities[data->mEntityNum].currentState.eType == ET_PLAYER)
	{ //normal player
		data->mAngles[PITCH] = 0.0f;
		data->mAngles[ROLL] = 0.0f;
	}
	else if (cg_entities[data->mEntityNum].currentState.eType == ET_NPC)
	{ //an NPC
		Vehicle_t *pVeh = cg_entities[data->mEntityNum].m_pVehicle;
		if (!pVeh)
		{ //for vehicles, we may or may not want to 0 out pitch and roll
			data->mAngles[PITCH] = 0.0f;
			data->mAngles[ROLL] = 0.0f;
		}
		else if (pVeh->m_pVehicleInfo->type == VH_SPEEDER)
		{ //speeder wants no pitch but a roll
			data->mAngles[PITCH] = 0.0f;
		}
		else if (pVeh->m_pVehicleInfo->type != VH_FIGHTER)
		{ //fighters want all angles
			data->mAngles[PITCH] = 0.0f;
			data->mAngles[ROLL] = 0.0f;
		}
	}
}

static void C_Trace(void)
{
	TCGTrace	*td = (TCGTrace *)cg.sharedBuffer;

	CG_Trace(&td->mResult, td->mStart, td->mMins, td->mMaxs, td->mEnd, td->mSkipNumber, td->mMask);
}

static void C_G2Trace(void)
{
	TCGTrace	*td = (TCGTrace *)cg.sharedBuffer;

	CG_G2Trace(&td->mResult, td->mStart, td->mMins, td->mMaxs, td->mEnd, td->mSkipNumber, td->mMask);
}

static void C_G2Mark(void)
{
	TCGG2Mark	*td = (TCGG2Mark *)cg.sharedBuffer;
	trace_t		tr;
	vec3_t		end;

	VectorMA(td->start, 64, td->dir, end);
	CG_G2Trace(&tr, td->start, NULL, NULL, end, ENTITYNUM_NONE, MASK_PLAYERSOLID);

	if (tr.entityNum < ENTITYNUM_WORLD &&
		cg_entities[tr.entityNum].ghoul2)
	{ //hit someone with a ghoul2 instance, let's project the decal on them then.
		centity_t *cent = &cg_entities[tr.entityNum];

		//CG_TestLine(tr.endpos, end, 2000, 0x0000ff, 1);

		CG_AddGhoul2Mark(td->shader, td->size, tr.endpos, end, tr.entityNum,
			cent->lerpOrigin, cent->lerpAngles[YAW], cent->ghoul2, cent->modelScale,
			Q_irand(2000, 4000));
		//I'm making fx system decals have a very short lifetime.
	}
}

static void CG_DebugBoxLines(vec3_t mins, vec3_t maxs, int duration)
{
	vec3_t start;
	vec3_t end;
	vec3_t vert;

	float x = maxs[0] - mins[0];
	float y = maxs[1] - mins[1];

	start[2] = maxs[2];
	vert[2] = mins[2];

	vert[0] = mins[0];
	vert[1] = mins[1];
	start[0] = vert[0];
	start[1] = vert[1];
	CG_TestLine(start, vert, duration, 0x00000ff, 1);

	vert[0] = mins[0];
	vert[1] = maxs[1];
	start[0] = vert[0];
	start[1] = vert[1];
	CG_TestLine(start, vert, duration, 0x00000ff, 1);

	vert[0] = maxs[0];
	vert[1] = mins[1];
	start[0] = vert[0];
	start[1] = vert[1];
	CG_TestLine(start, vert, duration, 0x00000ff, 1);

	vert[0] = maxs[0];
	vert[1] = maxs[1];
	start[0] = vert[0];
	start[1] = vert[1];
	CG_TestLine(start, vert, duration, 0x00000ff, 1);

	// top of box
	VectorCopy(maxs, start);
	VectorCopy(maxs, end);
	start[0] -= x;
	CG_TestLine(start, end, duration, 0x00000ff, 1);
	end[0] = start[0];
	end[1] -= y;
	CG_TestLine(start, end, duration, 0x00000ff, 1);
	start[1] = end[1];
	start[0] += x;
	CG_TestLine(start, end, duration, 0x00000ff, 1);
	CG_TestLine(start, maxs, duration, 0x00000ff, 1);
	// bottom of box
	VectorCopy(mins, start);
	VectorCopy(mins, end);
	start[0] += x;
	CG_TestLine(start, end, duration, 0x00000ff, 1);
	end[0] = start[0];
	end[1] += y;
	CG_TestLine(start, end, duration, 0x00000ff, 1);
	start[1] = end[1];
	start[0] -= x;
	CG_TestLine(start, end, duration, 0x00000ff, 1);
	CG_TestLine(start, mins, duration, 0x00000ff, 1);
}

//handle ragdoll callbacks, for events and debugging -rww
static int CG_RagCallback(int callType)
{
	switch(callType)
	{
	case RAG_CALLBACK_DEBUGBOX:
		{
			ragCallbackDebugBox_t *callData = (ragCallbackDebugBox_t *)cg.sharedBuffer;

			CG_DebugBoxLines(callData->mins, callData->maxs, callData->duration);
		}
		break;
	case RAG_CALLBACK_DEBUGLINE:
		{
			ragCallbackDebugLine_t *callData = (ragCallbackDebugLine_t *)cg.sharedBuffer;

			CG_TestLine(callData->start, callData->end, callData->time, callData->color, callData->radius);
		}
		break;
	case RAG_CALLBACK_BONESNAP:
		{
			ragCallbackBoneSnap_t *callData = (ragCallbackBoneSnap_t *)cg.sharedBuffer;
			centity_t *cent = &cg_entities[callData->entNum];
			int snapSound = cgi.S_RegisterSound(va("sound/player/bodyfall_human%i.wav", Q_irand(1, 3)));

			cgi.S_StartSound(cent->lerpOrigin, callData->entNum, CHAN_AUTO, snapSound);
		}
	case RAG_CALLBACK_BONEIMPACT:
		break;
	case RAG_CALLBACK_BONEINSOLID:
#if 0
		{
			ragCallbackBoneInSolid_t *callData = (ragCallbackBoneInSolid_t *)cg.sharedBuffer;

			if (callData->solidCount > 16)
			{ //don't bother if we're just tapping into solidity, we'll probably recover on our own
				centity_t *cent = &cg_entities[callData->entNum];
				vec3_t slideDir;

				VectorSubtract(cent->lerpOrigin, callData->bonePos, slideDir);
				VectorAdd(cent->ragOffsets, slideDir, cent->ragOffsets);

				cent->hasRagOffset = qtrue;
			}
		}
#endif
		break;
	case RAG_CALLBACK_TRACELINE:
		{
			ragCallbackTraceLine_t *callData = (ragCallbackTraceLine_t *)cg.sharedBuffer;

			CG_Trace(&callData->tr, callData->start, callData->mins, callData->maxs,
				callData->end, callData->ignore, callData->mask);
		}
		break;
	default:
		Com_Error(ERR_DROP, "Invalid callType in CG_RagCallback");
		break;
	}

	return 0;
}

static void C_ImpactMark(void)
{
	TCGImpactMark	*data = (TCGImpactMark *)cg.sharedBuffer;

	/*
	CG_ImpactMark((int)arg0, (const float *)arg1, (const float *)arg2, (float)arg3,
		(float)arg4, (float)arg5, (float)arg6, (float)arg7, qtrue, (float)arg8, qfalse);
	*/
	CG_ImpactMark(data->mHandle, data->mPoint, data->mAngle, data->mRotation,
		data->mRed, data->mGreen, data->mBlue, data->mAlphaStart, qtrue, data->mSizeStart, qfalse);
}

void CG_MiscEnt(void)
{
	int			modelIndex;
	refEntity_t	*RefEnt;
	TCGMiscEnt	*data = (TCGMiscEnt *)cg.sharedBuffer;
	vec3_t		mins, maxs;
	float		*radius, *zOff;

	if (NumMiscEnts >= MAX_MISC_ENTS)
	{
		return;
	}
	
	radius = &Radius[NumMiscEnts];
	zOff = &zOffset[NumMiscEnts];
	RefEnt = &MiscEnts[NumMiscEnts++];

	modelIndex = cgi.R_RegisterModel(data->mModel);
	if (modelIndex == 0)
	{
		Com_Error(ERR_DROP, "client_model has invalid model definition");
		return;
	}

	*zOff = 0;

	memset(RefEnt, 0, sizeof(refEntity_t));
	RefEnt->reType = RT_MODEL;
	RefEnt->hModel = modelIndex;
	RefEnt->frame = 0;
	cgi.R_ModelBounds(modelIndex, mins, maxs);
	VectorCopy(data->mScale, RefEnt->modelScale);
	VectorCopy(data->mOrigin, RefEnt->origin);

	VectorScaleVector(mins, data->mScale, mins);
	VectorScaleVector(maxs, data->mScale, maxs);
	*radius = Distance(mins, maxs);

	AnglesToAxis( data->mAngles, RefEnt->axis );
	ScaleModelAxis(RefEnt);
}

void CG_DrawMiscEnts(void)
{
	int			i;
	refEntity_t	*RefEnt;
	float		*radius, *zOff;
	vec3_t		difference;
	vec3_t		cullOrigin;

	RefEnt = MiscEnts;
	radius = Radius;
	zOff = zOffset;
	for(i=0;i<NumMiscEnts;i++)
	{
		VectorCopy(RefEnt->origin, cullOrigin);
		cullOrigin[2] += 1.0f;

		if (*zOff)
		{
			cullOrigin[2] += *zOff;
		}

		if (cg.snap && cgi.R_inPVS(cg.refdef.vieworg, cullOrigin, cg.snap->areamask))
		{
			VectorSubtract(RefEnt->origin, cg.refdef.vieworg, difference);
			if (VectorLength(difference)-(*radius) <= cg.distanceCull)
			{
				cgi.R_AddRefEntityToScene(RefEnt);
			}
		}
		RefEnt++;
		radius++;
		zOff++;
	}
}

/*
Ghoul2 Insert Start
*/
/*
void CG_ResizeG2Bolt(boltInfo_v *bolt, int newCount)
{
	bolt->resize(newCount);
}

void CG_ResizeG2Surface(surfaceInfo_v *surface, int newCount)
{
	surface->resize(newCount);
}

void CG_ResizeG2Bone(boneInfo_v *bone, int newCount)
{
	bone->resize(newCount);
}

void CG_ResizeG2(CGhoul2Info_v *ghoul2, int newCount)
{
	ghoul2->resize(newCount);
}

void CG_ResizeG2TempBone(mdxaBone_v *tempBone, int newCount)
{
	tempBone->resize(newCount);
}
*/
/*
Ghoul2 Insert End
*/
cg_t				cg;
cgs_t				cgs;
centity_t			cg_entities[MAX_GENTITIES];

centity_t			*cg_permanents[MAX_GENTITIES]; //rwwRMG - added
int					cg_numpermanents = 0;

itemInfo_t			cg_items[MAX_ITEMS];

// Jedi Knight Galaxies, pain flashes
int PainFlashStart;
int PainFlashDuration;
int PainFlashIntensity;
//

vmCvar_t	cg_centertime;
vmCvar_t	cg_runpitch;
vmCvar_t	cg_runroll;
vmCvar_t	cg_bobup;
vmCvar_t	cg_bobpitch;
vmCvar_t	cg_bobroll;
//vmCvar_t	cg_swingSpeed;
vmCvar_t	cg_shadows;
vmCvar_t	cg_renderToTextureFX;
vmCvar_t	cg_drawTimer;
vmCvar_t	cg_drawFPS;
vmCvar_t	cg_drawSnapshot;
vmCvar_t	cg_draw3dIcons;
vmCvar_t	cg_drawIcons;
vmCvar_t	cg_drawAmmoWarning;
vmCvar_t	cg_drawCrosshair;
vmCvar_t	cg_drawCrosshairNames;
vmCvar_t	cg_drawRadar;
vmCvar_t	cg_drawVehLeadIndicator;
vmCvar_t	cg_dynamicCrosshair;
vmCvar_t	cg_dynamicCrosshairPrecision;
vmCvar_t	cg_drawScores;
vmCvar_t	cg_crosshairSize;
vmCvar_t	cg_crosshairX;
vmCvar_t	cg_crosshairY;
vmCvar_t	cg_crosshairHealth;
vmCvar_t	cg_draw2D;
vmCvar_t	cg_drawStatus;
vmCvar_t	cg_animSpeed;
vmCvar_t	cg_debugAnim;
vmCvar_t	cg_debugSaber;
vmCvar_t	cg_debugEvents;
vmCvar_t	cg_errorDecay;
vmCvar_t	cg_nopredict;
vmCvar_t	cg_noPlayerAnims;
vmCvar_t	cg_showmiss;
vmCvar_t	cg_showVehMiss;
vmCvar_t	cg_footsteps;
vmCvar_t	cg_addMarks;
vmCvar_t	cg_viewsize;
vmCvar_t	cg_drawGun;
vmCvar_t	cg_gun_frame;
vmCvar_t	cg_gun_x;
vmCvar_t	cg_gun_y;
vmCvar_t	cg_gun_z;
vmCvar_t	cg_simpleItems;
vmCvar_t	cg_fov;
vmCvar_t	cg_zoomFov;

vmCvar_t	cg_swingAngles;

vmCvar_t	cg_oldPainSounds;

vmCvar_t	cg_ragDoll;

vmCvar_t	cg_jumpSounds;

vmCvar_t	cg_autoMap;
vmCvar_t	cg_autoMapX;
vmCvar_t	cg_autoMapY;
vmCvar_t	cg_autoMapW;
vmCvar_t	cg_autoMapH;

#ifndef _XBOX	// Hmmm. This is also in game. I think this is safe.
vmCvar_t	bg_fighterAltControl;
#endif

// Enhanced joystick controls --eez
vmCvar_t	in_invertYLook;
vmCvar_t	in_invertXLook;
vmCvar_t	in_controlScheme;
vmCvar_t	in_rumbleIntensity;

vmCvar_t	cg_chatBox;
vmCvar_t	cg_chatBoxHeight;

vmCvar_t	cg_saberModelTraceEffect;

vmCvar_t	cg_saberClientVisualCompensation;

vmCvar_t	cg_g2TraceLod;

vmCvar_t	cg_ghoul2Marks;

//[TrueView]
vmCvar_t		cg_trueroll;
vmCvar_t		cg_trueflip;
vmCvar_t		cg_truespin;
vmCvar_t		cg_truemoveroll;
vmCvar_t		cg_truesaberonly;
vmCvar_t		cg_trueeyeposition;
vmCvar_t		cg_trueinvertsaber;
vmCvar_t		cg_truefov;
//[/TrueView]

//[TrueView]
vmCvar_t	cg_trueguns;
vmCvar_t	cg_fpls;
//[/TrueView]

vmCvar_t	cg_optvehtrace;

vmCvar_t	cg_saberDynamicMarks;
vmCvar_t	cg_saberDynamicMarkTime;

vmCvar_t	cg_saberContact;
vmCvar_t	cg_saberTrail;

vmCvar_t	cg_speedTrail;
vmCvar_t	cg_auraShell;

vmCvar_t	cg_repeaterOrb;

vmCvar_t	cg_animBlend;

vmCvar_t	cg_dismember;

vmCvar_t	cg_thirdPersonSpecialCam;

vmCvar_t	cg_thirdPerson;
vmCvar_t	cg_thirdPersonRange;
vmCvar_t	cg_thirdPersonAngle;
vmCvar_t	cg_thirdPersonPitchOffset;
vmCvar_t	cg_thirdPersonVertOffset;
vmCvar_t	cg_thirdPersonCameraDamp;
vmCvar_t	cg_thirdPersonTargetDamp;

vmCvar_t	cg_thirdPersonAlpha;
vmCvar_t	cg_thirdPersonHorzOffset;

vmCvar_t	cg_stereoSeparation;
vmCvar_t	cg_lagometer;
vmCvar_t	cg_drawEnemyInfo;
vmCvar_t	cg_synchronousClients;
vmCvar_t 	cg_stats;
vmCvar_t 	cg_buildScript;
vmCvar_t 	cg_forceModel;
vmCvar_t	cg_paused;
vmCvar_t	cg_predictItems;
vmCvar_t	cg_deferPlayers;
vmCvar_t	cg_drawTeamOverlay;
vmCvar_t	cg_teamOverlayUserinfo;
vmCvar_t	cg_drawFriend;
vmCvar_t	cg_teamChatsOnly;
vmCvar_t	cg_hudFiles;
vmCvar_t	cg_drawHUD;
vmCvar_t 	cg_scorePlum;
vmCvar_t 	cg_smoothClients;

vmCvar_t	pmove_fixed;
//vmCvar_t	cg_pmove_fixed;
vmCvar_t	pmove_msec;
// nmckenzie: DUEL_HEALTH
vmCvar_t	g_showDuelHealths;

vmCvar_t	cg_pmove_msec;
vmCvar_t	cg_cameraMode;
vmCvar_t	cg_cameraOrbit;
vmCvar_t	cg_cameraOrbitDelay;
vmCvar_t	cg_timescaleFadeEnd;
vmCvar_t	cg_timescaleFadeSpeed;
vmCvar_t	cg_timescale;
vmCvar_t	cg_noTaunt;
vmCvar_t	cg_noProjectileTrail;
//vmCvar_t	cg_trueLightning;
/*
Ghoul2 Insert Start
*/
vmCvar_t	cg_debugBB;
/*
Ghoul2 Insert End
*/
//vmCvar_t 	cg_redTeamName;
//vmCvar_t 	cg_blueTeamName;
vmCvar_t	cg_currentSelectedPlayer;
vmCvar_t	cg_currentSelectedPlayerName;
//vmCvar_t	cg_singlePlayerActive;
vmCvar_t	cg_showVehBounds;

vmCvar_t	ui_myteam;

vmCvar_t	cg_snapshotTimeout;

vmCvar_t	jkg_noletterbox;
vmCvar_t	jkg_gunlesscrosshair;
vmCvar_t	jkg_smoothcamera;

vmCvar_t	jkg_autoreload;
vmCvar_t    jkg_debugBBox;

vmCvar_t	jkg_viewmodelPopup;

/*
vmCvar_t	jkg_debugSprintPitch;
vmCvar_t	jkg_debugSprintYaw;
vmCvar_t	jkg_debugSprintRoll;
vmCvar_t	jkg_debugSprintX;
vmCvar_t	jkg_debugSprintY;
vmCvar_t	jkg_debugSprintZ;
vmCvar_t	jkg_debugSprintBobPitch;
vmCvar_t	jkg_debugSprintBobYaw;
vmCvar_t	jkg_debugSprintBobRoll;
vmCvar_t	jkg_debugSprintBobX;
vmCvar_t	jkg_debugSprintBobY;
vmCvar_t	jkg_debugSprintBobZ;
vmCvar_t	jkg_debugSprintStyle;
vmCvar_t	jkg_debugSprintBobSpeed;
*/

vmCvar_t	jkg_meleeScroll;
vmCvar_t	jkg_sprintFOV;

vmCvar_t    r_force_pot_textures;
vmCvar_t	r_force_arb_shaders;
vmCvar_t    r_bloom_factor;
vmCvar_t    r_bloom_threshold;
vmCvar_t	ui_hidehud;		// Used by the UI to disable the HUD at certain times
vmCvar_t	ui_blurbackground;	// Blur the background when UI is active?

vmCvar_t	jkg_simpleHUD;

extern vmCvar_t	jkg_nokillmessages;

#ifdef __SWF__
vmCvar_t	jkg_swf;
#endif //__SWF__

vmCvar_t jkg_normalMapping;
vmCvar_t jkg_debugNavmesh;

#ifdef __AUTOWAYPOINT__
vmCvar_t jkg_waypoint_render;
#endif //__AUTOWAYPOINT__

#ifdef __WEAPON_HOLSTER__
vmCvar_t	d_poff;
vmCvar_t	d_roff;
vmCvar_t	d_yoff;
#endif //__WEAPON_HOLSTER__

typedef struct {
	vmCvar_t	*vmCvar;
	char		*cvarName;
	char		*defaultString;
	int			cvarFlags;
} cvarTable_t;

static cvarTable_t cvarTable[] = { // bk001129
	{ &cg_drawGun, "cg_drawGun", "1", CVAR_ARCHIVE },
	{ &cg_zoomFov, "cg_zoomfov", "40.0", CVAR_ARCHIVE },
	{ &cg_fov, "cg_fov", "80", CVAR_ARCHIVE },
	{ &cg_viewsize, "cg_viewsize", "100", CVAR_ARCHIVE },
	{ &cg_stereoSeparation, "cg_stereoSeparation", "0.4", CVAR_ARCHIVE  },
	{ &cg_shadows, "cg_shadows", "1", CVAR_ARCHIVE  },
	{ &cg_renderToTextureFX, "cg_renderToTextureFX", "1", CVAR_ARCHIVE  },
	{ &cg_draw2D, "cg_draw2D", "1", CVAR_ARCHIVE  },
	{ &cg_drawStatus, "cg_drawStatus", "1", CVAR_ARCHIVE  },
	{ &cg_drawTimer, "cg_drawTimer", "0", CVAR_ARCHIVE  },
	{ &cg_drawFPS, "cg_drawFPS", "0", CVAR_ARCHIVE  },
	{ &cg_drawSnapshot, "cg_drawSnapshot", "0", CVAR_ARCHIVE  },
	{ &cg_draw3dIcons, "cg_draw3dIcons", "1", CVAR_ARCHIVE  },
	{ &cg_drawIcons, "cg_drawIcons", "1", CVAR_ARCHIVE  },
	{ &cg_drawAmmoWarning, "cg_drawAmmoWarning", "0", CVAR_ARCHIVE  },
	{ &cg_drawEnemyInfo, "cg_drawEnemyInfo", "1", CVAR_ARCHIVE  },
	{ &cg_drawCrosshair, "cg_drawCrosshair", "1", CVAR_ARCHIVE },
	{ &cg_drawCrosshairNames, "cg_drawCrosshairNames", "1", CVAR_ARCHIVE },
	{ &cg_drawRadar, "cg_drawRadar", "1", CVAR_ARCHIVE },
	{ &cg_drawVehLeadIndicator, "cg_drawVehLeadIndicator", "1", CVAR_ARCHIVE },
	{ &cg_drawScores,		  "cg_drawScores", "1", CVAR_ARCHIVE },
	{ &cg_dynamicCrosshair, "cg_dynamicCrosshair", "1", CVAR_ARCHIVE },
	//Enables ghoul2 traces for crosshair traces.. more precise when pointing at others, but slower.
	//And if the server doesn't have g2 col enabled, it won't match up the same.
	{ &cg_dynamicCrosshairPrecision, "cg_dynamicCrosshairPrecision", "1", CVAR_ARCHIVE },
	{ &cg_crosshairSize, "cg_crosshairSize", "24", CVAR_ARCHIVE },
	{ &cg_crosshairHealth, "cg_crosshairHealth", "0", CVAR_ARCHIVE },
	{ &cg_crosshairX, "cg_crosshairX", "0", CVAR_ARCHIVE },
	{ &cg_crosshairY, "cg_crosshairY", "0", CVAR_ARCHIVE },
	{ &cg_simpleItems, "cg_simpleItems", "0", CVAR_ARCHIVE },
	{ &cg_addMarks, "cg_marks", "1", CVAR_ARCHIVE },
	{ &cg_lagometer, "cg_lagometer", "0", CVAR_ARCHIVE },
	{ &cg_gun_frame, "cg_gun_frame", "0", CVAR_CHEAT },
	{ &cg_gun_x, "cg_gunX", "0", CVAR_CHEAT },
	{ &cg_gun_y, "cg_gunY", "0", CVAR_CHEAT },
	{ &cg_gun_z, "cg_gunZ", "0", CVAR_CHEAT },
	{ &cg_centertime, "cg_centertime", "3", CVAR_CHEAT },
	{ &cg_runpitch, "cg_runpitch", "0.002", CVAR_ARCHIVE},
	{ &cg_runroll, "cg_runroll", "0.005", CVAR_ARCHIVE },
	{ &cg_bobup , "cg_bobup", "0.005", CVAR_ARCHIVE },
	{ &cg_bobpitch, "cg_bobpitch", "0.002", CVAR_ARCHIVE },
	{ &cg_bobroll, "cg_bobroll", "0.002", CVAR_ARCHIVE },
	//{ &cg_swingSpeed, "cg_swingSpeed", "0.3", CVAR_CHEAT },
	{ &cg_animSpeed, "cg_animspeed", "1", CVAR_CHEAT },
	{ &cg_debugAnim, "cg_debuganim", "0", CVAR_CHEAT },
	{ &cg_debugSaber, "cg_debugsaber", "0", CVAR_CHEAT },
	{ &cg_debugEvents, "cg_debugevents", "0", CVAR_CHEAT },
	{ &cg_errorDecay, "cg_errordecay", "100", 0 },
	{ &cg_nopredict, "cg_nopredict", "0", 0 },
	{ &cg_noPlayerAnims, "cg_noplayeranims", "0", CVAR_CHEAT },
	{ &cg_showmiss, "cg_showmiss", "0", 0 },
	{ &cg_showVehMiss, "cg_showVehMiss", "0", 0 },
	{ &cg_footsteps, "cg_footsteps", "3", CVAR_ARCHIVE },
	{ &cg_swingAngles, "cg_swingAngles", "1", 0 },

	{ &cg_oldPainSounds, "cg_oldPainSounds", "0", 0 },

	{ &cg_ragDoll, "broadsword", "0", 0 },

	{ &cg_jumpSounds, "cg_jumpSounds", "0", 0 },

	{ &cg_autoMap, "r_autoMap", "0", CVAR_ARCHIVE },
	{ &cg_autoMapX, "r_autoMapX", "496", CVAR_ARCHIVE },
	{ &cg_autoMapY, "r_autoMapY", "32", CVAR_ARCHIVE },
	{ &cg_autoMapW, "r_autoMapW", "128", CVAR_ARCHIVE },
	{ &cg_autoMapH, "r_autoMapH", "128", CVAR_ARCHIVE },

#ifndef _XBOX	// lol fail
	{ &bg_fighterAltControl, "bg_fighterAltControl", "0", CVAR_SERVERINFO },
#endif

	// Enhanced Joystick controls <3
	{ &in_invertYLook, "in_invertYLook", "0", CVAR_ARCHIVE },
	{ &in_invertXLook, "in_invertXLook", "0", CVAR_ARCHIVE },
	{ &in_controlScheme, "in_controlScheme", "0", CVAR_ARCHIVE },
	{ &in_rumbleIntensity, "in_rumbleIntensity", "1.0", CVAR_ARCHIVE },

	{ &cg_chatBox, "cg_chatBox", "10000", CVAR_ARCHIVE },
	{ &cg_chatBoxHeight, "cg_chatBoxHeight", "350", CVAR_ARCHIVE },

	{ &cg_saberModelTraceEffect, "cg_saberModelTraceEffect", "0", 0 },

	//allows us to trace between server frames on the client to see if we're visually
	//hitting the last entity we detected a hit on from the server.
	{ &cg_saberClientVisualCompensation, "cg_saberClientVisualCompensation", "1", 0 },

	{ &cg_g2TraceLod, "cg_g2TraceLod", "2", 0 },

	{ &cg_fpls, "cg_fpls", "0", 0 },

	{ &cg_ghoul2Marks, "cg_ghoul2Marks", "16", 0 },

	{ &cg_optvehtrace, "com_optvehtrace", "0", 0 },

	{ &cg_saberDynamicMarks, "cg_saberDynamicMarks", "0", 0 },
	{ &cg_saberDynamicMarkTime, "cg_saberDynamicMarkTime", "60000", 0 },

	{ &cg_saberContact, "cg_saberContact", "1", 0 },
	{ &cg_saberTrail, "cg_saberTrail", "1", 0 },

	{ &cg_speedTrail, "cg_speedTrail", "1", 0 },
	{ &cg_auraShell, "cg_auraShell", "1", 0 },

	{ &cg_repeaterOrb, "cg_repeaterOrb", "0", 0 },

	{ &cg_animBlend, "cg_animBlend", "1", 0 },

	{ &cg_dismember, "cg_dismember", "0", CVAR_ARCHIVE },

	{ &cg_thirdPersonSpecialCam, "cg_thirdPersonSpecialCam", "0", 0 },

	//[TrueView]
	//True View Control cvars
	{ &cg_trueroll,	"cg_trueroll",	"0", CVAR_ARCHIVE },
	{ &cg_trueflip,	"cg_trueflip",	"0", CVAR_ARCHIVE },
	{ &cg_truespin,	"cg_truespin",	"0", CVAR_ARCHIVE },
	{ &cg_truemoveroll,	"cg_truemoveroll",	"0", CVAR_ARCHIVE },
	{ &cg_truesaberonly,	"cg_truesaberonly",	"0", CVAR_ARCHIVE },
	{ &cg_trueeyeposition,	"cg_trueeyeposition",	"0.0", 0},
	{ &cg_trueinvertsaber,	"cg_trueinvertsaber",	"1", CVAR_ARCHIVE},
	{ &cg_truefov,	"cg_truefov",	"80", CVAR_ARCHIVE},
	//[/TrueView]
	//[TrueView]
	{ &cg_trueguns, "cg_trueguns", "0", CVAR_ARCHIVE },
	//{ &cg_fpls, "cg_fpls", "0", 0 },
	//[/TrueView]


	{ &cg_thirdPerson, "cg_thirdPerson", "0", CVAR_ARCHIVE },
	{ &cg_thirdPersonRange, "cg_thirdPersonRange", "80", 0 },
	{ &cg_thirdPersonAngle, "cg_thirdPersonAngle", "0", CVAR_CHEAT },
	{ &cg_thirdPersonPitchOffset, "cg_thirdPersonPitchOffset", "0", 0 },
	{ &cg_thirdPersonVertOffset, "cg_thirdPersonVertOffset", "16", 0 },
	{ &cg_thirdPersonCameraDamp, "cg_thirdPersonCameraDamp", "0.3", 0 },
	{ &cg_thirdPersonTargetDamp, "cg_thirdPersonTargetDamp", "0.5", CVAR_CHEAT },
	
	{ &cg_thirdPersonHorzOffset, "cg_thirdPersonHorzOffset", "0", CVAR_CHEAT },
	{ &cg_thirdPersonAlpha,	"cg_thirdPersonAlpha",	"1.0", CVAR_CHEAT },

	{ &cg_forceModel, "cg_forceModel", "0", CVAR_ARCHIVE  },
	{ &cg_predictItems, "cg_predictItems", "1", CVAR_ARCHIVE },
	{ &cg_deferPlayers, "cg_deferPlayers", "1", CVAR_ARCHIVE },
	{ &cg_drawTeamOverlay, "cg_drawTeamOverlay", "0", CVAR_ARCHIVE },
	{ &cg_teamOverlayUserinfo, "teamoverlay", "0", CVAR_ROM | CVAR_USERINFO },
	{ &cg_stats, "cg_stats", "0", 0 },
	{ &cg_drawFriend, "cg_drawFriend", "1", CVAR_ARCHIVE },
	{ &cg_teamChatsOnly, "cg_teamChatsOnly", "0", CVAR_ARCHIVE },
	// the following variables are created in other parts of the system,
	// but we also reference them here
	{ &cg_buildScript, "com_buildScript", "0", 0 },	// force loading of all possible data amd error on failures
	{ &cg_paused, "cl_paused", "0", CVAR_ROM },
	{ &cg_synchronousClients, "g_synchronousClients", "0", 0 },	// communicated by systeminfo

//	{ &cg_redTeamName, "g_redteam", DEFAULT_REDTEAM_NAME, CVAR_ARCHIVE | CVAR_SERVERINFO | CVAR_USERINFO },
//	{ &cg_blueTeamName, "g_blueteam", DEFAULT_BLUETEAM_NAME, CVAR_ARCHIVE | CVAR_SERVERINFO | CVAR_USERINFO },
	{ &cg_currentSelectedPlayer, "cg_currentSelectedPlayer", "0", CVAR_ARCHIVE},
	{ &cg_currentSelectedPlayerName, "cg_currentSelectedPlayerName", "", CVAR_ARCHIVE},
//	{ &cg_singlePlayerActive, "ui_singlePlayerActive", "0", CVAR_USERINFO},

	{ &cg_cameraOrbit, "cg_cameraOrbit", "0", CVAR_CHEAT},
	{ &cg_cameraOrbitDelay, "cg_cameraOrbitDelay", "50", CVAR_ARCHIVE},
	{ &cg_timescaleFadeEnd, "cg_timescaleFadeEnd", "1", 0},
	{ &cg_timescaleFadeSpeed, "cg_timescaleFadeSpeed", "0", 0},
	{ &cg_timescale, "timescale", "1", 0},
	{ &cg_scorePlum, "cg_scorePlums", "1",  CVAR_ARCHIVE},
	{ &cg_hudFiles, "cg_hudFiles", "ui/jahud.txt", CVAR_ARCHIVE},
	// JKG
	{ &cg_drawHUD, "cg_drawhud", "1", CVAR_ARCHIVE },

	{ &cg_smoothClients, "cg_smoothClients", "1",  CVAR_ARCHIVE},
	{ &cg_cameraMode, "com_cameraMode", "0", CVAR_CHEAT},

	{ &pmove_fixed, "pmove_fixed", "0", 0},
	{ &pmove_msec, "pmove_msec", "8", 0},
	{ &cg_noTaunt, "cg_noTaunt", "0", CVAR_ARCHIVE},
	{ &cg_noProjectileTrail, "cg_noProjectileTrail", "0", CVAR_ARCHIVE},
//	{ &cg_trueLightning, "cg_trueLightning", "0.0", CVAR_ARCHIVE},
	{ &cg_showVehBounds, "cg_showVehBounds", "0", 0},

	{ &ui_myteam, "ui_myteam", "0", CVAR_ROM|CVAR_INTERNAL},
	{ &cg_snapshotTimeout, "cg_snapshotTimeout", "10", CVAR_ARCHIVE },

//	{ &cg_pmove_fixed, "cg_pmove_fixed", "0", CVAR_USERINFO | CVAR_ARCHIVE }
/*
Ghoul2 Insert Start
*/
	{ &cg_debugBB, "debugBB", "0", 0},
/*
Ghoul2 Insert End
*/
// Jedi Knight Galaxies
	{ &jkg_noletterbox, "jkg_noletterbox", "0", CVAR_ARCHIVE },

#ifdef __SWF__
	{ &jkg_swf, "jkg_swf", "0", CVAR_ARCHIVE },
#endif //__SWF__

	{ &jkg_gunlesscrosshair, "jkg_gunlesscrosshair", "0", CVAR_ARCHIVE },
	{ &jkg_nokillmessages, "jkg_nokillmessages", "1", CVAR_ARCHIVE },
	{ &jkg_smoothcamera, "jkg_smoothcamera", "1", CVAR_ARCHIVE },
	{ &r_bloom_threshold, "r_bloom_threshold", "0.8", CVAR_ARCHIVE },
	{ &r_bloom_factor, "r_bloom_factor", "0.5", CVAR_ARCHIVE },
	{ &r_force_pot_textures, "r_force_pot_textures", "0", CVAR_ARCHIVE },
	{ &r_force_arb_shaders, "r_force_arb_shaders", "0", CVAR_ARCHIVE },
	{ &jkg_autoreload, "jkg_autoreload", "1", CVAR_ARCHIVE },
	{ &jkg_debugBBox, "jkg_debugBBox", "0", CVAR_ARCHIVE | CVAR_CHEAT },

	{ &jkg_viewmodelPopup, "jkg_viewmodelPopup", "150", CVAR_ARCHIVE },
/*
	{ &jkg_debugSprintPitch, "jkg_debugSprintPitch", "0", CVAR_CHEAT },
	{ &jkg_debugSprintYaw, "jkg_debugSprintYaw", "0", CVAR_CHEAT },
	{ &jkg_debugSprintRoll, "jkg_debugSprintRoll", "0", CVAR_CHEAT },
	{ &jkg_debugSprintX, "jkg_debugSprintX", "0", CVAR_CHEAT },
	{ &jkg_debugSprintY, "jkg_debugSprintY", "0", CVAR_CHEAT },
	{ &jkg_debugSprintZ, "jkg_debugSprintZ", "0", CVAR_CHEAT },
	{ &jkg_debugSprintBobPitch, "jkg_debugSprintBobPitch", "0", CVAR_CHEAT },
	{ &jkg_debugSprintBobYaw, "jkg_debugSprintBobYaw", "0", CVAR_CHEAT },
	{ &jkg_debugSprintBobRoll, "jkg_debugSprintBobRoll", "0", CVAR_CHEAT },
	{ &jkg_debugSprintBobX, "jkg_debugSprintBobX", "0", CVAR_CHEAT },
	{ &jkg_debugSprintBobY, "jkg_debugSprintBobY", "0", CVAR_CHEAT },
	{ &jkg_debugSprintBobZ, "jkg_debugSprintBobZ", "0", CVAR_CHEAT },
	{ &jkg_debugSprintStyle, "jkg_debugSprintStyle", "-2", CVAR_CHEAT },
	{ &jkg_debugSprintBobSpeed, "jkg_debugSprintBobSpeed", "1.0", CVAR_CHEAT },
*/
	{ &jkg_simpleHUD, "jkg_simpleHUD", "1", CVAR_ARCHIVE },

	{ &jkg_meleeScroll, "jkg_meleeScroll", "1", CVAR_ARCHIVE },
	{ &jkg_sprintFOV, "jkg_sprintFOV", "100", CVAR_ARCHIVE },
	// Set by UI, but used in cgame
	{ &ui_hidehud,			"ui_hidehud",	"0", CVAR_INTERNAL},
	{ &ui_blurbackground,	"ui_blurbackground", "1", CVAR_ARCHIVE },
	{ &jkg_normalMapping, "jkg_normalMapping", "0", CVAR_ARCHIVE | CVAR_LATCH },
	{ &jkg_debugNavmesh, "jkg_debugNavmesh", "0", CVAR_ARCHIVE | CVAR_CHEAT },

#ifdef __AUTOWAYPOINT__
	{ &jkg_waypoint_render, "jkg_waypoint_render", "0", CVAR_ARCHIVE /*| CVAR_CHEAT*/ },
#endif //__AUTOWAYPOINT__

#ifdef __WEAPON_HOLSTER__
	{ &d_poff, "d_poff", "0", CVAR_ARCHIVE },
	{ &d_roff, "d_roff", "0", CVAR_ARCHIVE },
	{ &d_yoff, "d_yoff", "0", CVAR_ARCHIVE },
#endif //__WEAPON_HOLSTER__
};

static int  cvarTableSize = sizeof( cvarTable ) / sizeof( cvarTable[0] );

/*
=================
CG_RegisterCvars
=================
*/
void CG_RegisterCvars( void ) {
	int			i;
	cvarTable_t	*cv;
	char		var[MAX_TOKEN_CHARS];

	for ( i = 0, cv = cvarTable ; i < cvarTableSize ; i++, cv++ ) {
		cgi.Cvar_Register( cv->vmCvar, cv->cvarName,
			cv->defaultString, cv->cvarFlags );
	}

	// see if we are also running the server on this machine
	cgi.Cvar_VariableStringBuffer( "sv_running", var, sizeof( var ) );
	cgs.localServer = atoi( var );

	forceModelModificationCount = cg_forceModel.modificationCount;

	cgi.Cvar_Register(NULL, "model", DEFAULT_MODEL, CVAR_USERINFO | CVAR_ARCHIVE );
	cgi.Cvar_Register(NULL, "forcepowers", DEFAULT_FORCEPOWERS, CVAR_USERINFO | CVAR_ARCHIVE );
	cgi.Cvar_Register(NULL, "sex", DEFAULT_SEX, CVAR_USERINFO | CVAR_ARCHIVE );

	// Cvars uses for transferring data between client and server
	cgi.Cvar_Register(NULL, "ui_about_gametype",		"0", CVAR_ROM|CVAR_INTERNAL );
	cgi.Cvar_Register(NULL, "ui_about_fraglimit",		"0", CVAR_ROM|CVAR_INTERNAL );
	cgi.Cvar_Register(NULL, "ui_about_capturelimit",	"0", CVAR_ROM|CVAR_INTERNAL );
	cgi.Cvar_Register(NULL, "ui_about_duellimit",		"0", CVAR_ROM|CVAR_INTERNAL );
	cgi.Cvar_Register(NULL, "ui_about_timelimit",		"0", CVAR_ROM|CVAR_INTERNAL );
	cgi.Cvar_Register(NULL, "ui_about_maxclients",		"0", CVAR_ROM|CVAR_INTERNAL );
	cgi.Cvar_Register(NULL, "ui_about_dmflags",		"0", CVAR_ROM|CVAR_INTERNAL );
	cgi.Cvar_Register(NULL, "ui_about_mapname",		"0", CVAR_ROM|CVAR_INTERNAL );
	cgi.Cvar_Register(NULL, "ui_about_hostname",		"0", CVAR_ROM|CVAR_INTERNAL );
	cgi.Cvar_Register(NULL, "ui_about_needpass",		"0", CVAR_ROM|CVAR_INTERNAL );
	cgi.Cvar_Register(NULL, "ui_about_botminplayers",	"0", CVAR_ROM|CVAR_INTERNAL );

	cgi.Cvar_Register(NULL, "ui_tm1_cnt", "0", CVAR_ROM | CVAR_INTERNAL );
	cgi.Cvar_Register(NULL, "ui_tm2_cnt", "0", CVAR_ROM | CVAR_INTERNAL );
	cgi.Cvar_Register(NULL, "ui_tm3_cnt", "0", CVAR_ROM | CVAR_INTERNAL );

	cgi.Cvar_Register(NULL, "ui_tm1_c0_cnt", "0", CVAR_ROM | CVAR_INTERNAL );
	cgi.Cvar_Register(NULL, "ui_tm1_c1_cnt", "0", CVAR_ROM | CVAR_INTERNAL );
	cgi.Cvar_Register(NULL, "ui_tm1_c2_cnt", "0", CVAR_ROM | CVAR_INTERNAL );
	cgi.Cvar_Register(NULL, "ui_tm1_c3_cnt", "0", CVAR_ROM | CVAR_INTERNAL );
	cgi.Cvar_Register(NULL, "ui_tm1_c4_cnt", "0", CVAR_ROM | CVAR_INTERNAL );
	cgi.Cvar_Register(NULL, "ui_tm1_c5_cnt", "0", CVAR_ROM | CVAR_INTERNAL );

	cgi.Cvar_Register(NULL, "ui_tm2_c0_cnt", "0", CVAR_ROM | CVAR_INTERNAL );
	cgi.Cvar_Register(NULL, "ui_tm2_c1_cnt", "0", CVAR_ROM | CVAR_INTERNAL );
	cgi.Cvar_Register(NULL, "ui_tm2_c2_cnt", "0", CVAR_ROM | CVAR_INTERNAL );
	cgi.Cvar_Register(NULL, "ui_tm2_c3_cnt", "0", CVAR_ROM | CVAR_INTERNAL );
	cgi.Cvar_Register(NULL, "ui_tm2_c4_cnt", "0", CVAR_ROM | CVAR_INTERNAL );
	cgi.Cvar_Register(NULL, "ui_tm2_c5_cnt", "0", CVAR_ROM | CVAR_INTERNAL );
}

/*																																			
===================
CG_SetWeatherOverride
===================
*/
#if 0
void CG_SetWeatherOverride(int contents)
{
	if (contents != cgWeatherOverride)
	{ //only do the trap call if we aren't already set to this
		cgi.R_WeatherContentsOverride(contents);
	}
	cgWeatherOverride = contents; //keep track of it
}
#endif

/*																																			
===================
CG_ForceModelChange
===================
*/
static void CG_ForceModelChange( void ) {
	int		i;

	for (i=0 ; i<MAX_CLIENTS ; i++) {
		const char		*clientInfo;
		void	*oldGhoul2;

		clientInfo = CG_ConfigString( CS_PLAYERS+i );
		if ( !clientInfo[0] ) {
			continue;
		}

		oldGhoul2 = cgs.clientinfo[i].ghoul2Model;
		CG_NewClientInfo( i, qtrue);
	}
}

/*
=================
CG_UpdateCvars
=================
*/
void CG_UpdateCvars( void ) {
	int			i;
	cvarTable_t	*cv;
	static int drawTeamOverlayModificationCount = -1;

	for ( i = 0, cv = cvarTable ; i < cvarTableSize ; i++, cv++ ) {
		cgi.Cvar_Update( cv->vmCvar );
	}

	// check for modications here

	// If team overlay is on, ask for updates from the server.  If its off,
	// let the server know so we don't receive it
	if ( drawTeamOverlayModificationCount != cg_drawTeamOverlay.modificationCount ) {
		drawTeamOverlayModificationCount = cg_drawTeamOverlay.modificationCount;

		if ( cg_drawTeamOverlay.integer > 0 ) {
			cgi.Cvar_Set( "teamoverlay", "1" );
		} else {
			cgi.Cvar_Set( "teamoverlay", "0" );
		}
		// FIXME E3 HACK
		cgi.Cvar_Set( "teamoverlay", "1" );
	}

	// if force model changed
	if ( forceModelModificationCount != cg_forceModel.modificationCount ) {
		forceModelModificationCount = cg_forceModel.modificationCount;
		CG_ForceModelChange();
	}

	// Stuff hacked in pre-Phase 1 from GSA that allows us to have our weapons drawn in view properly
	{
		char buffer[16];
		cgi.Cvar_VariableStringBuffer("r_znear", buffer, sizeof(buffer));
		if(atoi(buffer) != 2)
		{
			cgi.Cvar_Set( "r_znear", "2" );
		}
	}
}

int CG_CrosshairPlayer( void ) {
	if ( cg.time > ( cg.crosshairClientTime + 1000 ) ) {
		return -1;
	}

	if (cg.crosshairClientNum >= MAX_CLIENTS)
	{
		return -1;
	}

	return cg.crosshairClientNum;
}

int CG_LastAttacker( void ) {
	if ( !cg.attackerTime ) {
		return -1;
	}
	return cg.snap->ps.persistant[PERS_ATTACKER];
}

void QDECL CG_Printf( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	Q_vsnprintf (text, 1024, msg, argptr);
	va_end (argptr);

	cgi.Print( text );
}

void QDECL CG_Error( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	Q_vsnprintf (text, 1024, msg, argptr);
	va_end (argptr);

	cgi.Error( text );
}

#ifndef CGAME_HARD_LINKED
// this is only here so the functions in q_shared.c and bg_*.c can link (FIXME)

void QDECL Com_Error( int level, const char *error, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, error);
	Q_vsnprintf (text, 1024, error, argptr);
	va_end (argptr);

	CG_Error( "%s", text);
}

void QDECL Com_Printf( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	Q_vsnprintf (text, 1024, msg, argptr);
	va_end (argptr);

	CG_Printf ("%s", text);
}

#endif

/*
================
CG_Argv
================
*/
const char *CG_Argv( int arg ) {
	static char	buffer[MAX_STRING_CHARS];

	cgi.Argv( arg, buffer, sizeof( buffer ) );

	return buffer;
}


//========================================================================

//so shared code can get the local time depending on the side it's executed on
int BG_GetTime(void)
{
	return cg.time;
}

/*
=================
CG_RegisterItemSounds

The server says this item is used on this level
=================
*/
static void CG_RegisterItemSounds( int itemNum ) {
	gitem_t			*item;
	char			data[MAX_QPATH];
	char			*s, *start;
	int				len;

	item = &bg_itemlist[ itemNum ];

	if( item->pickup_sound ) {
		cgi.S_RegisterSound( item->pickup_sound );
	}

	// parse the space seperated precache string for other media
	s = item->sounds;
	if (!s || !s[0])
		return;

	while (*s) {
		start = s;
		while (*s && *s != ' ') {
			s++;
		}

		len = s-start;
		if (len >= MAX_QPATH || len < 5) {
			CG_Error( "PrecacheItem: %s has bad precache string", 
				item->classname);
			return;
		}
		memcpy (data, start, len);
		data[len] = 0;
		if ( *s ) {
			s++;
		}

		cgi.S_RegisterSound( data );
	}

	// parse the space seperated precache string for other media
	s = item->precaches;
	if (!s || !s[0])
		return;

	while (*s) {
		start = s;
		while (*s && *s != ' ') {
			s++;
		}

		len = s-start;
		if (len >= MAX_QPATH || len < 5) {
			CG_Error( "PrecacheItem: %s has bad precache string", 
				item->classname);
			return;
		}
		memcpy (data, start, len);
		data[len] = 0;
		if ( *s ) {
			s++;
		}

		if ( !strcmp(data+len-3, "efx" )) {
			cgi.FX_RegisterEffect( data );
		}
	}
}

static void CG_AS_Register(void)
{
	const char *soundName;
	int i;

//	CG_LoadingString( "ambient sound sets" );

	//Load the ambient sets
#if 0 //as_preCacheMap was game-side.. that is evil.
	cgi.AS_AddPrecacheEntry( "#clear" );	// ;-)
	//FIXME: Don't ask... I had to get around a really nasty MS error in the templates with this...
	namePrecache_m::iterator	pi;
	STL_ITERATE( pi, as_preCacheMap )
	{
		cgi_AS_AddPrecacheEntry( ((*pi).first).c_str() );
	}
#else
	cgi.AS_AddPrecacheEntry( "#clear" );

	for ( i = 1 ; i < MAX_AMBIENT_SETS ; i++ ) {
		soundName = CG_ConfigString( CS_AMBIENT_SET+i );
		if ( !soundName || !soundName[0] )
		{
			break;
		}

		cgi.AS_AddPrecacheEntry(soundName);
	}
	soundName = CG_ConfigString( CS_GLOBAL_AMBIENT_SET );
	if (soundName && soundName[0] && Q_stricmp(soundName, "default"))
	{ //global soundset
		cgi.AS_AddPrecacheEntry(soundName);
	}
#endif

	cgi.AS_ParseSets();
}

//a global weather effect (rain, snow, etc)
void CG_ParseWeatherEffect(const char *str)
{
	char *sptr = (char *)str;
	sptr++; //pass the '*'
	cgi.R_WorldEffectCommand(sptr);
}

/*
=================
CG_RegisterSounds

called during a precache command
=================
*/
void CG_PrecacheNPCSounds(const char *str);
void CG_ParseSiegeObjectiveStatus(const char *str);
extern int cg_beatingSiegeTime;
extern int cg_siegeWinTeam;
static void CG_RegisterSounds( void ) {
	int		i;
	char	items[MAX_ITEMS+1];
	char	name[MAX_QPATH];
	const char	*soundName;

	CG_AS_Register();

	//CG_LoadingString( "Sounds" );

	cgi.S_RegisterSound( "sound/weapons/melee/punch1.mp3" );
	cgi.S_RegisterSound( "sound/weapons/melee/punch2.mp3" );
	cgi.S_RegisterSound( "sound/weapons/melee/punch3.mp3" );
	cgi.S_RegisterSound( "sound/weapons/melee/punch4.mp3" );
	cgi.S_RegisterSound("sound/movers/objects/saber_slam");

	cgi.S_RegisterSound("sound/player/bodyfall_human1.wav");
	cgi.S_RegisterSound("sound/player/bodyfall_human2.wav");
	cgi.S_RegisterSound("sound/player/bodyfall_human3.wav");

	//test effects
	cgi.FX_RegisterEffect("effects/mp/test_sparks.efx");
	cgi.FX_RegisterEffect("effects/mp/test_wall_impact.efx");

	cgs.media.oneMinuteSound = cgi.S_RegisterSound( "sound/chars/protocol/misc/40MOM004" );
	cgs.media.fiveMinuteSound = cgi.S_RegisterSound( "sound/chars/protocol/misc/40MOM005" );
	cgs.media.oneFragSound = cgi.S_RegisterSound( "sound/chars/protocol/misc/40MOM001" );
	cgs.media.twoFragSound = cgi.S_RegisterSound( "sound/chars/protocol/misc/40MOM002" );
	cgs.media.threeFragSound = cgi.S_RegisterSound( "sound/chars/protocol/misc/40MOM003");
	cgs.media.count3Sound = cgi.S_RegisterSound( "sound/chars/protocol/misc/40MOM035" );
	cgs.media.count2Sound = cgi.S_RegisterSound( "sound/chars/protocol/misc/40MOM036" );
	cgs.media.count1Sound = cgi.S_RegisterSound( "sound/chars/protocol/misc/40MOM037" );
	cgs.media.countFightSound = cgi.S_RegisterSound( "sound/chars/protocol/misc/40MOM038" );

	cgs.media.hackerIconShader			= cgi.R_RegisterShaderNoMip("gfx/mp/c_icon_tech");

	cgs.media.redSaberGlowShader		= cgi.R_RegisterShader( "gfx/effects/sabers/red_glow" );
	cgs.media.redSaberCoreShader		= cgi.R_RegisterShader( "gfx/effects/sabers/red_line" );
	cgs.media.orangeSaberGlowShader		= cgi.R_RegisterShader( "gfx/effects/sabers/orange_glow" );
	cgs.media.orangeSaberCoreShader		= cgi.R_RegisterShader( "gfx/effects/sabers/orange_line" );
	cgs.media.yellowSaberGlowShader		= cgi.R_RegisterShader( "gfx/effects/sabers/yellow_glow" );
	cgs.media.yellowSaberCoreShader		= cgi.R_RegisterShader( "gfx/effects/sabers/yellow_line" );
	cgs.media.greenSaberGlowShader		= cgi.R_RegisterShader( "gfx/effects/sabers/green_glow" );
	cgs.media.greenSaberCoreShader		= cgi.R_RegisterShader( "gfx/effects/sabers/green_line" );
	cgs.media.blueSaberGlowShader		= cgi.R_RegisterShader( "gfx/effects/sabers/blue_glow" );
	cgs.media.blueSaberCoreShader		= cgi.R_RegisterShader( "gfx/effects/sabers/blue_line" );
	cgs.media.purpleSaberGlowShader		= cgi.R_RegisterShader( "gfx/effects/sabers/purple_glow" );
	cgs.media.purpleSaberCoreShader		= cgi.R_RegisterShader( "gfx/effects/sabers/purple_line" );
	cgs.media.saberBlurShader			= cgi.R_RegisterShader( "gfx/effects/sabers/saberBlur" );
	cgs.media.swordTrailShader			= cgi.R_RegisterShader( "gfx/effects/sabers/swordTrail" );

	cgs.media.forceCoronaShader			= cgi.R_RegisterShaderNoMip( "gfx/hud/force_swirl" );

	cgs.media.yellowDroppedSaberShader	= cgi.R_RegisterShader("gfx/effects/yellow_glow");

	cgs.media.rivetMarkShader			= cgi.R_RegisterShader( "gfx/damage/rivetmark" );

	cgi.R_RegisterShader( "gfx/effects/saberFlare" );

	cgi.R_RegisterShader( "powerups/ysalimarishell" );
	
	cgi.R_RegisterShader( "gfx/effects/forcePush" );

	cgi.R_RegisterShader( "gfx/misc/red_dmgshield" );
	cgi.R_RegisterShader( "gfx/misc/red_portashield" );
	cgi.R_RegisterShader( "gfx/misc/blue_dmgshield" );
	cgi.R_RegisterShader( "gfx/misc/blue_portashield" );

	cgi.R_RegisterShader( "models/map_objects/imp_mine/turret_chair_dmg.tga" );

	for (i=1 ; i<9 ; i++)
	{
		cgi.S_RegisterSound(va("sound/weapons/saber/saberhup%i.wav", i));
	}

	for (i=1 ; i<10 ; i++)
	{
		cgi.S_RegisterSound(va("sound/weapons/saber/saberblock%i.wav", i));
	}

	for (i=1 ; i<4 ; i++)
	{
		cgi.S_RegisterSound(va("sound/weapons/saber/bounce%i.wav", i));
	}

	cgi.S_RegisterSound( "sound/weapons/saber/enemy_saber_on.wav" );
	cgi.S_RegisterSound( "sound/weapons/saber/enemy_saber_off.wav" );

	cgi.S_RegisterSound( "sound/weapons/saber/saberhum1.wav" );
	cgi.S_RegisterSound( "sound/weapons/saber/saberon.wav" );
	cgi.S_RegisterSound( "sound/weapons/saber/saberoffquick.wav" );
	cgi.S_RegisterSound( "sound/weapons/saber/saberhitwall1" );
	cgi.S_RegisterSound( "sound/weapons/saber/saberhitwall2" );
	cgi.S_RegisterSound( "sound/weapons/saber/saberhitwall3" );
	cgi.S_RegisterSound("sound/weapons/saber/saberhit.wav");
	cgi.S_RegisterSound("sound/weapons/saber/saberhit1.wav");
	cgi.S_RegisterSound("sound/weapons/saber/saberhit2.wav");
	cgi.S_RegisterSound("sound/weapons/saber/saberhit3.wav");

	cgi.S_RegisterSound("sound/weapons/saber/saber_catch.wav");

	cgs.media.teamHealSound = cgi.S_RegisterSound("sound/weapons/force/teamheal.wav");
	cgs.media.teamRegenSound = cgi.S_RegisterSound("sound/weapons/force/teamforce.wav");

	cgi.S_RegisterSound("sound/weapons/force/heal.wav");
	cgi.S_RegisterSound("sound/weapons/force/speed.wav");
	cgi.S_RegisterSound("sound/weapons/force/see.wav");
	cgi.S_RegisterSound("sound/weapons/force/rage.wav");
	cgi.S_RegisterSound("sound/weapons/force/lightning");
	cgi.S_RegisterSound("sound/weapons/force/lightninghit1");
	cgi.S_RegisterSound("sound/weapons/force/lightninghit2");
	cgi.S_RegisterSound("sound/weapons/force/lightninghit3");
	cgi.S_RegisterSound("sound/weapons/force/drain.wav");
	cgi.S_RegisterSound("sound/weapons/force/jumpbuild.wav");
	cgi.S_RegisterSound("sound/weapons/force/distract.wav");
	cgi.S_RegisterSound("sound/weapons/force/distractstop.wav");
	cgi.S_RegisterSound("sound/weapons/force/pull.wav");
	cgi.S_RegisterSound("sound/weapons/force/push.wav");

	for (i=1 ; i<3 ; i++)
	{
		cgi.S_RegisterSound(va("sound/weapons/thermal/bounce%i.wav", i));
	}

	cgi.S_RegisterSound("sound/movers/switches/switch2.wav");
	cgi.S_RegisterSound("sound/movers/switches/switch3.wav");
	cgi.S_RegisterSound("sound/ambience/spark5.wav");
	cgi.S_RegisterSound("sound/chars/turret/ping.wav");
	cgi.S_RegisterSound("sound/chars/turret/startup.wav");
	cgi.S_RegisterSound("sound/chars/turret/shutdown.wav");
	cgi.S_RegisterSound("sound/chars/turret/move.wav");
	cgi.S_RegisterSound("sound/player/pickuphealth.wav");
	cgi.S_RegisterSound("sound/player/pickupshield.wav");

	cgi.S_RegisterSound("sound/effects/glassbreak1.wav");

	cgi.S_RegisterSound( "sound/weapons/rocket/tick.wav" );
	cgi.S_RegisterSound( "sound/weapons/rocket/lock.wav" );

	cgi.S_RegisterSound("sound/weapons/force/speedloop.wav");

	cgi.S_RegisterSound("sound/weapons/force/protecthit.mp3"); //PDSOUND_PROTECTHIT
	cgi.S_RegisterSound("sound/weapons/force/protect.mp3"); //PDSOUND_PROTECT
	cgi.S_RegisterSound("sound/weapons/force/absorbhit.mp3"); //PDSOUND_ABSORBHIT
	cgi.S_RegisterSound("sound/weapons/force/absorb.mp3"); //PDSOUND_ABSORB
	cgi.S_RegisterSound("sound/weapons/force/jump.mp3"); //PDSOUND_FORCEJUMP
	cgi.S_RegisterSound("sound/weapons/force/grip.mp3"); //PDSOUND_FORCEGRIP

	if ( cgs.gametype >= GT_TEAM || cg_buildScript.integer ) {

#ifdef JK2AWARDS
		cgs.media.captureAwardSound = cgi.S_RegisterSound( "sound/teamplay/flagcapture_yourteam.wav" );
#endif
		cgs.media.redLeadsSound = cgi.S_RegisterSound( "sound/chars/protocol/misc/40MOM046");
		cgs.media.blueLeadsSound = cgi.S_RegisterSound( "sound/chars/protocol/misc/40MOM045");
		cgs.media.teamsTiedSound = cgi.S_RegisterSound( "sound/chars/protocol/misc/40MOM032" );

		cgs.media.redScoredSound = cgi.S_RegisterSound( "sound/chars/protocol/misc/40MOM044");
		cgs.media.blueScoredSound = cgi.S_RegisterSound( "sound/chars/protocol/misc/40MOM043" );

		if ( cgs.gametype == GT_CTF || cg_buildScript.integer ) {
			cgs.media.redFlagReturnedSound = cgi.S_RegisterSound( "sound/chars/protocol/misc/40MOM042" );
			cgs.media.blueFlagReturnedSound = cgi.S_RegisterSound( "sound/chars/protocol/misc/40MOM041" );
			cgs.media.redTookFlagSound = cgi.S_RegisterSound( "sound/chars/protocol/misc/40MOM040" );
			cgs.media.blueTookFlagSound = cgi.S_RegisterSound( "sound/chars/protocol/misc/40MOM039" );
		}
		if ( cgs.gametype == GT_CTY /*|| cg_buildScript.integer*/ ) {
			cgs.media.redYsalReturnedSound = cgi.S_RegisterSound( "sound/chars/protocol/misc/40MOM050" );
			cgs.media.blueYsalReturnedSound = cgi.S_RegisterSound( "sound/chars/protocol/misc/40MOM049" );
			cgs.media.redTookYsalSound = cgi.S_RegisterSound( "sound/chars/protocol/misc/40MOM048" );
			cgs.media.blueTookYsalSound = cgi.S_RegisterSound( "sound/chars/protocol/misc/40MOM047" );
		}
	}

	cgs.media.drainSound = cgi.S_RegisterSound("sound/weapons/force/drained.mp3");

	cgs.media.happyMusic = cgi.S_RegisterSound("music/goodsmall.mp3");
	cgs.media.dramaticFailure = cgi.S_RegisterSound("music/badsmall.mp3");

	//PRECACHE ALL MUSIC HERE (don't need to precache normally because it's streamed off the disk)
	if (cg_buildScript.integer)
	{
		cgi.S_StartBackgroundTrack( "music/mp/duel.mp3", "music/mp/duel.mp3", qfalse );
	}

	cg.loadLCARSStage = 1;

	cgs.media.selectSound = cgi.S_RegisterSound( "sound/weapons/change.wav" );

	cgs.media.teleInSound = cgi.S_RegisterSound( "sound/player/telein.wav" );
	cgs.media.teleOutSound = cgi.S_RegisterSound( "sound/player/teleout.wav" );
	cgs.media.respawnSound = cgi.S_RegisterSound( "sound/items/respawn1.wav" );

	cgi.S_RegisterSound( "sound/movers/objects/objectHit.wav" );

	cgs.media.talkSound = cgi.S_RegisterSound( "sound/player/talk.wav" );
	cgs.media.landSound = cgi.S_RegisterSound( "sound/player/land1.wav");
	cgs.media.fallSound = cgi.S_RegisterSound( "sound/player/fallsplat.wav");

	cgs.media.crackleSound = cgi.S_RegisterSound( "sound/effects/energy_crackle.wav" );
#ifdef JK2AWARDS
	cgs.media.impressiveSound = cgi.S_RegisterSound( "sound/chars/protocol/misc/40MOM025" );
	cgs.media.excellentSound = cgi.S_RegisterSound( "sound/chars/protocol/misc/40MOM053" );
	cgs.media.deniedSound = cgi.S_RegisterSound( "sound/chars/protocol/misc/40MOM017" );
	cgs.media.humiliationSound = cgi.S_RegisterSound( "sound/chars/protocol/misc/40MOM019" );
	cgs.media.defendSound = cgi.S_RegisterSound( "sound/chars/protocol/misc/40MOM024" );
#endif

	/*
	cgs.media.takenLeadSound = cgi.S_RegisterSound( "sound/chars/protocol/misc/40MOM051");
	cgs.media.tiedLeadSound = cgi.S_RegisterSound( "sound/chars/protocol/misc/40MOM032");
	cgs.media.lostLeadSound = cgi.S_RegisterSound( "sound/chars/protocol/misc/40MOM052");
	*/

	cgs.media.rollSound					= cgi.S_RegisterSound( "sound/player/roll1.wav");

	cgs.media.noforceSound				= cgi.S_RegisterSound( "sound/weapons/force/noforce" );

	cgs.media.watrInSound				= cgi.S_RegisterSound( "sound/player/watr_in.wav");
	cgs.media.watrOutSound				= cgi.S_RegisterSound( "sound/player/watr_out.wav");
	cgs.media.watrUnSound				= cgi.S_RegisterSound( "sound/player/watr_un.wav");

	cgs.media.explosionModel			= cgi.R_RegisterModel ( "models/map_objects/mp/sphere.md3" );
	cgs.media.surfaceExplosionShader	= cgi.R_RegisterShader( "surfaceExplosion" );

	cgs.media.disruptorShader			= cgi.R_RegisterShader( "gfx/effects/burn");

	if (cg_buildScript.integer)
	{
		cgi.R_RegisterShader( "gfx/effects/turretflashdie" );
	}

	cgs.media.solidWhite = cgi.R_RegisterShader( "gfx/effects/solidWhite_cull" );

	cgi.R_RegisterShader("gfx/misc/mp_light_enlight_disable");
	cgi.R_RegisterShader("gfx/misc/mp_dark_enlight_disable");

	cgi.R_RegisterModel ( "models/map_objects/mp/sphere.md3" );
	cgi.R_RegisterModel("models/items/remote.md3");

	cgs.media.holocronPickup = cgi.S_RegisterSound( "sound/player/holocron.wav" );

	// No ammo sound
	cgs.media.noAmmoSound = cgi.S_RegisterSound( "sound/weapons/noammo.wav" );

	// Zoom
	cgs.media.zoomStart = cgi.S_RegisterSound( "sound/interface/zoomstart.wav" );
	cgs.media.zoomLoop	= cgi.S_RegisterSound( "sound/interface/zoomloop.wav" );
	cgs.media.zoomEnd	= cgi.S_RegisterSound( "sound/interface/zoomend.wav" );

	for (i=0 ; i<4 ; i++) {
		Com_sprintf (name, sizeof(name), "sound/player/footsteps/stone_step%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_STONEWALK][i] = cgi.S_RegisterSound (name);
		Com_sprintf (name, sizeof(name), "sound/player/footsteps/stone_run%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_STONERUN][i] = cgi.S_RegisterSound (name);

		Com_sprintf (name, sizeof(name), "sound/player/footsteps/metal_step%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_METALWALK][i] = cgi.S_RegisterSound (name);
		Com_sprintf (name, sizeof(name), "sound/player/footsteps/metal_run%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_METALRUN][i] = cgi.S_RegisterSound (name);

		Com_sprintf (name, sizeof(name), "sound/player/footsteps/pipe_step%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_PIPEWALK][i] = cgi.S_RegisterSound (name);
		Com_sprintf (name, sizeof(name), "sound/player/footsteps/pipe_run%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_PIPERUN][i] = cgi.S_RegisterSound (name);

		Com_sprintf (name, sizeof(name), "sound/player/footsteps/water_run%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_SPLASH][i] = cgi.S_RegisterSound (name);

		Com_sprintf (name, sizeof(name), "sound/player/footsteps/water_walk%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_WADE][i] = cgi.S_RegisterSound (name);

		Com_sprintf (name, sizeof(name), "sound/player/footsteps/water_wade_0%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_SWIM][i] = cgi.S_RegisterSound (name);

		Com_sprintf (name, sizeof(name), "sound/player/footsteps/snow_step%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_SNOWWALK][i] = cgi.S_RegisterSound (name);
		Com_sprintf (name, sizeof(name), "sound/player/footsteps/snow_run%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_SNOWRUN][i] = cgi.S_RegisterSound (name);

		Com_sprintf (name, sizeof(name), "sound/player/footsteps/sand_walk%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_SANDWALK][i] = cgi.S_RegisterSound (name);
		Com_sprintf (name, sizeof(name), "sound/player/footsteps/sand_run%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_SANDRUN][i] = cgi.S_RegisterSound (name);

		Com_sprintf (name, sizeof(name), "sound/player/footsteps/grass_step%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_GRASSWALK][i] = cgi.S_RegisterSound (name);
		Com_sprintf (name, sizeof(name), "sound/player/footsteps/grass_run%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_GRASSRUN][i] = cgi.S_RegisterSound (name);

		Com_sprintf (name, sizeof(name), "sound/player/footsteps/dirt_step%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_DIRTWALK][i] = cgi.S_RegisterSound (name);
		Com_sprintf (name, sizeof(name), "sound/player/footsteps/dirt_run%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_DIRTRUN][i] = cgi.S_RegisterSound (name);

		Com_sprintf (name, sizeof(name), "sound/player/footsteps/mud_walk%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_MUDWALK][i] = cgi.S_RegisterSound (name);
		Com_sprintf (name, sizeof(name), "sound/player/footsteps/mud_run%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_MUDRUN][i] = cgi.S_RegisterSound (name);

		Com_sprintf (name, sizeof(name), "sound/player/footsteps/gravel_walk%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_GRAVELWALK][i] = cgi.S_RegisterSound (name);
		Com_sprintf (name, sizeof(name), "sound/player/footsteps/gravel_run%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_GRAVELRUN][i] = cgi.S_RegisterSound (name);

		Com_sprintf (name, sizeof(name), "sound/player/footsteps/rug_step%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_RUGWALK][i] = cgi.S_RegisterSound (name);
		Com_sprintf (name, sizeof(name), "sound/player/footsteps/rug_run%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_RUGRUN][i] = cgi.S_RegisterSound (name);

		Com_sprintf (name, sizeof(name), "sound/player/footsteps/wood_walk%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_WOODWALK][i] = cgi.S_RegisterSound (name);
		Com_sprintf (name, sizeof(name), "sound/player/footsteps/wood_run%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_WOODRUN][i] = cgi.S_RegisterSound (name);
	}

	// only register the items that the server says we need
	strcpy( items, CG_ConfigString( CS_ITEMS ) );

	for ( i = 1 ; i < bg_numItems ; i++ ) {
		if ( items[ i ] == '1' || cg_buildScript.integer ) {
			CG_RegisterItemSounds( i );
		}
	}

	for ( i = 1 ; i < MAX_SOUNDS ; i++ ) {
		soundName = CG_ConfigString( CS_SOUNDS+i );
		if ( !soundName[0] ) {
			break;
		}
		if ( soundName[0] == '*' )
		{
			if (soundName[1] == '$')
			{ //an NPC soundset
				CG_PrecacheNPCSounds(soundName);
			}
			continue;	// custom sound
		}
		cgs.gameSounds[i] = cgi.S_RegisterSound( soundName );
	}

	for ( i = 1 ; i < MAX_FX ; i++ ) {
		soundName = CG_ConfigString( CS_EFFECTS+i );
		if ( !soundName[0] ) {
			break;
		}

		if (soundName[0] == '*')
		{ //it's a special global weather effect
			CG_ParseWeatherEffect(soundName);
			cgs.gameEffects[i] = 0;
		}
		else
		{
			cgs.gameEffects[i] = cgi.FX_RegisterEffect( soundName );
		}
	}

	// register all the server specified icons
	for ( i = 1; i < MAX_ICONS; i ++ )
	{
		const char* iconName;

		iconName = CG_ConfigString ( CS_ICONS + i );
		if ( !iconName[0] )
		{
			break;
		}

		cgs.gameIcons[i] = cgi.R_RegisterShaderNoMip ( iconName );
	}

	cg.loadLCARSStage = 2;

	// FIXME: only needed with item
	cgs.media.deploySeeker = cgi.S_RegisterSound ("sound/chars/seeker/misc/hiss");
	cgs.media.medkitSound = cgi.S_RegisterSound ("sound/items/use_bacta.wav");
	
	cgs.media.winnerSound = cgi.S_RegisterSound( "sound/chars/protocol/misc/40MOM006" );
	cgs.media.loserSound = cgi.S_RegisterSound( "sound/chars/protocol/misc/40MOM010" );

	// eezstreet / JKG add: weapon/armor breaking
	cgs.media.armorBreakSound = cgi.S_RegisterSound( "sound/items/armor_break.wav" );
	cgs.media.weaponBreakSound = cgi.S_RegisterSound( "sound/items/weapon_break.wav" );
}


//-------------------------------------
// CG_RegisterEffects
// 
// Handles precaching all effect files
//	and any shader, model, or sound
//	files an effect may use.
//-------------------------------------
static void CG_RegisterEffects( void )
{
	/*
	const char	*effectName;
	int			i;

	for ( i = 1 ; i < MAX_FX ; i++ ) 
	{
		effectName = CG_ConfigString( CS_EFFECTS + i );

		if ( !effectName[0] ) 
		{
			break;
		}

		cgi.FX_RegisterEffect( effectName );
	}
	*/
	//the above was redundant as it's being done in CG_RegisterSounds

	// Set up the glass effects mini-system.
	CG_InitGlass();

	//footstep effects
	cgs.effects.footstepMud = cgi.FX_RegisterEffect( "materials/mud" );
	cgs.effects.footstepSand = cgi.FX_RegisterEffect( "materials/sand" );
	cgs.effects.footstepSnow = cgi.FX_RegisterEffect( "materials/snow" );
	cgs.effects.footstepGravel = cgi.FX_RegisterEffect( "materials/gravel" );
	//landing effects
	cgs.effects.landingMud = cgi.FX_RegisterEffect( "materials/mud_large" );
	cgs.effects.landingSand = cgi.FX_RegisterEffect( "materials/sand_large" );
	cgs.effects.landingDirt = cgi.FX_RegisterEffect( "materials/dirt_large" );
	cgs.effects.landingSnow = cgi.FX_RegisterEffect( "materials/snow_large" );
	cgs.effects.landingGravel = cgi.FX_RegisterEffect( "materials/gravel_large" );
	//splashes
	cgs.effects.waterSplash = cgi.FX_RegisterEffect( "env/water_impact" );
	cgs.effects.lavaSplash = cgi.FX_RegisterEffect( "env/lava_splash" );
	cgs.effects.acidSplash = cgi.FX_RegisterEffect( "env/acid_splash" );
}

//===================================================================================

extern char *forceHolocronModels[];
int CG_HandleAppendedSkin(char *modelName);
void CG_CacheG2AnimInfo(char *modelName);
/*
=================
CG_RegisterGraphics

This function may execute for a couple of minutes with a slow disk.
=================
*/
static void CG_RegisterGraphics( void ) {
	int			i;
	int			breakPoint;
	char		items[MAX_ITEMS+1];
	const char	*terrainInfo;
	int			terrainID;

	static char		*sb_nums[11] = {
		"gfx/2d/numbers/zero",
		"gfx/2d/numbers/one",
		"gfx/2d/numbers/two",
		"gfx/2d/numbers/three",
		"gfx/2d/numbers/four",
		"gfx/2d/numbers/five",
		"gfx/2d/numbers/six",
		"gfx/2d/numbers/seven",
		"gfx/2d/numbers/eight",
		"gfx/2d/numbers/nine",
		"gfx/2d/numbers/minus",
	};

	static char		*sb_t_nums[11] = {
		"gfx/2d/numbers/t_zero",
		"gfx/2d/numbers/t_one",
		"gfx/2d/numbers/t_two",
		"gfx/2d/numbers/t_three",
		"gfx/2d/numbers/t_four",
		"gfx/2d/numbers/t_five",
		"gfx/2d/numbers/t_six",
		"gfx/2d/numbers/t_seven",
		"gfx/2d/numbers/t_eight",
		"gfx/2d/numbers/t_nine",
		"gfx/2d/numbers/t_minus",
	};

	static char		*sb_c_nums[11] = {
		"gfx/2d/numbers/c_zero",
		"gfx/2d/numbers/c_one",
		"gfx/2d/numbers/c_two",
		"gfx/2d/numbers/c_three",
		"gfx/2d/numbers/c_four",
		"gfx/2d/numbers/c_five",
		"gfx/2d/numbers/c_six",
		"gfx/2d/numbers/c_seven",
		"gfx/2d/numbers/c_eight",
		"gfx/2d/numbers/c_nine",
		"gfx/2d/numbers/t_minus", //?????
	};

	static char		*plum_nums[11] = {
		"gfx/2d/plumdigit/digit0",
		"gfx/2d/plumdigit/digit1",
		"gfx/2d/plumdigit/digit2",
		"gfx/2d/plumdigit/digit3",
		"gfx/2d/plumdigit/digit4",
		"gfx/2d/plumdigit/digit5",
		"gfx/2d/plumdigit/digit6",
		"gfx/2d/plumdigit/digit7",
		"gfx/2d/plumdigit/digit8",
		"gfx/2d/plumdigit/digit9",
		"gfx/2d/plumdigit/digitminus",
	};

	// clear any references to old media
	memset( &cg.refdef, 0, sizeof( cg.refdef ) );
	cgi.R_ClearScene();

	//CG_LoadingString( cgs.mapname );        
	CG_LoadingString( cgs.mapname );

	cg.showMapLoadProgress = 1;
//#ifndef _XBOX
	cgi.R_LoadWorldMap( cgs.mapname );		// The hooks in here will update the progress of the BSP loading
//#endif
	cg.showMapLoadProgress = 0;

	// precache status bar pics
	CG_LoadingString( "Game Media" );

	for ( i=0 ; i<11 ; i++) {
		cgs.media.numberShaders[i] = cgi.R_RegisterShader( sb_nums[i] );
		cgs.media.plumShaders[i] = cgi.R_RegisterShader( plum_nums[i] );
		
	}

	cg.loadLCARSStage = 3;

	for ( i=0; i < 11; i++ )
	{
		cgs.media.numberShaders[i]			= cgi.R_RegisterShaderNoMip( sb_nums[i] );
		cgs.media.smallnumberShaders[i]		= cgi.R_RegisterShaderNoMip( sb_t_nums[i] );
		cgs.media.chunkyNumberShaders[i]	= cgi.R_RegisterShaderNoMip( sb_c_nums[i] );
	}

	cgi.R_RegisterShaderNoMip ( "gfx/mp/pduel_icon_lone" );
	cgi.R_RegisterShaderNoMip ( "gfx/mp/pduel_icon_double" );

	cgs.media.balloonShader = cgi.R_RegisterShader( "gfx/mp/chat_icon" );
	cgs.media.vchatShader = cgi.R_RegisterShader( "gfx/mp/vchat_icon" );

	cgs.media.deferShader = cgi.R_RegisterShaderNoMip( "gfx/2d/defer.tga" );

	cgs.media.radarShader			= cgi.R_RegisterShaderNoMip ( "gfx/menus/radar/radar.png" );
	cgs.media.siegeItemShader		= cgi.R_RegisterShaderNoMip ( "gfx/menus/radar/goalitem" );
	cgs.media.mAutomapPlayerIcon	= cgi.R_RegisterShader( "gfx/menus/radar/arrow_w" );
	cgs.media.mAutomapRocketIcon	= cgi.R_RegisterShader( "gfx/menus/radar/rocket" );

	cgs.media.wireframeAutomapFrame_left = cgi.R_RegisterShader( "gfx/mp_automap/mpauto_frame_left" );
	cgs.media.wireframeAutomapFrame_right = cgi.R_RegisterShader( "gfx/mp_automap/mpauto_frame_right" );
	cgs.media.wireframeAutomapFrame_top = cgi.R_RegisterShader( "gfx/mp_automap/mpauto_frame_top" );
	cgs.media.wireframeAutomapFrame_bottom = cgi.R_RegisterShader( "gfx/mp_automap/mpauto_frame_bottom" );

	cgs.media.lagometerShader = cgi.R_RegisterShaderNoMip("gfx/2d/lag" );
	cgs.media.connectionShader = cgi.R_RegisterShaderNoMip( "gfx/2d/net" );

	CG_LoadingString( "Effects" );
	cgi.FX_InitSystem(&cg.refdef);
	CG_RegisterEffects();

	cgs.media.boltShader = cgi.R_RegisterShader( "gfx/misc/blueLine" );

	cgs.effects.turretShotEffect = cgi.FX_RegisterEffect( "turret/shot" );
	cgs.effects.mEmplacedDeadSmoke = cgi.FX_RegisterEffect("emplaced/dead_smoke.efx");
	cgs.effects.mEmplacedExplode = cgi.FX_RegisterEffect("emplaced/explode.efx");
	cgs.effects.mTurretExplode = cgi.FX_RegisterEffect("turret/explode.efx");
	cgs.effects.mSparkExplosion = cgi.FX_RegisterEffect("sparks/spark_explosion.efx");
	cgs.effects.mTripmineExplosion = cgi.FX_RegisterEffect("tripMine/explosion.efx");
	cgs.effects.mDetpackExplosion = cgi.FX_RegisterEffect("detpack/explosion.efx");
	cgs.effects.mFlechetteAltBlow = cgi.FX_RegisterEffect("flechette/alt_blow.efx");
	cgs.effects.mStunBatonFleshImpact = cgi.FX_RegisterEffect("stunBaton/flesh_impact.efx");
	cgs.effects.mAltDetonate = cgi.FX_RegisterEffect("demp2/altDetonate.efx");
	cgs.effects.mSparksExplodeNoSound = cgi.FX_RegisterEffect("sparks/spark_exp_nosnd");
	cgs.effects.mTripMineLaser = cgi.FX_RegisterEffect("tripMine/laser.efx");
	cgs.effects.mEmplacedMuzzleFlash = cgi.FX_RegisterEffect( "effects/emplaced/muzzle_flash" );
	cgs.effects.mConcussionAltRing = cgi.FX_RegisterEffect("concussion/alt_ring");

	cgs.effects.mHyperspaceStars = cgi.FX_RegisterEffect("ships/hyperspace_stars");
	cgs.effects.mBlackSmoke = cgi.FX_RegisterEffect( "volumetric/black_smoke" );
	cgs.effects.mShipDestDestroyed = cgi.FX_RegisterEffect("effects/ships/dest_destroyed.efx");
	cgs.effects.mShipDestBurning = cgi.FX_RegisterEffect("effects/ships/dest_burning.efx");
	cgs.effects.mBobaJet = cgi.FX_RegisterEffect("effects/boba/jet.efx");
	
	cgs.effects.mJetpack = cgi.FX_RegisterEffect("effects/player/jetpack.efx"); //("effects/rockettrooper/flamenew.efx");


	cgs.effects.itemCone = cgi.FX_RegisterEffect("mp/itemcone.efx");
	cgs.effects.mTurretMuzzleFlash = cgi.FX_RegisterEffect("effects/turret/muzzle_flash.efx");
	cgs.effects.mSparks = cgi.FX_RegisterEffect("sparks/spark_nosnd.efx"); //sparks/spark.efx
	cgs.effects.mSaberCut = cgi.FX_RegisterEffect("saber/saber_cut.efx");
	cgs.effects.mSaberBlock = cgi.FX_RegisterEffect("saber/saber_block.efx");
	cgs.effects.mSaberBloodSparks = cgi.FX_RegisterEffect("saber/blood_sparks_mp.efx");
	cgs.effects.mSaberBloodSparksSmall = cgi.FX_RegisterEffect("saber/blood_sparks_25_mp.efx");
	cgs.effects.mSaberBloodSparksMid = cgi.FX_RegisterEffect("saber/blood_sparks_50_mp.efx");
	cgs.effects.mSpawn = cgi.FX_RegisterEffect("mp/spawn.efx");
	cgs.effects.mJediSpawn = cgi.FX_RegisterEffect("mp/jedispawn.efx");
	cgs.effects.mBlasterDeflect = cgi.FX_RegisterEffect("blaster/deflect.efx");
	cgs.effects.mBlasterSmoke = cgi.FX_RegisterEffect("blaster/smoke_bolton");
	cgs.effects.mForceConfustionOld = cgi.FX_RegisterEffect("force/confusion_old.efx");

	cgs.effects.forceLightning		= cgi.FX_RegisterEffect( "effects/force/lightning.efx" );
	cgs.effects.forceLightningWide	= cgi.FX_RegisterEffect( "effects/force/lightningwide.efx" );
	cgs.effects.forceDrain		= cgi.FX_RegisterEffect( "effects/mp/drain.efx" );
	cgs.effects.forceDrainWide	= cgi.FX_RegisterEffect( "effects/mp/drainwide.efx" );
	cgs.effects.forceDrained	= cgi.FX_RegisterEffect( "effects/mp/drainhit.efx");

	cgs.effects.mDisruptorDeathSmoke = cgi.FX_RegisterEffect("disruptor/death_smoke");

	for ( i = 0 ; i < NUM_CROSSHAIRS ; i++ ) {
		cgs.media.crosshairShader[i] = cgi.R_RegisterShaderNoMip( va("gfx/2d/crosshair%c", 'a'+i) );
	}

	cg.loadLCARSStage = 4;

	CG_LoadingString( "Powerups" );

	cgs.media.backTileShader = cgi.R_RegisterShader( "gfx/2d/backtile" );

	//precache the fpls skin
	//cgi.R_RegisterSkin("models/players/kyle/model_fpls2.skin");

	cgs.media.itemRespawningPlaceholder = cgi.R_RegisterShader("powerups/placeholder");
	cgs.media.itemRespawningRezOut = cgi.R_RegisterShader("powerups/rezout");

	cgs.media.playerShieldDamage = cgi.R_RegisterShader("gfx/misc/personalshield");
	cgs.media.protectShader = cgi.R_RegisterShader("gfx/misc/forceprotect");
	cgs.media.forceSightBubble = cgi.R_RegisterShader("gfx/misc/sightbubble");
	cgs.media.forceShell = cgi.R_RegisterShader("powerups/forceshell");
	cgs.media.sightShell = cgi.R_RegisterShader("powerups/sightshell");

	cgs.media.itemHoloModel = cgi.R_RegisterModel("models/map_objects/mp/holo.md3");

	if (cgs.gametype == GT_HOLOCRON || cg_buildScript.integer)
	{
		for ( i=0; i < NUM_FORCE_POWERS; i++ )
		{
			if (forceHolocronModels[i] &&
				forceHolocronModels[i][0])
			{
				cgi.R_RegisterModel(forceHolocronModels[i]);
			}
		}
	}

	if ( cgs.gametype == GT_CTF || cgs.gametype == GT_CTY || cgs.gametype == GT_WARZONE || cg_buildScript.integer ) {
		if (cg_buildScript.integer)
		{
			cgi.R_RegisterModel( "models/flags/r_flag.md3" );
			cgi.R_RegisterModel( "models/flags/b_flag.md3" );
			cgi.R_RegisterModel( "models/flags/r_flag_ysal.md3" );
			cgi.R_RegisterModel( "models/flags/b_flag_ysal.md3" );
		}

		if (cgs.gametype == GT_CTF || cgs.gametype == GT_WARZONE)
		{
			cgs.media.redFlagModel = cgi.R_RegisterModel( "models/flags/r_flag.md3" );
			cgs.media.blueFlagModel = cgi.R_RegisterModel( "models/flags/b_flag.md3" );
		}
		else
		{
			cgs.media.redFlagModel = cgi.R_RegisterModel( "models/flags/r_flag_ysal.md3" );
			cgs.media.blueFlagModel = cgi.R_RegisterModel( "models/flags/b_flag_ysal.md3" );
		}

		cgi.R_RegisterShaderNoMip( "gfx/hud/mpi_rflag_x" );
		cgi.R_RegisterShaderNoMip( "gfx/hud/mpi_bflag_x" );

		cgi.R_RegisterShaderNoMip( "gfx/hud/mpi_rflag_ys" );
		cgi.R_RegisterShaderNoMip( "gfx/hud/mpi_bflag_ys" );

		cgi.R_RegisterShaderNoMip( "gfx/hud/mpi_rflag" );
		cgi.R_RegisterShaderNoMip( "gfx/hud/mpi_bflag" );

		cgi.R_RegisterShaderNoMip("gfx/2d/net.tga");

		cgs.media.flagPoleModel = cgi.R_RegisterModel( "models/flag2/flagpole.md3" );
		cgs.media.flagFlapModel = cgi.R_RegisterModel( "models/flag2/flagflap3.md3" );

		cgs.media.redFlagBaseModel = cgi.R_RegisterModel( "models/mapobjects/flagbase/red_base.md3" );
		cgs.media.blueFlagBaseModel = cgi.R_RegisterModel( "models/mapobjects/flagbase/blue_base.md3" );
		cgs.media.neutralFlagBaseModel = cgi.R_RegisterModel( "models/mapobjects/flagbase/ntrl_base.md3" );
	}

	cgs.media.teamStatusBar = cgi.R_RegisterShader( "gfx/2d/colorbar.tga" );
	/*
	if ( cgs.gametype >= GT_TEAM || cg_buildScript.integer ) {
		cgs.media.teamRedShader = cgi.R_RegisterShader( "sprites/team_red" );
		cgs.media.teamBlueShader = cgi.R_RegisterShader( "sprites/team_blue" );
		//cgs.media.redQuadShader = cgi.R_RegisterShader("powerups/blueflag" );
		//cgs.media.teamStatusBar = cgi.R_RegisterShader( "gfx/2d/colorbar.tga" );
	}
	else if ( cgs.gametype == GT_JEDIMASTER )
	{
		cgs.media.teamRedShader = cgi.R_RegisterShader( "sprites/team_red" );
	}*/
	// JKG: Always load these (for now)
	// cgs.media.teamRedShader = cgi.R_RegisterShader( "sprites/team_red" );
	// cgs.media.teamBlueShader = cgi.R_RegisterShader( "sprites/team_blue" );
	cgs.media.teamRedShader = bgGangWarsTeams[cgs.redTeam].teamIcon;
	cgs.media.teamBlueShader = bgGangWarsTeams[cgs.blueTeam].teamIcon;

	if (cgs.gametype == GT_POWERDUEL || cg_buildScript.integer)
	{
		cgs.media.powerDuelAllyShader = cgi.R_RegisterShader("gfx/mp/pduel_icon_double");//cgi.R_RegisterShader("gfx/mp/pduel_gameicon_ally");
	}

	cgs.media.heartShader			= cgi.R_RegisterShaderNoMip( "ui/assets/statusbar/selectedhealth.tga" );

	cgs.media.ysaliredShader		= cgi.R_RegisterShader( "powerups/ysaliredshell");
	cgs.media.ysaliblueShader		= cgi.R_RegisterShader( "powerups/ysaliblueshell");
	cgs.media.ysalimariShader		= cgi.R_RegisterShader( "powerups/ysalimarishell");
	cgs.media.boonShader			= cgi.R_RegisterShader( "powerups/boonshell");
	cgs.media.endarkenmentShader	= cgi.R_RegisterShader( "powerups/endarkenmentshell");
	cgs.media.enlightenmentShader	= cgi.R_RegisterShader( "powerups/enlightenmentshell");
	cgs.media.invulnerabilityShader = cgi.R_RegisterShader( "powerups/invulnerabilityshell");

#ifdef JK2AWARDS
	cgs.media.medalImpressive		= cgi.R_RegisterShaderNoMip( "medal_impressive" );
	cgs.media.medalExcellent		= cgi.R_RegisterShaderNoMip( "medal_excellent" );
	cgs.media.medalGauntlet			= cgi.R_RegisterShaderNoMip( "medal_gauntlet" );
	cgs.media.medalDefend			= cgi.R_RegisterShaderNoMip( "medal_defend" );
	cgs.media.medalAssist			= cgi.R_RegisterShaderNoMip( "medal_assist" );
	cgs.media.medalCapture			= cgi.R_RegisterShaderNoMip( "medal_capture" );
#endif

	// Binocular interface
	cgs.media.binocularCircle		= cgi.R_RegisterShader( "gfx/2d/binCircle" );
	cgs.media.binocularMask			= cgi.R_RegisterShader( "gfx/2d/binMask" );
	cgs.media.binocularArrow		= cgi.R_RegisterShader( "gfx/2d/binSideArrow" );
	cgs.media.binocularTri			= cgi.R_RegisterShader( "gfx/2d/binTopTri" );
	cgs.media.binocularStatic		= cgi.R_RegisterShader( "gfx/2d/binocularWindow" );
	cgs.media.binocularOverlay		= cgi.R_RegisterShader( "gfx/2d/binocularNumOverlay" );

	cg.loadLCARSStage = 5;
	CG_LoadingString( "Models" );

	// Chunk models
	//FIXME: jfm:? bother to conditionally load these if an ent has this material type?
	for ( i = 0; i < NUM_CHUNK_MODELS; i++ )
	{
		cgs.media.chunkModels[CHUNK_METAL2][i]	= cgi.R_RegisterModel( va( "models/chunks/metal/metal1_%i.md3", i+1 ) ); //_ /switched\ _
		cgs.media.chunkModels[CHUNK_METAL1][i]	= cgi.R_RegisterModel( va( "models/chunks/metal/metal2_%i.md3", i+1 ) ); //  \switched/
		cgs.media.chunkModels[CHUNK_ROCK1][i]	= cgi.R_RegisterModel( va( "models/chunks/rock/rock1_%i.md3", i+1 ) );
		cgs.media.chunkModels[CHUNK_ROCK2][i]	= cgi.R_RegisterModel( va( "models/chunks/rock/rock2_%i.md3", i+1 ) );
		cgs.media.chunkModels[CHUNK_ROCK3][i]	= cgi.R_RegisterModel( va( "models/chunks/rock/rock3_%i.md3", i+1 ) );
		cgs.media.chunkModels[CHUNK_CRATE1][i]	= cgi.R_RegisterModel( va( "models/chunks/crate/crate1_%i.md3", i+1 ) );
		cgs.media.chunkModels[CHUNK_CRATE2][i]	= cgi.R_RegisterModel( va( "models/chunks/crate/crate2_%i.md3", i+1 ) );
		cgs.media.chunkModels[CHUNK_WHITE_METAL][i]	= cgi.R_RegisterModel( va( "models/chunks/metal/wmetal1_%i.md3", i+1 ) );
	}

	cgs.media.hitmarkerSound		= cgi.S_RegisterSound("sound/player/hitmarker");

	cgs.media.chunkSound			= cgi.S_RegisterSound("sound/weapons/explosions/glasslcar");
	cgs.media.grateSound			= cgi.S_RegisterSound( "sound/effects/grate_destroy" );
	cgs.media.rockBreakSound		= cgi.S_RegisterSound("sound/effects/wall_smash");
	cgs.media.rockBounceSound[0]	= cgi.S_RegisterSound("sound/effects/stone_bounce");
	cgs.media.rockBounceSound[1]	= cgi.S_RegisterSound("sound/effects/stone_bounce2");
	cgs.media.metalBounceSound[0]	= cgi.S_RegisterSound("sound/effects/metal_bounce");
	cgs.media.metalBounceSound[1]	= cgi.S_RegisterSound("sound/effects/metal_bounce2");
	cgs.media.glassChunkSound		= cgi.S_RegisterSound("sound/weapons/explosions/glassbreak1");
	cgs.media.crateBreakSound[0]	= cgi.S_RegisterSound("sound/weapons/explosions/crateBust1" );
	cgs.media.crateBreakSound[1]	= cgi.S_RegisterSound("sound/weapons/explosions/crateBust2" );
	
	cgs.media.bboxShader = cgi.R_RegisterShader ("gfx/effects/bbox");
	
	// Damage types
	cgs.media.stunOverlay = cgi.R_RegisterShader ("gfx/PlayerOverlays/stun");
	cgs.media.iceOverlay = cgi.R_RegisterShader ("gfx/PlayerOverlays/ice");
	cgs.media.carboniteOverlay = cgi.R_RegisterShader ("gfx/PlayerOverlays/carbonite");
    cgs.media.playerFireEffect = cgi.FX_RegisterEffect ("player/fire");

	CG_LoadingString( "Items" );
/*
Ghoul2 Insert Start
*/
	CG_InitItems();
/*
Ghoul2 Insert End
*/

	// only register the items that the server says we need
	strcpy( items, CG_ConfigString( CS_ITEMS) );

	for ( i = 1 ; i < bg_numItems ; i++ ) {
		if ( items[ i ] == '1' || cg_buildScript.integer ) {
			CG_LoadingItem( i );
			CG_RegisterItemVisuals( i );
		}
	}

	cg.loadLCARSStage = 6;

	cgs.media.glassShardShader	= cgi.R_RegisterShader( "gfx/misc/test_crackle" );

	// doing one shader just makes it look like a shell.  By using two shaders with different bulge offsets and different texture scales, it has a much more chaotic look
	cgs.media.electricBodyShader			= cgi.R_RegisterShader( "gfx/misc/electric" );
	cgs.media.electricBody2Shader			= cgi.R_RegisterShader( "gfx/misc/fullbodyelectric2" );

	cgs.media.fsrMarkShader					= cgi.R_RegisterShader( "footstep_r" );
	cgs.media.fslMarkShader					= cgi.R_RegisterShader( "footstep_l" );
	cgs.media.fshrMarkShader				= cgi.R_RegisterShader( "footstep_heavy_r" );
	cgs.media.fshlMarkShader				= cgi.R_RegisterShader( "footstep_heavy_l" );

	cgs.media.refractionShader				= cgi.R_RegisterShader("effects/refraction");

	cgs.media.cloakedShader					= cgi.R_RegisterShader( "gfx/effects/cloakedShader" );

	// wall marks
	cgs.media.shadowMarkShader	= cgi.R_RegisterShader( "markShadow" );
	cgs.media.wakeMarkShader	= cgi.R_RegisterShader( "wake" );

	cgs.media.viewPainShader					= cgi.R_RegisterShader( "gfx/misc/borgeyeflare" );
	cgs.media.viewPainShader_Shields			= cgi.R_RegisterShader( "gfx/mp/dmgshader_shields" );
	cgs.media.viewPainShader_ShieldsAndHealth	= cgi.R_RegisterShader( "gfx/mp/dmgshader_shieldsandhealth" );

	CG_LoadingString( "Brush Models" );
	// register the inline models
	breakPoint = cgs.numInlineModels = cgi.CM_NumInlineModels();
	for ( i = 1 ; i < cgs.numInlineModels ; i++ ) {
		char	name[10];
		vec3_t			mins, maxs;
		int				j;

		Com_sprintf( name, sizeof(name), "*%i", i );
		cgs.inlineDrawModel[i] = cgi.R_RegisterModel( name );
		if (!cgs.inlineDrawModel[i])
		{
			breakPoint = i;
			break;
		}

		cgi.R_ModelBounds( cgs.inlineDrawModel[i], mins, maxs );
		for ( j = 0 ; j < 3 ; j++ ) {
			cgs.inlineModelMidpoints[i][j] = mins[j] + 0.5 * ( maxs[j] - mins[j] );
		}
	}

	cg.loadLCARSStage = 7;

	CG_LoadingString( "Game Models" );
	// register all the server specified models
	for (i=1 ; i<MAX_MODELS ; i++) {
		const char		*cModelName;
		char modelName[MAX_QPATH];

		cModelName = CG_ConfigString( CS_MODELS+i );
		if ( !cModelName[0] ) {
			break;
		}

		strcpy(modelName, cModelName);
		if (strstr(modelName, ".glm") || modelName[0] == '$')
		{ //Check to see if it has a custom skin attached.
			CG_HandleAppendedSkin(modelName);
			CG_CacheG2AnimInfo(modelName);
		}

		if (modelName[0] != '$' && modelName[0] != '@')
		{ //don't register vehicle names and saber names as models.
			cgs.gameModels[i] = cgi.R_RegisterModel( modelName );
		}
		else
		{//FIXME: register here so that stuff gets precached!!!
			cgs.gameModels[i] = 0;
		}
	}
	cg.loadLCARSStage = 8;
/*
Ghoul2 Insert Start
*/


	CG_LoadingString( "BSP instances" );

	for(i = 1; i < MAX_SUB_BSP; i++)
	{
		const char		*bspName = 0;
		vec3_t			mins, maxs;
		int				j;
		int				sub = 0;
		char			temp[MAX_QPATH];

		bspName = CG_ConfigString( CS_BSP_MODELS+i );
		if ( !bspName[0] ) 
		{
			break;
		}
		CG_LoadingString( va("BSP instances: %s", bspName) );

		cgi.CM_LoadMap( bspName, qtrue );
		cgs.inlineDrawModel[breakPoint] = cgi.R_RegisterModel( bspName );
		cgi.R_ModelBounds( cgs.inlineDrawModel[breakPoint], mins, maxs );
		for ( j = 0 ; j < 3 ; j++ ) 
		{
			cgs.inlineModelMidpoints[breakPoint][j] = mins[j] + 0.5 * ( maxs[j] - mins[j] );
		}
		breakPoint++;
		for(sub=1;sub<MAX_MODELS;sub++)
		{
			Com_sprintf(temp, MAX_QPATH, "*%d-%d", i, sub);
			cgs.inlineDrawModel[breakPoint] = cgi.R_RegisterModel( temp );
			if (!cgs.inlineDrawModel[breakPoint])
			{
				break;
			}
			cgi.R_ModelBounds( cgs.inlineDrawModel[breakPoint], mins, maxs );
			for ( j = 0 ; j < 3 ; j++ ) 
			{
				cgs.inlineModelMidpoints[breakPoint][j] = mins[j] + 0.5 * ( maxs[j] - mins[j] );
			}
			breakPoint++;
		}
	}

	CG_LoadingString( "Creating terrain" );
	for(i = 0; i < MAX_TERRAINS; i++)
	{
		terrainInfo = CG_ConfigString( CS_TERRAINS + i + 1 );
		if ( !terrainInfo[0] )
		{
			break;
		}

		terrainID = cgi.CM_RegisterTerrain(terrainInfo);

		cgi.RMG_Init(terrainID, terrainInfo);

		// Send off the terrainInfo to the renderer
		cgi.RE_InitRendererTerrain( terrainInfo );
	}

	/*
	CG_LoadingString("skins");
	// register all the server specified models
	for (i=1 ; i<MAX_CHARSKINS ; i++) {
		const char		*modelName;

		modelName = CG_ConfigString( CS_CHARSKINS+i );
		if ( !modelName[0] ) {
			break;
		}
		cgs.skins[i] = cgi.R_RegisterSkin( modelName );
	}
	*/
	//rww - removed and replaced with CS_G2BONES. For custom skins
	//the new method is to append a * after an indexed model name and
	//then append the skin name after that (note that this is only
	//used for NPCs)

	CG_LoadingString("Weapons");

	CG_InitG2Weapons();

/*
Ghoul2 Insert End
*/
	cg.loadLCARSStage = 9;

	cgs.media.hitmarkerGraphic = cgi.R_RegisterShaderNoMip("gfx/2d/crosshair_hitmarker.tga");

	// new stuff
	cgs.media.patrolShader = cgi.R_RegisterShaderNoMip("ui/assets/statusbar/patrol.tga");
	cgs.media.assaultShader = cgi.R_RegisterShaderNoMip("ui/assets/statusbar/assault.tga");
	cgs.media.campShader = cgi.R_RegisterShaderNoMip("ui/assets/statusbar/camp.tga");
	cgs.media.followShader = cgi.R_RegisterShaderNoMip("ui/assets/statusbar/follow.tga");
	cgs.media.defendShader = cgi.R_RegisterShaderNoMip("ui/assets/statusbar/defend.tga");
	cgs.media.teamLeaderShader = cgi.R_RegisterShaderNoMip("ui/assets/statusbar/team_leader.tga");
	cgs.media.retrieveShader = cgi.R_RegisterShaderNoMip("ui/assets/statusbar/retrieve.tga");
	cgs.media.escortShader = cgi.R_RegisterShaderNoMip("ui/assets/statusbar/escort.tga");
	cgs.media.cursor = cgi.R_RegisterShaderNoMip( "menu/art/3_cursor2" );
	cgs.media.sizeCursor = cgi.R_RegisterShaderNoMip( "ui/assets/sizecursor.tga" );
	cgs.media.selectCursor = cgi.R_RegisterShaderNoMip( "ui/assets/selectcursor.tga" );
	cgs.media.flagShaders[0] = cgi.R_RegisterShaderNoMip("ui/assets/statusbar/flag_in_base.tga");
	cgs.media.flagShaders[1] = cgi.R_RegisterShaderNoMip("ui/assets/statusbar/flag_capture.tga");
	cgs.media.flagShaders[2] = cgi.R_RegisterShaderNoMip("ui/assets/statusbar/flag_missing.tga");

	// Jedi Knight Galaxies
	cgs.media.lowHealthAura = cgi.R_RegisterShader("gfx/jkg/lowhealthaura.png");

	cgs.media.halfShieldModel	= cgi.R_RegisterModel ( "models/weaphits/testboom.md3" );
	cgs.media.halfShieldShader	= cgi.R_RegisterShader( "halfShieldShell" );

	cgi.FX_RegisterEffect("force/force_touch");

	CG_ClearParticles ();
/*
	for (i=1; i<MAX_PARTICLES_AREAS; i++)
	{
		{
			int rval;

			rval = CG_NewParticleArea ( CS_PARTICLES + i);
			if (!rval)
				break;
		}
	}
*/

	// Jedi Knight Galaxies

	cgs.media.deathfont = cgi.R_RegisterFont("euraldb");
	cgs.media.hudfont1 = cgi.R_RegisterFont("hudfnt1");
	cgs.media.hudfont2 = cgi.R_RegisterFont("hudfnt2");
	cgs.media.horizgradient = cgi.R_RegisterShaderNoMip("gfx/jkg/horzgradient.png");
	cgs.media.avatar_placeholder = cgi.R_RegisterShaderNoMip("gfx/avatars/placeholder.png");
}


const char *CG_GetStringEdString(char *refSection, char *refName)
{
	static char text[2][1024]={0};	//just incase it's nested
	static int		index = 0;

	index ^= 1;
	cgi.SP_GetStringTextString(va("%s_%s", refSection, refName), text[index], sizeof(text[0]));
	return text[index];
}

const char *CG_GetStringEdString2(char *refName)
{
	static char text[2][1024]={0};	//just incase it's nested
	static int		index = 0;

	index ^= 1;
	if(refName[0] == '@')
	{
		// Reference it up!
		cgi.SP_GetStringTextString(refName+1, text[index], sizeof(text[0]));
	}
	else
	{
		strcpy(text[index], refName);
	}
	return text[index];
}

int CG_GetTeamNonScoreCount(team_t team);

/*																																			
=======================
CG_BuildSpectatorString

=======================
*/
void CG_BuildSpectatorString(void) {
	int i;
	cg.spectatorList[0] = 0;

	for (i = 0; i < MAX_CLIENTS; i++) {
		if (cgs.clientinfo[i].infoValid && cgs.clientinfo[i].team == TEAM_SPECTATOR ) {
			Q_strcat(cg.spectatorList, sizeof(cg.spectatorList), va("%s     ", cgs.clientinfo[i].name));
		}
	}
	i = strlen(cg.spectatorList);
	if (i != cg.spectatorLen) {
		cg.spectatorLen = i;
		cg.spectatorWidth = -1;
	}
}


/*																																			
===================
CG_RegisterClients
===================
*/
static void CG_RegisterClients( void ) {
	int		i;

	CG_LoadingClient(cg.clientNum);
	CG_NewClientInfo(cg.clientNum, qfalse);

	for (i=0 ; i<MAX_CLIENTS ; i++) {
		const char		*clientInfo;

		if (cg.clientNum == i) {
			continue;
		}

		clientInfo = CG_ConfigString( CS_PLAYERS+i );
		if ( !clientInfo[0]) {
			continue;
		}
		CG_LoadingClient( i );
		CG_NewClientInfo( i, qfalse);
	}
	CG_BuildSpectatorString();
}

//===========================================================================

/*
=================
CG_ConfigString
=================
*/
const char *CG_ConfigString( int index ) {
	if ( index < 0 || index >= MAX_CONFIGSTRINGS ) {
		CG_Error( "CG_ConfigString: bad index: %i", index );
	}
	return cgs.gameState.stringData + cgs.gameState.stringOffsets[ index ];
}

//==================================================================

/*
======================
CG_StartMusic

======================
*/
void CG_StartMusic( qboolean bForceStart ) {
	char	*s;
	char	parm1[MAX_QPATH], parm2[MAX_QPATH];

	// start the background music
	s = (char *)CG_ConfigString( CS_MUSIC );
	Q_strncpyz( parm1, COM_Parse( (const char **)&s ), sizeof( parm1 ) );
	Q_strncpyz( parm2, COM_Parse( (const char **)&s ), sizeof( parm2 ) );

	cgi.S_StartBackgroundTrack( parm1, parm2, !bForceStart );
}

char *CG_GetMenuBuffer(const char *filename) {
	int	len;
	fileHandle_t	f;
	static char buf[MAX_MENUFILE];

	len = cgi.FS_FOpenFile( filename, &f, FS_READ );
	if ( !f ) {
		cgi.Print( va( S_COLOR_RED "menu file not found: %s, using default\n", filename ) );
		return NULL;
	}
	if ( len >= MAX_MENUFILE ) {
		cgi.Print( va( S_COLOR_RED "menu file too large: %s is %i, max allowed is %i", filename, len, MAX_MENUFILE ) );
		cgi.FS_FCloseFile( f );
		return NULL;
	}

	cgi.FS_Read( buf, len, f );
	buf[len] = 0;
	cgi.FS_FCloseFile( f );

	return buf;
}

//
// ==============================
// new hud stuff ( mission pack )
// ==============================
//
qboolean CG_Asset_Parse(int handle) {
	pc_token_t token;

	if (!cgi.PC_ReadToken(handle, &token))
		return qfalse;
	if (Q_stricmp(token.string, "{") != 0) {
		return qfalse;
	}
    
	while ( 1 ) {
		if (!cgi.PC_ReadToken(handle, &token))
			return qfalse;

		if (Q_stricmp(token.string, "}") == 0) {
			return qtrue;
		}

		// font
		if (Q_stricmp(token.string, "font") == 0) {
			int pointSize;
			if (!cgi.PC_ReadToken(handle, &token) || !PC_Int_Parse(handle, &pointSize)) {
				return qfalse;
			}

//			cgDC.registerFont(token.string, pointSize, &cgDC.Assets.textFont);
			cgDC.Assets.qhMediumFont = cgDC.RegisterFont(token.string);
			continue;
		}

		// smallFont
		if (Q_stricmp(token.string, "smallFont") == 0) {
			int pointSize;
			if (!cgi.PC_ReadToken(handle, &token) || !PC_Int_Parse(handle, &pointSize)) {
				return qfalse;
			}
//			cgDC.registerFont(token.string, pointSize, &cgDC.Assets.smallFont);
			cgDC.Assets.qhSmallFont = cgDC.RegisterFont(token.string);
			continue;
		}

		// smallFont
		if (Q_stricmp(token.string, "small2Font") == 0) {
			int pointSize;
			if (!cgi.PC_ReadToken(handle, &token) || !PC_Int_Parse(handle, &pointSize)) {
				return qfalse;
			}
//			cgDC.registerFont(token.string, pointSize, &cgDC.Assets.smallFont);
			cgDC.Assets.qhSmall2Font = cgDC.RegisterFont(token.string);
			continue;
		}

		// Jedi Knight Galaxies: small3Font
		if (Q_stricmp(token.string, "small3Font") == 0) {
			int pointSize;
			if (!cgi.PC_ReadToken(handle, &token) || !PC_Int_Parse(handle, &pointSize)) {
				return qfalse;
			}
			cgDC.Assets.qhSmall3Font = cgDC.RegisterFont(token.string);
			continue;
		}

		if (Q_stricmp(token.string, "small4Font") == 0) {
			int pointSize;
			if (!cgi.PC_ReadToken(handle, &token) || !PC_Int_Parse(handle, &pointSize)) {
				return qfalse;
			}
			cgDC.Assets.qhSmall4Font = cgDC.RegisterFont(token.string);
			continue;
		}
		//end JKG

		// font
		if (Q_stricmp(token.string, "bigfont") == 0) {
			int pointSize;
			if (!cgi.PC_ReadToken(handle, &token) || !PC_Int_Parse(handle, &pointSize)) {
				return qfalse;
			}
//			cgDC.registerFont(token.string, pointSize, &cgDC.Assets.bigFont);
			cgDC.Assets.qhBigFont = cgDC.RegisterFont(token.string);
			continue;
		}

		// gradientbar
		if (Q_stricmp(token.string, "gradientbar") == 0) {
			if (!cgi.PC_ReadToken(handle, &token)) {
				return qfalse;
			}
			cgDC.Assets.gradientBar = cgi.R_RegisterShaderNoMip(token.string);
			continue;
		}

		// enterMenuSound
		if (Q_stricmp(token.string, "menuEnterSound") == 0) {
			if (!cgi.PC_ReadToken(handle, &token)) {
				return qfalse;
			}
			cgDC.Assets.menuEnterSound = cgi.S_RegisterSound( token.string );
			continue;
		}

		// exitMenuSound
		if (Q_stricmp(token.string, "menuExitSound") == 0) {
			if (!cgi.PC_ReadToken(handle, &token)) {
				return qfalse;
			}
			cgDC.Assets.menuExitSound = cgi.S_RegisterSound( token.string );
			continue;
		}

		// itemFocusSound
		if (Q_stricmp(token.string, "itemFocusSound") == 0) {
			if (!cgi.PC_ReadToken(handle, &token)) {
				return qfalse;
			}
			cgDC.Assets.itemFocusSound = cgi.S_RegisterSound( token.string );
			continue;
		}

		// menuBuzzSound
		if (Q_stricmp(token.string, "menuBuzzSound") == 0) {
			if (!cgi.PC_ReadToken(handle, &token)) {
				return qfalse;
			}
			cgDC.Assets.menuBuzzSound = cgi.S_RegisterSound( token.string );
			continue;
		}

		if (Q_stricmp(token.string, "cursor") == 0) {
			if (!PC_String_Parse(handle, &cgDC.Assets.cursorStr)) {
				return qfalse;
			}
			cgDC.Assets.cursor = cgi.R_RegisterShaderNoMip( cgDC.Assets.cursorStr);
			continue;
		}

		if (Q_stricmp(token.string, "fadeClamp") == 0) {
			if (!PC_Float_Parse(handle, &cgDC.Assets.fadeClamp)) {
				return qfalse;
			}
			continue;
		}

		if (Q_stricmp(token.string, "fadeCycle") == 0) {
			if (!PC_Int_Parse(handle, &cgDC.Assets.fadeCycle)) {
				return qfalse;
			}
			continue;
		}

		if (Q_stricmp(token.string, "fadeAmount") == 0) {
			if (!PC_Float_Parse(handle, &cgDC.Assets.fadeAmount)) {
				return qfalse;
			}
			continue;
		}

		if (Q_stricmp(token.string, "shadowX") == 0) {
			if (!PC_Float_Parse(handle, &cgDC.Assets.shadowX)) {
				return qfalse;
			}
			continue;
		}

		if (Q_stricmp(token.string, "shadowY") == 0) {
			if (!PC_Float_Parse(handle, &cgDC.Assets.shadowY)) {
				return qfalse;
			}
			continue;
		}

		if (Q_stricmp(token.string, "shadowColor") == 0) {
			if (!PC_Color_Parse(handle, &cgDC.Assets.shadowColor)) {
				return qfalse;
			}
			cgDC.Assets.shadowFadeClamp = cgDC.Assets.shadowColor[3];
			continue;
		}
	}
	return qfalse; // bk001204 - why not?
}

void CG_ParseMenu(const char *menuFile) {
	pc_token_t token;
	int handle;

	handle = cgi.PC_LoadSource(menuFile);
	if (!handle)
		handle = cgi.PC_LoadSource("ui/testhud.menu");
	if (!handle)
		return;

	while ( 1 ) {
		if (!cgi.PC_ReadToken( handle, &token )) {
			break;
		}

		//if ( Q_stricmp( token, "{" ) ) {
		//	Com_Printf( "Missing { in menu file\n" );
		//	break;
		//}

		//if ( menuCount == MAX_MENUS ) {
		//	Com_Printf( "Too many menus!\n" );
		//	break;
		//}

		if ( token.string[0] == '}' ) {
			break;
		}

		if (Q_stricmp(token.string, "assetGlobalDef") == 0) {
			if (CG_Asset_Parse(handle)) {
				continue;
			} else {
				break;
			}
		}


		if (Q_stricmp(token.string, "menudef") == 0) {
			// start a new menu
			Menu_New(handle);
		}
	}
	cgi.PC_FreeSource(handle);
}


qboolean CG_Load_Menu(const char **p) 
{

	char *token;

	token = COM_ParseExt((const char **)p, qtrue);

	if (token[0] != '{') {
		return qfalse;
	}

	while ( 1 ) {

		token = COM_ParseExt((const char **)p, qtrue);
    
		if (Q_stricmp(token, "}") == 0) {
			return qtrue;
		}

		if ( !token || token[0] == 0 ) {
			return qfalse;
		}

		CG_ParseMenu(token); 
	}
	return qfalse;
}


static qboolean CG_OwnerDrawHandleKey(int ownerDraw, int flags, float *special, int key, int ownerDrawID) {
	return qfalse;
}

static int CG_FeederCount(float feederID) {
	int i, count;
	count = 0;
	if (feederID == FEEDER_REDTEAM_LIST) {
		for (i = 0; i < cg.numScores; i++) {
			if (cg.scores[i].team == TEAM_RED) {
				count++;
			}
		}
	} else if (feederID == FEEDER_BLUETEAM_LIST) {
		for (i = 0; i < cg.numScores; i++) {
			if (cg.scores[i].team == TEAM_BLUE) {
				count++;
			}
		}
	} else if (feederID == FEEDER_SCOREBOARD) {
		return cg.numScores;
	}
	return count;
}


void CG_SetScoreSelection(void *p) {
	menuDef_t *menu = (menuDef_t*)p;
	playerState_t *ps = &cg.snap->ps;
	int i, red, blue;
	red = blue = 0;
	for (i = 0; i < cg.numScores; i++) {
		if (cg.scores[i].team == TEAM_RED) {
			red++;
		} else if (cg.scores[i].team == TEAM_BLUE) {
			blue++;
		}
		if (ps->clientNum == cg.scores[i].client) {
			cg.selectedScore = i;
		}
	}

	if (menu == NULL) {
		// just interested in setting the selected score
		return;
	}

	if ( cgs.gametype >= GT_TEAM ) {
		int feeder = FEEDER_REDTEAM_LIST;
		i = red;
		if (cg.scores[cg.selectedScore].team == TEAM_BLUE) {
			feeder = FEEDER_BLUETEAM_LIST;
			i = blue;
		}
		Menu_SetFeederSelection(menu, feeder, i, NULL);
	} else {
		Menu_SetFeederSelection(menu, FEEDER_SCOREBOARD, cg.selectedScore, NULL);
	}
}

// FIXME: might need to cache this info
static clientInfo_t * CG_InfoFromScoreIndex(int index, int team, int *scoreIndex) {
	int i, count;
	if ( cgs.gametype >= GT_TEAM ) {
		count = 0;
		for (i = 0; i < cg.numScores; i++) {
			if (cg.scores[i].team == team) {
				if (count == index) {
					*scoreIndex = i;
					return &cgs.clientinfo[cg.scores[i].client];
				}
				count++;
			}
		}
	}
	*scoreIndex = index;
	return &cgs.clientinfo[ cg.scores[index].client ];
}

static const char *CG_FeederItemText(float feederID, int index, int column,
									 qhandle_t *handle1, qhandle_t *handle2, qhandle_t *handle3) {
	gitem_t *item;
	int scoreIndex = 0;
	clientInfo_t *info = NULL;
	int team = -1;
	score_t *sp = NULL;

	*handle1 = *handle2 = *handle3 = -1;

	if (feederID == FEEDER_REDTEAM_LIST) {
		team = TEAM_RED;
	} else if (feederID == FEEDER_BLUETEAM_LIST) {
		team = TEAM_BLUE;
	}

	info = CG_InfoFromScoreIndex(index, team, &scoreIndex);
	sp = &cg.scores[scoreIndex];

	if (info && info->infoValid) {
		switch (column) {
			case 0:
				if ( info->powerups & ( 1 << PW_NEUTRALFLAG ) ) {
					item = BG_FindItemForPowerup( PW_NEUTRALFLAG );
					*handle1 = cg_items[ ITEM_INDEX(item) ].icon;
				} else if ( info->powerups & ( 1 << PW_REDFLAG ) ) {
					item = BG_FindItemForPowerup( PW_REDFLAG );
					*handle1 = cg_items[ ITEM_INDEX(item) ].icon;
				} else if ( info->powerups & ( 1 << PW_BLUEFLAG ) ) {
					item = BG_FindItemForPowerup( PW_BLUEFLAG );
					*handle1 = cg_items[ ITEM_INDEX(item) ].icon;
				} else {
					/*	
					if ( info->botSkill > 0 && info->botSkill <= 5 ) {
						*handle1 = cgs.media.botSkillShaders[ info->botSkill - 1 ];
					} else if ( info->handicap < 100 ) {
					return va("%i", info->handicap );
					}
					*/
				}
			break;
			case 1:
				if (team == -1) {
					return "";
				} else {
					*handle1 = CG_StatusHandle(info->teamTask);
				}
		  break;
			case 2:
				if ( cg.snap->ps.stats[ STAT_CLIENTS_READY ] & ( 1 << sp->client ) ) {
					return "Ready";
				}
				if (team == -1) {
					if (cgs.gametype == GT_DUEL || cgs.gametype == GT_POWERDUEL) {
						return va("%i/%i", info->wins, info->losses);
					} else if (info->infoValid && info->team == TEAM_SPECTATOR ) {
						return "Spectator";
					} else {
						return "";
					}
				} else {
					if (info->teamLeader) {
						return "Leader";
					}
				}
			break;
			case 3:
				return info->name;
			break;
			case 4:
				return va("%i", info->score);
			break;
			case 5:
				return va("%4i", sp->time);
			break;
			case 6:
				if ( sp->ping == -1 ) {
					return "connecting";
				} 
				return va("%4i", sp->ping);
			break;
		}
	}

	return "";
}

static qhandle_t CG_FeederItemImage(float feederID, int index) {
	return 0;
}

static qboolean CG_FeederSelection(float feederID, int index, itemDef_t *item) 
{
	if ( cgs.gametype >= GT_TEAM ) {
		int i, count;
		int team = (feederID == FEEDER_REDTEAM_LIST) ? TEAM_RED : TEAM_BLUE;
		count = 0;
		for (i = 0; i < cg.numScores; i++) {
			if (cg.scores[i].team == team) {
				if (index == count) {
					cg.selectedScore = i;
				}
				count++;
			}
		}
	} else {
		cg.selectedScore = index;
	}

	return qtrue;
}

static float CG_Cvar_Get(const char *cvar) {
	char buff[128];
	memset(buff, 0, sizeof(buff));
	cgi.Cvar_VariableStringBuffer(cvar, buff, sizeof(buff));
	return atof(buff);
}

void CG_Text_PaintWithCursor(float x, float y, float scale, vec4_t color, const char *text, int cursorPos, char cursor, int limit, int style, int iMenuFont) {
	CG_Text_Paint(x, y, scale, color, text, 0, limit, style, iMenuFont);
}

static int CG_OwnerDrawWidth(int ownerDraw, float scale) {
	switch (ownerDraw) {
	  case CG_GAME_TYPE:
			return CG_Text_Width(CG_GameTypeString(), scale, FONT_MEDIUM);
	  case CG_GAME_STATUS:
			return CG_Text_Width(CG_GetGameStatusText(), scale, FONT_MEDIUM);
			break;
	  case CG_KILLER:
			return CG_Text_Width(CG_GetKillerText(), scale, FONT_MEDIUM);
			break;
	  case CG_RED_NAME:
			return CG_Text_Width(DEFAULT_REDTEAM_NAME/*cg_redTeamName.string*/, scale, FONT_MEDIUM);
			break;
	  case CG_BLUE_NAME:
			return CG_Text_Width(DEFAULT_BLUETEAM_NAME/*cg_blueTeamName.string*/, scale, FONT_MEDIUM);
			break;


	}
	return 0;
}

static int CG_PlayCinematic(const char *name, float x, float y, float w, float h) {
  return cgi.CIN_PlayCinematic(name, x, y, w, h, CIN_loop);
}

static void CG_StopCinematic(int handle) {
  cgi.CIN_StopCinematic(handle);
}

static void CG_DrawCinematic(int handle, float x, float y, float w, float h) {
  cgi.CIN_SetExtents(handle, x, y, w, h);
  cgi.CIN_DrawCinematic(handle);
}

static void CG_RunCinematicFrame(int handle) {
  cgi.CIN_RunCinematic(handle);
}

/*
=================
CG_LoadMenus();

=================
*/
void CG_LoadMenus(const char *menuFile) 
{
	const char	*token;
	const char	*p;
	int	len;
	fileHandle_t	f;
	static char buf[MAX_MENUDEFFILE];

	len = cgi.FS_FOpenFile( menuFile, &f, FS_READ );

	if ( !f ) 
	{
		cgi.Print( va( S_COLOR_RED "menu file not found: %s, using default\n", menuFile ) );

		len = cgi.FS_FOpenFile( "ui/jahud.txt", &f, FS_READ );
		if (!f) 
		{
			cgi.Print( va( S_COLOR_RED "default menu file not found: ui/hud.txt, unable to continue!\n", menuFile ) );
		}
	}

	if ( len >= MAX_MENUDEFFILE ) 
	{
		cgi.Print( va( S_COLOR_RED "menu file too large: %s is %i, max allowed is %i", menuFile, len, MAX_MENUDEFFILE ) );
		cgi.FS_FCloseFile( f );
		return;
	}

	cgi.FS_Read( buf, len, f );
	buf[len] = 0;
	cgi.FS_FCloseFile( f );
	
	p = buf;

	while ( 1 ) 
	{
		token = COM_ParseExt( &p, qtrue );
		if( !token || token[0] == 0 || token[0] == '}') 
		{
			break;
		}

		if ( Q_stricmp( token, "}" ) == 0 ) 
		{
			break;
		}

		if (Q_stricmp(token, "loadmenu") == 0) 
		{
			if (CG_Load_Menu(&p)) 
			{
				continue;
			} 
			else 
			{
				break;
			}
		}
	}

	//Com_Printf("UI menu load time = %d milli seconds\n", cgi_Milliseconds() - start);
}

/*
=================
CG_LoadHudMenu();

=================
*/
void CG_LoadHudMenu() 
{
	const char *hudSet;

	cgDC.registerShaderNoMip = &cgi.R_RegisterShaderNoMip;
	cgDC.setColor = &cgi.R_SetColor;
	cgDC.drawHandlePic = &CG_DrawPic;
	cgDC.drawStretchPic = &cgi.R_DrawStretchPic;
	cgDC.drawText = reinterpret_cast<void (__cdecl *)(float, float, float, vec_t [], const char *, float, int, int, int)>(&CG_Text_Paint);
	cgDC.textWidth = &CG_Text_Width;
	cgDC.textHeight = &CG_Text_Height;
	cgDC.registerModel = &cgi.R_RegisterModel;
	cgDC.modelBounds = &cgi.R_ModelBounds;
	cgDC.fillRect = &CG_FillRect;
	cgDC.drawRect = &CG_DrawRect;   
	cgDC.drawSides = &CG_DrawSides;
	cgDC.drawTopBottom = &CG_DrawTopBottom;
	cgDC.clearScene = &cgi.R_ClearScene;
	cgDC.addRefEntityToScene = &cgi.R_AddRefEntityToScene;
	cgDC.renderScene = &cgi.R_RenderScene;
	cgDC.RegisterFont = &cgi.R_RegisterFont;
	cgDC.Font_StrLenPixels = &cgi.R_Font_StrLenPixels;
	cgDC.Font_StrLenChars = &cgi.R_Font_StrLenChars;
	cgDC.Font_HeightPixels = &cgi.R_Font_HeightPixels;
	cgDC.Font_DrawString = &cgi.R_Font_DrawString;
	cgDC.Language_IsAsian = &cgi.Language_IsAsian;
	cgDC.Language_UsesSpaces = &cgi.Language_UsesSpaces;
	cgDC.AnyLanguage_ReadCharFromString = &cgi.Language_ReadCharFromString;
	cgDC.ownerDrawItem = ( void ( * )( itemDef_t *, float, float, float, float, float, float, int, int, int, float, float, vec4_t, qhandle_t, int, int, int )) &CG_OwnerDraw;
	cgDC.getValue = &CG_GetValue;
	cgDC.ownerDrawVisible = &CG_OwnerDrawVisible;
	cgDC.runScript = &CG_RunMenuScript;
	cgDC.deferScript = &CG_DeferMenuScript;
	cgDC.getTeamColor = &CG_GetTeamColor;
	cgDC.setCVar = cgi.Cvar_Set;
	cgDC.getCVarString = cgi.Cvar_VariableStringBuffer;
	cgDC.getCVarValue = CG_Cvar_Get;
	cgDC.drawTextWithCursor = &CG_Text_PaintWithCursor;
	//cgDC.setOverstrikeMode = &trap_Key_SetOverstrikeMode;
	//cgDC.getOverstrikeMode = &trap_Key_GetOverstrikeMode;
	cgDC.startLocalSound = &cgi.S_StartLocalSound;
	cgDC.ownerDrawHandleKey = &CG_OwnerDrawHandleKey;
	cgDC.feederCount = &CG_FeederCount;
	cgDC.feederItemImage = &CG_FeederItemImage;
	cgDC.feederItemText = &CG_FeederItemText;
	cgDC.feederSelection = &CG_FeederSelection;
	//cgDC.setBinding = &cgi.Key_SetBinding;
	//cgDC.getBindingBuf = &cgi.Key_GetBindingBuf;
	//cgDC.keynumToStringBuf = &cgi.Key_KeynumToStringBuf;
	//cgDC.executeText = &cgi.Cmd_ExecuteText;
	cgDC.Error = &Com_Error; 
	cgDC.Print = &Com_Printf; 
	cgDC.ownerDrawWidth = &CG_OwnerDrawWidth;
	//cgDC.Pause = &CG_Pause;
	cgDC.registerSound = &cgi.S_RegisterSound;
	cgDC.startBackgroundTrack = &cgi.S_StartBackgroundTrack;
	cgDC.stopBackgroundTrack = &cgi.S_StopBackgroundTrack;
	cgDC.playCinematic = &CG_PlayCinematic;
	cgDC.stopCinematic = &CG_StopCinematic;
	cgDC.drawCinematic = &CG_DrawCinematic;
	cgDC.runCinematicFrame = &CG_RunCinematicFrame;
	
	Init_Display(&cgDC);

	Menu_Reset();

	hudSet = cg_hudFiles.string;
	if (hudSet[0] == '\0') 
	{
		hudSet = "ui/jahud.txt";
	}

	CG_LoadMenus(hudSet);
	CG_LoadMenus("ui/jkghud.txt");

}

void CG_AssetCache() {
	//if (Assets.textFont == NULL) {
	//  cgi.R_RegisterFont("fonts/arial.ttf", 72, &Assets.textFont);
	//}
	//Assets.background = cgi.R_RegisterShaderNoMip( ASSET_BACKGROUND );
	//Com_Printf("Menu Size: %i bytes\n", sizeof(Menus));
	cgDC.Assets.gradientBar = cgi.R_RegisterShaderNoMip( ASSET_GRADIENTBAR );
	cgDC.Assets.fxBasePic = cgi.R_RegisterShaderNoMip( ART_FX_BASE );
	cgDC.Assets.fxPic[0] = cgi.R_RegisterShaderNoMip( ART_FX_RED );
	cgDC.Assets.fxPic[1] = cgi.R_RegisterShaderNoMip( ART_FX_YELLOW );
	cgDC.Assets.fxPic[2] = cgi.R_RegisterShaderNoMip( ART_FX_GREEN );
	cgDC.Assets.fxPic[3] = cgi.R_RegisterShaderNoMip( ART_FX_TEAL );
	cgDC.Assets.fxPic[4] = cgi.R_RegisterShaderNoMip( ART_FX_BLUE );
	cgDC.Assets.fxPic[5] = cgi.R_RegisterShaderNoMip( ART_FX_CYAN );
	cgDC.Assets.fxPic[6] = cgi.R_RegisterShaderNoMip( ART_FX_WHITE );
	cgDC.Assets.scrollBar = cgi.R_RegisterShaderNoMip( ASSET_SCROLLBAR );
	cgDC.Assets.scrollBarArrowDown = cgi.R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWDOWN );
	cgDC.Assets.scrollBarArrowUp = cgi.R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWUP );
	cgDC.Assets.scrollBarArrowLeft = cgi.R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWLEFT );
	cgDC.Assets.scrollBarArrowRight = cgi.R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWRIGHT );
	cgDC.Assets.scrollBarThumb = cgi.R_RegisterShaderNoMip( ASSET_SCROLL_THUMB );
	cgDC.Assets.sliderBar = cgi.R_RegisterShaderNoMip( ASSET_SLIDER_BAR );
	cgDC.Assets.sliderThumb = cgi.R_RegisterShaderNoMip( ASSET_SLIDER_THUMB );
}

/*


/*
Ghoul2 Insert Start
*/

// initialise the cg_entities structure - take into account the ghoul2 stl stuff in the active snap shots
void CG_Init_CG(void)
{
#ifdef _XBOX
	qboolean widescreen = cg.widescreen;
#endif
	memset( &cg, 0, sizeof(cg));
	cg.ironsightsBlend = 1.0f;
	cg.sprintBlend = 1.0f;
#ifdef _XBOX
	cg.widescreen = widescreen;
#endif
	memset( cg.playerACI, -1, sizeof(cg.playerACI) );
}

#ifdef _XBOX
void CG_SetWidescreen(qboolean widescreen)
{
	cg.widescreen = widescreen;
}
#endif


// initialise the cg_entities structure - take into account the ghoul2 stl stuff
void CG_Init_CGents(void)
{
	
	memset(&cg_entities, 0, sizeof(cg_entities));
}


void CG_InitItems(void)
{
	memset( cg_items, 0, sizeof( cg_items ) );
}

void CG_TransitionPermanent(void)
{
	centity_t	*cent = cg_entities;
	int			i;

	cg_numpermanents = 0;
	for(i=0;i<MAX_GENTITIES;i++,cent++)
	{
		if (cgi.GetDefaultState(i, &cent->currentState))
		{
			cent->nextState = cent->currentState;
			VectorCopy (cent->currentState.origin, cent->lerpOrigin);
			VectorCopy (cent->currentState.angles, cent->lerpAngles);
			cent->currentValid = qtrue;

			cg_permanents[cg_numpermanents++] = cent;
		}
	}
}


//this is a 32k custom pool for parsing ents, it can get reset between ent parsing
//so we don't need a whole lot of memory -rww
#define MAX_CGSTRPOOL_SIZE		32768
static int cg_strPoolSize = 0;
static byte cg_strPool[MAX_CGSTRPOOL_SIZE];

char *CG_StrPool_Alloc(int size)
{
	char *giveThemThis;

	if (cg_strPoolSize+size >= MAX_CGSTRPOOL_SIZE)
	{
		Com_Error(ERR_DROP, "You exceeded the cgame string pool size. Bad programmer!\n");
	}

	giveThemThis = (char *) &cg_strPool[cg_strPoolSize];
	cg_strPoolSize += size;

	//memset it for them, just to be nice.
	memset(giveThemThis, 0, size);

	return giveThemThis;
}

void CG_StrPool_Reset(void)
{
	cg_strPoolSize = 0;
}

/*
=============
CG_NewString

Builds a copy of the string, translating \n to real linefeeds
so message texts can be multi-line
=============
*/
char *CG_NewString( const char *string )
{
	char	*newb, *new_p;
	int		i,l;
	
	l = strlen(string) + 1;

	newb = CG_StrPool_Alloc( l );

	new_p = newb;

	// turn \n into a real linefeed
	for ( i=0 ; i< l ; i++ ) {
		if (string[i] == '\\' && i < l-1) {
			i++;
			if (string[i] == 'n') {
				*new_p++ = '\n';
			} else {
				*new_p++ = '\\';
			}
		} else {
			*new_p++ = string[i];
		}
	}
	
	return newb;
}

//data to grab our spawn info into
typedef struct cgSpawnEnt_s
{
	char		*classname;
	vec3_t		origin;
	vec3_t		angles;
	float		angle;
	vec3_t		scale;
	float		fScale;
	vec3_t		mins;
	vec3_t		maxs;
	char		*model;
	float		zoffset;
	int			onlyFogHere;
	float		fogstart;
	float		radarrange;
} cgSpawnEnt_t;

#define	CGFOFS(x) ((int)&(((cgSpawnEnt_t *)0)->x))

//spawn fields for our cgame "entity"
BG_field_t cg_spawnFields[] =
{
	{"classname", CGFOFS(classname), F_LSTRING},
	{"origin", CGFOFS(origin), F_VECTOR},
	{"angles", CGFOFS(angles), F_VECTOR},
	{"angle", CGFOFS(angle), F_FLOAT},
	{"modelscale", CGFOFS(fScale), F_FLOAT},
	{"modelscale_vec", CGFOFS(scale), F_VECTOR},
	{"model", CGFOFS(model), F_LSTRING},
	{"mins", CGFOFS(mins), F_VECTOR},
	{"maxs", CGFOFS(maxs), F_VECTOR},
	{"zoffset", CGFOFS(zoffset), F_FLOAT},
	{"onlyfoghere", CGFOFS(onlyFogHere), F_INT},
	{"fogstart", CGFOFS(fogstart), F_FLOAT},
	{"radarrange", CGFOFS(radarrange), F_FLOAT},
	{NULL}
};

static int cg_numSpawnVars;
static int cg_numSpawnVarChars;
static char *cg_spawnVars[MAX_SPAWN_VARS][2];
static char cg_spawnVarChars[MAX_SPAWN_VARS_CHARS];

//get some info from the skyportal ent on the map
qboolean cg_noFogOutsidePortal = qfalse;
void CG_CreateSkyPortalFromSpawnEnt(cgSpawnEnt_t *ent)
{
	if (ent->onlyFogHere)
	{ //only globally fog INSIDE the sky portal
		cg_noFogOutsidePortal = qtrue;
	}
}

//create a skybox portal orientation entity. there -should- only
//be one of these things per level. if there's more than one the
//next will just stomp over the last. -rww
qboolean cg_skyOri = qfalse;
vec3_t cg_skyOriPos;
float cg_skyOriScale = 0.0f;
void CG_CreateSkyOriFromSpawnEnt(cgSpawnEnt_t *ent)
{
    cg_skyOri = qtrue;
	VectorCopy(ent->origin, cg_skyOriPos);
	cg_skyOriScale = ent->fScale;
}

//get brush box extents, note this does not care about bsp instances.
void CG_CreateBrushEntData(cgSpawnEnt_t *ent)
{
	cgi.R_ModelBounds(cgi.R_RegisterModel(ent->model), ent->mins, ent->maxs);
}

void CG_CreateWeatherZoneFromSpawnEnt(cgSpawnEnt_t *ent)
{
	CG_CreateBrushEntData(ent);
	cgi.WE_AddWeatherZone(ent->mins, ent->maxs);
}

//create a new cgame-only model
void CG_CreateModelFromSpawnEnt(cgSpawnEnt_t *ent)
{
	int			modelIndex;
	refEntity_t	*RefEnt;
	vec3_t		mins, maxs;
	float		*radius;
	float		*zOff;

	if (NumMiscEnts >= MAX_MISC_ENTS)
	{
		Com_Error(ERR_DROP, "Too many misc_model_static's on level, ask a programmer to raise the limit (currently %i), or take some out.", MAX_MISC_ENTS);
		return;
	}
	
	if (!ent || !ent->model || !ent->model[0])
	{
		Com_Error(ERR_DROP, "misc_model_static with no model.");
		return;
	}

	radius = &Radius[NumMiscEnts];
	zOff = &zOffset[NumMiscEnts];
	RefEnt = &MiscEnts[NumMiscEnts++];

	modelIndex = cgi.R_RegisterModel(ent->model);
	
	if (modelIndex == 0)
	{
		//Com_Error(ERR_DROP, "misc_model_static failed to load model '%s'",ent->model);
		Com_Printf("misc_model_static failed to load model '%s'.\n",ent->model);
		return;
	}

	memset(RefEnt, 0, sizeof(refEntity_t));
	RefEnt->reType = RT_MODEL;
	RefEnt->hModel = modelIndex;
	RefEnt->frame = 0;
	cgi.R_ModelBounds(modelIndex, mins, maxs);
	VectorCopy(ent->scale, RefEnt->modelScale);
	if (ent->fScale)
	{ //use same scale on each axis then
		RefEnt->modelScale[0] = RefEnt->modelScale[1] = RefEnt->modelScale[2] = ent->fScale;
	}
	VectorCopy(ent->origin, RefEnt->origin);
	VectorCopy(ent->origin, RefEnt->lightingOrigin);

	//[Invalid Model Bounds Fix]
	//VectorScaleVector(mins, ent->scale, mins);
	//VectorScaleVector(maxs, ent->scale, maxs);
	VectorScaleVector(mins, RefEnt->modelScale, mins);
	VectorScaleVector(maxs, RefEnt->modelScale, maxs);
	//[/Invalid Model Bounds Fix]
	*radius = Distance(mins, maxs);
	*zOff = ent->zoffset;

	if (ent->angle)
	{ //only yaw supplied...
		ent->angles[YAW] = ent->angle;
	}

	AnglesToAxis( ent->angles, RefEnt->axis );
	ScaleModelAxis(RefEnt);
}

/*
====================
CG_AddSpawnVarToken
====================
*/
char *CG_AddSpawnVarToken( const char *string )
{
	int		l;
	char	*dest;

	l = strlen( string );
	if ( cg_numSpawnVarChars + l + 1 > MAX_SPAWN_VARS_CHARS ) {
		CG_Error( "CG_AddSpawnVarToken: MAX_SPAWN_VARS" );
	}

	dest = cg_spawnVarChars + cg_numSpawnVarChars;
	memcpy( dest, string, l+1 );

	cg_numSpawnVarChars += l + 1;

	return dest;
}

/*
====================
CG_ParseSpawnVars

cgame version of G_ParseSpawnVars, for ents that don't really
need to take up an entity slot (e.g. static models) -rww
====================
*/
qboolean CG_ParseSpawnVars( void )
{
	char		keyname[MAX_TOKEN_CHARS];
	char		com_token[MAX_TOKEN_CHARS];

	cg_numSpawnVars = 0;
	cg_numSpawnVarChars = 0;

	// parse the opening brace
	if ( !cgi.GetEntityToken( com_token, sizeof( com_token ) ) ) {
		// end of spawn string
		return qfalse;
	}
	if ( com_token[0] != '{' ) {
		CG_Error( "CG_ParseSpawnVars: found %s when expecting {",com_token );
	}

	// go through all the key / value pairs
	while ( 1 )
	{	
		// parse key
		if ( !cgi.GetEntityToken( keyname, sizeof( keyname ) ) )
		{
			CG_Error( "CG_ParseSpawnVars: EOF without closing brace" );
		}

		if ( keyname[0] == '}' )
		{
			break;
		}
		
		// parse value	
		if ( !cgi.GetEntityToken( com_token, sizeof( com_token ) ) )
		{ //this happens on mike's test level, I don't know why. Fixme?
			//CG_Error( "CG_ParseSpawnVars: EOF without closing brace" );
			break;
		}

		if ( com_token[0] == '}' )
		{
			CG_Error( "CG_ParseSpawnVars: closing brace without data" );
		}
		if ( cg_numSpawnVars == MAX_SPAWN_VARS )
		{
			CG_Error( "CG_ParseSpawnVars: MAX_SPAWN_VARS" );
		}
		cg_spawnVars[ cg_numSpawnVars ][0] = CG_AddSpawnVarToken( keyname );
		cg_spawnVars[ cg_numSpawnVars ][1] = CG_AddSpawnVarToken( com_token );
		cg_numSpawnVars++;
	}

	return qtrue;
}

/*
==============
CG_SpawnCGameEntFromVars

See if we should do something for this ent cgame-side -rww
==============
*/
void BG_ParseField( BG_field_t *l_fields, const char *key, const char *value, byte *ent );

extern float cg_linearFogOverride; //cg_view.c
extern float cg_radarRange;//cg_draw.c
void CG_SpawnCGameEntFromVars(void)
{
	int i;
	cgSpawnEnt_t ent;

	memset(&ent, 0, sizeof(cgSpawnEnt_t));

	for (i = 0; i < cg_numSpawnVars; i++)
	{ //shove all this stuff into our data structure used specifically for getting spawn info
		BG_ParseField( cg_spawnFields, cg_spawnVars[i][0], cg_spawnVars[i][1], (byte *)&ent );
	}

	if (ent.classname && ent.classname[0])
	{ //we'll just stricmp this bastard, since there aren't all that many cgame-only things, and they all have special handling
		if (!Q_stricmp(ent.classname, "worldspawn"))
		{ //I'd like some info off this guy
            if (ent.fogstart)
			{ //linear fog method
				cg_linearFogOverride = ent.fogstart;
			}
			//get radarRange off of worldspawn
            if (ent.radarrange)
			{ //linear fog method
				cg_radarRange = ent.radarrange;
			}
		}
		else if (!Q_stricmp(ent.classname, "misc_model_static"))
		{ //we've got us a static model
            CG_CreateModelFromSpawnEnt(&ent);			
		}
		else if (!Q_stricmp(ent.classname, "misc_skyportal_orient"))
		{ //a sky portal orientation point
            CG_CreateSkyOriFromSpawnEnt(&ent);            
		}
		else if (!Q_stricmp(ent.classname, "misc_skyportal"))
		{ //might as well parse this thing cgame side for the extra info I want out of it
			CG_CreateSkyPortalFromSpawnEnt(&ent);            
		}
		else if (!Q_stricmp(ent.classname, "misc_weather_zone"))
		{ //might as well parse this thing cgame side for the extra info I want out of it
			CG_CreateWeatherZoneFromSpawnEnt(&ent);
		}
	}

	//reset the string pool for the next entity, if there is one
    CG_StrPool_Reset();
}

/*
==============
CG_SpawnCGameOnlyEnts

Parses entity string data for cgame-only entities, that we can throw away on
the server and never even bother sending. -rww
==============
*/
void CG_SpawnCGameOnlyEnts(void)
{
	//make sure it is reset
	cgi.GetEntityToken(NULL, -1);

	if (!CG_ParseSpawnVars())
	{ //first one is gonna be the world spawn
		CG_Error("no entities for cgame parse");
	}
	else
	{ //parse the world spawn info we want
		CG_SpawnCGameEntFromVars();
	}

	while(CG_ParseSpawnVars())
	{ //now run through the whole list, and look for things we care about cgame-side
		CG_SpawnCGameEntFromVars();
	}		
}

/*
Ghoul2 Insert End
*/

extern playerState_t *cgSendPS[MAX_GENTITIES]; //is not MAX_CLIENTS because NPCs exceed MAX_CLIENTS
void CG_PmoveClientPointerUpdate();

void WP_SaberLoadParms( void );
void BG_VehicleLoadParms( void );


/*
=================
CG_Init

Called after every level change or subsystem restart
Will perform callbacks to make the loading info screen update.
=================
*/

void CinBuild_Init();
void ChatBox_InitSystem();
void MiniMap_Init();
void JKG_WeaponIndicators_Init();

#include "jkg_cg_auxlib.h"
#include "jkg_chatcmds.h"

static void CG_OpenPartyManagement( void ) {
	uiImports->PartyMngtNotify( 10 );
}

static void CG_OpenInventory ( void )
{
	uiImports->InventoryNotify( 0 );
}

void CG_SetupChatCmds() {
	//CCmd_AddCommand("party", CG_OpenPartyManagement);
	CCmd_AddCommand ("inventory", CG_OpenInventory);
}

extern void JKG_CG_InitItems( void );
extern void JKG_CG_InitArmor( void );
extern void CG_InitializeCrossoverAPI( void );
void CG_Init( int serverMessageNum, int serverCommandSequence, int clientNum )
{
	static gitem_t *item;
	char buf[64];
	const char	*s;
	int i = 0;

	cgame_initializing = qtrue;

	// Do the engine patches
	ChatBox_InitSystem();
	cgi.Cvar_Set("connmsg", ""); // Clear connection message override

	BG_InitAnimsets(); //clear it out

	cgi.CG_RegisterSharedMemory(cg.sharedBuffer);

	//Load external vehicle data
	BG_VehicleLoadParms();

	CG_InitializeCrossoverAPI();

	// clear everything
/*
Ghoul2 Insert Start
*/

//	memset( cg_entities, 0, sizeof( cg_entities ) );
	CG_Init_CGents();
// this is a No-No now we have stl vector classes in here.
//	memset( &cg, 0, sizeof( cg ) );
	CG_Init_CG();
	CG_InitItems();

	//create the global jetpack instance
	CG_InitJetpackGhoul2();

	CG_PmoveClientPointerUpdate();
	
	// Yum, ammo
	BG_InitializeAmmo();
	
	/* Initialize the weapon data table */
	BG_InitializeWeapons();

	JKG_InitializeConstants();

	/* Here be crystals */
	JKG_InitializeSaberCrystalData();

	// and here is some stance data too
	JKG_InitializeStanceData();

	// Jedi Knight Galaxies
	CinBuild_Init();
	CG_SetupChatCmds();
/*
Ghoul2 Insert End
*/

	//Load sabers.cfg data
	WP_SaberLoadParms();

	// this is kinda dumb as well, but I need to pre-load some fonts in order to have the text available
	//	to say I'm loading the assets.... which includes loading the fonts. So I'll set these up as reasonable
	//	defaults, then let the menu asset parser (which actually specifies the ingame fonts) load over them
	//	if desired during parse.  Dunno how legal it is to store in these cgDC things, but it causes no harm
	//	and even if/when they get overwritten they'll be legalised by the menu asset parser :-)
//	CG_LoadFonts();
	cgDC.Assets.qhSmallFont  = cgi.R_RegisterFont("ocr_a");
	cgDC.Assets.qhMediumFont = cgi.R_RegisterFont("ergoec");
	cgDC.Assets.qhSmall3Font = cgi.R_RegisterFont("bankgothic");
	cgDC.Assets.qhSmall4Font = cgi.R_RegisterFont("segoeui");
	cgDC.Assets.qhBigFont = cgDC.Assets.qhMediumFont;

	memset( &cgs, 0, sizeof( cgs ) );
	CG_InitWeapons();

	cg.clientNum = clientNum;

	cgs.processedSnapshotNum = serverMessageNum;
	cgs.serverCommandSequence = serverCommandSequence;

	cg.loadLCARSStage		= 0;

	cg.itemSelect = -1;
	cg.forceSelect = -1;
	
	// load a few needed things before we do any screen updates
	cgs.media.charsetShader		= cgi.R_RegisterShaderNoMip( "gfx/2d/charsgrid_med" );
	cgs.media.whiteShader		= cgi.R_RegisterShader( "white" );

	//Jedi Knight Galaxies: more char sets
	cgs.media.charset_Arial		= cgi.R_RegisterShaderNoMip( "gfx/2d/charsgrid_arial" );
	cgs.media.charset_Courier	= cgi.R_RegisterShaderNoMip( "gfx/2d/charsgrid_courier" );
	cgs.media.charset_Fixedsys	= cgi.R_RegisterShaderNoMip( "gfx/2d/charsgrid_fixedsys" );
	cgs.media.charset_Segoeui	= cgi.R_RegisterShaderNoMip( "gfx/2d/charsgrid_chat" );

	cgs.media.loadBarLED		= cgi.R_RegisterShaderNoMip( "gfx/hud/load_tick" );
	cgs.media.loadBarLEDCap		= cgi.R_RegisterShaderNoMip( "gfx/hud/load_tick_cap" );
	cgs.media.loadBarLEDSurround= cgi.R_RegisterShaderNoMip( "gfx/hud/mp_levelload" );

	// Force HUD set up
	cg.forceHUDActive = qtrue;
	cg.forceHUDTotalFlashTime = 0;
	cg.forceHUDNextFlashTime = 0;

	cg.numItemsInInventory = 0;

	//eezstreet add
	JKG_CG_InitItems();
	JKG_CG_InitArmor();
	BG_LoadDefaultWeaponItems();

	/*i = WP_NONE+1;
	while (i <= LAST_USEABLE_WEAPON)
	{
		item = BG_FindItemForWeapon(i);

		if (item && item->icon && item->icon[0])
		{
			cgs.media.weaponIcons[i] = cgi.R_RegisterShaderNoMip(item->icon);
			cgs.media.weaponIcons_NA[i] = cgi.R_RegisterShaderNoMip(va("%s_na", item->icon));
		}
		else
		{ //make sure it is zero'd (default shader)
			cgs.media.weaponIcons[i] = 0;
			cgs.media.weaponIcons_NA[i] = 0;
		}
		i++;
	}*/

	cgi.Cvar_VariableStringBuffer("com_buildscript", buf, sizeof(buf));
	if (atoi(buf))
	{
		cgi.R_RegisterShaderNoMip("gfx/hud/w_icon_saberstaff");
		cgi.R_RegisterShaderNoMip("gfx/hud/w_icon_duallightsaber");
	}
	i = 0;

	// HUD artwork for cycling inventory,weapons and force powers 
	cgs.media.weaponIconBackground		= cgi.R_RegisterShaderNoMip( "gfx/hud/background");
	cgs.media.forceIconBackground		= cgi.R_RegisterShaderNoMip( "gfx/hud/background_f");
	cgs.media.inventoryIconBackground	= cgi.R_RegisterShaderNoMip( "gfx/hud/background_i");

	//rww - precache holdable item icons here
	while (i < bg_numItems)
	{
		if (bg_itemlist[i].giType == IT_HOLDABLE)
		{
			if (bg_itemlist[i].icon)
			{
				cgs.media.invenIcons[bg_itemlist[i].giTag] = cgi.R_RegisterShaderNoMip(bg_itemlist[i].icon);
			}
			else
			{
				cgs.media.invenIcons[bg_itemlist[i].giTag] = 0;
			}
		}

		i++;
	}

	//rww - precache force power icons here
	i = 0;

	while (i < NUM_FORCE_POWERS)
	{
		cgs.media.forcePowerIcons[i] = cgi.R_RegisterShaderNoMip(HolocronIcons[i]);

		i++;
	}
	cgs.media.rageRecShader = cgi.R_RegisterShaderNoMip("gfx/mp/f_icon_ragerec");


	//body decal shaders -rww
	cgs.media.bdecal_bodyburn1 = cgi.R_RegisterShader("gfx/damage/bodyburnmark1");
	cgs.media.bdecal_saberglow = cgi.R_RegisterShader("gfx/damage/saberglowmark");
	cgs.media.bdecal_burn1 = cgi.R_RegisterShader("gfx/damage/bodybigburnmark1");
	cgs.media.mSaberDamageGlow = cgi.R_RegisterShader("gfx/effects/saberDamageGlow");

	//MB_InitMotionBlur();

	CG_RegisterCvars();

	CG_InitConsoleCommands();

	cg.weaponSelect = 0;

	cgs.redflag = cgs.blueflag = -1; // For compatibily, default to unset for
	cgs.flagStatus = -1;
	// old servers

	// get the rendering configuration from the client system
	cgi.GetGlconfig( &cgs.glconfig );
	cgs.screenXScale = cgs.glconfig.vidWidth / 640.0;
	cgs.screenYScale = cgs.glconfig.vidHeight / 480.0;

	// get the gamestate from the client system
	cgi.GetGameState( &cgs.gameState );

	CG_TransitionPermanent(); //rwwRMG - added

	// check version
	s = CG_ConfigString( CS_GAME_VERSION );
	if ( strcmp( s, GAME_VERSION ) ) {
		//CG_Error( "Client/Server game mismatch: %s/%s", GAME_VERSION, s );
	}

	s = CG_ConfigString( CS_LEVEL_START_TIME );
	cgs.levelStartTime = atoi( s );

	CG_ParseServerinfo();

	// load the new map
//	CG_LoadingString( "collision map" );
	cgi.CM_LoadMap( cgs.mapname, qfalse );
	
#ifdef __DISABLED
	JKG_Nav_Init (cgs.mapname);
#endif

	String_Init();

	cg.loading = qtrue;		// force players to load instead of defer

	//CG_LoadingString ( "TrueView" );
	//[TrueView]
	CG_TrueViewInit();
	//[/TrueView]

	//CG_LoadingString ( "Sounds" );
	CG_RegisterSounds();

	//CG_LoadingString( "Graphics" );
	if(cgs.gametype >= GT_TEAM)
	{
		// Gang Wars stuff
		char *info = (char *)CG_ConfigString( CS_TEAMS );
		JKG_BG_GangWarsInit();
		cgs.redTeam = JKG_GetTeamByReference( Info_ValueForKey( info, "redTeam" ) );
		cgs.blueTeam = JKG_GetTeamByReference( Info_ValueForKey( info, "blueTeam" ) );
	}

	CG_RegisterGraphics();

	//CG_LoadingString( "Clients" );

	CG_RegisterClients();		// if low on memory, some clients will be deferred

	CG_AssetCache();
	CG_LoadHudMenu();      // load new hud stuff

	cg.loading = qfalse;	// future players will be deferred

	CG_InitLocalEntities();

	CG_InitMarkPolys();

	// Load the minimaps
	MiniMap_Init();
	// remove the last loading update
	cg.infoScreenText[0] = 0;

	// Make sure we have update values (scores)
	CG_SetConfigValues();

	CG_StartMusic(qfalse);

//	CG_LoadingString( "Clearing light styles" );
	CG_ClearLightStyles();

//	CG_LoadingString( "Creating automap data" );
	//init automap
#ifndef _XBOX
	cgi.R_InitWireframeAutomap();
#endif

	CG_LoadingString( "" );

	CG_ShaderStateChanged();

	cgi.S_ClearLoopingSounds();

	cgi.R_GetDistanceCull(&cg.distanceCull);

	//now get all the cgame only cents
	CG_SpawnCGameOnlyEnts();

	/* Initialize the party list table */
	for ( i = 0; i < MAX_CLIENTS; i++ )
	{
		cgs.partyList[i].id = PARTY_SLOT_EMPTY;
	}

	for ( i = 0; i < PARTY_SLOT_INVITES; i++ )
	{
		cgs.party.invites[i].id = PARTY_SLOT_EMPTY;
	}

	if(cg.turnOnBlurCvar)
	{
		cgi.Cvar_Set("ui_blurbackground", "1");
		cg.turnOnBlurCvar = qfalse;
	}

	cgs.media.swfTestShader = cgi.R_RegisterShaderNoMip("animation/swf/test");

	cgame_initializing = qfalse;
}

//makes sure returned string is in localized format
const char *CG_GetLocationString(const char *loc)
{
	static char text[1024]={0};

	if (!loc || loc[0] != '@')
	{ //just a raw string
		return loc;
	}

	cgi.SP_GetStringTextString(loc+1, text, sizeof(text));
	return text;
}

//clean up all the ghoul2 allocations, the nice and non-hackly way -rww
void CG_KillCEntityG2(int entNum);
void CG_DestroyAllGhoul2(void)
{
	int i = 0;
	int j;

//	Com_Printf("... CGameside GHOUL2 Cleanup\n");
	while (i < MAX_GENTITIES)
	{ //free all dynamically allocated npc client info structs and ghoul2 instances
		CG_KillCEntityG2(i);	
		i++;
	}
	
	//Clean the weapon instances
	CG_ShutDownG2Weapons();

	i = 0;
	while (i < MAX_ITEMS)
	{ //and now for items
		j = 0;
		while (j < MAX_ITEM_MODELS)
		{
			if (cg_items[i].g2Models[j] && cgi.G2_HaveWeGhoul2Models(cg_items[i].g2Models[j]))
			{
				cgi.G2API_CleanGhoul2Models(&cg_items[i].g2Models[j]);
				cg_items[i].g2Models[j] = NULL;
			}
			j++;
		}
		i++;
	}

	//Clean the global jetpack instance
	CG_CleanJetpackGhoul2();
}

/*
=================
CG_Shutdown

Called before every level change or subsystem restart
=================
*/

void CG_Shutdown( void ) 
{
	BG_ClearAnimsets(); //free all dynamic allocations made through the engine

    CG_DestroyAllGhoul2();

	// Jedi Knight Galaxies, terminate the crossover
	cgi.CO_Shutdown();

//	Com_Printf("... FX System Cleanup\n");
	cgi.FX_FreeSystem();
	cgi.ROFF_Clean();
#ifdef __DISABLED
	JKG_Nav_Destroy();
#endif

	if (cgWeatherOverride)
	{
		cgi.R_WeatherContentsOverride(0); //rwwRMG - reset it engine-side
	}

	//reset weather
	cgi.R_WorldEffectCommand("die");

	UI_CleanupGhoul2();
	//If there was any ghoul2 stuff in our side of the shared ui code, then remove it now.

	// some mods may need to do cleanup work here,
	// like closing files or archiving session data
}

/*
===============
CG_NextForcePower_f
===============
*/
void CG_NextForcePower_f( void ) 
{
	int current;
	usercmd_t cmd;
	if ( !cg.snap )
	{
		return;
	}

	if (cg.predictedPlayerState.pm_type == PM_SPECTATOR)
	{
		return;
	}

	current = cgi.GetCurrentCmdNumber();
	cgi.GetUserCmd(current, &cmd);
	if ((cmd.buttons & BUTTON_USE) || CG_NoUseableForce())
	{
		CG_NextInventory_f();
		return;
	}

	if (cg.snap->ps.pm_flags & PMF_FOLLOW)
	{
		return;
	}

//	BG_CycleForce(&cg.snap->ps, 1);
	if (cg.forceSelect != -1)
	{
		cg.snap->ps.fd.forcePowerSelected = cg.forceSelect;
	}

	BG_CycleForce(&cg.snap->ps, 1);

	if (cg.snap->ps.fd.forcePowersKnown & (1 << cg.snap->ps.fd.forcePowerSelected))
	{
		cg.forceSelect = cg.snap->ps.fd.forcePowerSelected;
		cg.forceSelectTime = cg.time;
	}
}

/*
===============
CG_PrevForcePower_f
===============
*/
void CG_PrevForcePower_f( void ) 
{
	int current;
	usercmd_t cmd;
	if ( !cg.snap )
	{
		return;
	}

	if (cg.predictedPlayerState.pm_type == PM_SPECTATOR)
	{
		return;
	}

	current = cgi.GetCurrentCmdNumber();
	cgi.GetUserCmd(current, &cmd);
	if ((cmd.buttons & BUTTON_USE) || CG_NoUseableForce())
	{
		CG_PrevInventory_f();
		return;
	}

	if (cg.snap->ps.pm_flags & PMF_FOLLOW)
	{
		return;
	}

//	BG_CycleForce(&cg.snap->ps, -1);
	if (cg.forceSelect != -1)
	{
		cg.snap->ps.fd.forcePowerSelected = cg.forceSelect;
	}

	BG_CycleForce(&cg.snap->ps, -1);

	if (cg.snap->ps.fd.forcePowersKnown & (1 << cg.snap->ps.fd.forcePowerSelected))
	{
		cg.forceSelect = cg.snap->ps.fd.forcePowerSelected;
		cg.forceSelectTime = cg.time;
	}
}

void CG_NextInventory_f(void)
{
	if ( !cg.snap )
	{
		return;
	}

	if (cg.snap->ps.pm_flags & PMF_FOLLOW)
	{
		return;
	}

	if (cg.predictedPlayerState.pm_type == PM_SPECTATOR)
	{
		return;
	}

	if (cg.itemSelect != -1)
	{
		cg.snap->ps.stats[STAT_HOLDABLE_ITEM] = BG_GetItemIndexByTag(cg.itemSelect, IT_HOLDABLE);
	}
	BG_CycleInven(&cg.snap->ps, 1);

	if (cg.snap->ps.stats[STAT_HOLDABLE_ITEM])
	{
		cg.itemSelect = bg_itemlist[cg.snap->ps.stats[STAT_HOLDABLE_ITEM]].giTag;
		cg.invenSelectTime = cg.time;
	}
}

void CG_PrevInventory_f(void)
{
	if ( !cg.snap )
	{
		return;
	}

	if (cg.snap->ps.pm_flags & PMF_FOLLOW)
	{
		return;
	}

	if (cg.predictedPlayerState.pm_type == PM_SPECTATOR)
	{
		return;
	}

	if (cg.itemSelect != -1)
	{
		cg.snap->ps.stats[STAT_HOLDABLE_ITEM] = BG_GetItemIndexByTag(cg.itemSelect, IT_HOLDABLE);
	}
	BG_CycleInven(&cg.snap->ps, -1);

	if (cg.snap->ps.stats[STAT_HOLDABLE_ITEM])
	{
		cg.itemSelect = bg_itemlist[cg.snap->ps.stats[STAT_HOLDABLE_ITEM]].giTag;
		cg.invenSelectTime = cg.time;
	}
}



// GetGameAPI
// Call this from the executable

cgameImport_t cgi;

cgameExport_t* QDECL GetGameAPI( int apiVersion, cgameImport_t *import )
{
	if( apiVersion != CGAME_IMPORT_API_VERSION )
	{
		// rut-roh
		assert( 0 );
		return NULL;
	}

	cgi = *import;

	cgameExport_t cge;
	cge.APIversion = apiVersion;

	// TODO: finish this
}