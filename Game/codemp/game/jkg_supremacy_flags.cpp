#include "..\game\g_local.h"

//====================================================================================================
// File         : scenario_flags.c
// Description  : This .c file is for an external file method of loading JK scenario flag positions.
// Author       : Unique1
//====================================================================================================

#define MAX_FLAG_POSITIONS 50
qboolean flags_loaded = qfalse;
extern int number_of_flags; 
extern void SP_Spawn_Scenario_Flag_New ( gentity_t* ent );
extern void Spawn_Scenario_Flag_Auto ( vec3_t origin, int teamowner );

qboolean flag_file_loaded = qfalse;

//===========================================================================
// Routine      : Warzone_Flag_Add
// Description  : Adds a flag position to the array.
void Warzone_Flag_Add ( vec3_t origin, int team )
{
	if (g_gametype.integer != GT_WARZONE)
		return;

	if (number_of_flags > MAX_FLAG_POSITIONS)
	{
		G_Printf("^1*** ^3Warzone^1: ^3Warning! ^5Hit maximum flag positions (^7%i^5)!\n", MAX_FLAG_POSITIONS);
		return;
	}

	Spawn_Scenario_Flag_Auto ( origin, team );

	G_Printf("^1*** ^3Warzone^1: ^5Flag number ^7%i^5 added at position ^7%f %f %f^5.\n", number_of_flags, origin[0], origin[1], origin[2] );

	//number_of_flags++; // Will always be in front of the actual number by one while creating.
}

// UQ1: Depreciated - Only keeping this for old file support...
//===========================================================================
// Routine      : Warzone_Flag_Loadpositions
// Description  : Loads flag positions from .flags file on disk
void Warzone_Flag_Loadpositions( void )
{// Does each online player's data.
	char *s, *t;
	int len;
	fileHandle_t	f;
	char *buf;
	char *loadPath;
	int statnum = 0;
	float stats[50*4]; // 1 extra.
//	int loop = 0;
	int flag_number = 0;
	vmCvar_t		mapname;
	int		num_flags = 0;

	G_Printf("^1*** ^3Warzone^1: ^5Loading scenario flag position table...\n");

	loadPath = (char *)malloc(1024*4);

	trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );

	Com_sprintf(loadPath, 1024*4, "flag_positions/%s.flags\0", mapname.string);

	len = trap_FS_FOpenFile( loadPath, &f, FS_READ );
	if ( !f )
	{
		G_Printf(" ^3FAILED!!!\n");
		G_Printf("^1*** ^3Warzone^1: ^5No file exists! (^3%s^5)\n", loadPath);
		free(loadPath);
		return;
	}
	if ( !len )
	{ //empty file
		G_Printf(" ^3FAILED!!!\n");
		G_Printf("^1*** ^3Warzone^1: ^5Empty file!\n");
		trap_FS_FCloseFile( f );
		free(loadPath);
		return;
	}

	if ( (buf = (char *)malloc(len+1)) == 0 )
	{//alloc memory for buffer
		G_Printf(" ^3FAILED!!!\n");
		G_Printf("^1*** ^3Warzone^1: ^5Unable to allocate buffer.\n");
		free(loadPath);
		return;
	}
	trap_FS_Read( buf, len, f );
	trap_FS_FCloseFile( f );

	for (t = s = buf; *t; /* */ ) 
	{
		s = strchr(s, ' ');
		
		if (!s)
			break;

		while (*s == ' ')
			*s++ = 0;

		if (*t)
		{
			if (statnum == 0)
			{
				(int)num_flags = atoi(t);
				
				if (num_flags < 2)
				{
					G_Printf(" ^3FAILED!!!\n");
					G_Printf("^1*** ^3Warzone^1: ^5You need at least 2 flag points!\n");
					return;
				}
				else
				{
					statnum++;
					t = s;
					continue;
				}
			}

			(float)stats[statnum] = (float)atof(va("%s", t));

			statnum++;
		}

		t = s;
	}

	statnum = 1;

	while (flag_number < num_flags)
	{
		int reference = 0;
		vec3_t origin;
		int team = 0;

		while (reference <= 3)
		{
			if (reference <= 2)
			{
				(float)origin[reference] = (float)stats[statnum];
			}
			else
			{
				(int)team = (float)stats[statnum];
			}

			statnum++;
			reference++;
		}

		//G_Printf("Origin is %f %f %f.\n", origin[0], origin[1], origin[2]);

		if (origin[0] == 0 && origin[1] == 0)
			break;

		if (origin[0] == 0 && origin[2] == 64)
			break;

		//number_of_flags--;
		Spawn_Scenario_Flag_Auto ( origin, team );
		//Warzone_Flag_Add ( origin, team );

		flag_number++;
	}

	free(buf);
	free(loadPath);

	//G_Printf("^3Completed OK.\n");
	G_Printf("^1*** ^3Warzone^1: ^5Total Flag Positions: ^7%i^5.\n", number_of_flags);
	flags_loaded = qtrue;
}

