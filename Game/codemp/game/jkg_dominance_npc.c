#include "g_local.h"
#include "ai_main.h"

#ifdef __DOMINANCE_NPC__

extern gNPC_t		*NPCInfo;
extern int gWPNum;
extern wpobject_t *gWPArray[MAX_WPARRAY_SIZE];
extern float VectorDistance(vec3_t v1, vec3_t v2);
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

extern gentity_t *NPC;
extern usercmd_t ucmd;

const char* NPCObjectiveList[] = {
	// True goals, multiple copies to make them more common...
	"misc_control_point",
	"flag",

	// Useable entities...
	"emplaced_gun",
	"misc_turret",
	"misc_turretG2",
	"misc_weapon_shooter",
	"misc_security_panel",
	"misc_ammo_floor_unit",
	"misc_shield_floor_unit",
	"misc_model_shield_power_converter",
	"misc_model_ammo_power_converter",
	"misc_model_health_power_converter",
	"func_usable",
	"func_button",

	// Lifts, doors, etc...
	"func_plat",
	"func_door",
	"func_useable",

	"trigger_door",
	"trigger_multiple",
	"target_speaker",
	"target_scriptrunner",
	"fx_runner",

	// Nav goals...
	"item_botroam",
	"point_combat",
	"NPC_goal",
	"waypoint_navgoal",
	"waypoint_navgoal_8",
	"waypoint_navgoal_4",
	"waypoint_navgoal_2",
	"waypoint_navgoal_1",
	"misc_siege_item",
	"info_siege_objective",

	// Ammo, guns, etc...
	"misc_model_gun_rack",
	"misc_model_ammo_rack",
	"ammo_blaster",
	"ammo_all",
	"ammo_thermal",
	"ammo_tripmine",
	"ammo_detpack",
	"ammo_force",
	"ammo_blaster",
	"ammo_powercell",
	"ammo_metallic_bolts",
	"ammo_rockets",
	"ammo_all",
	"weapon_stun_baton", 
	"weapon_melee", 
	"weapon_saber", 
	"weapon_bryar_pistol", 
	"weapon_bryar_pistol_old", 
	"weapon_blaster_pistol", 
	"weapon_concussion_rifle", 
	"weapon_bryar_pistol", 
	"weapon_blaster", 
	"weapon_disruptor",
	"weapon_bowcaster",
	"weapon_repeater", 
	"weapon_demp2", 
	"weapon_flechette", 
	"weapon_rocket_launcher",
	"weapon_thermal",
	"weapon_trip_mine", 
	"weapon_det_pack", 
	"weapon_emplaced", 
	"weapon_turretwp", 

	// Players...
	"player",

	"",
};

int NPCObjectives[MAX_GENTITIES];
int num_NPC_objectives = 0;

int next_goal_update = -1;

void Update_NPC_Goal_Lists ( void )
{// Create arrays containing up to date valid goal entity numbers...
	int loop = 0;

	// Initialize the max values...
	num_NPC_objectives = 0;

	for (loop = 0;loop < MAX_GENTITIES;loop++)
	{
		int nameNum = 0;

		for (nameNum = 0;NPCObjectiveList[nameNum] != "";nameNum++)
		{
			if (!Q_stricmp( g_entities[loop].classname, NPCObjectiveList[nameNum] ))
			{
				NPCObjectives[num_NPC_objectives] = loop;
				num_NPC_objectives++;
				break;
			}
		}
	}

	num_NPC_objectives--;

	if (num_NPC_objectives < 0)
		num_NPC_objectives = 0;

	G_Printf("NPC WAYPOINTING DEBUG: There are currently %i NPC objectives.\n", num_NPC_objectives);
}

int NPC_Find_Goal_EntityNum ( int ignoreEnt, int ignoreEnt2, vec3_t current_org, int teamNum )
{// Return standard goals only for soldiers...
	gentity_t *goal = NULL;
	int loop = 0;

	if (next_goal_update < level.time)
	{// Update new goal lists.. Unique1
		Update_NPC_Goal_Lists();
		next_goal_update = level.time + 10000;
	}

	if (num_NPC_objectives > 0)
	{
		int choice = NPCObjectives[Q_irand(0, num_NPC_objectives)];
	
		if (choice != ignoreEnt && choice != ignoreEnt2)
			return choice;
	}

	return -1;
}

extern qboolean JKG_CheckBelowWaypoint( int wp );
extern qboolean JKG_CheckRoutingFrom( int wp );

int NPC_FindGoal( gentity_t *NPC )
{
	/*
	int wp = -1;
	gentity_t *ent = NULL;

	ent = &g_entities[NPC_Find_Goal_EntityNum( -1, -1, NPC->r.currentOrigin, NPC->s.teamowner )];

	if (!ent || ent->s.teamowner == NPC->s.teamowner)
	{
		//G_Printf("Failed to find a goal entity!\n");
		//return -1;

		return irand(0, gWPNum-1); // UQ1: Will try using random waypoints...
	}

	//G_Printf("NPC_FindGoal: Found a goal entity %s (%i).\n", ent->classname, ent->s.number);

	wp = DOM_GetBestWaypoint(ent->s.origin, NPC->s.number, -1);

	return wp;
	*/




	// UQ1: Will just use random waypoints (wanderring) for now - as our maps have few entities to be used as goals...
	//return irand(0, gWPNum-1); // UQ1: Will try using random waypoints...
	



	/*int waypoint = irand(0, gWPNum-1);
	int tries = 0;

	while (gWPArray[waypoint]->inuse == qfalse || !JKG_CheckBelowWaypoint(waypoint) || !JKG_CheckRoutingFrom( waypoint ))
	{
		gWPArray[waypoint]->inuse = qfalse; // set it bad!

		if (tries >= 2)
		{
			return -1; // Try again on next check...
		}

		// Find a new one... This is probably a bad waypoint...
		waypoint = irand(0, gWPNum-1);
		tries++;
	}*/

	int waypoint = irand(0, gWPNum-1);

	if (gWPArray[waypoint]->inuse == qfalse || !JKG_CheckBelowWaypoint(waypoint) || !JKG_CheckRoutingFrom( waypoint ))
	{
		gWPArray[waypoint]->inuse = qfalse; // set it bad!
		return -1; // Try again on next check...
	}

	return waypoint;
}

#ifndef WAYPOINT_NONE
#define WAYPOINT_NONE -1
#endif //WAYPOINT_NONE

#define PATH_OK 0
#define PATH_BLOCKED 1

typedef struct wpneighborInfo_s
{
	int flags;
} wpneighborInfo_t;

typedef struct wpobjectinfo_s
{
	wpneighborInfo_t neighbors[MAX_NEIGHBOR_SIZE];
} wpobjectInfo_t;

wpobjectInfo_t *gWPArrayInfo[MAX_WPARRAY_SIZE];

qboolean WPFlagsInitialized = qfalse;

void InitWPLinkFlags ( void )
{// To be called externally to block bad points ingame...
	int wp = 0;
	int neighbor = 0;

	WPFlagsInitialized = qtrue;

	for (wp = 0; wp < gWPNum;wp++)
	{
		for (neighbor = 0; neighbor < MAX_NEIGHBOR_SIZE;neighbor++)
		{
			gWPArrayInfo[wp] = (wpobjectInfo_t *)malloc(sizeof(wpobjectInfo_t));
			gWPArrayInfo[wp]->neighbors[neighbor].flags = PATH_OK;
		}
	}
}

void MarkWPLinkBad ( gentity_t *NPC, int wp )
{// To be called externally to block bad points ingame...
	int neighbor = 0;
	int current_neighbor = 0;
	int wpLast = NPC->wpLast;

	if (!WPFlagsInitialized)
		InitWPLinkFlags();
}

//Find the ideal (shortest) route between the start wp and the end wp
//badwp is for situations where you need to recalc a path when you dynamically discover
//that a wp is bad (door locked, blocked, etc).
//doRoute = actually set botRoute
//float FindIdealPathtoWP(bot_state_t *bs, int start, int end, int badwp, bot_route_t Route)

#define NODE_INVALID -1
#define MAX_NODELINKS 32
#define MAX_PATHLIST_NODES MAX_WPARRAY_SIZE //4096

extern int BOT_GetFCost ( gentity_t *bot, int to, int num, int parentNum, float *gcost );
extern int DOM_GetBestWaypoint(vec3_t org, int ignore, int badwp);
extern int DOM_FindIdealPathtoWP(bot_state_t *bs, int from, int to, int badwp2, int *pathlist);

/*
===========================================================================
UpdategWPArray
Called every frame to update the unreachable/objective flags for gWPArray,
so that the NPCs are aware an obstruction has been removed, etc
===========================================================================
*/
void UpdategWPArray( void )
{
	int i, j;

	for (i = 0; i < gWPNum; i++)		//loop through all the gWPArray
	{
		//loop through all the links from this node
		for (j = 0; j < gWPArray[i]->neighbornum; j++)
		{
			if (gWPArrayInfo[i]->neighbors[j].flags & PATH_BLOCKED)	//if the path is blocked
			{
				trace_t tr;
				vec3_t start, end;

				VectorCopy(gWPArray[i]->origin, start);
				VectorCopy(gWPArray[gWPArray[i]->neighbors[j].num]->origin, end);
				
				//do a trace to see if there is still anything blocking the path
				trap_Trace(&tr, start, NULL, NULL, end, ENTITYNUM_NONE, MASK_DEADSOLID);

				//if the path is open, clear the 'blocked' flag
				if (tr.fraction == 1.0)
					gWPArrayInfo[i]->neighbors[j].flags &= ~PATH_BLOCKED;
			}
		}
	}
}

//find a given wpNum on the given route and return it's address.  return -1 if not on route.
//use wpNum = -1 to find the last wp on route.
int NPC_FindOnRoute( int wpNum, int Route[MAX_WPARRAY_SIZE] )
{
	int i;

	for( i=0; i < MAX_WPARRAY_SIZE; i++ )
	{
		//G_Printf("Checking %i = %i.\n", wpNum, Route[i]);

		if( wpNum == Route[i] )
		{//Success!
			return i;
		}
	}

	//Couldn't find it
	return -1;
}


