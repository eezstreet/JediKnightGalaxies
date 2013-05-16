// Copyright (C) 1999-2000 Id Software, Inc.
//

#include "g_local.h"
#include "jkg_gangwars.h"

qboolean	G_SpawnString( const char *key, const char *defaultString, char **out ) {
	int		i;

	if ( !level.spawning ) {
		*out = (char *)defaultString;
//		G_Error( "G_SpawnString() called while not spawning" );
	}

	for ( i = 0 ; i < level.numSpawnVars ; i++ ) {
		if ( !Q_stricmp( key, level.spawnVars[i][0] ) ) {
			*out = level.spawnVars[i][1];
			return qtrue;
		}
	}

	*out = (char *)defaultString;
	return qfalse;
}

qboolean	G_SpawnFloat( const char *key, const char *defaultString, float *out ) {
	char		*s;
	qboolean	present;

	present = G_SpawnString( key, defaultString, &s );
	*out = atof( s );
	return present;
}

qboolean	G_SpawnInt( const char *key, const char *defaultString, int *out ) {
	char		*s;
	qboolean	present;

	present = G_SpawnString( key, defaultString, &s );
	*out = atoi( s );
	return present;
}

qboolean	G_SpawnVector( const char *key, const char *defaultString, float *out ) {
	char		*s;
	qboolean	present;

	present = G_SpawnString( key, defaultString, &s );
	sscanf( s, "%f %f %f", &out[0], &out[1], &out[2] );
	return present;
}



BG_field_t fields[] = {
	{"classname", FOFS(classname), F_LSTRING},
	{"teamnodmg", FOFS(teamnodmg), F_INT},
	{"teamowner", FOFS(s.teamowner), F_INT},
	{"teamuser", FOFS(alliedTeam), F_INT},
	{"alliedTeam", FOFS(alliedTeam), F_INT},//for misc_turrets
	{"roffname", FOFS(roffname), F_LSTRING},
	{"rofftarget", FOFS(rofftarget), F_LSTRING},
	{"healingclass", FOFS(healingclass), F_LSTRING},
	{"healingsound", FOFS(healingsound), F_LSTRING},
	{"healingrate", FOFS(healingrate), F_INT},
	{"ownername", FOFS(ownername), F_LSTRING},
	{"origin", FOFS(s.origin), F_VECTOR},
	{"model", FOFS(model), F_LSTRING},
	{"model2", FOFS(model2), F_LSTRING},
	{"spawnflags", FOFS(spawnflags), F_INT},
	{"speed", FOFS(speed), F_FLOAT},
	{"target", FOFS(target), F_LSTRING},
	{"target2", FOFS(target2), F_LSTRING},
	{"target3", FOFS(target3), F_LSTRING},
	{"target4", FOFS(target4), F_LSTRING},
	{"target5", FOFS(target5), F_LSTRING},
	{"target6", FOFS(target6), F_LSTRING},
	{"NPC_targetname", FOFS(NPC_targetname), F_LSTRING},
	{"NPC_target", FOFS(NPC_target), F_LSTRING},
	{"NPC_target2", FOFS(target2), F_LSTRING},//NPC_spawner only
	{"NPC_target4", FOFS(target4), F_LSTRING},//NPC_spawner only
	{"NPC_type", FOFS(NPC_type), F_LSTRING},
	{"targetname", FOFS(targetname), F_LSTRING},
	{"message", FOFS(message), F_LSTRING},
	{"team", FOFS(team), F_LSTRING},
	{"wait", FOFS(wait), F_FLOAT},
	{"delay", FOFS(delay), F_INT},
	{"random", FOFS(random), F_FLOAT},
	{"count", FOFS(count), F_INT},
	{"health", FOFS(health), F_INT},
	{"light", 0, F_IGNORE},
	{"dmg", FOFS(damage), F_INT},
	{"angles", FOFS(s.angles), F_VECTOR},
	{"angle", FOFS(s.angles), F_ANGLEHACK},
	{"targetShaderName", FOFS(targetShaderName), F_LSTRING},
	{"targetShaderNewName", FOFS(targetShaderNewName), F_LSTRING},
	{"linear", FOFS(alt_fire), F_INT},//for movers to use linear movement

	{"closetarget", FOFS(closetarget), F_LSTRING},//for doors
	{"opentarget", FOFS(opentarget), F_LSTRING},//for doors
	{"paintarget", FOFS(paintarget), F_LSTRING},//for doors

	{"goaltarget", FOFS(goaltarget), F_LSTRING},//for siege
	{"idealclass", FOFS(idealclass), F_LSTRING},//for siege spawnpoints

	//rww - icarus stuff:
	{"spawnscript", FOFS(behaviorSet[BSET_SPAWN]), F_LSTRING},//name of script to run
	{"usescript", FOFS(behaviorSet[BSET_USE]), F_LSTRING},//name of script to run
	{"awakescript", FOFS(behaviorSet[BSET_AWAKE]), F_LSTRING},//name of script to run
	{"angerscript", FOFS(behaviorSet[BSET_ANGER]), F_LSTRING},//name of script to run
	{"attackscript", FOFS(behaviorSet[BSET_ATTACK]), F_LSTRING},//name of script to run
	{"victoryscript", FOFS(behaviorSet[BSET_VICTORY]), F_LSTRING},//name of script to run
	{"lostenemyscript", FOFS(behaviorSet[BSET_LOSTENEMY]), F_LSTRING},//name of script to run
	{"painscript", FOFS(behaviorSet[BSET_PAIN]), F_LSTRING},//name of script to run
	{"fleescript", FOFS(behaviorSet[BSET_FLEE]), F_LSTRING},//name of script to run
	{"deathscript", FOFS(behaviorSet[BSET_DEATH]), F_LSTRING},//name of script to run
	{"delayscript", FOFS(behaviorSet[BSET_DELAYED]), F_LSTRING},//name of script to run
	{"delayscripttime", FOFS(delayScriptTime), F_INT},//name of script to run
	{"blockedscript", FOFS(behaviorSet[BSET_BLOCKED]), F_LSTRING},//name of script to run
	{"ffirescript", FOFS(behaviorSet[BSET_FFIRE]), F_LSTRING},//name of script to run
	{"ffdeathscript", FOFS(behaviorSet[BSET_FFDEATH]), F_LSTRING},//name of script to run
	{"mindtrickscript", FOFS(behaviorSet[BSET_MINDTRICK]), F_LSTRING},//name of script to run
	{"script_targetname", FOFS(script_targetname), F_LSTRING},//scripts look for this when "affecting"

	{"fullName", FOFS(fullName), F_LSTRING},

	{"soundSet", FOFS(soundSet), F_LSTRING},
	{"radius", FOFS(radius), F_FLOAT},
	{"numchunks", FOFS(radius), F_FLOAT},//for func_breakables
	{"chunksize", FOFS(mass), F_FLOAT},//for func_breakables

//Script parms - will this handle clamping to 16 or whatever length of parm[0] is?
	{"parm1", 0, F_PARM1},
	{"parm2", 0, F_PARM2},
	{"parm3", 0, F_PARM3},
	{"parm4", 0, F_PARM4},
	{"parm5", 0, F_PARM5},
	{"parm6", 0, F_PARM6},
	{"parm7", 0, F_PARM7},
	{"parm8", 0, F_PARM8},
	{"parm9", 0, F_PARM9},
	{"parm10", 0, F_PARM10},
	{"parm11", 0, F_PARM11},
	{"parm12", 0, F_PARM12},
	{"parm13", 0, F_PARM13},
	{"parm14", 0, F_PARM14},
	{"parm15", 0, F_PARM15},
	{"parm16", 0, F_PARM16},

	{NULL}
};

// Used in G_FreeEntity, to free all string allocations
void G_FreeSpawnVars(gentity_t *ent) {
	BG_field_t *fd;
	byte *b = (byte *)ent;
	for (fd = fields; fd->name; fd++) {
		if (fd->type == F_LSTRING) {
			G_Free((char **)(b+fd->ofs));
		}
	}
	// And free npcscript too
	if (ent->npcscript) {
		G_Free(ent->npcscript);
	}
}


typedef struct {
	char	*name;
	qboolean	logical;
	void	(*spawn)(gentity_t *ent);
} spawn_t;

void SP_info_player_start (gentity_t *ent);
void SP_info_player_duel( gentity_t *ent );
void SP_info_player_duel1( gentity_t *ent );
void SP_info_player_duel2( gentity_t *ent );
void SP_info_player_deathmatch (gentity_t *ent);
void SP_info_player_intermission (gentity_t *ent);
void SP_info_player_intermission_red (gentity_t *ent);
void SP_info_player_intermission_blue (gentity_t *ent);
void SP_info_jedimaster_start (gentity_t *ent);
void SP_info_player_start_red (gentity_t *ent);
void SP_info_player_start_blue (gentity_t *ent);
void SP_info_firstplace(gentity_t *ent);
void SP_info_secondplace(gentity_t *ent);
void SP_info_thirdplace(gentity_t *ent);
void SP_info_podium(gentity_t *ent);