#ifdef __OLD__
//===========================================================================
// Routine      : Warzone_Flags_Savepositions
// Description  : Saves flag positions to a .flags file on disk
void Warzone_Flags_Savepositions( void )
{
	fileHandle_t	f;
	char			*fileString;
	char			*savePath;
	vmCvar_t		mapname;
	char			lineout[MAX_INFO_STRING];
	int				loop = 0;

	number_of_flags--;

	G_Printf("^3*** ^3AIMod: ^7Saving flag position table.\n");

	fileString = NULL;

	savePath = (char *)malloc(1024*4);

	trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );

	Com_sprintf(savePath, 1024*4, "flag_positions/%s.flags\0", mapname.string);

	trap_FS_FOpenFile(savePath, &f, FS_WRITE);

	if ( !f )
	{
		free(savePath);
		return;
	}

	Com_sprintf( lineout, sizeof(lineout), "%i ", number_of_flags+1);
	trap_FS_Write( lineout, strlen(lineout), f);

	while (loop < number_of_flags+1)
	{
		char lineout[MAX_INFO_STRING];

		Com_sprintf( lineout, sizeof(lineout), "%f %f %f %i ", 
				flag_list[loop].flagentity->s.origin[0],
				flag_list[loop].flagentity->s.origin[1],
				flag_list[loop].flagentity->s.origin[2],
				flag_list[loop].flagentity->s.teamowner);
		
		trap_FS_Write( lineout, strlen(lineout), f);

		loop++;
	}

	G_Printf("^3*** ^3AIMod: ^7Flag Position table saved %i flag positions to file %s.\n", number_of_flags+1, savePath);

	trap_FS_FCloseFile( f );
	
	free(savePath);
}
#endif //__OLD__

void WARZONE_LoadGameInfo ( void ); // below...

//===========================================================================
// Routine      : Warzone_Create_Flags
// Description  : Put flags on the map...
void Warzone_Create_Flags( void )
{// Load and put saved flags on the map...
	if (number_of_flags >= 2)
		return; // Some are on the map.. Don't need to load external file...

	if (!flag_file_loaded)
	{
		WARZONE_LoadGameInfo();
		flag_file_loaded = qtrue;
	}
}

//
// Ammo & Health Crates...
//

//extern qboolean ClientNeedsAmmo( int client );
//extern gentity_t *Drop_Item( gentity_t *ent, gitem_t *item, float angle);

//===========================================================================
// Routine      : ammo_crate_think
// Description  : Main think function for ammo crates.
void ammo_crate_think( gentity_t *ent )
{
	vec3_t	mins;
	vec3_t	maxs;
	int		touch[MAX_GENTITIES];
	int		num = 0;
	int		i = 0;
	float	crate_use_distance = ent->s.generic1;
	int		num_flags = number_of_flags;

	ent->s.eFlags |= EF_RADAROBJECT;

	if (g_gametype.integer == GT_WARZONE && num_flags >= 2)
	{
		float	best_distance = 99999.9f;
		int		best_flag = 0;

		for (i = 0; i < num_flags; i++)
		{
			float distance = Distance(flag_list[i].flagentity->s.origin, ent->s.origin);
			
			if (distance <= best_distance)
			{
				best_flag = i;
				best_distance = distance;
			}
		}

		if (best_distance < 99999.9)
		{
			int flagteam = flag_list[best_flag].flagentity->s.modelindex;

			if (flagteam != TEAM_BLUE && flagteam != TEAM_RED)
				ent->s.frame = 0; // Use closed box!
			else
				ent->s.frame = 1; // Use open box!
		}
	}

	VectorSet( mins, 0-(crate_use_distance), 0-(crate_use_distance), 0-(crate_use_distance) );
	VectorSet( maxs, crate_use_distance, crate_use_distance, crate_use_distance );

	VectorAdd( mins, ent->s.origin, mins );
	VectorAdd( maxs, ent->s.origin, maxs );

	if (ent->nextthink > level.time)
		return;

	if (!ent->s.number)
		return;

	if (ent->s.frame == 0)
	{// Box is currently closed! Don't think any more!
		// Set next think...
		ent->nextthink = level.time + 2000;
		return; 
	}

	// Run the think... Look for players...
	num = trap_EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

	for ( i=0 ; i<num ; i++ ) 
	{
		gentity_t *hit = &g_entities[touch[i]];

		if (!hit)
			continue;

		//G_Printf("%s is in box!\n", hit->classname);

		if (!hit->client)
			continue;

		if (hit->s.eType != ET_PLAYER)
			continue;

		if (hit->s.number > MAX_CLIENTS)
			continue;

		if (!hit->client->sess.sessionTeam)
			continue;

		if (!hit->client->pers.connected)
			continue;

		if (hit->health <= 0)
			continue;

//		if (VectorDistance(hit->r.currentOrigin, ent->s.origin) > crate_use_distance)
//			continue;

		// Give them ammo! -- UQ1: Fixme!
		/*
		if (ClientNeedsAmmo(touch[i]))
		{
			gitem_t		*item;
			gentity_t	*drop = NULL;
			item = BG_FindItem("Ammo Pack");
			drop = Drop_Item (hit, item, 0);
			drop->count = 1;
			continue;
		}
		*/
	}

	// Set next think...
	ent->nextthink = level.time + 2000;
}

