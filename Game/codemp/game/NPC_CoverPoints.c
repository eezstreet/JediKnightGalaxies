#include "g_local.h"
#include "ai_main.h"

#ifdef __DOMINANCE_NPC__

extern gNPC_t		*NPCInfo;
extern int gWPNum;
extern wpobject_t *gWPArray[MAX_WPARRAY_SIZE];
extern int FindCloseList(int wpNum);
extern void ClearRoute( int Route[MAX_WPARRAY_SIZE] );
extern void AddtoRoute( int wpNum, int Route[MAX_WPARRAY_SIZE] );
extern qboolean OpenListEmpty(void);
extern int FindOpenList(int wpNum);
extern void AddCloseList( int openListpos );
extern void RemoveFirstOpenList( void );
extern float VectorDistanceNoHeight ( vec3_t v1, vec3_t v2 );
extern qboolean NPC_FacePosition( vec3_t position, qboolean doPitch );

extern void NPC_SetAnim(gentity_t *ent, int setAnimParts, int anim, int setAnimFlags);
extern qboolean PM_InKnockDown( playerState_t *ps );

extern int DOM_GetBestWaypoint(vec3_t org, int ignore, int badwp);
extern int DOM_FindIdealPathtoWP(bot_state_t *bs, int from, int to, int badwp2, int *pathlist);
extern int ASTAR_FindPath(int from, int to, int *pathlist);
extern int GetNearestWP(vec3_t org, int badwp);

extern qboolean JKG_CheckRoutingFrom( int wp );
extern qboolean JKG_CheckBelowWaypoint( int wp );

extern qboolean InFOV3( vec3_t spot, vec3_t from, vec3_t fromAngles, int hFOV, int vFOV );
extern void G_UcmdMoveForDir( gentity_t *self, usercmd_t *cmd, vec3_t dir );
extern qboolean DOM_Jedi_ClearPathToSpot( gentity_t *NPC, vec3_t dest, int impactEntNum );
extern void PM_SetPMViewAngle(playerState_t *ps, vec3_t angle, usercmd_t *ucmd);

extern visibility_t NPC_CheckVisibility ( gentity_t *ent, int flags );
extern gentity_t *NPC_PickEnemyExt( qboolean checkAlerts );
extern void G_AddVoiceEvent( gentity_t *self, int event, int speakDebounceTime );

#define CHECK_PVS		1
#define CHECK_360		2
#define CHECK_FOV		4
#define CHECK_SHOOT		8
#define CHECK_VISRANGE	16

// Conversations...
extern void NPC_NPCConversation();
extern void NPC_FindConversationPartner();
extern void NPC_StormTrooperConversation();


extern gentity_t *NPC;
extern usercmd_t ucmd;

int			num_cover_spots = 0;
int			cover_nodes[MAX_WPARRAY_SIZE];

qboolean AIMOD_LoadCoverPoints ( void )
{
	//FILE			*pIn;
	int				i = 0;
	fileHandle_t	f;
	int				num_map_waypoints = 0;
	vmCvar_t		mapname;

	// Init...
	num_cover_spots = 0;

	trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );

	trap_FS_FOpenFile( va( "nodes/%s.cpw", mapname.string), &f, FS_READ );

	if (!f)
	{
		G_Printf( "^1ERROR: Reading coverpoints from /nodes/%s.cpw failed\n", mapname.string );
		return qfalse;
	}

	trap_FS_Read( &num_map_waypoints, sizeof(int), f );

	if (num_map_waypoints != gWPNum)
	{// Is an old file! We need to make a new one!
		G_Printf( "^1*** ^3%s^5: Reading coverpoints from ^7/nodes/%s.cpw^3 failed ^5(old coverpoint file)^5!!!\n", GAME_VERSION, mapname.string );
		trap_FS_FCloseFile( f );
		return qfalse;
	}

	trap_FS_Read( &num_cover_spots, sizeof(int), f );	

	for ( i = 0; i < num_cover_spots; i++ )
	{
		int j = 0;

		trap_FS_Read( &(cover_nodes[i]), sizeof(int), f );

		if (!(gWPArray[cover_nodes[i]]->flags & WPFLAG_COVER))
			gWPArray[cover_nodes[i]]->flags |= WPFLAG_COVER;

		//CG_Printf("Cover spot #%i (node %i) is at %f %f %f.\n", i, cover_nodes[i], nodes[cover_nodes[i]].origin[0], nodes[cover_nodes[i]].origin[1], nodes[cover_nodes[i]].origin[2]);
	}

	trap_FS_FCloseFile( f );

	G_Printf( "^1*** ^3%s^5: Successfully loaded %i cover points from file ^7/nodes/%s.cpw^5.\n", GAME_VERSION, num_cover_spots, mapname.string);

	return qtrue;
}

int CoverOrgVisible ( vec3_t org1, vec3_t org2, int ignore )
{
	trace_t tr;
	trap_Trace( &tr, org1, NULL, NULL, org2, ignore, MASK_SOLID | MASK_OPAQUE | MASK_WATER );
	
	if ( tr.fraction == 1 )
	{
		return ( 1 );
	}

	if ( tr.entityNum >= 0 && tr.entityNum < ENTITYNUM_MAX_NORMAL )
	{
		gentity_t *ent = &g_entities[tr.entityNum];

		if (ent && ent->inuse)
		{
			if (ent->s.eType == ET_NPC || ent->s.eType == ET_PLAYER)
				return ( 1 );
		}
	}

	return ( 0 );
}

qboolean NPC_IsCoverpointFor ( int thisWP, gentity_t *enemy )
{
	vec3_t up_org, up_org2;

	VectorCopy(gWPArray[thisWP]->origin, up_org);
	up_org[2]+=DEFAULT_VIEWHEIGHT; // Standing height!

	VectorCopy(enemy->r.currentOrigin, up_org2);
	up_org2[2]+=DEFAULT_VIEWHEIGHT; // Standing height!

	if (!CoverOrgVisible(up_org, up_org2, -1))
		return qtrue;

	return qfalse;
}

qboolean NPC_CoverpointVisible ( gentity_t *NPC, int coverWP )
{
	vec3_t up_org, up_org2;

	VectorCopy(gWPArray[coverWP]->origin, up_org);
	up_org[2]+=18;//DEFAULT_VIEWHEIGHT; // Standing height!

	VectorCopy(NPC->r.currentOrigin, up_org2);
	up_org2[2]+=DEFAULT_VIEWHEIGHT; // Standing height!

	if (!CoverOrgVisible(up_org, up_org2, -1))
		return qtrue;

	return qfalse;
}

#endif //__DOMINANCE_NPC__