void SP_func_plat (gentity_t *ent);
void SP_func_static (gentity_t *ent);
void SP_func_rotating (gentity_t *ent);
void SP_func_bobbing (gentity_t *ent);
void SP_func_pendulum( gentity_t *ent );
void SP_func_button (gentity_t *ent);
void SP_func_door (gentity_t *ent);
void SP_func_train (gentity_t *ent);
void SP_func_timer (gentity_t *self);
void SP_func_breakable (gentity_t *ent);
void SP_func_glass (gentity_t *ent);
void SP_func_usable( gentity_t *ent);
void SP_func_wall( gentity_t *ent );

void SP_trigger_lightningstrike( gentity_t *ent );

void SP_trigger_always (gentity_t *ent);
void SP_trigger_multiple (gentity_t *ent);
void SP_trigger_once( gentity_t *ent );
void SP_trigger_push (gentity_t *ent);
void SP_trigger_teleport (gentity_t *ent);
void SP_trigger_hurt (gentity_t *ent);
void SP_trigger_space(gentity_t *self);
void SP_trigger_shipboundary(gentity_t *self);
void SP_trigger_hyperspace(gentity_t *self);
void SP_trigger_asteroid_field(gentity_t *self);

void SP_target_remove_powerups( gentity_t *ent );
void SP_target_give (gentity_t *ent);
void SP_target_delay (gentity_t *ent);
void SP_target_speaker (gentity_t *ent);
void SP_target_print (gentity_t *ent);
void SP_target_laser (gentity_t *self);
void SP_target_character (gentity_t *ent);
void SP_target_score( gentity_t *ent );
void SP_target_teleporter( gentity_t *ent );
void SP_target_relay (gentity_t *ent);
void SP_target_kill (gentity_t *ent);
void SP_target_position (gentity_t *ent);
void SP_target_location (gentity_t *ent);
void SP_target_counter (gentity_t *self);
void SP_target_random (gentity_t *self);
void SP_target_scriptrunner( gentity_t *self );
void SP_target_interest (gentity_t *self);
void SP_target_activate (gentity_t *self);
void SP_target_deactivate (gentity_t *self);
void SP_target_level_change( gentity_t *self );
void SP_target_play_music( gentity_t *self );
void SP_target_push (gentity_t *ent);

void SP_light (gentity_t *self);
void SP_info_null (gentity_t *self);
void SP_info_notnull (gentity_t *self);
void SP_info_camp (gentity_t *self);
void SP_path_corner (gentity_t *self);

void SP_misc_teleporter_dest (gentity_t *self);
void SP_misc_model(gentity_t *ent);
void SP_misc_model_static(gentity_t *ent);
void SP_misc_G2model(gentity_t *ent);
void SP_misc_portal_camera(gentity_t *ent);
void SP_misc_portal_surface(gentity_t *ent);
void SP_misc_weather_zone( gentity_t *ent );

void SP_misc_bsp (gentity_t *ent);
void SP_terrain (gentity_t *ent);
void SP_misc_skyportal_orient (gentity_t *ent);
void SP_misc_skyportal (gentity_t *ent);

void SP_misc_ammo_floor_unit(gentity_t *ent);
void SP_misc_shield_floor_unit( gentity_t *ent );
void SP_misc_model_shield_power_converter( gentity_t *ent );
void SP_misc_model_ammo_power_converter( gentity_t *ent );
void SP_misc_model_health_power_converter( gentity_t *ent );

void SP_fx_runner( gentity_t *ent );

void SP_target_screenshake(gentity_t *ent);
void SP_target_escapetrig(gentity_t *ent);

void SP_misc_maglock ( gentity_t *self );

void SP_misc_faller(gentity_t *ent);

void SP_misc_holocron(gentity_t *ent);

void SP_reference_tag ( gentity_t *ent );

void SP_misc_weapon_shooter( gentity_t *self );

void SP_NPC_spawner( gentity_t *self );

void SP_NPC_Vehicle( gentity_t *self);

void SP_NPC_Kyle( gentity_t *self );
void SP_NPC_Lando( gentity_t *self );
void SP_NPC_Jan( gentity_t *self );
void SP_NPC_Luke( gentity_t *self );
void SP_NPC_MonMothma( gentity_t *self );
void SP_NPC_Tavion( gentity_t *self );
void SP_NPC_Tavion_New( gentity_t *self );
void SP_NPC_Alora( gentity_t *self );
void SP_NPC_Reelo( gentity_t *self );
void SP_NPC_Galak( gentity_t *self );
void SP_NPC_Desann( gentity_t *self );
void SP_NPC_Bartender( gentity_t *self );
void SP_NPC_MorganKatarn( gentity_t *self );
void SP_NPC_Jedi( gentity_t *self );
void SP_NPC_Prisoner( gentity_t *self );
void SP_NPC_Rebel( gentity_t *self );
void SP_NPC_Stormtrooper( gentity_t *self );
void SP_NPC_StormtrooperOfficer( gentity_t *self );
void SP_NPC_Snowtrooper( gentity_t *self);
void SP_NPC_Tie_Pilot( gentity_t *self );
void SP_NPC_Ugnaught( gentity_t *self );
void SP_NPC_Jawa( gentity_t *self );
void SP_NPC_Gran( gentity_t *self );
void SP_NPC_Rodian( gentity_t *self );
void SP_NPC_Weequay( gentity_t *self );
void SP_NPC_Trandoshan( gentity_t *self );
void SP_NPC_Tusken( gentity_t *self );
void SP_NPC_Noghri( gentity_t *self );
void SP_NPC_SwampTrooper( gentity_t *self );
void SP_NPC_Imperial( gentity_t *self );
void SP_NPC_ImpWorker( gentity_t *self );
void SP_NPC_BespinCop( gentity_t *self );
void SP_NPC_Reborn( gentity_t *self );
void SP_NPC_ShadowTrooper( gentity_t *self );
void SP_NPC_Monster_Murjj( gentity_t *self );
void SP_NPC_Monster_Swamp( gentity_t *self );
void SP_NPC_Monster_Howler( gentity_t *self );
void SP_NPC_Monster_Claw( gentity_t *self );
void SP_NPC_Monster_Glider( gentity_t *self );
void SP_NPC_Monster_Flier2( gentity_t *self );
void SP_NPC_Monster_Lizard( gentity_t *self );
void SP_NPC_Monster_Fish( gentity_t *self );
void SP_NPC_Monster_Wampa( gentity_t *self );
void SP_NPC_Monster_Rancor( gentity_t *self );
void SP_NPC_MineMonster( gentity_t *self );
void SP_NPC_Droid_Interrogator( gentity_t *self );
void SP_NPC_Droid_Probe( gentity_t *self );
void SP_NPC_Droid_Mark1( gentity_t *self );
void SP_NPC_Droid_Mark2( gentity_t *self );
void SP_NPC_Droid_ATST( gentity_t *self );
void SP_NPC_Droid_Seeker( gentity_t *self );
void SP_NPC_Droid_Remote( gentity_t *self );
void SP_NPC_Droid_Sentry( gentity_t *self );
void SP_NPC_Droid_Gonk( gentity_t *self );
void SP_NPC_Droid_Mouse( gentity_t *self );
void SP_NPC_Droid_R2D2( gentity_t *self );
void SP_NPC_Droid_R5D2( gentity_t *self );
void SP_NPC_Droid_Protocol( gentity_t *self );

void SP_NPC_Reborn_New( gentity_t *self);
void SP_NPC_Cultist( gentity_t *self );
void SP_NPC_Cultist_Saber( gentity_t *self );
void SP_NPC_Cultist_Saber_Powers( gentity_t *self );
void SP_NPC_Cultist_Destroyer( gentity_t *self );
void SP_NPC_Cultist_Commando( gentity_t *self );

void SP_waypoint (gentity_t *ent);
void SP_waypoint_small (gentity_t *ent);
void SP_waypoint_navgoal (gentity_t *ent);
void SP_waypoint_navgoal_8 (gentity_t *ent);
void SP_waypoint_navgoal_4 (gentity_t *ent);
void SP_waypoint_navgoal_2 (gentity_t *ent);
void SP_waypoint_navgoal_1 (gentity_t *ent);

void SP_CreateSpaceDust( gentity_t *ent );
void SP_CreateSnow( gentity_t *ent );
void SP_CreateRain( gentity_t *ent );

void SP_point_combat( gentity_t *self );

void SP_shooter_blaster( gentity_t *ent );

void SP_team_CTF_redplayer( gentity_t *ent );
void SP_team_CTF_blueplayer( gentity_t *ent );

void SP_team_CTF_redspawn( gentity_t *ent );
void SP_team_CTF_bluespawn( gentity_t *ent );

void SP_misc_turret( gentity_t *ent );
void SP_misc_turretG2( gentity_t *base );

// Warzone...
void SP_misc_control_point (gentity_t* ent);
void SP_ammo_crate_spawn ( gentity_t* ent );
void SP_health_crate_spawn ( gentity_t* ent );

void SP_item_botroam( gentity_t *ent )
{
}