//===========================================================================
// Routine      : SP_ammo_crate_spawn
// Description  : Spawn an ammo crate.
void SP_ammo_crate_spawn ( gentity_t* ent )
{// Ammo Crate... Used by map entitiy to call the flag's spawn...
	char *model, *model2;
	int radius = 0;

	if (g_gametype.integer != GT_WARZONE)
	{
		G_FreeEntity(ent);
		return;
	}

	G_SpawnString("model", "", &model); // Custom model..
	G_SpawnString("model2", "", &model2); // Custom model..
	G_SpawnInt("radius", "", &radius); // Point's capture radius..
	G_SpawnInt( "spawnflags", "", &ent->spawnflags );
	//G_SpawnFloat( "scale", "", &ent->scale );
	G_SpawnFloat( "angle", "", &ent->angle );

	G_SetOrigin( ent, ent->s.origin );

	if(!model || model == "" || !strcmp(model, "" ))
	{// ent->model can be specified...
		//ent->model = "models/multiplayer/supplies/ammobox_wm.md3";
		ent->model = "models/doa/doa_cabinet/ammo_open.md3";
		
		if (!radius)
		{// Default radius...
			ent->s.generic1 = 64; // Set capture radius...
		}
		else
		{
			ent->s.generic1 = radius; // Set capture radius...
		}
	}
	else
	{
		ent->model = model;

		if (!radius)
		{// Default radius...
			ent->s.generic1 = 64; // Set capture radius...
		}
		else
		{
			ent->s.generic1 = radius; // Set capture radius...
		}
	}

	if(!model2 || model2 == "" || !strcmp(model2, "" ))
	{// ent->model can be specified...
		//ent->model = "models/multiplayer/supplies/healthbox_wm.md3";
		ent->model2 = "models/doa/doa_cabinet/ammo_close.md3";
	}
	else
	{
		ent->model2 = model;
	}

	ent->s.angles[YAW] = ent->angle;

	ent->s.modelindex = G_ModelIndex (ent->model);
	ent->s.modelindex2 = G_ModelIndex (ent->model2);

	ent->targetname = NULL;
	ent->classname = "ammo_crate";
	ent->s.eType = ET_AMMO_CRATE;
	ent->think = ammo_crate_think;
	
	// UQ1: Need to fix this! Only basic angles for now!
	if (ent->angle == 90 || ent->angle == 270)
	{
		VectorSet( ent->r.mins, -16, -32, 0 );
		VectorSet( ent->r.maxs, 16, 32, 64 );
	}
	else if (ent->angle == 45 || ent->angle == 135 || ent->angle == 225 || ent->angle == 315)
	{
		VectorSet( ent->r.mins, -24, -24, 0 );
		VectorSet( ent->r.maxs, 24, 24, 64 );
	}
	else
	{
		VectorSet( ent->r.mins, -32, -16, 0 );
		VectorSet( ent->r.maxs, 32, 16, 64 );
	}

	ent->r.contents = CONTENTS_SOLID|CONTENTS_BODY|CONTENTS_MONSTERCLIP;

	trap_LinkEntity (ent);

	G_Printf("Spawned ammo crate at %f %f %f.\n", ent->s.origin[0], ent->s.origin[1], ent->s.origin[2]);

	ent->nextthink = level.time;
}

//===========================================================================
// Routine      : health_crate_think
// Description  : Main think function for health crates.
void health_crate_think( gentity_t *ent )
{
	vec3_t	mins;
	vec3_t	maxs;
	int		touch[MAX_GENTITIES];
	int		num = 0;
	int		i = 0;
	float	crate_use_distance = ent->s.generic1;
	int		num_flags = number_of_flags;

	ent->s.eFlags |= EF_RADAROBJECT;

	if (g_gametype.integer == GT_WARZONE && num_flags >= 2)
	{
		float	best_distance = 99999.9f;
		int		best_flag = 0;

		for (i = 0; i < num_flags; i++)
		{
			float distance = Distance(flag_list[i].flagentity->s.origin, ent->s.origin);
			
			if (distance <= best_distance)
			{
				best_flag = i;
				best_distance = distance;
			}
		}

		if (best_distance < 99999.9)
		{
			int flagteam = flag_list[best_flag].flagentity->s.modelindex;

			if (flagteam != TEAM_BLUE && flagteam != TEAM_RED)
				ent->s.frame = 0; // Use closed box!
			else
				ent->s.frame = 1; // Use open box!
		}
	}

	VectorSet( mins, 0-(crate_use_distance), 0-(crate_use_distance), 0-(crate_use_distance) );
	VectorSet( maxs, crate_use_distance, crate_use_distance, crate_use_distance );

	VectorAdd( mins, ent->s.origin, mins );
	VectorAdd( maxs, ent->s.origin, maxs );

	if (ent->nextthink > level.time)
		return;

	if (!ent->s.number)
		return;

	if (ent->s.frame == 0)
	{// Box is currently closed! Don't think any more!
		// Set next think...
		ent->nextthink = level.time + 2000;
		return; 
	}

	// Run the think... Look for players...
	num = trap_EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

	for ( i=0 ; i<num ; i++ ) 
	{
		gentity_t *hit = &g_entities[touch[i]];

		if (!hit)
			continue;

		//G_Printf("%s is in box!\n", hit->classname);

		if (!hit->client)
			continue;

		if (hit->s.eType != ET_PLAYER)
			continue;

		if (hit->s.number > MAX_CLIENTS)
			continue;

		if (!hit->client->sess.sessionTeam)
			continue;

		if (!hit->client->pers.connected)
			continue;

		if (hit->health <= 0)
			continue;

//		if (VectorDistance(hit->r.currentOrigin, ent->s.origin) > crate_use_distance)
//			continue;

		// Give them ammo!
		//G_Printf("%s should be recieving health!\n", hit->client->pers.netname);
		if (hit->client->ps.stats[STAT_HEALTH] < hit->client->ps.stats[STAT_MAX_HEALTH])
		{
			// UQ1: We have no equivalent "generic health" item in JKG. Will add directly... For now...
			//gitem_t		*item;
			//gentity_t	*drop = NULL;
			//item = BG_FindItemForClassName("item_health");
			//drop = Drop_Item (hit, item, 0, qfalse);
			//drop->count = 1;

			hit->client->ps.stats[STAT_HEALTH] = hit->client->ps.stats[STAT_HEALTH] + 20;

			if (hit->client->ps.stats[STAT_HEALTH] > hit->client->ps.stats[STAT_MAX_HEALTH]) 
				hit->client->ps.stats[STAT_HEALTH] = hit->client->ps.stats[STAT_MAX_HEALTH];
			continue;
		}
	}

	// Set next think...
	ent->nextthink = level.time + 2000;
}