/*///////////////////////////////////////////////////
NPCGetNextNode
if the NPC has reached a node, this function selects the next node
that he will go to, and returns it
right now it's being developed, feel free to experiment
*////////////////////////////////////////////////////

short int NPCGetNextNode(gentity_t *NPC)
{
	/*short*/ int node = WAYPOINT_NONE;
	int	temp = 0;

	//we should never call this in NPCSTATE_MOVE with no goal
	//setup the goal/path in HandleIdleState
	if (NPC->longTermGoal == WAYPOINT_NONE)
	{
		return WAYPOINT_NONE;
	}

	if (NPC->pathsize <= 0)	//if the NPC is at the end of his path, this shouldn't have been called
	{
		NPC->longTermGoal = WAYPOINT_NONE;	//reset to having no goal
		return WAYPOINT_NONE;
	}

	//node = NPC->pathlist[NPC->pathsize-1];	//pathlist is in reverse order
	//NPC->pathsize--;	//mark that we've moved another node
	temp = NPC_FindOnRoute( NPC->wpCurrent, NPC->pathlist );
	node = NPC->pathlist[temp-1];
	//G_Printf("Node found at pos %i in array.\n",temp);
	return node;
}

/*
===========================================================================
MyVisible
This is just a copy of the built-in 'visible' function
but accounting for the players' viewheight.  This lets the
bots see players slightly more often and in a more realistic way
===========================================================================
*/
qboolean MyVisible (gentity_t *self, gentity_t *other)
{
	vec3_t selfView, otherView;		//basically stores the locations of the players' eyes
	trace_t		tr;					//used in performing the collision info
	gentity_t	*traceEnt;			//holds the entity returned by the trace function

	if (other->s.eType != ET_NPC && other->s.eType != ET_PLAYER)
		return qfalse;

	if (!other->client)
		return qfalse;

	VectorCopy(self->r.currentOrigin, selfView);		//copy the bot's origin to this variable

	selfView[2] += self->client->ps.viewheight;			//add the bot's viewheight	

	VectorCopy(other->r.currentOrigin, otherView);		//copy the target's origin

	otherView[2] += other->client->ps.viewheight;		//add the target's viewheight

	//check if a shot from the bot's viewheight to the player's viewheight would hit
	trap_Trace (&tr, selfView, NULL, NULL, otherView, self->s.number, MASK_SHOT);

	traceEnt = &g_entities[tr.entityNum];		//set traceEnt to the entity the shot would hit

	if (traceEnt == other)		//if it would hit the player, return true
	{
		return qtrue;
	}

	return qfalse;				//it would hit something other than the player (or nothing) so return false

}

void NPC_FixBotWaypointNeighbors ( void )
{// Because JKA's waypoint neighbors don't include the nodes above and below... Grrr...
	int i = 0;
	int j = 0;

	if (gWPNum <= 0)
		return;

	G_Printf("^1*** ^3DominancE^5: Repairing waypoint database...\n");

	for (i = 0; i < gWPNum && gWPArray[i]->neighbornum < 32; i++)
	{
		vec3_t eye;

		VectorCopy(gWPArray[i]->origin, eye);
		eye[2]+=32;

		if (i - 1 < 0)
		{
			//if (OrgVisible(eye, gWPArray[i+1]->origin, ENTITYNUM_NONE))
			{
				gWPArray[i]->neighbornum++;
				gWPArray[i]->neighbors[gWPArray[i]->neighbornum].num = i+1;
			}
		}
		else if (i + 1 >= gWPNum)
		{
			//if (OrgVisible(eye, gWPArray[i-1]->origin, ENTITYNUM_NONE))
			{
				gWPArray[i]->neighbornum++;
				gWPArray[i]->neighbors[gWPArray[i]->neighbornum].num = i-1;
			}
		}
		else
		{
			//if (OrgVisible(eye, gWPArray[i+1]->origin, ENTITYNUM_NONE))
			{
				gWPArray[i]->neighbornum++;
				gWPArray[i]->neighbors[gWPArray[i]->neighbornum].num = i+1;
			}
			
			//if (OrgVisible(eye, gWPArray[i-1]->origin, ENTITYNUM_NONE))
			{
				gWPArray[i]->neighbornum++;
				gWPArray[i]->neighbors[gWPArray[i]->neighbornum].num = i-1;
			}
		}
	}

	G_Printf("^1*** ^3DominancE^5: Waypoint database repaired...\n");
}

//===========================================================================
// Routine      : NPC_MOVEMENT_ReachableBy
// Description  : Determine if a blocked goal vector is reachable.. Returns how to get there...
#define NOT_REACHABLE -1
#define REACHABLE 0
#define REACHABLE_JUMP 1
#define REACHABLE_DUCK 2
#define REACHABLE_STRAFE_LEFT 3
#define REACHABLE_STRAFE_RIGHT 4

static vec3_t	playerMins = {-15, -15, DEFAULT_MINS_2};
static vec3_t	playerMaxs = {15, 15, DEFAULT_MAXS_2};

int NPC_MOVEMENT_ReachableBy(gentity_t *NPC, vec3_t goal)
{
    trace_t trace;
    vec3_t v, v2, eyes, mins, maxs, Org, forward, right, up, rightOrigin, leftOrigin;

	// First check direct movement...
	VectorCopy(NPC->r.currentOrigin, Org);

    VectorCopy(NPC->r.mins,v);
    v[2] += 18; // Stepsize
	trap_Trace(&trace, Org, v, NPC->r.maxs, goal, NPC->s.number, MASK_SOLID|MASK_OPAQUE);

	//if (trace.entityNum > 0)
	//	G_Printf("Hit entity %i (%s).\n", trace.entityNum, g_entities[trace.entityNum].classname);

	//if (g_entities[trace.entityNum].s.eType == ET_NPC)
	//	G_Printf("Hit an NPC!\n");

    if (trace.fraction == 1.0)
        return REACHABLE; // Yes we can see it

	// Now look for jump access...
	VectorAdd( Org, playerMins, mins );
	VectorAdd( Org, playerMaxs, maxs );

	VectorCopy(NPC->r.mins,v);
    v[2] += 18; // Stepsize
	VectorCopy(Org, eyes);
	eyes[2]+=32;
	trap_Trace(&trace, eyes, v, NPC->r.maxs, goal, NPC->s.number, MASK_SOLID|MASK_OPAQUE);
    if (trace.fraction == 1.0)
        return REACHABLE_JUMP; // Yes we can see it

	// Now look for crouch access...
	//CROUCH_VIEWHEIGHT ??
	VectorCopy(NPC->r.mins,v);
    v[2] += 18; // Stepsize
	VectorCopy(NPC->r.maxs,v2);
    v2[2] *= 0.5; // Stepsize
	trap_Trace(&trace, Org, v, v2, goal, NPC->s.number, MASK_SOLID|MASK_OPAQUE);
    if (trace.fraction == 1.0)
        return REACHABLE_DUCK; // Yes we can see it

	// Now look for strafe access... Left or Right...
	AngleVectors( NPC->client->ps.viewangles, forward, right, up );
	
	VectorMA( NPC->r.currentOrigin, 64, right, rightOrigin );
	VectorMA( NPC->r.currentOrigin, -64, right, leftOrigin );

	if (OrgVisible(leftOrigin, gWPArray[NPC->wpCurrent]->origin, NPC->s.number))
	{
		NPC->bot_strafe_left_timer = level.time + 201;
		VectorCopy(leftOrigin, NPC->bot_strafe_target_position);
		return REACHABLE_STRAFE_LEFT; // Yes we can see it
	}

	if (OrgVisible(rightOrigin, gWPArray[NPC->wpCurrent]->origin, NPC->s.number))
	{
		NPC->bot_strafe_right_timer = level.time + 201;
		VectorCopy(rightOrigin, NPC->bot_strafe_target_position);
		return REACHABLE_STRAFE_RIGHT; // Yes we can see it
	}

	// And strafing a bit further away...
	VectorMA( NPC->r.currentOrigin, 128, right, rightOrigin );
	VectorMA( NPC->r.currentOrigin, -128, right, leftOrigin );

	if (OrgVisible(leftOrigin, gWPArray[NPC->wpCurrent]->origin, NPC->s.number))
	{
		NPC->bot_strafe_left_timer = level.time + 401;
		VectorCopy(leftOrigin, NPC->bot_strafe_target_position);
		return REACHABLE_STRAFE_LEFT; // Yes we can see it
	}

	if (OrgVisible(rightOrigin, gWPArray[NPC->wpCurrent]->origin, NPC->s.number))
	{
		NPC->bot_strafe_right_timer = level.time + 401;
		VectorCopy(rightOrigin, NPC->bot_strafe_target_position);
		return REACHABLE_STRAFE_RIGHT; // Yes we can see it
	}

	// And strafing a bit further away...
	VectorMA( NPC->r.currentOrigin, 192, right, rightOrigin );
	VectorMA( NPC->r.currentOrigin, -192, right, leftOrigin );

	if (OrgVisible(leftOrigin, gWPArray[NPC->wpCurrent]->origin, NPC->s.number))
	{
		NPC->bot_strafe_left_timer = level.time + 601;
		VectorCopy(leftOrigin, NPC->bot_strafe_target_position);
		return REACHABLE_STRAFE_LEFT; // Yes we can see it
	}

	if (OrgVisible(rightOrigin, gWPArray[NPC->wpCurrent]->origin, NPC->s.number))
	{
		NPC->bot_strafe_right_timer = level.time + 601;
		VectorCopy(rightOrigin, NPC->bot_strafe_target_position);
		return REACHABLE_STRAFE_RIGHT; // Yes we can see it
	}

	return NOT_REACHABLE;
}

extern int CheckForFunc(vec3_t org, int ignore);