void SP_gametype_item ( gentity_t* ent )
{
	gitem_t *item = NULL;
	char *value;
	int team = -1;

	G_SpawnString("teamfilter", "", &value);

	G_SetOrigin( ent, ent->s.origin );

	// If a team filter is set then override any team settings for the spawns
	if ( level.mTeamFilter[0] )
	{
		if ( Q_stricmp ( level.mTeamFilter, "red") == 0 )
		{
			team = TEAM_RED;
		}
		else if ( Q_stricmp ( level.mTeamFilter, "blue") == 0 )
		{
			team = TEAM_BLUE;
		}
	}

	if (ent->targetname && ent->targetname[0])
	{
		if (team != -1)
		{
			if (strstr(ent->targetname, "flag"))
			{
				if (team == TEAM_RED)
				{
					item = BG_FindItem("team_CTF_redflag");
				}
				else
				{ //blue
					item = BG_FindItem("team_CTF_blueflag");
				}
			}
		}
		else if (strstr(ent->targetname, "red_flag"))
		{
			item = BG_FindItem("team_CTF_redflag");
		}
		else if (strstr(ent->targetname, "blue_flag"))
		{
			item = BG_FindItem("team_CTF_blueflag");
		}
		else
		{
			item = NULL;
		}

		if (item)
		{
			ent->targetname = NULL;
			ent->classname = item->classname;
			G_SpawnItem( ent, item );
		}
	}
}

void SP_emplaced_gun( gentity_t *ent );

spawn_t	spawns[] = {
	// info entities don't do anything at all, but provide positional
	// information for things controlled by other processes
	{"info_player_start", qtrue, SP_info_player_start},
	{"info_player_duel", qtrue, SP_info_player_duel},
	{"info_player_duel1", qtrue, SP_info_player_duel1},
	{"info_player_duel2", qtrue, SP_info_player_duel2},
	{"info_player_deathmatch", qtrue, SP_info_player_deathmatch},
	{"info_player_intermission", qtrue, SP_info_player_intermission},
	{"info_player_intermission_red", qtrue, SP_info_player_intermission_red},
	{"info_player_intermission_blue", qtrue, SP_info_player_intermission_blue},
	{"info_jedimaster_start", qtrue, SP_info_jedimaster_start},
	{"info_player_start_red", qtrue, SP_info_player_start_red},
	{"info_player_start_blue", qtrue, SP_info_player_start_blue},
	{"info_null", qtrue, SP_info_null},
	{"info_notnull", qtrue, SP_info_notnull},		// use target_position instead
	{"info_camp", qtrue, SP_info_camp},

	{"func_plat", qfalse, SP_func_plat},
	{"func_button", qfalse, SP_func_button},
	{"func_door", qfalse, SP_func_door},
	{"func_static", qfalse, SP_func_static},
	{"func_rotating", qfalse, SP_func_rotating},
	{"func_bobbing", qfalse, SP_func_bobbing},
	{"func_pendulum", qfalse, SP_func_pendulum},
	{"func_train", qfalse, SP_func_train},
	{"func_group", qfalse, SP_info_null},
	{"func_timer", qfalse, SP_func_timer},			// rename trigger_timer?
	{"func_breakable", qfalse, SP_func_breakable},
	{"func_glass", qfalse, SP_func_glass},
	{"func_usable", qfalse, SP_func_usable},
	{"func_wall", qfalse, SP_func_wall},

	// Triggers are brush objects that cause an effect when contacted
	// by a living player, usually involving firing targets.
	// While almost everything could be done with
	// a single trigger class and different targets, triggered effects
	// could not be client side predicted (push and teleport).
	{"trigger_lightningstrike", qfalse, SP_trigger_lightningstrike},

	{"trigger_always", qfalse, SP_trigger_always},
	{"trigger_multiple", qfalse, SP_trigger_multiple},
	{"trigger_once", qfalse, SP_trigger_once},
	{"trigger_push", qfalse, SP_trigger_push},
	{"trigger_teleport", qfalse, SP_trigger_teleport},
	{"trigger_hurt", qfalse, SP_trigger_hurt},
	{"trigger_space", qfalse, SP_trigger_space},
	{"trigger_shipboundary", qfalse, SP_trigger_shipboundary},
	{"trigger_hyperspace", qfalse, SP_trigger_hyperspace},
	{"trigger_asteroid_field", qfalse, SP_trigger_asteroid_field},

	// targets perform no action by themselves, but must be triggered
	// by another entity
	{"target_give", qtrue, SP_target_give},
	{"target_remove_powerups", qtrue, SP_target_remove_powerups},
	{"target_delay", qtrue, SP_target_delay},
	{"target_speaker", qfalse, SP_target_speaker},	// These guys actually DO need to be networked to function
	{"target_print", qtrue, SP_target_print},
	{"target_laser", qtrue, SP_target_laser},
	{"target_score", qtrue, SP_target_score},
	{"target_teleporter", qtrue, SP_target_teleporter},
	{"target_relay", qtrue, SP_target_relay},
	{"target_kill", qtrue, SP_target_kill},
	{"target_position", qtrue, SP_target_position},
	{"target_location", qtrue, SP_target_location},
	{"target_counter", qtrue, SP_target_counter},
	{"target_random", qtrue, SP_target_random},
	{"target_scriptrunner", qfalse, SP_target_scriptrunner},	// These normally need icarus, so don't make them logical
	{"target_interest", qtrue, SP_target_interest},
	{"target_activate", qtrue, SP_target_activate},
	{"target_deactivate", qtrue, SP_target_deactivate},
	{"target_level_change", qtrue, SP_target_level_change},
	{"target_play_music", qtrue, SP_target_play_music},
	{"target_push", qtrue, SP_target_push},

	{"light", qtrue, SP_light},
	{"path_corner", qtrue, SP_path_corner},

	{"misc_teleporter_dest", qtrue, SP_misc_teleporter_dest},
	{"misc_model", qtrue, SP_misc_model},
	{"misc_model_static", qtrue, SP_misc_model_static},
	{"misc_G2model", qtrue, SP_misc_G2model},
	{"misc_portal_surface", qfalse, SP_misc_portal_surface},
	{"misc_portal_camera", qfalse, SP_misc_portal_camera},
	{"misc_weather_zone", qtrue, SP_misc_weather_zone},

	{"misc_bsp", qfalse, SP_misc_bsp},
	{"terrain", qfalse, SP_terrain},
	{"misc_skyportal_orient", qtrue, SP_misc_skyportal_orient},
	{"misc_skyportal", qtrue, SP_misc_skyportal},

	//rwwFIXMEFIXME: only for testing rmg team stuff
	{"gametype_item", qtrue, SP_gametype_item },

	{"misc_ammo_floor_unit", qfalse, SP_misc_ammo_floor_unit},
	{"misc_shield_floor_unit", qfalse, SP_misc_shield_floor_unit},
	{"misc_model_shield_power_converter", qfalse, SP_misc_model_shield_power_converter},
	{"misc_model_ammo_power_converter", qfalse, SP_misc_model_ammo_power_converter},
	{"misc_model_health_power_converter", qfalse, SP_misc_model_health_power_converter},

	{"fx_runner", qfalse, SP_fx_runner},

	{"target_screenshake", qtrue, SP_target_screenshake},
	{"target_escapetrig", qtrue, SP_target_escapetrig},

	{"misc_maglock", qfalse, SP_misc_maglock},

	{"misc_faller", qtrue, SP_misc_faller},

	{"ref_tag",	qtrue, SP_reference_tag},
	{"ref_tag_huge", qtrue, SP_reference_tag},

	{"misc_weapon_shooter", qtrue, SP_misc_weapon_shooter},

	//new NPC ents
	{"NPC_spawner", qtrue, SP_NPC_spawner},

	{"NPC_Vehicle", qtrue, SP_NPC_Vehicle },
	{"NPC_Kyle", qtrue, SP_NPC_Kyle },
	{"NPC_Lando", qtrue, SP_NPC_Lando },
	{"NPC_Jan", qtrue, SP_NPC_Jan },
	{"NPC_Luke", qtrue, SP_NPC_Luke },
	{"NPC_MonMothma", qtrue, SP_NPC_MonMothma },
	{"NPC_Tavion", qtrue, SP_NPC_Tavion },
	
	//new tavion
	{"NPC_Tavion_New", qtrue, SP_NPC_Tavion_New },

	//new alora
	{"NPC_Alora", qtrue, SP_NPC_Alora },

	{"NPC_Reelo", qtrue, SP_NPC_Reelo },
	{"NPC_Galak", qtrue, SP_NPC_Galak },
	{"NPC_Desann", qtrue, SP_NPC_Desann },
	{"NPC_Bartender", qtrue, SP_NPC_Bartender },
	{"NPC_MorganKatarn", qtrue, SP_NPC_MorganKatarn },
	{"NPC_Jedi", qtrue, SP_NPC_Jedi },
	{"NPC_Prisoner", qtrue, SP_NPC_Prisoner },
	{"NPC_Rebel", qtrue, SP_NPC_Rebel },
	{"NPC_Stormtrooper", qtrue, SP_NPC_Stormtrooper },
	{"NPC_StormtrooperOfficer", qtrue, SP_NPC_StormtrooperOfficer },
	{"NPC_Snowtrooper", qtrue, SP_NPC_Snowtrooper },
	{"NPC_Tie_Pilot", qtrue, SP_NPC_Tie_Pilot },
	{"NPC_Ugnaught", qtrue, SP_NPC_Ugnaught },
	{"NPC_Jawa", qtrue, SP_NPC_Jawa },
	{"NPC_Gran", qtrue, SP_NPC_Gran },
	{"NPC_Rodian", qtrue, SP_NPC_Rodian },
	{"NPC_Weequay", qtrue, SP_NPC_Weequay },
	{"NPC_Trandoshan", qtrue, SP_NPC_Trandoshan },
	{"NPC_Tusken", qtrue, SP_NPC_Tusken },
	{"NPC_Noghri", qtrue, SP_NPC_Noghri },
	{"NPC_SwampTrooper", qtrue, SP_NPC_SwampTrooper },
	{"NPC_Imperial", qtrue, SP_NPC_Imperial },
	{"NPC_ImpWorker", qtrue, SP_NPC_ImpWorker },
	{"NPC_BespinCop", qtrue, SP_NPC_BespinCop },
	{"NPC_Reborn", qtrue, SP_NPC_Reborn },
	{"NPC_ShadowTrooper", qtrue, SP_NPC_ShadowTrooper },
	{"NPC_Monster_Murjj", qtrue, SP_NPC_Monster_Murjj },
	{"NPC_Monster_Swamp", qtrue, SP_NPC_Monster_Swamp },
	{"NPC_Monster_Howler", qtrue, SP_NPC_Monster_Howler },
	{"NPC_MineMonster",	qtrue, SP_NPC_MineMonster },
	{"NPC_Monster_Claw", qtrue, SP_NPC_Monster_Claw },
	{"NPC_Monster_Glider", qtrue, SP_NPC_Monster_Glider },
	{"NPC_Monster_Flier2", qtrue, SP_NPC_Monster_Flier2 },
	{"NPC_Monster_Lizard", qtrue, SP_NPC_Monster_Lizard },
	{"NPC_Monster_Fish", qtrue, SP_NPC_Monster_Fish },
	{"NPC_Monster_Wampa", qtrue, SP_NPC_Monster_Wampa },
	{"NPC_Monster_Rancor", qtrue, SP_NPC_Monster_Rancor },
	{"NPC_Droid_Interrogator", qtrue, SP_NPC_Droid_Interrogator },
	{"NPC_Droid_Probe", qtrue, SP_NPC_Droid_Probe },
	{"NPC_Droid_Mark1", qtrue, SP_NPC_Droid_Mark1 },
	{"NPC_Droid_Mark2", qtrue, SP_NPC_Droid_Mark2 },
	{"NPC_Droid_ATST", qtrue, SP_NPC_Droid_ATST },
	{"NPC_Droid_Seeker", qtrue, SP_NPC_Droid_Seeker },
	{"NPC_Droid_Remote", qtrue, SP_NPC_Droid_Remote },
	{"NPC_Droid_Sentry", qtrue, SP_NPC_Droid_Sentry },
	{"NPC_Droid_Gonk", qtrue, SP_NPC_Droid_Gonk },
	{"NPC_Droid_Mouse", qtrue, SP_NPC_Droid_Mouse },
	{"NPC_Droid_R2D2", qtrue, SP_NPC_Droid_R2D2 },
	{"NPC_Droid_R5D2", qtrue, SP_NPC_Droid_R5D2 },
	{"NPC_Droid_Protocol", qtrue, SP_NPC_Droid_Protocol },

	//maybe put these guys in some day, for now just spawn reborns in their place.
	{"NPC_Reborn_New", qtrue, SP_NPC_Reborn_New },
	{"NPC_Cultist", qtrue, SP_NPC_Cultist },
	{"NPC_Cultist_Saber", qtrue, SP_NPC_Cultist_Saber },
	{"NPC_Cultist_Saber_Powers", qtrue, SP_NPC_Cultist_Saber_Powers },
	{"NPC_Cultist_Destroyer", qtrue, SP_NPC_Cultist_Destroyer },
	{"NPC_Cultist_Commando", qtrue, SP_NPC_Cultist_Commando },

	//rwwFIXMEFIXME: Faked for testing NPCs (another other things) in RMG with sof2 assets
	{"NPC_Colombian_Soldier", qtrue, SP_NPC_Reborn },
	{"NPC_Colombian_Rebel", qtrue, SP_NPC_Reborn },
	{"NPC_Colombian_EmplacedGunner", qtrue, SP_NPC_ShadowTrooper },
	{"NPC_Manuel_Vergara_RMG", qtrue, SP_NPC_Desann },
//	{"info_NPCnav", SP_waypoint},

	{"waypoint", qtrue, SP_waypoint},
	{"waypoint_small", qtrue, SP_waypoint_small},
	{"waypoint_navgoal", qtrue, SP_waypoint_navgoal},
	{"waypoint_navgoal_8", qtrue, SP_waypoint_navgoal_8},
	{"waypoint_navgoal_4", qtrue, SP_waypoint_navgoal_4},
	{"waypoint_navgoal_2", qtrue, SP_waypoint_navgoal_2},
	{"waypoint_navgoal_1", qtrue, SP_waypoint_navgoal_1},

	{"fx_spacedust", qtrue, SP_CreateSpaceDust},
	{"fx_rain", qtrue, SP_CreateRain},
	{"fx_snow", qtrue, SP_CreateSnow},

	{"point_combat", qtrue, SP_point_combat},

	{"misc_holocron", qfalse, SP_misc_holocron},

	{"shooter_blaster", qfalse, SP_shooter_blaster},

	{"team_CTF_redplayer", qtrue, SP_team_CTF_redplayer},
	{"team_CTF_blueplayer", qtrue, SP_team_CTF_blueplayer},

	{"team_CTF_redspawn", qtrue, SP_team_CTF_redspawn},
	{"team_CTF_bluespawn", qtrue, SP_team_CTF_bluespawn},

	{"item_botroam", qtrue, SP_item_botroam},

	{"emplaced_gun", qfalse, SP_emplaced_gun},

	{"misc_turret", qfalse, SP_misc_turret},
	{"misc_turretG2", qfalse, SP_misc_turretG2},

	//JKG Entities
	{"jkg_target_vendor", qtrue, JKG_SP_target_vendor},

	// Warzone Entities...
	{"misc_control_point", qfalse, SP_misc_control_point},
	{"misc_ammo_crate", qfalse, SP_ammo_crate_spawn},
	{"misc_health_crate", qfalse, SP_health_crate_spawn},

	{0, 0}
};