//===========================================================================
// Routine      : SP_health_crate_spawn
// Description  : Spawn a health crate.
void SP_health_crate_spawn ( gentity_t* ent )
{// Health Crate... Used by map entitiy to call the flag's spawn...
	char *model, *model2;
	int radius = 0;

	if (g_gametype.integer != GT_WARZONE)
	{
		G_FreeEntity(ent);
		return;
	}

	G_SpawnString("model", "", &model); // Custom model..
	G_SpawnString("model2", "", &model2); // Custom model..
	G_SpawnInt("radius", "", &radius); // Point's capture radius..
	G_SpawnInt( "spawnflags", "", &ent->spawnflags );
	//G_SpawnFloat( "scale", "", &ent->scale );
	G_SpawnFloat( "angle", "", &ent->angle );

	G_SetOrigin( ent, ent->s.origin );

	if(!model || model == "" || !strcmp(model, "" ))
	{// ent->model can be specified...
		ent->model = "models/doa/doa_cabinet/health_open.md3";

		if (!radius)
		{// Default radius...
			ent->s.generic1 = 64; // Set capture radius...
		}
		else
		{
			ent->s.generic1 = radius; // Set capture radius...
		}
	}
	else
	{
		ent->model = model;

		if (!radius)
		{// Default radius...
			ent->s.generic1 = 64; // Set capture radius...
		}
		else
		{
			ent->s.generic1 = radius; // Set capture radius...
		}
	}

	if(!model2 || model2 == "" || !strcmp(model2, "" ))
	{// ent->model can be specified...
		ent->model2 = "models/doa/doa_cabinet/health_close.md3";
	}
	else
	{
		ent->model2 = model;
	}

	ent->s.angles[YAW] = ent->angle;

	ent->s.modelindex = G_ModelIndex (ent->model);
	ent->s.modelindex2 = G_ModelIndex (ent->model2);

	ent->targetname = NULL;
	ent->classname = "health_crate";
	ent->s.eType = ET_HEALTH_CRATE;
	ent->think = health_crate_think;
	
	// UQ1: Need to fix this! Only basic angles for now!
	if (ent->angle == 90 || ent->angle == 270)
	{
		VectorSet( ent->r.mins, -16, -32, 0 );
		VectorSet( ent->r.maxs, 16, 32, 64 );
	}
	else if (ent->angle == 45 || ent->angle == 135 || ent->angle == 225 || ent->angle == 315)
	{
		VectorSet( ent->r.mins, -24, -24, 0 );
		VectorSet( ent->r.maxs, 24, 24, 64 );
	}
	else
	{
		VectorSet( ent->r.mins, -32, -16, 0 );
		VectorSet( ent->r.maxs, 32, 16, 64 );
	}

	ent->r.contents = CONTENTS_SOLID|CONTENTS_BODY|CONTENTS_MONSTERCLIP;

	trap_LinkEntity (ent);

	G_Printf("Spawned health crate at %f %f %f.\n", ent->s.origin[0], ent->s.origin[1], ent->s.origin[2]);

	ent->nextthink = level.time;
}

//
// Warzone gametype editing!
//

int		warzone_edit_num_flags = 0;
vec3_t	warzone_edit_flags[1024];
int		warzone_edit_flags_team[1024];

void WARZONE_AddFlag ( vec3_t origin, int team )
{
	VectorCopy(origin, warzone_edit_flags[warzone_edit_num_flags]);
	warzone_edit_flags[warzone_edit_num_flags][2]-=80;
	warzone_edit_flags_team[warzone_edit_num_flags] = team;
	warzone_edit_num_flags++;
}

int		warzone_edit_num_vehicles = 0;
vec3_t	warzone_edit_vehicles_origins[1024];
float	warzone_edit_vehicles_angle[1024];
int		warzone_edit_vehicles_type[1024];