qboolean NPC_WaitForFunc ( gentity_t *NPC )
{
	gentity_t *test = NULL;
	vec3_t size, center, pos1, pos2, origin;
	qboolean pull = qfalse;
	int i;

	for (i = 0;i < MAX_GENTITIES;i++)
	{
		test = &g_entities[i];

		if (!test)
			continue;

		if (Q_stricmp( "func_plat", test->classname ) &&  Q_stricmp( "func_door", test->classname ))
			continue;

		VectorCopy( NPC->r.currentOrigin, origin );

		//find center, pos1, pos2
		if ( VectorCompare( vec3_origin, test->s.origin ) )
		{//does not have an origin brush, so pos1 & pos2 are relative to world origin, need to calc center
			VectorSubtract( test->r.absmax, test->r.absmin, size );
			VectorMA( test->r.absmin, 0.5, size, center );
			if ( (test->spawnflags&1) && test->moverState == MOVER_POS1 )
			{//if at pos1 and started open, make sure we get the center where it *started* because we're going to add back in the relative values pos1 and pos2
				VectorSubtract( center, test->pos1, center );
			}
			else if ( !(test->spawnflags&1) && test->moverState == MOVER_POS2 )
			{//if at pos2, make sure we get the center where it *started* because we're going to add back in the relative values pos1 and pos2
				VectorSubtract( center, test->pos2, center );
			}
			VectorAdd( center, test->pos1, pos1 );
			VectorAdd( center, test->pos2, pos2 );
		}
		else
		{//actually has an origin, pos1 and pos2 are absolute
			VectorCopy( test->r.currentOrigin, center );
			VectorCopy( test->pos1, pos1 );
			VectorCopy( test->pos2, pos2 );
		}

		if(VectorDistance(origin, center) > 256/*192*//*400*/)
		{//too far away
			continue;
		}

		if (NPC->npc_mover_start_pos == MOVER_POS1 && test->moverState == MOVER_POS2)
		{// Ready to get off the lift...
			return qfalse;
		}

		if (NPC->npc_mover_start_pos == MOVER_POS2 && test->moverState == MOVER_POS1)
		{// Ready to get off the lift...
			return qfalse;
		}

		if (test->moverState == MOVER_1TO2 || test->moverState == MOVER_2TO1)
		{// If it's moving, then always wait!
			return qtrue;
		}

		if ( Distance( pos1, NPC->r.currentOrigin ) < Distance( pos2, NPC->r.currentOrigin ) )
		{//pos1 is closer
			if ( test->moverState == MOVER_POS1 )
			{//at the closest pos
				return qtrue;
			}
		}
		else
		{//pos2 is closer
			if ( test->moverState == MOVER_POS2 )
			{//at closest pos
				return qtrue;
			}
		}
		//we have a winner!
		break;
	}

	if (test && NPC->npc_mover_start_pos < 0)
	{// Mark the position we got on the mover at!
		NPC->npc_mover_start_pos = test->moverState;
	}
	else
	{
		NPC->npc_mover_start_pos = -1;
	}

	return qfalse;
}

/*///////////////////////////////////////////////////
NPC_GetNextNode
if the bot has reached a node, this function selects the next node
that he will go to, and returns it
right now it's being developed, feel free to experiment
*////////////////////////////////////////////////////

int NPC_GetNextNode(gentity_t *NPC)
{
	short int node = WAYPOINT_NONE;

	//we should never call this in BOTSTATE_MOVE with no goal
	//setup the goal/path in HandleIdleState
	if (NPC->longTermGoal == WAYPOINT_NONE)
	{
		return WAYPOINT_NONE;
	}

	if (NPC->pathsize == 0)	//if the bot is at the end of his path, this shouldn't have been called
	{
		NPC->longTermGoal = WAYPOINT_NONE;	//reset to having no goal
		return WAYPOINT_NONE;
	}

	node = NPC->pathlist[NPC->pathsize-1];	//pathlist is in reverse order
	NPC->pathsize--;	//mark that we've moved another node

	return node;
}

