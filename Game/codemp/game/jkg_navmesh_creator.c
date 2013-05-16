/*=========================================================
// Navigation Mesh Editor
//---------------------------------------------------------
// Description:
// Code for the creation, and editing of the navigation mesh
// used by NPCs.
//
// Version:
// $Id$
//=========================================================*/

#include <math.h>
#include "bg_public.h"
#include "jkg_dynarrays.h"
#include "jkg_navmesh_creator.h"

#define GRID_SIZE (30.0f)

typedef enum direction_e
{
    NORTH = 0,
    EAST,
    SOUTH,
    WEST,
    
    NUM_DIRECTIONS
} direction_t;

typedef enum generationStage_e
{
    GS_NONE,
    GS_CREATING_NODES,
    GS_CREATING_AREAS,
} generationStage_t;

typedef struct meshNode_s
{
    vec3_t position;
    vec3_t normal;
    
    qboolean visited;
    int directionsVisited;
} meshNode_t;

// Mesh node builder
static jkgArray_t nodeList;
static meshNode_t *startBuildNode = NULL;
static generationStage_t buildStage = GS_NONE;

// Mesh node visualization
static qboolean forceRevisualization = qfalse;
static int showLabelsToClient;

extern vmCvar_t jkg_nav_edit;

static void JKG_Nav_Visualize ( void );
static void JKG_Nav_Generate ( void );

//=========================================================
// SnapToGrid
//---------------------------------------------------------
// Description:
// Snaps an XY coordinate to a given grid size.
//=========================================================
static void SnapToGrid ( float x, float y, float *newX, float *newY )
{
    float cx = x / GRID_SIZE;
    float cy = y / GRID_SIZE;
    
    cx = floor (cx + 0.5f);
    cy = floor (cy + 0.5f);
    
    *newX = GRID_SIZE * cx;
    *newY = GRID_SIZE * cy;
}

//=========================================================
// JKG_Nav_Init
//---------------------------------------------------------
// Description:
// Initializes the navigation mesh system.
//=========================================================
void JKG_Nav_Init ( void )
{
    JKG_Array_Init (&nodeList, sizeof (meshNode_t), 16);
    showLabelsToClient = 0;
}

//=========================================================
// JKG_Nav_Shutdown
//---------------------------------------------------------
// Description:
// Shuts down the navigation mesh system.
//=========================================================
void JKG_Nav_Shutdown ( void )
{
    JKG_Array_Free (&nodeList);
}

//=========================================================
// JKG_Nav_Cmd_Generate_f
//---------------------------------------------------------
// Description:
// Begins the navigation mesh generation.
//=========================================================
void JKG_Nav_Cmd_Generate_f ( gentity_t *ent )
{
    if ( !jkg_nav_edit.integer )
    {
        return;
    }
    
    if ( nodeList.size == 0 )
    {
        return;
    }
    
    startBuildNode = (meshNode_t *)nodeList.data;
    buildStage = GS_CREATING_NODES;
}

//=========================================================
// JKG_Nav_Cmd_MarkWalkableSurface_f
//---------------------------------------------------------
// Description:
// Marks the surface the player is looking at as a walkable
// surface.
//=========================================================
void JKG_Nav_Cmd_MarkWalkableSurface_f ( gentity_t *ent )
{
    vec3_t forward;
    trace_t trace;
    vec3_t start, end;
    qboolean reachedGroundOrEnd = qfalse;
    
    if ( !jkg_nav_edit.integer )
    {
        return;
    }
    
    if ( !ent->client )
    {
        return;
    }
    
    VectorCopy (ent->client->ps.origin, start);
    start[2] += ent->client->ps.viewheight;
    
    AngleVectors (ent->client->ps.viewangles, forward, NULL, NULL);
    
    do
    {
        VectorMA (start, 16384.0f, forward, end);
    
        trap_Trace (&trace, start, NULL, NULL, end, ent->s.number, CONTENTS_SOLID);
        
        if ( trace.fraction == 1.0f )
        {
            // Managed to reach the end without hitting anything???
            reachedGroundOrEnd = qtrue;
        }
        else if ( trace.startsolid != qtrue )
        {
            if ( trace.entityNum == ENTITYNUM_WORLD )
            {
                meshNode_t node;
                VectorCopy (trace.endpos, node.position);
                SnapToGrid (node.position[0], node.position[1], &node.position[0], &node.position[1]);
                
                VectorCopy (trace.plane.normal, node.normal);
                node.directionsVisited = 0;
                node.visited = qfalse;
                
                #ifdef _DEBUG
                trap_SendServerCommand (ent->s.number, va ("print \"Placed mesh node at %s.\n\"", vtos (node.position)));
                #endif
                
                JKG_Array_Add (&nodeList, (void *)&node);
                
                reachedGroundOrEnd = qtrue;
                forceRevisualization = qtrue;
            }
        }
    }
    while ( !reachedGroundOrEnd );
}

//=========================================================
// JKG_Nav_Editor_Run
//---------------------------------------------------------
// Description:
// Performs per-frame actions for the navigation mesh such
// as generating the mesh and sending visual data back to
// the players.
//=========================================================
void JKG_Nav_Editor_Run ( void )
{
    JKG_Nav_Visualize();
}