void WARZONE_AddVehicle ( vec3_t origin, vec3_t angles, int vehicleType )
{
	float angle;

	angle = angles[YAW];

	while (angle > 360)
		angle -= 360;

	while (angle < -360)
		angle += 360;

	// Force 90 degree angles...
	if (angle > -45 && angle < 45)
	{
		angle = 0;
	}
	else if (angle >= 45 && angle <= 135)
	{
		angle = 90;
	}
	else if (angle >= 135 && angle <= 225)
	{
		angle = 180;
	}
	else if (angle >= 225 && angle <= 315)
	{
		angle = 270;
	}
	else if (angle >= 315 && angle <= 405)
	{
		angle = 360;
	}
	else if (angle <= -45 && angle >= -135)
	{
		angle = 90;
	}
	else if (angle <= -135 && angle >= -225)
	{
		angle = 180;
	}
	else if (angle <= -225 && angle >= -315)
	{
		angle = 270;
	}
	else if (angle <= -315 && angle >= -405)
	{
		angle = 360;
	}

	VectorCopy(origin, warzone_edit_vehicles_origins[warzone_edit_num_vehicles]);
	warzone_edit_vehicles_angle[warzone_edit_num_vehicles] = angle;
	warzone_edit_vehicles_type[warzone_edit_num_vehicles] = vehicleType;
	warzone_edit_num_vehicles++;
}

int		warzone_edit_num_ammo_crates = 0;
vec3_t	warzone_edit_ammo_crates_origins[1024];
float	warzone_edit_ammo_crates_angle[1024];

void WARZONE_AddAmmoCrate ( vec3_t origin, vec3_t angles )
{
	float angle;

	angle = angles[YAW];

	while (angle > 360)
		angle -= 360;

	while (angle < -360)
		angle += 360;

	// Force 90 degree angles...
	if (angle > -45 && angle < 45)
	{
		angle = 0;
	}
	else if (angle >= 45 && angle <= 135)
	{
		angle = 90;
	}
	else if (angle >= 135 && angle <= 225)
	{
		angle = 180;
	}
	else if (angle >= 225 && angle <= 315)
	{
		angle = 270;
	}
	else if (angle >= 315 && angle <= 405)
	{
		angle = 360;
	}
	else if (angle <= -45 && angle >= -135)
	{
		angle = 90;
	}
	else if (angle <= -135 && angle >= -225)
	{
		angle = 180;
	}
	else if (angle <= -225 && angle >= -315)
	{
		angle = 270;
	}
	else if (angle <= -315 && angle >= -405)
	{
		angle = 360;
	}

	//angle -= 90;

	VectorCopy(origin, warzone_edit_ammo_crates_origins[warzone_edit_num_ammo_crates]);
	warzone_edit_ammo_crates_origins[warzone_edit_num_ammo_crates][2]-=64;
	warzone_edit_ammo_crates_angle[warzone_edit_num_ammo_crates] = angle;
	warzone_edit_num_ammo_crates++;
}

int		warzone_edit_num_health_crates = 0;
vec3_t	warzone_edit_health_crates_origins[1024];
float	warzone_edit_health_crates_angle[1024];

void WARZONE_AddHealthCrate ( vec3_t origin, vec3_t angles )
{
	float angle;

	angle = angles[YAW];

	while (angle > 360)
		angle -= 360;

	while (angle < -360)
		angle += 360;

	// Force 90 degree angles...
	if (angle > -45 && angle < 45)
	{
		angle = 0;
	}
	else if (angle >= 45 && angle <= 135)
	{
		angle = 90;
	}
	else if (angle >= 135 && angle <= 225)
	{
		angle = 180;
	}
	else if (angle >= 225 && angle <= 315)
	{
		angle = 270;
	}
	else if (angle >= 315 && angle <= 405)
	{
		angle = 360;
	}
	else if (angle <= -45 && angle >= -135)
	{
		angle = 90;
	}
	else if (angle <= -135 && angle >= -225)
	{
		angle = 180;
	}
	else if (angle <= -225 && angle >= -315)
	{
		angle = 270;
	}
	else if (angle <= -315 && angle >= -405)
	{
		angle = 360;
	}

	//angle -= 90;

	VectorCopy(origin, warzone_edit_health_crates_origins[warzone_edit_num_health_crates]);
	warzone_edit_health_crates_origins[warzone_edit_num_health_crates][2]-=64;
	warzone_edit_health_crates_angle[warzone_edit_num_health_crates] = angle;
	warzone_edit_num_health_crates++;
}

// Repair stuff..
void WARZONE_ChangeFlagTeam ( int flag_number, int new_team )
{
	warzone_edit_flags_team[flag_number] = new_team;
}

float WARZONE_FloorHeightAt ( vec3_t org )
{
	trace_t tr;
	vec3_t org1, org2;
//	float height = 0;

	VectorCopy(org, org1);
	org1[2]+=256;

	VectorCopy(org, org2);
	org2[2]= -65536.0f;

	trap_Trace( &tr, org1, NULL, NULL, org2, -1, MASK_PLAYERSOLID );
	
	//if (tr.contents & CONTENTS_WATER)
	//{// Water...
	//	return tr.endpos[2];
	//}

	if ( tr.startsolid || tr.allsolid )
	{
		return -65536.0f;
	}

	return tr.endpos[2];
}