void NPC_SelectMoveAnimation()
{
	if (NPC->client->ps.crouchheight <= 0)
		NPC->client->ps.crouchheight = CROUCH_MAXS_2;

	if (NPC->client->ps.standheight <= 0)
		NPC->client->ps.standheight = DEFAULT_MAXS_2;

	/*
	if (NPC->client->ps.pm_flags & PMF_DUCKED && NPC->r.maxs[2] > NPC->client->ps.crouchheight)
	{
		VectorCopy(playerMins, NPC->r.mins);
		VectorCopy(playerMaxs, NPC->r.maxs);

		NPC->r.maxs[2] = NPC->client->ps.crouchheight;
		trap_LinkEntity(NPC);
	}
	else if (!(NPC->client->ps.pm_flags & PMF_DUCKED) && NPC->r.maxs[2] < NPC->client->ps.standheight)
	{
		VectorCopy(playerMins, NPC->r.mins);
		VectorCopy(playerMaxs, NPC->r.maxs);

		NPC->r.maxs[2] = NPC->client->ps.standheight;
		trap_LinkEntity(NPC);
	}
	*/

	if (VectorLength(NPC->client->ps.velocity) < 4)
	{// Standing still...
		if (NPC->client->ps.pm_flags & PMF_DUCKED)
		{
			NPC_SetAnim(NPC, SETANIM_BOTH, BOTH_CROUCH1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
		}
		else if ( NPC->client->ps.eFlags2 & EF2_USE_ALT_ANIM )
		{//holding someone
			NPC_SetAnim( NPC, SETANIM_BOTH, BOTH_STAND4, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD );
		}
		else if ( NPC->client->ps.eFlags2 & EF2_ALERTED )
		{//have an enemy or have had one since we spawned
			NPC_SetAnim( NPC, SETANIM_BOTH, BOTH_STAND2, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD );
		}
		else
		{//just stand there
			NPC_SetAnim( NPC, SETANIM_BOTH, BOTH_STAND1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD );
		}
	}
	else if (VectorLength(NPC->client->ps.velocity) <= 64)
	{// Use walking anims..
		if (NPC->client->ps.pm_flags & PMF_DUCKED)
		{
			NPC_SetAnim(NPC, SETANIM_BOTH, BOTH_CROUCH1WALK, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
		}
		else if (NPC->client->ps.weapon == WP_SABER)
		{// Walk with saber...
			NPC_SetAnim( NPC, SETANIM_BOTH, BOTH_WALK1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD );
			//NPC_SetAnim( NPC, SETANIM_TORSO, TORSO_WEAPONREADY3, SETANIM_FLAG_NORMAL );
		}
		else
		{// Standard walk anim..
			NPC_SetAnim( NPC, SETANIM_LEGS, BOTH_WALK2, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD );
			NPC_SetAnim( NPC, SETANIM_TORSO, TORSO_WEAPONREADY3, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD );
		}
	}
	else if ( NPC->client->ps.eFlags2 & EF2_USE_ALT_ANIM )
	{//full on run, on all fours
		if (NPC->client->ps.pm_flags & PMF_DUCKED)
		{
			NPC_SetAnim(NPC, SETANIM_BOTH, BOTH_CROUCH1WALK, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
		}
		else 
		{
			NPC_SetAnim( NPC, SETANIM_BOTH, BOTH_RUN1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD );
		}
	}
	else
	{//regular, upright run
		if (NPC->client->ps.pm_flags & PMF_DUCKED)
		{
			NPC_SetAnim(NPC, SETANIM_BOTH, BOTH_CROUCH1WALK, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
		}
		else 
		{
			NPC_SetAnim( NPC, SETANIM_BOTH, BOTH_RUN2, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD );
		}
	}

	NPC->client->ps.torsoTimer = 200;
	NPC->client->ps.legsTimer = 200;
}

void NPC_Run()
{
	//NPC->client->pers.cmd.forwardmove = 127;
	ucmd.forwardmove = 127;
	NPC->client->ps.speed = NPCInfo->stats.runSpeed;
	NPC_SelectMoveAnimation();
}

void NPC_Walk()
{
	//NPC->client->pers.cmd.forwardmove = 48;
	ucmd.forwardmove = 48;
	NPC->client->ps.speed = NPCInfo->stats.walkSpeed;
	NPC_SelectMoveAnimation();
}

void NPC_SelectMoveType()
{
	if (NPC->enemy && NPC->enemy->s.eType == ET_PLAYER)
		NPC_Run();

	NPC_Walk();
}

#ifdef __CRAPPY_NPC_AVOIDANCE__
int AVOIDANCE_METHOD = 0;

void NPC_Avoidance()
{
	// Try some (currently really really simple) avoidance stuff...
	if (AVOIDANCE_METHOD <= 5)
	{
		ucmd.rightmove = 127;
		NPC_SelectMoveAnimation();
	}
	else if (AVOIDANCE_METHOD <= 10)
	{
		ucmd.rightmove = -127;
		NPC_SelectMoveAnimation();
	}
	else
	{
		ucmd.upmove = 127;
		ucmd.forwardmove = 127;
		NPC_SelectMoveAnimation();
	}

	AVOIDANCE_METHOD++;

	if (AVOIDANCE_METHOD >= 12)
		AVOIDANCE_METHOD = 0;
}

#else //!__CRAPPY_NPC_AVOIDANCE__

qboolean JKG_CheckBelowPoint( vec3_t point )
{
	trace_t tr;
	vec3_t org, org2;

	VectorCopy(point, org);
	VectorCopy(point, org2);
	org2[2] = -65536.0f;

	trap_Trace( &tr, org, NULL, NULL, org2, -1, MASK_PLAYERSOLID|CONTENTS_TRIGGER);
	
	if ( tr.startsolid )
	{
		//G_Printf("Waypoint %i is in solid.\n", wp);
		return qfalse;
	}

	if ( tr.allsolid )
	{
		//G_Printf("Waypoint %i is in solid.\n", wp);
		return qfalse;
	}

	if ( tr.fraction == 1 )
	{
		//G_Printf("Waypoint %i is too high above ground.\n", wp);
		return qfalse;
	}

	if ( tr.contents & CONTENTS_LAVA )
	{
		//G_Printf("Waypoint %i is in lava.\n", wp);
		return qfalse;
	}
	
	if ( tr.contents & CONTENTS_SLIME )
	{
		//G_Printf("Waypoint %i is in slime.\n", wp);
		return qfalse;
	}

	if ( tr.contents & CONTENTS_TRIGGER )
	{
		//G_Printf("Waypoint %i is in trigger.\n", wp);
		return qfalse;
	}

	return qtrue;
}

//#define	APEX_HEIGHT		200.0f
#define	APEX_HEIGHT		128.0f
#define	PARA_WIDTH		128.0f
//#define	PARA_WIDTH		(sqrt(APEX_HEIGHT)+sqrt(APEX_HEIGHT))
//#define	JUMP_SPEED		200.0f
#define	JUMP_SPEED		128.0f

static qboolean NPC_Jump( gentity_t *NPC, vec3_t dest )
{//FIXME: if land on enemy, knock him down & jump off again
	if ( 1 )
	{
		float	targetDist, shotSpeed = 300, travelTime, impactDist, bestImpactDist = Q3_INFINITE;//fireSpeed, 
		vec3_t	targetDir, shotVel, failCase; 
		trace_t	trace;
		trajectory_t	tr;
		qboolean	blocked;
		int		elapsedTime, timeStep = 500, hitCount = 0, maxHits = 7;
		vec3_t	lastPos, testPos, bottom;

		while ( hitCount < maxHits )
		{
			VectorSubtract( dest, NPC->r.currentOrigin, targetDir );
			targetDist = VectorNormalize( targetDir );

			VectorScale( targetDir, shotSpeed, shotVel );
			travelTime = targetDist/shotSpeed;
			shotVel[2] += travelTime * 0.5 * NPC->client->ps.gravity;

			if ( !hitCount )		
			{//save the first one as the worst case scenario
				VectorCopy( shotVel, failCase );
			}

			if ( 1 )//tracePath )
			{//do a rough trace of the path
				blocked = qfalse;

				VectorCopy( NPC->r.currentOrigin, tr.trBase );
				VectorCopy( shotVel, tr.trDelta );
				tr.trType = TR_GRAVITY;
				tr.trTime = level.time;
				travelTime *= 1000.0f;
				VectorCopy( NPC->r.currentOrigin, lastPos );
				
				//This may be kind of wasteful, especially on long throws... use larger steps?  Divide the travelTime into a certain hard number of slices?  Trace just to apex and down?
				for ( elapsedTime = timeStep; elapsedTime < floor(travelTime)+timeStep; elapsedTime += timeStep )
				{
					if ( (float)elapsedTime > travelTime )
					{//cap it
						elapsedTime = floor( travelTime );
					}
					BG_EvaluateTrajectory( &tr, level.time + elapsedTime, testPos );
					if ( testPos[2] < lastPos[2] )
					{//going down, ignore botclip
						trap_Trace( &trace, lastPos, NPC->r.mins, NPC->r.maxs, testPos, NPC->s.number, NPC->clipmask );
					}
					else
					{//going up, check for botclip
						trap_Trace( &trace, lastPos, NPC->r.mins, NPC->r.maxs, testPos, NPC->s.number, NPC->clipmask|CONTENTS_BOTCLIP );
					}

					if ( trace.allsolid || trace.startsolid )
					{
						blocked = qtrue;
						break;
					}
					if ( trace.fraction < 1.0f )
					{//hit something
						if ( Distance( trace.endpos, dest ) < 96 )
						{//hit the spot, that's perfect!
							//Hmm, don't want to land on him, though...
							break;
						}
						else 
						{
							if ( trace.contents & CONTENTS_BOTCLIP )
							{//hit a do-not-enter brush
								blocked = qtrue;
								break;
							}
							if ( trace.plane.normal[2] > 0.7 && DistanceSquared( trace.endpos, dest ) < 4096 )//hit within 64 of desired location, should be okay
							{//close enough!
								break;
							}
							else
							{//FIXME: maybe find the extents of this brush and go above or below it on next try somehow?
								impactDist = DistanceSquared( trace.endpos, dest );
								if ( impactDist < bestImpactDist )
								{
									bestImpactDist = impactDist;
									VectorCopy( shotVel, failCase );
								}
								blocked = qtrue;
								break;
							}
						}
					}
					if ( elapsedTime == floor( travelTime ) )
					{//reached end, all clear
						if ( trace.fraction >= 1.0f )
						{//hmm, make sure we'll land on the ground...
							//FIXME: do we care how far below ourselves or our dest we'll land?
							VectorCopy( trace.endpos, bottom );
							bottom[2] -= 128;
							trap_Trace( &trace, trace.endpos, NPC->r.mins, NPC->r.maxs, bottom, NPC->s.number, NPC->clipmask );
							if ( trace.fraction >= 1.0f )
							{//would fall too far
								blocked = qtrue;
							}
						}
						break;
					}
					else
					{
						//all clear, try next slice
						VectorCopy( testPos, lastPos );
					}
				}
				if ( blocked )
				{//hit something, adjust speed (which will change arc)
					hitCount++;
					shotSpeed = 300 + ((hitCount-2) * 100);//from 100 to 900 (skipping 300)
					if ( hitCount >= 2 )
					{//skip 300 since that was the first value we tested
						shotSpeed += 100;
					}
				}
				else
				{//made it!
					break;
				}
			}
			else
			{//no need to check the path, go with first calc
				break;
			}
		}

		if ( hitCount >= maxHits )
		{//NOTE: worst case scenario, use the one that impacted closest to the target (or just use the first try...?)
			//NOTE: or try failcase?
			VectorCopy( failCase, NPC->client->ps.velocity );
		}
		VectorCopy( shotVel, NPC->client->ps.velocity );
	}
	else
	{//a more complicated jump
		vec3_t		dir, p1, p2, apex;
		float		time, height, forward, z, xy, dist, apexHeight;

		if ( NPC->r.currentOrigin[2] > dest[2] )//NPCInfo->goalEntity->r.currentOrigin
		{
			VectorCopy( NPC->r.currentOrigin, p1 );
			VectorCopy( dest, p2 );//NPCInfo->goalEntity->r.currentOrigin
		}
		else if ( NPC->r.currentOrigin[2] < dest[2] )//NPCInfo->goalEntity->r.currentOrigin
		{
			VectorCopy( dest, p1 );//NPCInfo->goalEntity->r.currentOrigin
			VectorCopy( NPC->r.currentOrigin, p2 );
		}
		else
		{
			VectorCopy( NPC->r.currentOrigin, p1 );
			VectorCopy( dest, p2 );//NPCInfo->goalEntity->r.currentOrigin
		}

		//z = xy*xy
		VectorSubtract( p2, p1, dir );
		dir[2] = 0;

		//Get xy and z diffs
		xy = VectorNormalize( dir );
		z = p1[2] - p2[2];

		apexHeight = APEX_HEIGHT/2;

		//Determine most desirable apex height
		//FIXME: length of xy will change curve of parabola, need to account for this
		//somewhere... PARA_WIDTH
		/*
		apexHeight = (APEX_HEIGHT * PARA_WIDTH/xy) + (APEX_HEIGHT * z/128);
		if ( apexHeight < APEX_HEIGHT * 0.5 )
		{
			apexHeight = APEX_HEIGHT*0.5;
		}
		else if ( apexHeight > APEX_HEIGHT * 2 )
		{
			apexHeight = APEX_HEIGHT*2;
		}
		*/

		z = (sqrt(apexHeight + z) - sqrt(apexHeight));

		assert(z >= 0);

//		Com_Printf("apex is %4.2f percent from p1: ", (xy-z)*0.5/xy*100.0f);

		xy -= z;
		xy *= 0.5;
		
		assert(xy > 0);

		VectorMA( p1, xy, dir, apex );
		apex[2] += apexHeight;

		VectorCopy(apex, NPC->pos1);
		
		//Now we have the apex, aim for it
		height = apex[2] - NPC->r.currentOrigin[2];
		time = sqrt( height / ( .5 * NPC->client->ps.gravity ) );//was 0.5, but didn't work well for very long jumps
		if ( !time ) 
		{
			//Com_Printf( S_COLOR_RED"ERROR: no time in jump\n" );
			return qfalse;
		}

		VectorSubtract ( apex, NPC->r.currentOrigin, NPC->client->ps.velocity );
		NPC->client->ps.velocity[2] = 0;
		dist = VectorNormalize( NPC->client->ps.velocity );

		forward = dist / time * 1.25;//er... probably bad, but...
		VectorScale( NPC->client->ps.velocity, forward, NPC->client->ps.velocity );

		//FIXME:  Uh.... should we trace/EvaluateTrajectory this to make sure we have clearance and we land where we want?
		NPC->client->ps.velocity[2] = time * NPC->client->ps.gravity;

		//Com_Printf("Jump Velocity: %4.2f, %4.2f, %4.2f\n", NPC->client->ps.velocity[0], NPC->client->ps.velocity[1], NPC->client->ps.velocity[2] );
	}
	return qtrue;
}

//extern void G_SoundOnEnt( gentity_t *ent, int channel, const char *soundPath );

static qboolean NPC_TryJump( gentity_t *NPC, vec3_t goal )
{//FIXME: never does a simple short, regular jump...
	if ( TIMER_Done( NPC, "jumpChaseDebounce" ) )
	{
		{
			if ( !PM_InKnockDown( &NPC->client->ps ) && !BG_InRoll( &NPC->client->ps, NPC->client->ps.legsAnim ) )
			{//enemy is on terra firma
				vec3_t goal_diff;
				float goal_z_diff;
				float goal_xy_dist;
				VectorSubtract( goal, NPC->r.currentOrigin, goal_diff );
				goal_z_diff = goal_diff[2];
				goal_diff[2] = 0;
				goal_xy_dist = VectorNormalize( goal_diff );
				if ( goal_xy_dist < 550 && goal_z_diff > -400/*was -256*/ )//for now, jedi don't take falling damage && (NPC->health > 20 || goal_z_diff > 0 ) && (NPC->health >= 100 || goal_z_diff > -128 ))//closer than @512
				{
					qboolean debounce = qfalse;
					if ( NPC->health < 150 && ((NPC->health < 30 && goal_z_diff < 0) || goal_z_diff < -128 ) )
					{//don't jump, just walk off... doesn't help with ledges, though
						debounce = qtrue;
					}
					else if ( goal_z_diff < 32 && goal_xy_dist < 200 )
					{//what is their ideal jump height?
						
						ucmd.upmove = 127;
						debounce = qtrue;
					}
					else
					{
						/*
						//NO!  All Jedi can jump-navigate now...
						if ( NPCInfo->rank != RANK_CREWMAN && NPCInfo->rank <= RANK_LT_JG )
						{//can't do acrobatics
							return qfalse;
						}
						*/
						if ( goal_z_diff > 0 || goal_xy_dist > 128 )
						{//Fake a force-jump
							//Screw it, just do my own calc & throw
							vec3_t dest;
							VectorCopy( goal, dest );
							
							{
								int	sideTry = 0;
								while( sideTry < 10 )
								{//FIXME: make it so it doesn't try the same spot again?
									trace_t	trace;
									vec3_t	bottom;

									VectorCopy( dest, bottom );
									bottom[2] -= 128;
									trap_Trace( &trace, dest, NPC->r.mins, NPC->r.maxs, bottom, NPC->s.number, NPC->clipmask );
									if ( trace.fraction < 1.0f )
									{//hit floor, okay to land here
										break;
									}
									sideTry++;
								}
								if ( sideTry >= 10 )
								{//screw it, just jump right at him?
									VectorCopy( goal, dest );
								}
							}

							if ( NPC_Jump( NPC, dest ) )
							{
								//Com_Printf( "(%d) pre-checked force jump\n", level.time );

								//FIXME: store the dir we;re going in in case something gets in the way of the jump?
								//? = vectoyaw( NPC->client->ps.velocity );
								/*
								if ( NPC->client->ps.velocity[2] < 320 )
								{
									NPC->client->ps.velocity[2] = 320;
								}
								else
								*/
								{//FIXME: make this a function call
									int jumpAnim;
									//FIXME: this should be more intelligent, like the normal force jump anim logic
									//if ( NPC->client->NPC_class == CLASS_BOBAFETT 
									//	||( NPCInfo->rank != RANK_CREWMAN && NPCInfo->rank <= RANK_LT_JG ) )
									//{//can't do acrobatics
									//	jumpAnim = BOTH_FORCEJUMP1;
									//}
									//else
									//{
										jumpAnim = BOTH_FLIP_F;
									//}
									G_SetAnim( NPC, &ucmd, SETANIM_BOTH, jumpAnim, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD, 0 );
								}

								NPC->client->ps.fd.forceJumpZStart = NPC->r.currentOrigin[2];
								//NPC->client->ps.pm_flags |= PMF_JUMPING;

								NPC->client->ps.weaponTime = NPC->client->ps.torsoTimer;
								NPC->client->ps.fd.forcePowersActive |= ( 1 << FP_LEVITATION );
								
								//if ( NPC->client->NPC_class == CLASS_BOBAFETT )
								//{
								//	G_SoundOnEnt( NPC, CHAN_ITEM, "sound/boba/jeton.wav" );
								//	NPC->client->jetPackTime = level.time + Q_irand( 1000, 3000 );
								//}
								//else
								//{
								//	G_SoundOnEnt( NPC, CHAN_BODY, "sound/weapons/force/jump.wav" );
								//}

								TIMER_Set( NPC, "forceJumpChasing", Q_irand( 2000, 3000 ) );
								debounce = qtrue;
							}
						}
					}

					if ( debounce )
					{
						//Don't jump again for another 2 to 5 seconds
						TIMER_Set( NPC, "jumpChaseDebounce", Q_irand( 2000, 5000 ) );
						ucmd.forwardmove = 127;
						VectorClear( NPC->client->ps.moveDir );
						TIMER_Set( NPC, "duck", -level.time );
						return qtrue;
					}
				}
			}
		}
	}

	return qfalse;
}

qboolean NPC_ClearPathToJump( gentity_t *NPC, vec3_t dest, int impactEntNum )
{
	trace_t	trace;
	vec3_t	mins, start, end, dir;
	float	dist, drop;
	float	i;

	//Offset the step height
	VectorSet( mins, NPC->r.mins[0], NPC->r.mins[1], NPC->r.mins[2] + STEPSIZE );
	
	trap_Trace( &trace, NPC->r.currentOrigin, mins, NPC->r.maxs, dest, NPC->s.number, NPC->clipmask );

	//Do a simple check
	if ( trace.allsolid || trace.startsolid )
	{//inside solid
		return qfalse;
	}

	if ( trace.fraction < 1.0f )
	{//hit something
		if ( impactEntNum != ENTITYNUM_NONE && trace.entityNum == impactEntNum )
		{//hit what we're going after
			return qtrue;
		}
		else
		{
			return qfalse;
		}
	}

	//otherwise, clear path in a straight line.  
	//Now at intervals of my size, go along the trace and trace down STEPSIZE to make sure there is a solid floor.
	VectorSubtract( dest, NPC->r.currentOrigin, dir );
	dist = VectorNormalize( dir );
	if ( dest[2] > NPC->r.currentOrigin[2] )
	{//going up, check for steps
		drop = STEPSIZE;
	}
	else
	{//going down or level, check for moderate drops
		drop = 64;
	}
	for ( i = NPC->r.maxs[0]*2; i < dist; i += NPC->r.maxs[0]*2 )
	{//FIXME: does this check the last spot, too?  We're assuming that should be okay since the enemy is there?
		VectorMA( NPC->r.currentOrigin, i, dir, start );
		VectorCopy( start, end );
		end[2] -= drop;
		trap_Trace( &trace, start, mins, NPC->r.maxs, end, NPC->s.number, NPC->clipmask );//NPC->r.mins?
		if ( trace.fraction < 1.0f || trace.allsolid || trace.startsolid )
		{//good to go
			continue;
		}
		//no floor here! (or a long drop?)
		return qfalse;
	}
	//we made it!
	return qtrue;
}

qboolean DOM_NPC_ClearPathBetweenSpots( vec3_t from, vec3_t dest, int impactEntNum )
{
	trace_t	trace;
	vec3_t	/*start, end, dir,*/ org, destorg;
//	float	dist, drop;
//	float	i;

	//Offset the step height
	//vec3_t	mins = {-18, -18, -24};
	vec3_t	mins = {-8, -8, -6};
	//vec3_t	maxs = {18, 18, 48};
	vec3_t	maxs = {8, 8, NPC->client->ps.crouchheight};

	VectorCopy(from, org);
	org[2]+=STEPSIZE;

	VectorCopy(dest, destorg);
	destorg[2]+=STEPSIZE;

	trap_Trace( &trace, org, mins, maxs, destorg, NPC->s.number, CONTENTS_PLAYERCLIP/*NPC->clipmask*/ );

	//Do a simple check
	if ( trace.allsolid || trace.startsolid )
	{//inside solid
		//G_Printf("SOLID!\n");
		return qfalse;
	}

	if ( trace.fraction < 1.0f )
	{//hit something
		if ( (impactEntNum != ENTITYNUM_NONE && trace.entityNum == impactEntNum )
			/*|| !Q_stricmp(g_entities[trace.entityNum].classname, "worldspawn")*/ )
		{//hit what we're going after
			//G_Printf("OK!\n");
			return qtrue;
		}
		else
		{
			//G_Printf("TRACE FAIL! - NPC %i hit entity %i (%s).\n", NPC->s.number, trace.entityNum, g_entities[trace.entityNum].classname);
			return qfalse;
		}
	}

	/*
	//otherwise, clear path in a straight line.  
	//Now at intervals of my size, go along the trace and trace down STEPSIZE to make sure there is a solid floor.
	VectorSubtract( dest, NPC->r.currentOrigin, dir );
	dist = VectorNormalize( dir );
	if ( dest[2] > NPC->r.currentOrigin[2] )
	{//going up, check for steps
		drop = STEPSIZE;
	}
	else
	{//going down or level, check for moderate drops
		drop = 64;
	}
	for ( i = NPC->r.maxs[0]*2; i < dist; i += NPC->r.maxs[0]*2 )
	{//FIXME: does this check the last spot, too?  We're assuming that should be okay since the enemy is there?
		VectorMA( NPC->r.currentOrigin, i, dir, start );
		VectorCopy( start, end );
		end[2] -= drop;
		trap_Trace( &trace, start, mins, NPC->r.maxs, end, NPC->s.number, NPC->clipmask );//NPC->r.mins?
		if ( trace.fraction < 1.0f || trace.allsolid || trace.startsolid )
		{//good to go
			continue;
		}
		G_Printf("FLOOR!\n");
		//no floor here! (or a long drop?)
		return qfalse;
	}*/
	//we made it!
	return qtrue;
}

qboolean DOM_NPC_CrouchPathToSpot( gentity_t *NPC, vec3_t dest, int impactEntNum )
{
	trace_t	trace;
	vec3_t	/*start, end, dir,*/ org, destorg;
//	float	dist, drop;
//	float	i;

	//Offset the step height
	//vec3_t	mins = {-18, -18, -24};
	vec3_t	mins = {-8, -8, -6};
	//vec3_t	maxs = {18, 18, 48};
	vec3_t	maxs = {8, 8, NPC->client->ps.crouchheight};

	VectorCopy(NPC->s.origin, org);
	org[2]+=STEPSIZE;

	VectorCopy(dest, destorg);
	destorg[2]+=STEPSIZE;

	trap_Trace( &trace, org, mins, maxs, destorg, NPC->s.number, CONTENTS_PLAYERCLIP/*NPC->clipmask*/ );

	//Do a simple check
	if ( trace.allsolid || trace.startsolid )
	{//inside solid
		//G_Printf("SOLID!\n");
		return qfalse;
	}

	if ( trace.fraction < 1.0f )
	{//hit something
		if ( (impactEntNum != ENTITYNUM_NONE && trace.entityNum == impactEntNum )
			/*|| !Q_stricmp(g_entities[trace.entityNum].classname, "worldspawn")*/ )
		{//hit what we're going after
			//G_Printf("OK!\n");
			return qtrue;
		}
		else
		{
			//G_Printf("TRACE FAIL! - NPC %i hit entity %i (%s).\n", NPC->s.number, trace.entityNum, g_entities[trace.entityNum].classname);
			return qfalse;
		}
	}

	/*
	//otherwise, clear path in a straight line.  
	//Now at intervals of my size, go along the trace and trace down STEPSIZE to make sure there is a solid floor.
	VectorSubtract( dest, NPC->r.currentOrigin, dir );
	dist = VectorNormalize( dir );
	if ( dest[2] > NPC->r.currentOrigin[2] )
	{//going up, check for steps
		drop = STEPSIZE;
	}
	else
	{//going down or level, check for moderate drops
		drop = 64;
	}
	for ( i = NPC->r.maxs[0]*2; i < dist; i += NPC->r.maxs[0]*2 )
	{//FIXME: does this check the last spot, too?  We're assuming that should be okay since the enemy is there?
		VectorMA( NPC->r.currentOrigin, i, dir, start );
		VectorCopy( start, end );
		end[2] -= drop;
		trap_Trace( &trace, start, mins, NPC->r.maxs, end, NPC->s.number, NPC->clipmask );//NPC->r.mins?
		if ( trace.fraction < 1.0f || trace.allsolid || trace.startsolid )
		{//good to go
			continue;
		}
		G_Printf("FLOOR!\n");
		//no floor here! (or a long drop?)
		return qfalse;
	}*/
	//we made it!
	return qtrue;
}

typedef enum
{// Avoidance methods...
	AVOIDANCE_NONE,
	AVOIDANCE_STRAFE_RIGHT,
	AVOIDANCE_STRAFE_LEFT,
	AVOIDANCE_STRAFE_CROUCH,
	AVOIDANCE_STRAFE_JUMP,
} avoidanceMethods_t;

qboolean DOM_NPC_ClearPathToSpot( gentity_t *NPC, vec3_t dest, int impactEntNum );// below
/*
qboolean NPC_FindTemporaryWaypoint()
{
	int i = 0;
	int j = 0;

	for (i = 0; i < gWPArray[NPC->wpCurrent]->neighbornum; i++)
	{
		int test_wp = gWPArray[NPC->wpCurrent]->neighbors[i].num;

		if (DOM_NPC_ClearPathToSpot( NPC, gWPArray[test_wp]->origin, -1 ) && Q_irand(0,2) == 1)
		{// This link is walkable for us... Now check if it can see the current waypoint...
			if (DOM_NPC_ClearPathBetweenSpots( gWPArray[test_wp]->origin, gWPArray[NPC->wpCurrent]->origin, -1 ))
			{// Found one!
				VectorCopy(gWPArray[test_wp]->origin, NPC->bot_strafe_target_position);
				NPC->wpTravelTime = level.time + 10000; // give them time to traverse this wp...
				NPC->bot_strafe_target_timer = level.time + 5000;
				G_Printf("NPC %s is using an avoidance waypoint.\n", NPC->NPC_type);
				return qtrue;
			}
		}
	}

	return qfalse;
}
*/

vec3_t jumpLandPosition;

qboolean NPC_NeedJump()
{
	trace_t		tr;
	vec3_t		org1, org2;
	vec3_t		forward, right;

	VectorCopy(NPC->r.currentOrigin, org1);

	AngleVectors( NPC->r.currentAngles, forward, NULL, NULL );

	// Check jump...
	org1[2] += 10;
	VectorMA( org1, 32, forward, org2 );
	forward[PITCH] = forward[ROLL] = 0;
	trap_Trace( &tr, org1, NULL, NULL, org2, NPC->s.number, MASK_PLAYERSOLID );

	if (NPC->waterlevel > 0)
	{
		VectorCopy(org1, jumpLandPosition);
		return qtrue;
	}

	if (tr.fraction < 1.0f)
	{
		VectorCopy(org1, jumpLandPosition);
		return qtrue;
	}

	return qfalse;
}

int NPC_SelectBestAvoidanceMethod()
{// Just find the largest visible distance direction...
	trace_t		tr;
	vec3_t		org1, org2;
	vec3_t		forward, right;
	float		distance = 0.0f;
	float		BEST_DISTANCE = 0.0f;
	int			BEST_AVOID_TYPE = AVOIDANCE_NONE;

	if (NPC->bot_strafe_right_timer > level.time)
		return AVOIDANCE_STRAFE_RIGHT;

	if (NPC->bot_strafe_left_timer > level.time)
		return AVOIDANCE_STRAFE_LEFT;

	if (NPC->bot_strafe_crouch_timer > level.time)
		return AVOIDANCE_STRAFE_CROUCH;

	if (NPC->bot_strafe_jump_timer > level.time)
		return AVOIDANCE_STRAFE_JUMP;

	//if (NPC->bot_strafe_target_timer > level.time)
	//	return AVOIDANCE_NONE;

	//if (NPC_FindTemporaryWaypoint())
	//	return AVOIDANCE_NONE;

	VectorCopy(NPC->r.currentOrigin, org1);
	org1[2] += 8;

	AngleVectors( NPC->r.currentAngles, forward, right, NULL );
	
	// Check right side...
	VectorMA( org1, 256, right, org2 );
	trap_Trace( &tr, org1, NULL, NULL, org2, NPC->s.number, MASK_PLAYERSOLID );
	distance = Distance(NPC->r.currentOrigin, tr.endpos);

	/*
	if ( tr.fraction < 1.0f )
	{//hit something
		if ( tr.entityNum == ENTITYNUM_WORLD )
			G_Printf("Strafe trace hit world entity!\n");
	}
	*/
	
	if (JKG_CheckBelowPoint(org2))
	{
		BEST_DISTANCE = distance;
		BEST_AVOID_TYPE = AVOIDANCE_STRAFE_RIGHT;
	}

	// Check left side...
	VectorMA( org1, -256, right, org2 );
	trap_Trace( &tr, org1, NULL, NULL, org2, NPC->s.number, MASK_PLAYERSOLID );
	distance = Distance(NPC->r.currentOrigin, tr.endpos);

	/*
	if ( tr.fraction < 1.0f )
	{//hit something
		if ( tr.entityNum == ENTITYNUM_WORLD )
			G_Printf("Strafe trace 2 hit world entity!\n");
	}
	*/
	
	if (distance > BEST_DISTANCE && JKG_CheckBelowPoint(org2))
	{// Better...
		BEST_DISTANCE = distance;
		BEST_AVOID_TYPE = AVOIDANCE_STRAFE_LEFT;
	}

	/*
	// Check jump...
	VectorCopy(NPC->r.currentOrigin, org1);
	org1[2] += 24;
	VectorMA( org1, 256, forward, org2 );
	trap_Trace( &tr, org1, NULL, NULL, org2, NPC->s.number, MASK_PLAYERSOLID );
	distance = Distance(NPC->r.currentOrigin, tr.endpos);

	if (tr.fraction < 1.0f)
	{
		if (tr.entityNum != ENTITYNUM_NONE && tr.entityNum != ENTITYNUM_WORLD)
		{
			if (g_entities[tr.entityNum].bot_strafe_right_timer > level.time)
				return AVOIDANCE_STRAFE_RIGHT;
			if (g_entities[tr.entityNum].bot_strafe_left_timer > level.time)
				return AVOIDANCE_STRAFE_LEFT;
			if (g_entities[tr.entityNum].bot_strafe_crouch_timer > level.time)
				return Q_irand(AVOIDANCE_STRAFE_RIGHT, AVOIDANCE_STRAFE_LEFT);
			if (g_entities[tr.entityNum].bot_strafe_jump_timer > level.time)
				return Q_irand(AVOIDANCE_STRAFE_RIGHT, AVOIDANCE_STRAFE_LEFT);
		}
	}
	
	if (distance >= BEST_DISTANCE && JKG_CheckBelowPoint(org2))
	{// Better...
		BEST_DISTANCE = distance;
		BEST_AVOID_TYPE = AVOIDANCE_STRAFE_JUMP;
	}
	*/

	// Check crouch...
	VectorCopy(NPC->r.currentOrigin, org1);
	org1[2] += 8;
	VectorMA( org1, 256, forward, org2 );
	trap_Trace( &tr, org1, NULL, NULL, org2, NPC->s.number, MASK_PLAYERSOLID );
	distance = Distance(NPC->r.currentOrigin, tr.endpos);

	if (tr.fraction < 1.0f)
	{
		if (tr.entityNum != ENTITYNUM_NONE && tr.entityNum != ENTITYNUM_WORLD)
		{
			if (g_entities[tr.entityNum].bot_strafe_right_timer > level.time)
				return AVOIDANCE_STRAFE_RIGHT;
			if (g_entities[tr.entityNum].bot_strafe_left_timer > level.time)
				return AVOIDANCE_STRAFE_LEFT;
			if (g_entities[tr.entityNum].bot_strafe_crouch_timer > level.time)
				return Q_irand(AVOIDANCE_STRAFE_RIGHT, AVOIDANCE_STRAFE_LEFT);
			if (g_entities[tr.entityNum].bot_strafe_jump_timer > level.time)
				return Q_irand(AVOIDANCE_STRAFE_RIGHT, AVOIDANCE_STRAFE_LEFT);
		}
	}
	
	if (distance >= BEST_DISTANCE && DOM_NPC_CrouchPathToSpot( NPC, gWPArray[NPC->wpCurrent]->origin, -1 ) && JKG_CheckBelowPoint(org2))
	{// Better...
		BEST_DISTANCE = distance;
		BEST_AVOID_TYPE = AVOIDANCE_STRAFE_CROUCH;
	}

	if (BEST_DISTANCE < 64 && NPC_ClearPathToJump( NPC, gWPArray[NPC->wpCurrent]->origin, -1 ))
	{
		return AVOIDANCE_STRAFE_JUMP;
	}

	return BEST_AVOID_TYPE;
}

void NPC_Avoidance()
{
	int BEST_METHOD = NPC_SelectBestAvoidanceMethod();
	
	switch (BEST_METHOD)
	{
	case AVOIDANCE_STRAFE_RIGHT:
		ucmd.rightmove = 48;
		NPC->bot_strafe_right_timer = level.time + 100;
		break;
	case AVOIDANCE_STRAFE_LEFT:
		ucmd.rightmove = -48;
		NPC->bot_strafe_left_timer = level.time + 100;
		break;
	case AVOIDANCE_STRAFE_CROUCH:
		NPC->playerState->pm_flags |= PMF_DUCKED;
		NPC->bot_strafe_crouch_timer = level.time + 1000;
		break;
	case AVOIDANCE_STRAFE_JUMP:
		NPC_TryJump( NPC, jumpLandPosition );
		break;
	default:
		break;
	}
}

#endif //__CRAPPY_NPC_AVOIDANCE__

qboolean NPC_FindNewWaypoint()
{
	if (NPC->noWaypointTime > level.time)
	{// Only try to find a new waypoint every 5 seconds...
		return qfalse;
	}

	NPC->wpCurrent = DOM_GetBestWaypoint(NPC->r.currentOrigin, NPC->s.number, NPC->wpCurrent);
	NPC->noWaypointTime = level.time + 3000; // 3 seconds before we try again... (it will run avoidance in the meantime)

	if (NPC->wpSeenTime < NPC->noWaypointTime)
		NPC->wpSeenTime = NPC->noWaypointTime; // also make sure we don't try to make a new route for the same length of time...

	if (NPC->wpCurrent < 0 || NPC->wpCurrent >= gWPNum)
	{
		G_Printf("NPC Waypointing Debug: NPC %i (%s) failed to find a waypoint for itself.", NPC->s.number, NPC->NPC_type);
		return qfalse; // failed... try again after som avoidance code...
	}

	return qtrue; // all good, we have a new waypoint...
}

void NPC_SetNewGoalAndPath()
{
	if (NPC->wpSeenTime > level.time)
		return; // wait for next route creation...

	if (!NPC_FindNewWaypoint())
		return; // wait before trying to get a new waypoint...

	// Find a new generic goal...
	NPC->longTermGoal = NPC_FindGoal( NPC );

	if (NPC->longTermGoal >= 0)
	{
		memset(NPC->pathlist, WAYPOINT_NONE, MAX_WPARRAY_SIZE);
		NPC->pathsize = ASTAR_FindPathFast(NPC->wpCurrent, NPC->longTermGoal, NPC->pathlist, qtrue);

		//if (NPC->pathsize <= 0) // Use the alternate (older) A* pathfinding code as alternative/fallback...
		//	NPC->pathsize = DOM_FindIdealPathtoWP(NULL, NPC->wpCurrent, NPC->longTermGoal, -1, NPC->pathlist);

		if (NPC->pathsize > 0)
		{
			//G_Printf("NPC Waypointing Debug: NPC %i created a %i waypoint path for a random goal between waypoints %i and %i.", NPC->s.number, NPC->pathsize, NPC->wpCurrent, NPC->longTermGoal);
			NPC->wpLast = -1;
			NPC->wpNext = NPC_GetNextNode(NPC);		//move to this node first, since it's where our path starts from
		}
		else
		{
			//G_Printf("NPC Waypointing Debug: NPC %i failed to create a route between waypoints %i and %i.", NPC->s.number, NPC->wpCurrent, NPC->longTermGoal);
		}
	}
	else
	{
		//G_Printf("NPC Waypointing Debug: NPC %i failed to find a goal waypoint.", NPC->s.number);
	}

	// Delay before next route creation...
	NPC->wpSeenTime = level.time + 2000;//30000;
	// Delay before giving up on this new waypoint/route...
	NPC->wpTravelTime = level.time + 15000;
}

void NPC_SetEnemyGoal()
{
	if (NPC->wpSeenTime > level.time)
		return; // wait for next route creation...

	if (!NPC_FindNewWaypoint())
		return; // wait before trying to get a new waypoint...

	// UQ1: FIXME - Non-SABER NPCs should find a cover spot???
	NPC->longTermGoal = DOM_GetBestWaypoint(NPC->enemy->r.currentOrigin, NPC->enemy->s.number, NPC->wpCurrent);

	if (NPC->longTermGoal >= 0)
	{
		memset(NPC->pathlist, WAYPOINT_NONE, MAX_WPARRAY_SIZE);
		NPC->pathsize = ASTAR_FindPathWithTimeLimit(NPC->wpCurrent, NPC->longTermGoal, NPC->pathlist);

		//if (NPC->pathsize <= 0) // Use the alternate (older) A* pathfinding code as alternative/fallback...
			//NPC->pathsize = DOM_FindIdealPathtoWP(NULL, NPC->wpCurrent, NPC->longTermGoal, -1, NPC->pathlist);
			//NPC->pathsize = ASTAR_FindPathFast(NPC->wpCurrent, NPC->longTermGoal, NPC->pathlist, qtrue);
		
		if (NPC->pathsize > 0)
		{
			if (NPC->enemy->s.eType == ET_PLAYER)
				G_Printf("NPC Waypointing Debug: NPC %i (%s) created a %i waypoint path between waypoints %i and %i for enemy player %s.", NPC->s.number, NPC->NPC_type, NPC->pathsize, NPC->wpCurrent, NPC->longTermGoal, NPC->enemy->client->pers.netname);
			else
				G_Printf("NPC Waypointing Debug: NPC %i (%s) created a %i waypoint path between waypoints %i and %i for enemy %s.", NPC->s.number, NPC->NPC_type, NPC->pathsize, NPC->wpCurrent, NPC->longTermGoal, NPC->enemy->classname);

			NPC->wpLast = -1;
			NPC->wpNext = NPC_GetNextNode(NPC);		//move to this node first, since it's where our path starts from
		}
		else if (NPC->enemy->s.eType == ET_PLAYER)
		{
			G_Printf("NPC Waypointing Debug: NPC %i (%s) failed to create a route between waypoints %i and %i for enemy player %s.", NPC->s.number, NPC->NPC_type, NPC->wpCurrent, NPC->longTermGoal, NPC->enemy->client->pers.netname);
		}
	}
	else
	{
		if (NPC->enemy->s.eType == ET_PLAYER)
			G_Printf("NPC Waypointing Debug: NPC %i (%s) failed to find a waypoint for enemy player %s.", NPC->s.number, NPC->NPC_type, NPC->enemy->client->pers.netname);
	}

	// Delay before next route creation...
	NPC->wpSeenTime = level.time + 2000;
	// Delay before giving up on this new waypoint/route...
	NPC->wpTravelTime = level.time + 5000;
}

// Position to position
qboolean NPC_EnemyVisible( gentity_t *self, gentity_t *enemy )
{
	trace_t		tr;
	vec3_t		selfEyes, enemyEyes;

	if (!enemy)
	{
		return qfalse;
	}

	if (!enemy->client)
	{
		self->enemy = NULL;
		return qfalse;
	}

	if (NPC->genericValue15 < level.time)
	{
		VectorCopy(self->r.currentOrigin, selfEyes);
		
		if (self->client->ps.pm_flags & PMF_DUCKED)
			selfEyes[2]+=16;
		else
			selfEyes[2]+=32;

		VectorCopy(enemy->r.currentOrigin, enemyEyes);
		
		if (enemy->client->ps.pm_flags & PMF_DUCKED)
			enemyEyes[2]+=16;
		else
			enemyEyes[2]+=32;
	
		trap_Trace ( &tr, selfEyes, NULL, NULL, enemyEyes, self->s.number, MASK_SHOT );

		if (tr.entityNum == enemy->s.number)
		{
			// Set a new waypoint...
			//NPC_SetEnemyGoal();
			self->genericValue15 = level.time + 4000; // set as visible for 4 secs...
			self->genericValue14 = level.time + 30000; // set give up timer for 30 secs...
			return qtrue;
		}

		return qfalse;
	}

	// Enemy still visible from last check...
	return qtrue;
}

qboolean DOM_NPC_ClearPathToSpot( gentity_t *NPC, vec3_t dest, int impactEntNum )
{
	trace_t	trace;
	vec3_t	/*start, end, dir,*/ org, destorg;
//	float	dist, drop;
//	float	i;

	//Offset the step height
	//vec3_t	mins = {-18, -18, -24};
	vec3_t	mins = {-8, -8, -6};
	//vec3_t	maxs = {18, 18, 48};
	vec3_t	maxs = {8, 8, NPC->client->ps.crouchheight};

	VectorCopy(NPC->s.origin, org);
	org[2]+=STEPSIZE;

	VectorCopy(dest, destorg);
	destorg[2]+=STEPSIZE;

	trap_Trace( &trace, org, mins, maxs, destorg, NPC->s.number, CONTENTS_PLAYERCLIP/*NPC->clipmask*/ );

	//Do a simple check
	if ( trace.allsolid || trace.startsolid )
	{//inside solid
		//G_Printf("SOLID!\n");
		return qfalse;
	}

	if ( trace.fraction < 1.0f )
	{//hit something
		if ( (impactEntNum != ENTITYNUM_NONE && trace.entityNum == impactEntNum )
			/*|| !Q_stricmp(g_entities[trace.entityNum].classname, "worldspawn")*/ )
		{//hit what we're going after
			//G_Printf("OK!\n");
			return qtrue;
		}
		else
		{
			//G_Printf("TRACE FAIL! - NPC %i hit entity %i (%s).\n", NPC->s.number, trace.entityNum, g_entities[trace.entityNum].classname);
			return qfalse;
		}
	}

	/*
	//otherwise, clear path in a straight line.  
	//Now at intervals of my size, go along the trace and trace down STEPSIZE to make sure there is a solid floor.
	VectorSubtract( dest, NPC->r.currentOrigin, dir );
	dist = VectorNormalize( dir );
	if ( dest[2] > NPC->r.currentOrigin[2] )
	{//going up, check for steps
		drop = STEPSIZE;
	}
	else
	{//going down or level, check for moderate drops
		drop = 64;
	}
	for ( i = NPC->r.maxs[0]*2; i < dist; i += NPC->r.maxs[0]*2 )
	{//FIXME: does this check the last spot, too?  We're assuming that should be okay since the enemy is there?
		VectorMA( NPC->r.currentOrigin, i, dir, start );
		VectorCopy( start, end );
		end[2] -= drop;
		trap_Trace( &trace, start, mins, NPC->r.maxs, end, NPC->s.number, NPC->clipmask );//NPC->r.mins?
		if ( trace.fraction < 1.0f || trace.allsolid || trace.startsolid )
		{//good to go
			continue;
		}
		G_Printf("FLOOR!\n");
		//no floor here! (or a long drop?)
		return qfalse;
	}*/
	//we made it!
	return qtrue;
}

extern qboolean InFOV3( vec3_t spot, vec3_t from, vec3_t fromAngles, int hFOV, int vFOV );
extern void G_UcmdMoveForDir( gentity_t *self, usercmd_t *cmd, vec3_t dir );
extern qboolean DOM_Jedi_ClearPathToSpot( gentity_t *NPC, vec3_t dest, int impactEntNum );

// Conversations...
extern void NPC_FindConversationPartner();
extern void NPC_StormTrooperConversation();

qboolean NPC_FollowRoutes( void ) 
{// Quick method of following bot routes...
	vec3_t	angles, origin, velocity_vec;//, fwd;
	float	velocity;

	if (gWPNum <= 0)
	{// No waypoints available...
		return qfalse;
	}

	if (NPC->NPC->conversationPartner)
	{// Chatting with another NPC... Stay still!
		NPC_StormTrooperConversation();
		return qfalse;
	}

	if (!NPC->enemy && !NPC->NPC->conversationPartner)
	{// UQ1: Strange place to do this, but whatever... ;)
		NPC_FindConversationPartner();
	}

	VectorCopy(NPC->client->ps.velocity, velocity_vec);
	//velocity_vec[ROLL] = 0;
	velocity = VectorLength(velocity_vec);

	/*
	if ((gWPArray[NPC->wpCurrent]->flags & WPFLAG_WAITFORFUNC) 
		&& CheckForFunc(gWPArray[NPC->wpCurrent]->origin, NPC->s.number) 
		&& NPC_WaitForFunc(NPC))
	{
		//G_Printf("Waiting for func!\n");
		ucmd.forwardmove = 0;
		ucmd.rightmove = 0;
		ucmd.upmove = 0;
		return qfalse; // Waiting for a lift...
	}*/

	if (NPC->wpCurrent < 0 
		|| NPC->wpCurrent >= gWPNum 
		|| NPC->longTermGoal < 0 
		|| NPC->longTermGoal >= gWPNum 
		|| (NPC->wpTravelTime < level.time && velocity < 16))
	{// We hit a problem in route, or don't have one yet.. Find a new goal and path...
		//VectorSet(NPC->bot_strafe_target_position, 0, 0, 0); // init avoidance temporary waypoint...

		if (NPC->enemy)
			NPC_SetEnemyGoal();
		else
			NPC_SetNewGoalAndPath();

		return qfalse; // next think...
	}

	if (NPC->wpCurrent < 0 || NPC->wpCurrent >= gWPNum || NPC->longTermGoal < 0 || NPC->longTermGoal >= gWPNum)
	{// Try again to find a path. This time it should ignore wpCurrent and find another start waypoint...
		//NPC_Avoidance();
		return qfalse; // next think...
	}

	if (NPC->enemy && NPC_EnemyVisible( NPC, NPC->enemy ))
	{// Can already can see them... Close enough!
		return qfalse;
	}

	/*
	if (!DOM_NPC_ClearPathToSpot( NPC, gWPArray[NPC->wpCurrent]->origin, -1 ))
	{// Next waypoint is not walkable... Idle until we pick a new path...
		NPC_Avoidance();
		return qtrue;
	}
	*/

	/*
	if (NPC->bot_strafe_target_position[0] != 0 
		&& NPC->bot_strafe_target_position[1] != 0 
		&& NPC->bot_strafe_target_position[2] != 0)
	{// Using temporary waypoint for obstacle avoidance...
		if (VectorDistanceNoHeight(NPC->bot_strafe_target_position, NPC->r.currentOrigin) < 16) // 24?
		{// Got there, we can continue along our normal path!
			VectorSet(NPC->bot_strafe_target_position, 0, 0, 0);
			NPC->wpTravelTime = level.time + 10000;
			G_Printf("NPC %s hit his avoidance spot.\n", NPC->NPC_type);
		}
	}
	*/

	if (VectorDistanceNoHeight(gWPArray[NPC->longTermGoal]->origin, NPC->r.currentOrigin) < 24)//16)
	{// We're at out goal! Find a new goal...
		//G_Printf("NPC %i hit it's goal waypoint!!!\n", NPC->s.number);
		NPC_SetNewGoalAndPath();
		return qfalse; // next think...
	}

	if (NPC->wpCurrent < 0 || NPC->wpCurrent >= gWPNum || NPC->longTermGoal < 0 || NPC->longTermGoal >= gWPNum)
	{// FIXME: Try to roam out of problems...
		//NPC_Avoidance();
		return qfalse; // next think...
	}

	if (VectorDistanceNoHeight(gWPArray[NPC->wpCurrent]->origin, NPC->r.currentOrigin) < 24)//16)
	{// At current node.. Pick next in the list...
		NPC->wpLast = NPC->wpCurrent;
		NPC->wpCurrent = NPC->wpNext;
		NPC->wpNext = NPC_GetNextNode(NPC);
		NPC->wpTravelTime = level.time + 15000; // maximum of 10 seconds to traverse to the next waypoint...

		if (NPC->enemy && NPC->enemy->s.eType & ET_PLAYER)
			NPC->wpTravelTime = level.time + 5000; // maximum of 5 seconds to traverse to the next waypoint...
		//G_Printf("NPC %i hit a waypoint. Pathsize is now %i.\n", NPC->s.number, NPC->pathsize);
	}

	if (NPC->wpCurrent < 0 || NPC->wpCurrent >= gWPNum || NPC->longTermGoal < 0 || NPC->longTermGoal >= gWPNum)
	{// FIXME: Try to roam out of problems...
		//NPC_Avoidance();
		return qfalse; // next think...
	}

	/*
	if (NPC->bot_strafe_target_position[0] != 0 
		&& NPC->bot_strafe_target_position[1] != 0 
		&& NPC->bot_strafe_target_position[2] != 0)
	{// Using temporary waypoint for obstacle avoidance...
		VectorCopy(NPC->bot_strafe_target_position, origin);
		VectorSubtract( origin, NPC->r.currentOrigin , NPC->move_vector );
		vectoangles( NPC->move_vector, angles );
		G_SetAngles(NPC, angles);
		VectorCopy(angles, NPC->client->ps.viewangles);
		NPC_FacePosition( NPC->bot_strafe_target_position, qfalse );
	}
	else*/
	{// Aim for current waypoint...
		VectorCopy(gWPArray[NPC->wpCurrent]->origin, origin);
		VectorSubtract( origin, NPC->r.currentOrigin , NPC->move_vector );
		vectoangles( NPC->move_vector, angles );
		G_SetAngles(NPC, angles);
		VectorCopy(angles, NPC->client->ps.viewangles);
		NPC_FacePosition( gWPArray[NPC->wpCurrent]->origin, qfalse );
	}

	ucmd.angles[YAW] = ANGLE2SHORT( NPC->client->ps.viewangles[YAW] ) - NPC->client->ps.delta_angles[YAW];
	ucmd.angles[PITCH] = ANGLE2SHORT( NPC->client->ps.viewangles[PITCH] ) - NPC->client->ps.delta_angles[PITCH];
	ucmd.angles[ROLL] = ANGLE2SHORT( NPC->client->ps.viewangles[ROLL] ) - NPC->client->ps.delta_angles[ROLL];

	if (NPC_NeedJump())
	{
		//if (NPC_ClearPathToJump( NPC, gWPArray[NPC->wpCurrent]->origin, -1 ))
		{
			if (NPC_TryJump( NPC, jumpLandPosition ))
			{
				NPC_SelectMoveType();
				return qtrue;
			}
		}
	}

	if ((velocity <= 16 && InFOV3( gWPArray[NPC->wpCurrent]->origin, NPC->r.currentOrigin, NPC->client->ps.viewangles, 30, 120 )) // InFOV so they don't do this while turning...
		|| NPC->bot_strafe_right_timer > level.time
		|| NPC->bot_strafe_left_timer > level.time
		|| NPC->bot_strafe_jump_timer > level.time
		|| NPC->bot_strafe_crouch_timer > level.time)
	{// Trouble moving, need some (currently really really simple) avoidance stuff...
		//if (NPC_ClearPathToJump( NPC, gWPArray[NPC->wpCurrent]->origin, -1 ))
		{
			if (!NPC_TryJump( NPC, jumpLandPosition ))
				NPC_Avoidance();
		}
		//else
		//	NPC_Avoidance();
	}

	if (NPC->client->ps.pm_flags & PMF_DUCKED)
	{// Clear any previous flags...
		NPC->client->ps.pm_flags &= ~PMF_DUCKED;
	}

	if (NPC->client->ps.pm_flags & PMF_JUMP_HELD)
	{// Clear any previous flags...
		NPC->client->ps.pm_flags &= ~PMF_JUMP_HELD;
	}

	//
	// UQ1: Note that (ucmd) NPC jumping causes wierd screen shaking in JKA... Best avoided if possible...
	//

	if (gWPArray[NPC->wpCurrent]->flags & WPFLAG_DUCK)
	{// Duck...
		NPC->client->ps.pm_flags |= PMF_DUCKED;
	}
	else if (gWPArray[NPC->wpCurrent]->flags & WPFLAG_JUMP
		|| gWPArray[NPC->wpCurrent]->origin[2] > NPC->r.currentOrigin[2])
	{// Jump...
		//if (NPC_ClearPathToJump( NPC, gWPArray[NPC->wpCurrent]->origin, -1 ))
		{
			NPC_TryJump( NPC, jumpLandPosition );
		}
	}

	/*
	if (velocity <= 8)
	{
		NPC_TryJump( NPC, jumpLandPosition );
	}
	*/

	// Set speed and select an animation based on velocity...
	//NPC_Avoidance();
	NPC_SelectMoveType();

	//G_UcmdMoveForDir( NPC, &ucmd, NPC->move_vector );

	return qtrue;
}

#endif //__DOMINANCE_NPC__