/*
===============
G_CallSpawn

Finds the spawn function for the entity and calls it,
returning qfalse if not found
===============
*/

int GLua_Spawn_Entity(gentity_t* ent);
int GLua_GetEntityTypeID(const char* classname);

qboolean G_IsLogicalEntity(const char *classname) {
	spawn_t	*s;

	if (!classname) {
		return qfalse;
	}

	// check normal spawn functions
	for ( s=spawns ; s->name ; s++ ) {
		if ( !strcmp(s->name, classname) ) {
			// found it
			if (s->logical) {
				return qtrue;
			} else {
				return qfalse;
			}
		}
	}
	// Try Lua entities next
	if (GLua_GetEntityTypeID(classname) == 2 /* Logical ent */) {
		return qtrue;
	}

	return qfalse;
}


qboolean G_CallSpawn( gentity_t *ent ) {
	spawn_t	*s;
	gitem_t	*item;

	if ( !ent->classname ) {
		G_Printf ("G_CallSpawn: NULL classname\n");
		return qfalse;
	}

	// check item spawn functions
	for ( item=bg_itemlist+1 ; item->classname ; item++ ) {
		if ( !strcmp(item->classname, ent->classname) ) {
#ifdef __DISABLE_UNUSED_SPAWNS__
			if ( !Q_strncmp(ent->classname, "weapon_", 7) )
			{// UQ1: Since we can't pick them up anyway, don't even spawn weapons...
				G_Printf("%s spawn disabled.\n", ent->classname);
				G_FreeEntity(ent);
				return qfalse;
			}
			else if ( !Q_strncmp(ent->classname, "ammo_", 5) )
			{// UQ1: Since we can't pick them up anyway, don't even spawn ammo...
				G_Printf("%s spawn disabled.\n", ent->classname);
				G_FreeEntity(ent);
				return qfalse;
			}
#endif //__DISABLE_UNUSED_SPAWNS__

			G_SpawnItem( ent, item );
			return qtrue;
		}
	}

	// check normal spawn functions
	for ( s=spawns ; s->name ; s++ ) {
		if ( !strcmp(s->name, ent->classname) ) {
			// found it
			if (ent->healingsound && ent->healingsound[0])
			{ //yeah...this can be used for anything, so.. precache it if it's there
				G_SoundIndex(ent->healingsound);
			}
			s->spawn(ent);
			return qtrue;
		}
	}
	// Jedi Knight Galaxies
	// Try Lua entities next
	if (GLua_Spawn_Entity(ent)) {
		return qtrue;
	}

	G_Printf ("%s doesn't have a spawn function\n", ent->classname);
	return qfalse;
}