void WARZONE_RemoveEntAt ( vec3_t org )
{
	trace_t tr;
	vec3_t org1, org2;

	VectorCopy(org, org1);
	org1[2]+=256;

	VectorCopy(org, org2);
	org2[2]= -65536.0f;

	trap_Trace( &tr, org1, NULL, NULL, org2, -1, MASK_PLAYERSOLID );
	
	if (tr.entityNum < ENTITYNUM_MAX_NORMAL)
	{
		G_FreeEntity(&g_entities[tr.entityNum]);
	}
}

void WARZONE_RaiseAllWarzoneEnts ( int modifier )
{
	int i;

	for (i = 0; i < warzone_edit_num_flags; i++)
	{
		warzone_edit_flags[i][2] = WARZONE_FloorHeightAt( warzone_edit_flags[i] );
	}

	for (i = 0; i < warzone_edit_num_vehicles; i++)
	{
		warzone_edit_vehicles_origins[i][2] = WARZONE_FloorHeightAt( warzone_edit_flags[i] );
	}

	for (i = 0; i < warzone_edit_num_ammo_crates; i++)
	{
		warzone_edit_ammo_crates_origins[i][2] = WARZONE_FloorHeightAt( warzone_edit_flags[i] );
	}

	for (i = 0; i < warzone_edit_num_health_crates; i++)
	{
		warzone_edit_health_crates_origins[i][2] = WARZONE_FloorHeightAt( warzone_edit_flags[i] );
	}
}

void WARZONE_RemoveAllWarzoneEnts ( void )
{
	int i;

	for (i = 0; i < warzone_edit_num_flags; i++)
	{
		WARZONE_RemoveEntAt( warzone_edit_flags[i] );
	}

	for (i = 0; i < warzone_edit_num_vehicles; i++)
	{
		WARZONE_RemoveEntAt( warzone_edit_flags[i] );
	}

	for (i = 0; i < warzone_edit_num_ammo_crates; i++)
	{
		WARZONE_RemoveEntAt( warzone_edit_flags[i] );
	}

	for (i = 0; i < warzone_edit_num_health_crates; i++)
	{
		WARZONE_RemoveEntAt( warzone_edit_flags[i] );
	}
}

void WARZONE_RemoveAmmoCrate ( void )
{
	warzone_edit_num_ammo_crates--;
}

void WARZONE_RemoveHealthCrate ( void )
{
	warzone_edit_num_health_crates--;
}

void WARZONE_ShowInfo ( void )
{
	int				i;

	G_Printf("^3FLAGS:\n");

	for (i = 0; i < warzone_edit_num_flags; i++)
	{
		if (warzone_edit_flags_team[i] == TEAM_RED)
			G_Printf("^5[^2Flag #^3%i^5] - ^1RED\n", i);
		else if (warzone_edit_flags_team[i] == TEAM_BLUE)
			G_Printf("^5[^2Flag #^3%i^5] - ^4BLUE\n", i);
		else
			G_Printf("^5[^2Flag #^3%i^5] - ^7NEUTRAL\n", i);
	}

	G_Printf("^5Total of ^3%i^5 warzone flags.\n", warzone_edit_num_flags-1);

#ifdef __VEHICLES__
	G_Printf("^3VEHICLES:\n");

	for (i = 0; i < warzone_edit_num_vehicles; i++)
	{
		if (warzone_edit_vehicles_type[i] == VEHICLE_CLASS_LIGHT_TANK)
			G_Printf("^5[^2Vehicle #^3%i^5] - ^1LIGHT TANK\n", i);
		else if (warzone_edit_vehicles_type[i] == VEHICLE_CLASS_MEDIUM_TANK)
			G_Printf("^5[^2Vehicle #^3%i^5] - ^1MEDIUM TANK\n", i);
		else if (warzone_edit_vehicles_type[i] == VEHICLE_CLASS_FLAMETANK)
			G_Printf("^5[^2Vehicle #^3%i^5] - ^1FLAMER TANK\n", i);
		else
			G_Printf("^5[^2Vehicle #^3%i^5] - ^1HEAVY TANK\n", i);
	}
#endif //__VEHICLES__

	G_Printf("^5Total of ^3%i^5 warzone vehicles.\n", warzone_edit_num_vehicles-1);
	G_Printf("^3AMMO CRATES: ^5Total of ^3%i^5 warzone ammo crates.\n", warzone_edit_num_ammo_crates-1);
	G_Printf("^3HEALTH CRATES: ^5Total of ^3%i^5 warzone health crates.\n", warzone_edit_num_health_crates-1);
}