//=========================================================
// JKG_Nav_Cmd_VisLabelsClient_f
//---------------------------------------------------------
// Description:
// Command to set the client who can see the navigation
// mesh visuals.
//=========================================================
void JKG_Nav_Cmd_VisLabelsClient_f ( gentity_t *ent )
{
    char buffer[MAX_TOKEN_CHARS] = { 0 };
    int clientId = -1;
    gentity_t *e = NULL;
    
    if ( !jkg_nav_edit.integer )
    {
        return;
    }
    
    if ( trap_Argc() < 2 )
    {
        trap_SendServerCommand (ent->s.number, "print \"Usage: nav_vislabels_client <client id>\n\"");
        return;
    }
    
    trap_Argv (1, buffer, sizeof (buffer));
    if ( buffer[0] < '0' || buffer[0] > '9' )
    {
        trap_SendServerCommand (ent->s.number, "print \"Invalid client ID.\n\"");
        return;
    }
    
    clientId = atoi (buffer);
    if ( clientId >= MAX_CLIENTS )
    {
        trap_SendServerCommand (ent->s.number, "print \"Invalid client ID.\n\"");
        return;
    }
    
    e = &g_entities[clientId];
    if ( !e->client || e->client->pers.connected != CON_CONNECTED )
    {
        trap_SendServerCommand (ent->s.number, "print \"Client is not connected.\n\"");
        return;
    }
    
    showLabelsToClient = clientId;
}

static void JKG_Nav_Node_Recurse ( meshNode_t *node )
{
	int i;
	//trace_t tr;
	const int crouchHeight = CROUCH_MAXS_2 - DEFAULT_MINS_2;

	if ( node->visited )
	{
		return;
	}

	for ( i = 0; i < NUM_DIRECTIONS; i++ )
	{
		if ( !(node->directionsVisited & NORTH) )
		{
			vec3_t start, end;

			VectorCopy (node->position, start);
			VectorCopy (node->position, end);

			end[0] -= GRID_SIZE;

			// Check if we've already visited such a place.

			// Check deadly drop and get new Z height for end position.

			// Make sure the player can at least crouch
			start[2] += crouchHeight;
			end[2] += crouchHeight;

			// Do trace, make sure we can reachend position.
			
			// All good, we add a new node to the list.
		}
	}
}

static void JKG_Nav_Create_Nodes ( void )
{
    //int i;

    if ( !jkg_nav_edit.integer || buildStage != GS_CREATING_NODES )
    {
        return;
    }
    
    
}

//=========================================================
// JKG_Nav_Generate
//---------------------------------------------------------
// Description:
// Performs more generation of the navigation mesh. Does
// nothing if a navigation mesh is not flagged to be
// generated.
//=========================================================
static void JKG_Nav_Generate ( void )
{
    if ( !jkg_nav_edit.integer )
    {
        return;
    }
    
    if ( buildStage == GS_NONE )
    {
        return;
    }
    
	while ( buildStage != GS_NONE )
	{
		switch ( buildStage )
		{
		case GS_CREATING_NODES:
			JKG_Nav_Create_Nodes ();
			buildStage = GS_CREATING_AREAS;
		break;
	    
		case GS_CREATING_AREAS:
			buildStage = GS_NONE;
		break;
	    
		default:
			G_Printf ("Unknown build stage of navigation mesh generation reached. Stopping generation...\n");
			buildStage = GS_NONE;
		break;
		}
	}
}

//=========================================================
// JKG_Nav_Visualize
//---------------------------------------------------------
// Description:
// Displays visually the navigation mesh to a single
// player.
//=========================================================
extern void G_TestLine(vec3_t start, vec3_t end, int color, int time);
static void JKG_Nav_Visualize ( void )
{
    static int nextDrawTime = 0;
    gentity_t *show = NULL;
    
    if ( !jkg_nav_edit.integer )
    {
        return;
    }
    
    show = &g_entities[showLabelsToClient];
    if ( !show->inuse || !show->client )
    {
        return;
    }
    
    if ( show->client->pers.connected != CON_CONNECTED )
    {
        return;
    }
    
    if ( forceRevisualization || nextDrawTime <= level.time )
    {
        int i;
        vec3_t lineEnd;
        vec3_t displacement;
        meshNode_t *nodeListData = (meshNode_t *)nodeList.data;
        meshNode_t *closestNode = NULL;
        float shortestDistance = 999999999.9f;
        float distance = 0.0f;
    
        forceRevisualization = qfalse;
        for ( i = 0; i < nodeList.size; i++ )
        {
            VectorMA (nodeListData[i].position, 20.0f, nodeListData[i].normal, lineEnd);
            G_TestLine (nodeListData[i].position, lineEnd, 0x000000FF, 5000);
            
            VectorSubtract (nodeListData[i].position, show->s.origin, displacement);
            
            distance = VectorLength (displacement);
            if ( distance < shortestDistance )
            {
                shortestDistance = distance;
                closestNode = &nodeListData[i];
            }
        }
        
        // TODO: do something with closest node. somehow display information about it to the player?
        
        nextDrawTime = level.time + 4000;
    }
}