/*
=============
G_NewString

Builds a copy of the string, translating \n to real linefeeds
so message texts can be multi-line
=============
*/
char *G_NewString( const char *string ) {
	char	*newb, *new_p;
	int		i,l;
	
	l = strlen(string) + 1;

	newb = (char *) G_Alloc( l );

	new_p = newb;

	// turn \n into a real linefeed
	for ( i=0 ; i< l ; i++ ) {
		if (string[i] == '\\' && i < l-1) {
			if (string[i+1] == 'n') {
				*new_p++ = '\n';
				i++;
			} else {
				*new_p++ = '\\';
			}
		} else {
			*new_p++ = string[i];
		}
	}
	
	return newb;
}

// Updated version, instead of returning the pointer, you pass it to it
// It'll free the current string (*IF* its allocated by G_Alloc) before makin a new alloc
void G_NewString2( void** data, const char *string ) {
	char	*newb, *new_p;
	int		i,l;
	
	if (*data) {
		G_Free(*data);
	}
	l = strlen(string) + 1;

	newb = (char *) G_Alloc( l );

	new_p = newb;

	// turn \n into a real linefeed
	for ( i=0 ; i< l ; i++ ) {
		if (string[i] == '\\' && i < l-1) {
			if (string[i+1] == 'n') {
				*new_p++ = '\n';
				i++;
			} else {
				*new_p++ = '\\';
			}
		} else {
			*new_p++ = string[i];
		}
	}
	*data = newb;
}




/*
===================
G_SpawnGEntityFromSpawnVars

Spawn an entity and fill in all of the level fields from
level.spawnVars[], then call the class specfic spawn function
===================
*/
#include "../namespace_begin.h"
void BG_ParseField( BG_field_t *l_fields, const char *key, const char *value, byte *ent );
#include "../namespace_end.h"

void G_SpawnGEntityFromSpawnVars( qboolean inSubBSP ) {
	int			i;
	gentity_t	*ent;
	char		*s, *value, *gametypeName;
	KeyPairSet_t *spv;
	static char *gametypeNames[] = {"ffa", "holocron", "jedimaster", "duel", "powerduel", "single", "team", "siege", "ctf", "cty", "warzone"};

	// JKG - Before we begin, determine the type of the entity (logical or normal)
	// Then spawn the appropriate entity

	G_SpawnString( "classname", NULL, &value );
	if (!value) {
		return;	// Dont even bother spawning an ent without a classname
	}
	if (G_IsLogicalEntity(value)) {
		// Check if the entity wants to be nonlogical anyway
		G_SpawnInt("nological", "0", &i);
		if (i) {				// Despite it being a logical entity, it wants to be nonlogical
			ent = G_Spawn();	// possibly because it wants to use icarus for example
		} else {
			// Get the next free logical entity
			ent = G_SpawnLogical();
		}
	} else {
		// Get the next free normal entity
		ent = G_Spawn();
	}

	for ( i = 0 ; i < level.numSpawnVars ; i++ ) {
		BG_ParseField( fields, level.spawnVars[i][0], level.spawnVars[i][1], (byte *)ent );
	}

	// check for "notsingle" flag
	if ( g_gametype.integer == GT_SINGLE_PLAYER ) {
		G_SpawnInt( "notsingle", "0", &i );
		if ( i ) {
			G_FreeEntity( ent );
			return;
		}
	}
	// check for "notteam" flag (GT_FFA, GT_DUEL, GT_SINGLE_PLAYER)
	if ( g_gametype.integer >= GT_TEAM ) {
		G_SpawnInt( "notteam", "0", &i );
		if ( i ) {
			G_FreeEntity( ent );
			return;
		}
	} else {
		G_SpawnInt( "notfree", "0", &i );
		if ( i ) {
			G_FreeEntity( ent );
			return;
		}
	}

	G_SpawnInt( "notta", "0", &i );
	if ( i ) {
		G_FreeEntity( ent );
		return;
	}

	if( G_SpawnString( "gametype", NULL, &value ) ) {
		if( g_gametype.integer >= GT_FFA && g_gametype.integer < GT_MAX_GAME_TYPE ) {
			gametypeName = gametypeNames[g_gametype.integer];

			s = strstr( value, gametypeName );
			if( !s ) {
				G_FreeEntity( ent );
				return;
			}
		}
	}

	// move editor origin to pos
	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	VectorCopy( ent->s.origin, ent->r.currentOrigin );

	
	
	// Store the spawnvars for later use
	spv = &g_spawnvars[ent->s.number];
	for (i=0; i < level.numSpawnVars; i++) {
		JKG_Pairs_Add(spv, level.spawnVars[i][0], level.spawnVars[i][1]);
	}

	// if we didn't get a classname, don't bother spawning anything
	if ( !G_CallSpawn( ent ) ) {
		G_FreeEntity( ent );
	}

	//Tag on the ICARUS scripting information only to valid recipients
	if ( trap_ICARUS_ValidEnt( ent ) )
	{
		trap_ICARUS_InitEnt( ent );

		if ( ent->classname && ent->classname[0] )
		{
			if ( Q_strncmp( "NPC_", ent->classname, 4 ) != 0 )
			{//Not an NPC_spawner (rww - probably don't even care for MP, but whatever)
				G_ActivateBehavior( ent, BSET_SPAWN );
			}
		}
	}
}

// Used by the lua entity factory
void G_SpawnEntity(gentity_t **outent) {
	int			i;
	gentity_t	*ent;
	char		*s, *value, *gametypeName;
	KeyPairSet_t *spv;
	static char *gametypeNames[] = {"ffa", "holocron", "jedimaster", "duel", "powerduel", "single", "team", "siege", "ctf", "cty"};

	G_SpawnString( "classname", NULL, &value );
	if (!value) {
		return;	// Dont even bother spawning an ent without a classname
	}
	if (G_IsLogicalEntity(value)) {
		// Check if the entity wants to be nonlogical anyway
		G_SpawnInt("nological", "0", &i);
		if (i) {				// Despite it being a logical entity, it wants to be nonlogical
			ent = G_Spawn();	// possibly because it wants to use icarus for example
		} else {
			// Get the next free logical entity
			ent = G_SpawnLogical();
		}
	} else {
		// Get the next free normal entity
		ent = G_Spawn();
	}

	*outent = ent;

	for ( i = 0 ; i < level.numSpawnVars ; i++ ) {
		BG_ParseField( fields, level.spawnVars[i][0], level.spawnVars[i][1], (byte *)ent );
	}

	// check for "notsingle" flag
	if ( g_gametype.integer == GT_SINGLE_PLAYER ) {
		G_SpawnInt( "notsingle", "0", &i );
		if ( i ) {
			G_FreeEntity( ent );
			return;
		}
	}
	// check for "notteam" flag (GT_FFA, GT_DUEL, GT_SINGLE_PLAYER)
	if ( g_gametype.integer >= GT_TEAM ) {
		G_SpawnInt( "notteam", "0", &i );
		if ( i ) {
			G_FreeEntity( ent );
			return;
		}
	} else {
		G_SpawnInt( "notfree", "0", &i );
		if ( i ) {
			G_FreeEntity( ent );
			return;
		}
	}

	G_SpawnInt( "notta", "0", &i );
	if ( i ) {
		G_FreeEntity( ent );
		return;
	}

	if( G_SpawnString( "gametype", NULL, &value ) ) {
		if( g_gametype.integer >= GT_FFA && g_gametype.integer < GT_MAX_GAME_TYPE ) {
			gametypeName = gametypeNames[g_gametype.integer];

			s = strstr( value, gametypeName );
			if( !s ) {
				G_FreeEntity( ent );
				return;
			}
		}
	}

	// move editor origin to pos
	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	VectorCopy( ent->s.origin, ent->r.currentOrigin );

	
	
	// Store the spawnvars for later use
	spv = &g_spawnvars[ent->s.number];
	for (i=0; i < level.numSpawnVars; i++) {
		JKG_Pairs_Add(spv, level.spawnVars[i][0], level.spawnVars[i][1]);
	}

	// if we didn't get a classname, don't bother spawning anything
	if ( !G_CallSpawn( ent ) ) {
		G_FreeEntity( ent );
	}

	//Tag on the ICARUS scripting information only to valid recipients
	if ( trap_ICARUS_ValidEnt( ent ) )
	{
		trap_ICARUS_InitEnt( ent );

		if ( ent->classname && ent->classname[0] )
		{
			if ( Q_strncmp( "NPC_", ent->classname, 4 ) != 0 )
			{//Not an NPC_spawner (rww - probably don't even care for MP, but whatever)
				G_ActivateBehavior( ent, BSET_SPAWN );
			}
		}
	}
}



/*
====================
G_AddSpawnVarToken
====================
*/
char *G_AddSpawnVarToken( const char *string ) {
	int		l;
	char	*dest;

	l = strlen( string );
	if ( level.numSpawnVarChars + l + 1 > MAX_SPAWN_VARS_CHARS ) {
		G_Error( "G_AddSpawnVarToken: MAX_SPAWN_CHARS" );
	}

	dest = level.spawnVarChars + level.numSpawnVarChars;
	memcpy( dest, string, l+1 );

	level.numSpawnVarChars += l + 1;

	return dest;
}