void WARZONE_SaveGameInfo ( void )
{
	vmCvar_t		mapname;
	char			filename[60];
	fileHandle_t	f;
	int				i;

	strcpy( filename, "warzone/" );

	trap_Cvar_VariableStringBuffer( "g_scriptName", filename, sizeof(filename) );
	if ( strlen( filename) > 0 )
	{
		trap_Cvar_Register( &mapname, "g_scriptName", "", CVAR_ROM );
	}
	else
	{
		trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );
	}

	Q_strcat( filename, sizeof(filename), mapname.string );

	trap_FS_FOpenFile( va( "warzone/%s.info", filename), &f, FS_WRITE );

	// Save flags...
	trap_FS_Write( &warzone_edit_num_flags, sizeof(int), f );

	for (i = 0; i < warzone_edit_num_flags; i++)
	{
		trap_FS_Write( &warzone_edit_flags[i], sizeof(vec3_t), f );
		trap_FS_Write( &warzone_edit_flags_team[i], sizeof(int), f );
	}

	// Save vehicles...
	trap_FS_Write( &warzone_edit_num_vehicles, sizeof(int), f );

	for (i = 0; i < warzone_edit_num_vehicles; i++)
	{
		trap_FS_Write( &warzone_edit_vehicles_origins[i], sizeof(vec3_t), f );
		trap_FS_Write( &warzone_edit_vehicles_angle[i], sizeof(float), f );
		trap_FS_Write( &warzone_edit_vehicles_type[i], sizeof(int), f );
	}

	if (warzone_edit_num_ammo_crates < 0) warzone_edit_num_ammo_crates = 0;

	// Save ammo crates...
	trap_FS_Write( &warzone_edit_num_ammo_crates, sizeof(int), f );

	for (i = 0; i < warzone_edit_num_ammo_crates; i++)
	{
		trap_FS_Write( &warzone_edit_ammo_crates_origins[i], sizeof(vec3_t), f );
		trap_FS_Write( &warzone_edit_ammo_crates_angle[i], sizeof(float), f );
	}

	if (warzone_edit_num_health_crates < 0) warzone_edit_num_health_crates = 0;

	// Save health crates...
	trap_FS_Write( &warzone_edit_num_health_crates, sizeof(int), f );

	for (i = 0; i < warzone_edit_num_health_crates; i++)
	{
		trap_FS_Write( &warzone_edit_health_crates_origins[i], sizeof(vec3_t), f );
		trap_FS_Write( &warzone_edit_health_crates_angle[i], sizeof(float), f );
	}

	trap_FS_FCloseFile( f );													//close the file
	G_Printf( "^1*** ^3%s^5: Successfully saved warzone info file ^7warzone/%s.info^5.\n", GAME_VERSION, filename );
}

void SP_ammo_crate_spawn ( gentity_t* ent );
void SP_health_crate_spawn ( gentity_t* ent );

void WARZONE_LoadGameInfo2 ( void )
{
	vmCvar_t		mapname;
	char			filename[60];
	FILE			*f;
	int				i;
	vmCvar_t		fs_game;

	trap_Cvar_Register( &fs_game, "fs_game", "", CVAR_SERVERINFO | CVAR_ROM );
	trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );

	f = fopen( va( "%s/warzone/%s.info", fs_game.string, mapname.string), "rb" );
	
	if ( !f )
	{
		f = fopen( va( "warzone/%s.info", filename), "rb" );

		if ( !f )
		{
			//G_Printf( "^1*** ^3WARNING^5: Reading from ^7warzone/%s.info^3 failed^5!!!\n", mapname.string );
			//G_Printf( "^1*** ^3       ^5  No warzone support found for this map!\n" );
			Warzone_Flag_Loadpositions();
			return;
		}
	}

	// Save flags...
	fread( &warzone_edit_num_flags, sizeof(int), 1, f);

	for (i = 0; i < warzone_edit_num_flags; i++)
	{
		fread( &warzone_edit_flags[i], sizeof(vec3_t), 1, f );
		fread( &warzone_edit_flags_team[i], sizeof(int), 1, f );

		Warzone_Flag_Add( warzone_edit_flags[i], warzone_edit_flags_team[i]);
	}

	// Save vehicles...
	fread( &warzone_edit_num_vehicles, sizeof(int), 1, f );

#ifdef __VEHICLES__
	for (i = 0; i < warzone_edit_num_vehicles; i++)
	{
		fread( &warzone_edit_vehicles_origins[i], sizeof(vec3_t), 1, f );
		fread( &warzone_edit_vehicles_angle[i], sizeof(float), 1, f );
		fread( &warzone_edit_vehicles_type[i], sizeof(int), 1, f );

		{
			gentity_t *self = G_Spawn();

			G_SetOrigin(self, warzone_edit_vehicles_origins[i]);
			self->angle = warzone_edit_vehicles_angle[i];

			switch (warzone_edit_vehicles_type[i])
			{
			case VEHICLE_CLASS_LIGHT_TANK:
				SP_tank_light(self);
				break;
			case VEHICLE_CLASS_MEDIUM_TANK:
				SP_tank_medium(self);
				break;
			case VEHICLE_CLASS_FLAMETANK:
				SP_flametank(self);
				break;
			case VEHICLE_CLASS_HEAVY_TANK:
				SP_tank_heavy(self);
				break;
			default:		
				SP_tank_light(self);
				break;
			}
		}
	}