void AddSpawnField(char *field, char *value)
{
	int	i;

	for(i=0;i<level.numSpawnVars;i++)
	{
		if (Q_stricmp(level.spawnVars[i][0], field) == 0)
		{
			level.spawnVars[ i ][1] = G_AddSpawnVarToken( value );
			return;
		}
	}

	level.spawnVars[ level.numSpawnVars ][0] = G_AddSpawnVarToken( field );
	level.spawnVars[ level.numSpawnVars ][1] = G_AddSpawnVarToken( value );
	level.numSpawnVars++;
}

#define NOVALUE "novalue"

static void HandleEntityAdjustment(void)
{
	char		*value;
	vec3_t		origin, newOrigin, angles;
	char		temp[MAX_QPATH];
	float		rotation;

	G_SpawnString("origin", NOVALUE, &value);
	if (Q_stricmp(value, NOVALUE) != 0)
	{
		sscanf( value, "%f %f %f", &origin[0], &origin[1], &origin[2] );
	}
	else
	{
		origin[0] = origin[1] = origin[2] = 0.0;
	}

	rotation = DEG2RAD(level.mRotationAdjust);
	newOrigin[0] = origin[0]*cos(rotation) - origin[1]*sin(rotation);
	newOrigin[1] = origin[0]*sin(rotation) + origin[1]*cos(rotation);
	newOrigin[2] = origin[2];
	VectorAdd(newOrigin, level.mOriginAdjust, newOrigin);
	// damn VMs don't handle outputing a float that is compatible with sscanf in all cases
	Com_sprintf(temp, MAX_QPATH, "%0.0f %0.0f %0.0f", newOrigin[0], newOrigin[1], newOrigin[2]);
	AddSpawnField("origin", temp);

	G_SpawnString("angles", NOVALUE, &value);
	if (Q_stricmp(value, NOVALUE) != 0)
	{
		sscanf( value, "%f %f %f", &angles[0], &angles[1], &angles[2] );

		angles[1] = fmod(angles[1] + level.mRotationAdjust, 360.0f);
		// damn VMs don't handle outputing a float that is compatible with sscanf in all cases
		Com_sprintf(temp, MAX_QPATH, "%0.0f %0.0f %0.0f", angles[0], angles[1], angles[2]);
		AddSpawnField("angles", temp);
	}
	else
	{
		G_SpawnString("angle", NOVALUE, &value);
		if (Q_stricmp(value, NOVALUE) != 0)
		{
			sscanf( value, "%f", &angles[1] );
		}
		else
		{
			angles[1] = 0.0;
		}
		angles[1] = fmod(angles[1] + level.mRotationAdjust, 360.0f);
		Com_sprintf(temp, MAX_QPATH, "%0.0f", angles[1]);
		AddSpawnField("angle", temp);
	}

	// RJR experimental code for handling "direction" field of breakable brushes
	// though direction is rarely ever used.
	G_SpawnString("direction", NOVALUE, &value);
	if (Q_stricmp(value, NOVALUE) != 0)
	{
		sscanf( value, "%f %f %f", &angles[0], &angles[1], &angles[2] );
	}
	else
	{
		angles[0] = angles[1] = angles[2] = 0.0;
	}
	angles[1] = fmod(angles[1] + level.mRotationAdjust, 360.0f);
	Com_sprintf(temp, MAX_QPATH, "%0.0f %0.0f %0.0f", angles[0], angles[1], angles[2]);
	AddSpawnField("direction", temp);


	AddSpawnField("BSPInstanceID", level.mTargetAdjust);

	G_SpawnString("targetname", NOVALUE, &value);
	if (Q_stricmp(value, NOVALUE) != 0)
	{
		Com_sprintf(temp, MAX_QPATH, "%s%s", level.mTargetAdjust, value);
		AddSpawnField("targetname", temp);
	}

	G_SpawnString("target", NOVALUE, &value);
	if (Q_stricmp(value, NOVALUE) != 0)
	{
		Com_sprintf(temp, MAX_QPATH, "%s%s", level.mTargetAdjust, value);
		AddSpawnField("target", temp);
	}

	G_SpawnString("killtarget", NOVALUE, &value);
	if (Q_stricmp(value, NOVALUE) != 0)
	{
		Com_sprintf(temp, MAX_QPATH, "%s%s", level.mTargetAdjust, value);
		AddSpawnField("killtarget", temp);
	}

	G_SpawnString("brushparent", NOVALUE, &value);
	if (Q_stricmp(value, NOVALUE) != 0)
	{
		Com_sprintf(temp, MAX_QPATH, "%s%s", level.mTargetAdjust, value);
		AddSpawnField("brushparent", temp);
	}

	G_SpawnString("brushchild", NOVALUE, &value);
	if (Q_stricmp(value, NOVALUE) != 0)
	{
		Com_sprintf(temp, MAX_QPATH, "%s%s", level.mTargetAdjust, value);
		AddSpawnField("brushchild", temp);
	}

	G_SpawnString("enemy", NOVALUE, &value);
	if (Q_stricmp(value, NOVALUE) != 0)
	{
		Com_sprintf(temp, MAX_QPATH, "%s%s", level.mTargetAdjust, value);
		AddSpawnField("enemy", temp);
	}

	G_SpawnString("ICARUSname", NOVALUE, &value);
	if (Q_stricmp(value, NOVALUE) != 0)
	{
		Com_sprintf(temp, MAX_QPATH, "%s%s", level.mTargetAdjust, value);
		AddSpawnField("ICARUSname", temp);
	}
}

/*
====================
G_ParseSpawnVars

Parses a brace bounded set of key / value pairs out of the
level's entity strings into level.spawnVars[]

This does not actually spawn an entity.
====================
*/
qboolean G_ParseSpawnVars( qboolean inSubBSP ) {
	char		keyname[MAX_TOKEN_CHARS];
	char		com_token[MAX_TOKEN_CHARS];

	level.numSpawnVars = 0;
	level.numSpawnVarChars = 0;

	// parse the opening brace
	if ( !trap_GetEntityToken( com_token, sizeof( com_token ) ) ) {
		// end of spawn string
		return qfalse;
	}
	if ( com_token[0] != '{' ) {
		G_Error( "G_ParseSpawnVars: found %s when expecting {",com_token );
	}

	// go through all the key / value pairs
	while ( 1 ) {	
		// parse key
		if ( !trap_GetEntityToken( keyname, sizeof( keyname ) ) ) {
			G_Error( "G_ParseSpawnVars: EOF without closing brace" );
		}

		if ( keyname[0] == '}' ) {
			break;
		}
		
		// parse value	
		if ( !trap_GetEntityToken( com_token, sizeof( com_token ) ) ) {
			G_Error( "G_ParseSpawnVars: EOF without closing brace" );
		}

		if ( com_token[0] == '}' ) {
			G_Error( "G_ParseSpawnVars: closing brace without data" );
		}
		if ( level.numSpawnVars == MAX_SPAWN_VARS ) {
			G_Error( "G_ParseSpawnVars: MAX_SPAWN_VARS" );
		}
		level.spawnVars[ level.numSpawnVars ][0] = G_AddSpawnVarToken( keyname );
		level.spawnVars[ level.numSpawnVars ][1] = G_AddSpawnVarToken( com_token );
		level.numSpawnVars++;
	}

	if (inSubBSP)
	{
		HandleEntityAdjustment();
	}

	return qtrue;
}

#ifdef __ENTITY_OVERRIDES__
/*
====================
G_ParseSpawnVarsEx

Parses a brace bounded set of key / value pairs out of the
level's entity strings into level.spawnVars[]

This does not actually spawn an entity.
====================
*/
qboolean G_ParseSpawnVarsEx( int handle ) {
	pc_token_t	token;
	char		keyname[MAX_TOKEN_CHARS];
	
	level.numSpawnVars = 0;
	level.numSpawnVarChars = 0;

	// parse the opening brace
	if (trap_PC_ReadToken(handle, &token) == 0)
		// end of spawn string
		return qfalse;

	if (Q_stricmp(token.string, "{") != 0)
		G_Error( "G_ParseSpawnVarsEx: found %s when expecting {", token.string );

	// go through all the key / value pairs
	while ( 1 ) {	
		// parse key
		if (trap_PC_ReadToken( handle, &token) == 0)
			G_Error( "G_ParseSpawnVarsEx: EOF without closing brace" );

		if (Q_stricmp(token.string, "}") == 0)
			break;
		
		strcpy(keyname, token.string);

		// parse value	
		if (trap_PC_ReadToken( handle, &token) == 0)
			G_Error("G_ParseSpawnVarsEx: EOF without closing brace" );

		if (Q_stricmp(token.string, "}") == 0)
			G_Error("G_ParseSpawnVarsEx: closing brace without data");

		if (level.numSpawnVars == MAX_SPAWN_VARS)
			G_Error("G_ParseSpawnVarsEx: MAX_SPAWN_VARS");

		level.spawnVars[ level.numSpawnVars ][0] = G_AddSpawnVarToken(keyname);
		level.spawnVars[ level.numSpawnVars ][1] = G_AddSpawnVarToken(token.string);
		level.numSpawnVars++;
	}

	return qtrue;
}
#endif //__ENTITY_OVERRIDES__