#endif //__VEHICLES__

	// Save ammo crates...
	fread( &warzone_edit_num_ammo_crates, sizeof(int), 1, f );

	if (warzone_edit_num_ammo_crates < 0) warzone_edit_num_ammo_crates = 0;

	for (i = 0; i < warzone_edit_num_ammo_crates; i++)
	{
		fread( &warzone_edit_ammo_crates_origins[i], sizeof(vec3_t), 1, f );
		fread( &warzone_edit_ammo_crates_angle[i], sizeof(float), 1, f );

		{
			gentity_t *self = G_Spawn();

			VectorCopy(warzone_edit_ammo_crates_origins[i], self->s.origin);
			self->angle = warzone_edit_ammo_crates_angle[i];

			self->s.origin[2] = WARZONE_FloorHeightAt(self->s.origin)-16;
			SP_ammo_crate_spawn(self);
		}
	}

	// Save health crates...
	fread( &warzone_edit_num_health_crates, sizeof(int)+1, 1, f );

	if (warzone_edit_num_health_crates < 0) warzone_edit_num_health_crates = 0;

	for (i = 0; i < warzone_edit_num_health_crates; i++)
	{
		fread( &warzone_edit_health_crates_origins[i], sizeof(vec3_t), 1, f );
		fread( &warzone_edit_health_crates_angle[i], sizeof(float), 1, f );

		{
			gentity_t *self = G_Spawn();

			VectorCopy(warzone_edit_health_crates_origins[i], self->s.origin);
			self->angle = warzone_edit_health_crates_angle[i];

			self->s.origin[2] = WARZONE_FloorHeightAt(self->s.origin)-16;
			SP_health_crate_spawn(self);
		}
	}

	fclose(f);													//close the file
	G_Printf( "^1*** ^3%s^5: Successfully loaded warzone info file ^7warzone/%s.info^5.\n", GAME_VERSION, mapname.string );
}

void WARZONE_LoadGameInfo ( void )
{
	vmCvar_t		mapname;
//	char			filename[60];
	fileHandle_t	f;
	int				i;

	trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );

	trap_FS_FOpenFile( va( "warzone/%s.info", mapname.string), &f, FS_READ );

	if ( !f )
	{
		WARZONE_LoadGameInfo2();
		return;
	}

	// Save flags...
	trap_FS_Read( &warzone_edit_num_flags, sizeof(int), f );

	for (i = 0; i < warzone_edit_num_flags; i++)
	{
		trap_FS_Read( &warzone_edit_flags[i], sizeof(vec3_t), f );
		trap_FS_Read( &warzone_edit_flags_team[i], sizeof(int), f );

		Warzone_Flag_Add( warzone_edit_flags[i], warzone_edit_flags_team[i]);

	}

	// Save vehicles...
	trap_FS_Read( &warzone_edit_num_vehicles, sizeof(int), f );

#ifdef __VEHICLES__
	for (i = 0; i < warzone_edit_num_vehicles; i++)
	{
		trap_FS_Read( &warzone_edit_vehicles_origins[i], sizeof(vec3_t), f );
		trap_FS_Read( &warzone_edit_vehicles_angle[i], sizeof(float), f );
		trap_FS_Read( &warzone_edit_vehicles_type[i], sizeof(int), f );

		{
			gentity_t *self = G_Spawn();

			G_SetOrigin(self, warzone_edit_vehicles_origins[i]);
			self->angle = warzone_edit_vehicles_angle[i];

			switch (warzone_edit_vehicles_type[i])
			{
			case VEHICLE_CLASS_LIGHT_TANK:
				SP_tank_light(self);
				break;
			case VEHICLE_CLASS_MEDIUM_TANK:
				SP_tank_medium(self);
				break;
			case VEHICLE_CLASS_FLAMETANK:
				SP_flametank(self);
				break;
			case VEHICLE_CLASS_HEAVY_TANK:
				SP_tank_heavy(self);
				break;
			default:		
				SP_tank_light(self);
				break;
			}
		}
	}
#endif //__VEHICLES__

	// Save ammo crates...
	trap_FS_Read( &warzone_edit_num_ammo_crates, sizeof(int), f );

	if (warzone_edit_num_ammo_crates < 0) warzone_edit_num_ammo_crates = 0;

	for (i = 0; i < warzone_edit_num_ammo_crates; i++)
	{
		trap_FS_Read( &warzone_edit_ammo_crates_origins[i], sizeof(vec3_t), f );
		trap_FS_Read( &warzone_edit_ammo_crates_angle[i], sizeof(float), f );

		{
			gentity_t *self = G_Spawn();

			VectorCopy(warzone_edit_ammo_crates_origins[i], self->s.origin);
			self->angle = warzone_edit_ammo_crates_angle[i];

			self->s.origin[2] = WARZONE_FloorHeightAt(self->s.origin)-16;
			SP_ammo_crate_spawn(self);
		}
	}

	// Save health crates...
	trap_FS_Read( &warzone_edit_num_health_crates, sizeof(int), f );

	if (warzone_edit_num_health_crates < 0) warzone_edit_num_health_crates = 0;

	for (i = 0; i < warzone_edit_num_health_crates; i++)
	{
		trap_FS_Read( &warzone_edit_health_crates_origins[i], sizeof(vec3_t), f );
		trap_FS_Read( &warzone_edit_health_crates_angle[i], sizeof(float), f );

		{
			gentity_t *self = G_Spawn();

			VectorCopy(warzone_edit_health_crates_origins[i], self->s.origin);
			self->angle = warzone_edit_health_crates_angle[i];

			self->s.origin[2] = WARZONE_FloorHeightAt(self->s.origin)-16;
			SP_health_crate_spawn(self);
		}
	}

	trap_FS_FCloseFile( f );													//close the file
	G_Printf( "^1*** ^3%s^5: Successfully loaded warzone info file ^7warzone/%s.info^5.\n", GAME_VERSION, mapname.string );
}