static	char *defaultStyles[32][3] = 
{
	{	// 0 normal
		"z",
		"z",
		"z"
	},
	{	// 1 FLICKER (first variety)
		"mmnmmommommnonmmonqnmmo",
		"mmnmmommommnonmmonqnmmo",
		"mmnmmommommnonmmonqnmmo"
	},
	{	// 2 SLOW STRONG PULSE
		"abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcb",
		"abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcb",
		"abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcb"
	},
	{	// 3 CANDLE (first variety)
		"mmmmmaaaaammmmmaaaaaabcdefgabcdefg",
		"mmmmmaaaaammmmmaaaaaabcdefgabcdefg",
		"mmmmmaaaaammmmmaaaaaabcdefgabcdefg"
	},
	{	// 4 FAST STROBE
		"mamamamamama",
		"mamamamamama",
		"mamamamamama"
	},
	{	// 5 GENTLE PULSE 1
		"jklmnopqrstuvwxyzyxwvutsrqponmlkj",
		"jklmnopqrstuvwxyzyxwvutsrqponmlkj",
		"jklmnopqrstuvwxyzyxwvutsrqponmlkj"
	},
	{	// 6 FLICKER (second variety)
		"nmonqnmomnmomomno",
		"nmonqnmomnmomomno",
		"nmonqnmomnmomomno"
	},
	{	// 7 CANDLE (second variety)
		"mmmaaaabcdefgmmmmaaaammmaamm",
		"mmmaaaabcdefgmmmmaaaammmaamm",
		"mmmaaaabcdefgmmmmaaaammmaamm"
	},
	{	// 8 CANDLE (third variety)
		"mmmaaammmaaammmabcdefaaaammmmabcdefmmmaaaa",
		"mmmaaammmaaammmabcdefaaaammmmabcdefmmmaaaa",
		"mmmaaammmaaammmabcdefaaaammmmabcdefmmmaaaa"
	},
	{	// 9 SLOW STROBE (fourth variety)
		"aaaaaaaazzzzzzzz",
		"aaaaaaaazzzzzzzz",
		"aaaaaaaazzzzzzzz"
	},
	{	// 10 FLUORESCENT FLICKER
		"mmamammmmammamamaaamammma",
		"mmamammmmammamamaaamammma",
		"mmamammmmammamamaaamammma"
	},
	{	// 11 SLOW PULSE NOT FADE TO BLACK
		"abcdefghijklmnopqrrqponmlkjihgfedcba",
		"abcdefghijklmnopqrrqponmlkjihgfedcba",
		"abcdefghijklmnopqrrqponmlkjihgfedcba"
	},
	{	// 12 FAST PULSE FOR JEREMY
		"mkigegik",
		"mkigegik",
		"mkigegik"
	},
	{	// 13 Test Blending
		"abcdefghijklmqrstuvwxyz",
		"zyxwvutsrqmlkjihgfedcba",
		"aammbbzzccllcckkffyyggp"
	},
	{	// 14
		"",
		"",
		""
	},
	{	// 15
		"",
		"",
		""
	},
	{	// 16
		"",
		"",
		""
	},
	{	// 17
		"",
		"",
		""
	},
	{	// 18
		"",
		"",
		""
	},
	{	// 19
		"",
		"",
		""
	},
	{	// 20
		"",
		"",
		""
	},
	{	// 21
		"",
		"",
		""
	},
	{	// 22
		"",
		"",
		""
	},
	{	// 23
		"",
		"",
		""
	},
	{	// 24
		"",
		"",
		""
	},
	{	// 25
		"",
		"",
		""
	},
	{	// 26
		"",
		"",
		""
	},
	{	// 27
		"",
		"",
		""
	},
	{	// 28
		"",
		"",
		""
	},
	{	// 29
		"",
		"",
		""
	},
	{	// 30
		"",
		"",
		""
	},
	{	// 31
		"",
		"",
		""
	}
};

void *precachedKyle = 0;
void scriptrunner_run (gentity_t *self);

/*QUAKED worldspawn (0 0 0) ?

Every map should have exactly one worldspawn.
"music"		music wav file
"gravity"	800 is default gravity
"message"	Text to print during connection process

BSP Options
"gridsize"     size of lighting grid to "X Y Z". default="64 64 128"
"ambient"      scale of global light (from _color)
"fog"          shader name of the global fog texture - must include the full path, such as "textures/rj/fog1"
"distancecull" value for vis for the maximum viewing distance
"chopsize"     value for bsp on the maximum polygon / portal size
"ls_Xr"	override lightstyle X with this pattern for Red.
"ls_Xg"	green (valid patterns are "a-z")
"ls_Xb"	blue (a is OFF, z is ON)

"fogstart"		override fog start distance and force linear
"radarrange" for Siege/Vehicle radar - default range is 2500
*/
extern void EWebPrecache(void); //g_items.c
float g_cullDistance;
void SP_worldspawn( void ) 
{
	char		*text, temp[32];
	int			i;
	int			lengthRed, lengthBlue, lengthGreen;

	//I want to "cull" entities out of net sends to clients to reduce
	//net traffic on our larger open maps -rww
	G_SpawnFloat("distanceCull", "6000.0", &g_cullDistance);
	trap_SetServerCull(g_cullDistance);

	G_SpawnString( "classname", "", &text );
	if ( Q_stricmp( text, "worldspawn" ) ) {
		G_Error( "SP_worldspawn: The first entity isn't 'worldspawn'" );
	}

	for ( i = 0 ; i < level.numSpawnVars ; i++ ) 
	{
		if ( Q_stricmp( "spawnscript", level.spawnVars[i][0] ) == 0 )
		{//ONly let them set spawnscript, we don't want them setting an angle or something on the world.
			BG_ParseField( fields, level.spawnVars[i][0], level.spawnVars[i][1], (byte *)&g_entities[ENTITYNUM_WORLD] );
		}
	}
	//The server will precache the standard model and animations, so that there is no hit
	//when the first client connnects.
	if (!BGPAFtextLoaded)
	{
		BG_ParseAnimationFile("models/players/_humanoid/animation.cfg", bgHumanoidAnimations, qtrue);
	}

	if (!precachedKyle)
	{
		int defSkin;

		trap_G2API_InitGhoul2Model(&precachedKyle, "models/players/kyle/model.glm", 0, 0, -20, 0, 0);

		if (precachedKyle)
		{
			defSkin = trap_R_RegisterSkin("models/players/kyle/model_default.skin");
			trap_G2API_SetSkin(precachedKyle, 0, defSkin, defSkin);
		}
	}

	if (!g2SaberInstance)
	{
		trap_G2API_InitGhoul2Model(&g2SaberInstance, "models/weapons2/saber/saber_w.glm", 0, 0, -20, 0, 0);

		if (g2SaberInstance)
		{
			// indicate we will be bolted to model 0 (ie the player) on bolt 0 (always the right hand) when we get copied
			trap_G2API_SetBoltInfo(g2SaberInstance, 0, 0);
			// now set up the gun bolt on it
			trap_G2API_AddBolt(g2SaberInstance, 0, "*blade1");
		}
	}

	// make some data visible to connecting client
	trap_SetConfigstring( CS_GAME_VERSION, GAME_VERSION );

	trap_SetConfigstring( CS_LEVEL_START_TIME, va("%i", level.startTime ) );

	G_SpawnInt("serverinit", "0", &level.serverInit);

	G_SpawnString( "music", "", &text );
	trap_SetConfigstring( CS_MUSIC, text );

	G_SpawnString( "message", "", &text );
	trap_SetConfigstring( CS_MESSAGE, text );				// map specific message

	trap_SetConfigstring( CS_MOTD, g_motd.string );		// message of the day

	G_SpawnString( "gravity", "800", &text );
	trap_Cvar_Set( "g_gravity", text );

	G_SpawnString( "enableBreath", "0", &text );
	trap_Cvar_Set( "g_enableBreath", text );

	G_SpawnString( "soundSet", "default", &text );
	trap_SetConfigstring( CS_GLOBAL_AMBIENT_SET, text );

	G_SpawnString( "defaultWeapon", "pistol_DL-18", &text );
	if(text)
	{
		strcpy(level.startingWeapon, text);
	}
	g_entities[ENTITYNUM_WORLD].s.number = ENTITYNUM_WORLD;
	g_entities[ENTITYNUM_WORLD].classname = "worldspawn";

	// see if we want a warmup time
	trap_SetConfigstring( CS_WARMUP, "" );
	if ( g_restarted.integer ) {
		trap_Cvar_Set( "g_restarted", "0" );
		level.warmupTime = 0;
	} 
	/*
	else if ( g_doWarmup.integer && g_gametype.integer != GT_DUEL && g_gametype.integer != GT_POWERDUEL ) { // Turn it on
		level.warmupTime = -1;
		trap_SetConfigstring( CS_WARMUP, va("%i", level.warmupTime) );
		G_LogPrintf( "Warmup:\n" );
	}
	*/

	// Gang Wars
	if( g_gametype.integer >= GT_TEAM )
	{
		char *redString, *blueString;
		char teamInfo[MAX_INFO_STRING];

		G_SpawnString("gwTeamRed", "red", &redString);
		G_SpawnString("gwTeamBlue", "blue", &blueString);
		
		teamInfo[0] = '\0';

		Info_SetValueForKey(teamInfo, "redTeam", redString);
		Info_SetValueForKey(teamInfo, "blueTeam", blueString);

		if(!Info_Validate(teamInfo))
		{
			Com_Error(ERR_FATAL, "GW: Info_Validate returned qfalse.\n");
			return;
		}

		trap_SetConfigstring(CS_TEAMS, teamInfo);

		level.redTeam = JKG_GetTeamByReference( redString );
		level.blueTeam = JKG_GetTeamByReference( blueString );

		if( level.redTeam < 0 || level.blueTeam < 0 )
		{
			Com_Printf("^3WARNING: Improper team for team %s\n", (level.redTeam < 0) ? "Red" : "Blue");
		}
	}

	trap_SetConfigstring(CS_LIGHT_STYLES+(LS_STYLES_START*3)+0, defaultStyles[0][0]);
	trap_SetConfigstring(CS_LIGHT_STYLES+(LS_STYLES_START*3)+1, defaultStyles[0][1]);
	trap_SetConfigstring(CS_LIGHT_STYLES+(LS_STYLES_START*3)+2, defaultStyles[0][2]);
	
	for(i=1;i<LS_NUM_STYLES;i++)
	{
		Com_sprintf(temp, sizeof(temp), "ls_%dr", i);
		G_SpawnString(temp, defaultStyles[i][0], &text);
		lengthRed = strlen(text);
		trap_SetConfigstring(CS_LIGHT_STYLES+((i+LS_STYLES_START)*3)+0, text);

		Com_sprintf(temp, sizeof(temp), "ls_%dg", i);
		G_SpawnString(temp, defaultStyles[i][1], &text);
		lengthGreen = strlen(text);
		trap_SetConfigstring(CS_LIGHT_STYLES+((i+LS_STYLES_START)*3)+1, text);

		Com_sprintf(temp, sizeof(temp), "ls_%db", i);
		G_SpawnString(temp, defaultStyles[i][2], &text);
		lengthBlue = strlen(text);
		trap_SetConfigstring(CS_LIGHT_STYLES+((i+LS_STYLES_START)*3)+2, text);

		if (lengthRed != lengthGreen || lengthGreen != lengthBlue)
		{
			Com_Error(ERR_DROP, "Style %d has inconsistent lengths: R %d, G %d, B %d", 
				i, lengthRed, lengthGreen, lengthBlue);
		}
	}		
}

//rww - Planning on having something here?
qboolean SP_bsp_worldspawn ( void )
{
	return qtrue;
}

void G_PrecacheSoundsets( void )
{
	gentity_t	*ent = NULL;
	int i;
	int countedSets = 0;

	for ( i = 0; i < MAX_GENTITIES; i++ )
	{
		ent = &g_entities[i];

		if (ent->inuse && ent->soundSet && ent->soundSet[0])
		{
			if (countedSets >= MAX_AMBIENT_SETS)
			{
				Com_Error(ERR_DROP, "MAX_AMBIENT_SETS was exceeded! (too many soundsets)\n");
			}

			ent->s.soundSetIndex = G_SoundSetIndex(ent->soundSet);
			countedSets++;
		}
	}
}

/*
==============
G_SpawnEntitiesFromString

Parses textual entity definitions out of an entstring and spawns gentities.
==============
*/
void G_SpawnEntitiesFromString( qboolean inSubBSP ) {
#ifdef __ENTITY_OVERRIDES__
	int			handle;
	vmCvar_t	mapname;

	trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );
#endif //__ENTITY_OVERRIDES__

	// allow calls to G_Spawn*()
	level.spawning = qtrue;
	level.numSpawnVars = 0;

	// the worldspawn is not an actual entity, but it still
	// has a "spawn" function to perform any global setup
	// needed by a level (setting configstrings or cvars, etc)
#ifdef __ENTITY_OVERRIDES__
	//
	// UQ1: Allows different entities defintions to be loaded per gametype for a map with a .ovrents file...
	// This version overrides the bsp's spawns completely.
	//
	if (g_gametype.integer == GT_WARZONE /*|| g_gametype.integer == GT_WARZONE_CAMPAIGN*/)
		handle = trap_PC_LoadSource(va("maps/%s_scenario.ovrents", mapname.string));
	else
#ifdef __RPG__
	if (g_gametype.integer == GT_RPG_CITY)
		handle = trap_PC_LoadSource(va("maps/%s_city_rpg.ovrents", mapname.string));
	else if (g_gametype.integer == GT_RPG_WILDERNESS)
		handle = trap_PC_LoadSource(va("maps/%s_city_rpg.ovrents", mapname.string));
	else 
#endif //__RPG__
	if (g_gametype.integer == GT_SINGLE_PLAYER)
		handle = trap_PC_LoadSource(va("maps/%s_coop.ovrents", mapname.string));
	else
		handle = trap_PC_LoadSource(va("maps/%s.ovrents", mapname.string));

	if (handle)
	{
		if ( !G_ParseSpawnVarsEx(handle) )
		{
			G_Error( "SpawnEntities: no entities" );
			trap_PC_FreeSource(handle); // UQ added!
		}
	}
	else
#endif //__ENTITY_OVERRIDES__
	if ( !G_ParseSpawnVars(qfalse) ) {
		G_Error( "SpawnEntities: no entities" );
	}

	if (!inSubBSP)
	{
		SP_worldspawn();
	}
	else
	{
		// Skip this guy if its worldspawn fails
		if ( !SP_bsp_worldspawn() )
		{
			return;
		}
	}

#ifdef __ENTITY_OVERRIDES__ // UQ added.. We need to pass all the new entity list, not just worldspawn...
	if (handle)
	{// Pass all the new .ovrents file's entities one at a time...
		while( G_ParseSpawnVarsEx(handle) )
			G_SpawnGEntityFromSpawnVars(inSubBSP);

		trap_PC_FreeSource(handle); // UQ: Release the handle here instead of above...
	}
	else
#endif //__ENTITY_OVERRIDES__
	{// Parse ents from the actual map bsp.
		while( G_ParseSpawnVars(inSubBSP) ) {
			G_SpawnGEntityFromSpawnVars(inSubBSP);
		}	
	}

#ifdef __ENTITY_OVERRIDES__
	//
	// UQ1: This version loads only 'extra' entitiyes from a .entities file...
	//
	if (!handle) 
	{
		// parse possible external entities map files
		// it's used to add new ents to existing pure ET map
		if (g_gametype.integer == GT_WARZONE /*|| g_gametype.integer == GT_WARZONE_CAMPAIGN*/)
			handle = trap_PC_LoadSource(va("maps/%s_scenario.entities", mapname.string));
		else
#ifdef __RPG__
		if (g_gametype.integer == GT_RPG_CITY)
			handle = trap_PC_LoadSource(va("maps/%s_city_rpg.entities", mapname.string));
		else if (g_gametype.integer == GT_RPG_WILDERNESS)
			handle = trap_PC_LoadSource(va("maps/%s_city_rpg.entities", mapname.string));
		else 
#endif //__RPG__
		if (g_gametype.integer == GT_SINGLE_PLAYER)
			handle = trap_PC_LoadSource(va("maps/%s_coop.entities", mapname.string));
		else
			handle = trap_PC_LoadSource(va("maps/%s.entities", mapname.string));

		if (handle)
		{
			if (G_ParseSpawnVarsEx(handle) == qfalse)
				G_Error( "SpawnEntities: no entities" );

			// parse ents
			while (G_ParseSpawnVarsEx(handle))
				G_SpawnGEntityFromSpawnVars(inSubBSP);

			trap_PC_FreeSource(handle);
		}
	}
#endif //__ENTITY_OVERRIDES__

	if( g_entities[ENTITYNUM_WORLD].behaviorSet[BSET_SPAWN] && g_entities[ENTITYNUM_WORLD].behaviorSet[BSET_SPAWN][0] )
	{//World has a spawn script, but we don't want the world in ICARUS and running scripts,
		//so make a scriptrunner and start it going.
		gentity_t *script_runner = G_Spawn();
		if ( script_runner )
		{
			script_runner->behaviorSet[BSET_USE] = g_entities[ENTITYNUM_WORLD].behaviorSet[BSET_SPAWN];
			script_runner->count = 1;
			script_runner->think = scriptrunner_run;
			script_runner->nextthink = level.time + 100;

			if ( script_runner->inuse )
			{
				trap_ICARUS_InitEnt( script_runner );
			}
		}
	}

	if (!inSubBSP)
	{
		level.spawning = qfalse;			// any future calls to G_Spawn*() will be errors
	}

	G_PrecacheSoundsets();
}

