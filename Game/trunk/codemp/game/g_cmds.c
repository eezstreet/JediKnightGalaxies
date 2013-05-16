// Copyright (C) 1999-2000 Id Software, Inc.
//
#include "g_local.h"
#include "bg_saga.h"
#include "jkg_gangwars.h"

#include "../ui/menudef.h"			// for the voice chats

//rww - for getting bot commands...
int AcceptBotCommand(char *cmd, gentity_t *pl);
//end rww

#include "../namespace_begin.h"
void WP_SetSaber( int entNum, saberInfo_t *sabers, int saberNum, const char *saberName );
#include "../namespace_end.h"

void Cmd_NPC_f( gentity_t *ent );
void SetTeamQuick(gentity_t *ent, int team, qboolean doBegin);

extern void AIMod_CheckMapPaths ( gentity_t *ent );
extern void AIMod_CheckObjectivePaths ( gentity_t *ent );

extern void WARZONE_SaveGameInfo ( void );
extern void WARZONE_LoadGameInfo ( void );
extern void WARZONE_AddHealthCrate ( vec3_t origin, vec3_t angles );
extern void WARZONE_AddAmmoCrate ( vec3_t origin, vec3_t angles );
extern void WARZONE_AddFlag ( vec3_t origin, int team );
extern void WARZONE_ChangeFlagTeam ( int flag_number, int new_team );
extern void WARZONE_RaiseAllWarzoneEnts ( int modifier );
extern void WARZONE_ShowInfo ( void );
extern void WARZONE_RemoveAllWarzoneEnts ( void );
extern void WARZONE_RemoveAmmoCrate ( void );
extern void WARZONE_RemoveHealthCrate ( void );

// Include GLua stuff
#include "../GLua/glua.h"
#include "jkg_admin.h"
#ifdef __UNUSED__
#include "jkg_navmesh_creator.h"
#endif //__UNUSED__

/*
==================
DeathmatchScoreboardMessage

==================
*/
void DeathmatchScoreboardMessage( gentity_t *ent ) {
#ifndef __MMO__ 
	// UQ1: This is a very spammy one!
	// Suggest, use an event for each player. Staggered over time. 
	// Scores don't need to be instant, and in future maybe not even needed until after a match.
	char		entry[1024];
	char		string[1400];
	int			stringlength;
	int			i, j;
	gclient_t	*cl;
	int			numSorted, scoreFlags, accuracy, perfect;

	// send the latest information on all clients
	string[0] = 0;
	stringlength = 0;
	scoreFlags = 0;

	numSorted = level.numConnectedClients;
	
	if (numSorted > MAX_CLIENT_SCORE_SEND)
	{
		numSorted = MAX_CLIENT_SCORE_SEND;
	}

	for (i=0 ; i < numSorted ; i++) {
		int		ping;

		cl = &level.clients[level.sortedClients[i]];

		if ( cl->pers.connected == CON_CONNECTING ) {
			ping = -1;
		} else {
			ping = cl->ps.ping < 999 ? cl->ps.ping : 999;
		}

		if( cl->accuracy_shots ) {
			accuracy = cl->accuracy_hits * 100 / cl->accuracy_shots;
		}
		else {
			accuracy = 0;
		}
		perfect = ( cl->ps.persistant[PERS_RANK] == 0 && cl->ps.persistant[PERS_KILLED] == 0 ) ? 1 : 0;

		Com_sprintf (entry, sizeof(entry),
			" %i %i %i %i %i %i %i %i %i %i %i %i %i %i", level.sortedClients[i],
			cl->sess.sessionTeam == TEAM_SPECTATOR ? 0 : cl->ps.persistant[PERS_SCORE], ping, (level.time - cl->pers.enterTime)/60000,
			scoreFlags, g_entities[level.sortedClients[i]].s.powerups, accuracy, 
			cl->ps.persistant[PERS_CREDITS],
			cl->ps.persistant[PERS_EXCELLENT_COUNT],
			cl->ps.persistant[PERS_GAUNTLET_FRAG_COUNT], 
			cl->ps.persistant[PERS_DEFEND_COUNT], 
			cl->ps.persistant[PERS_ASSIST_COUNT], 
			perfect,
			cl->ps.persistant[PERS_CAPTURES]);
		j = strlen(entry);
		if (stringlength + j > 1022)
			break;
		strcpy (string + stringlength, entry);
		stringlength += j;
	}

	//still want to know the total # of clients
	i = level.numConnectedClients;

	trap_SendServerCommand( ent-g_entities, va("scores %i %i %i%s", i, 
		level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE],
		string ) );
#endif //__MMO__
}

/*
==================
JKG_ItemLookup_f

(eezstreet add)
Looks up a range of items and prints them in a list.
Ideally a GM or admin-only command
==================
*/
extern itemData_t itemLookupTable[MAX_ITEM_TABLE_SIZE];
void JKG_ItemLookup_f(gentity_t *ent)
{
	unsigned int i;
	int badItems = 0;
	int goodItems = 0;
	char buffer[64];
	char lookupString[960];
	int maximum, minimum;

	//Grab our args.
	trap_Argv(1, buffer, 64);
	minimum = atoi(buffer);
	trap_Argv(2, buffer, 64);
	maximum = atoi(buffer);

	if(!minimum || !maximum)
	{ //Bad args.
		trap_SendServerCommand(ent-g_entities, "print \"format: itemLookup <minimum> <maximum>\n\"");
		return;
	}
	if(minimum > maximum)
	{ //Oops, mixed the args most likely.
		trap_SendServerCommand(ent-g_entities, "print \"Minimum must be less than maximum.\n\"");
		return;
	}
	if((maximum - minimum) > 20)
	{ //Huge range.
		trap_SendServerCommand(ent-g_entities, "print \"Maximum range of itemLookup is 20.\n\"");
		return;
	}

	lookupString[0] = '\0';
	for(i = (unsigned int)minimum; i < (unsigned int)maximum; i++)
	{
		if(itemLookupTable[i].itemID)
		{
			if(i < maximum-1)
			{
				Q_strcat(lookupString, 960, va("%s (%u),", itemLookupTable[i].displayName, i));
			}
			else
			{
				Q_strcat(lookupString, 960, va("%s (%u)\n", itemLookupTable[i].displayName, i));
			}
			goodItems++;
		}
		else
		{
			badItems++;
		}
	}
	if(!goodItems)
	{
		trap_SendServerCommand(ent-g_entities, "print \"No valid items in that range.\n\"");
		return;
	}
	else
	{
		trap_SendServerCommand(ent-g_entities, va("print \"%s\"", lookupString));
		if(badItems)
		{
			trap_SendServerCommand(ent-g_entities, va("print \"%i bad indeces\"", badItems));
		}
	}
}

/*
==================
JKG_ItemCheck_f

Gives details about an item
==================
*/
void JKG_ItemCheck_f(gentity_t *ent)
{
	char buffer[64];
	int itemNum;
	int i;

	trap_Argv(1, buffer, 64);

	if(atoi(buffer))
	{
		itemNum = atoi(buffer);
		if(itemLookupTable[itemNum].itemID)
		{
			if(itemLookupTable[itemNum].itemType == ITEM_ARMOR)
			{
				//Meant to put some sort of check for armor type, but didn't bother.
				trap_SendServerCommand(ent-g_entities, va("print \"%s (%i) - Armor\n\"", itemLookupTable[itemNum].displayName, itemNum));
				return;
			}
			else if(itemLookupTable[itemNum].itemType == ITEM_WEAPON)
			{
				trap_SendServerCommand(ent-g_entities, va("print \"%s (%i) - Weapon (w %i, v %i)\n\"", itemLookupTable[itemNum].displayName, itemNum,
					itemLookupTable[itemNum].weapon, itemLookupTable[itemNum].variation));
				return;
			}
			else
			{
				trap_SendServerCommand(ent-g_entities, va("print \"%s (%i) - Other\n\"", itemLookupTable[itemNum].displayName, itemNum));
				return;
			}
		}
	}
	else
	{
		for(i = 0; i < MAX_ITEM_TABLE_SIZE; i++)
		{
			if(itemLookupTable[i].itemID)
			{
				if(!Q_stricmp(itemLookupTable[i].displayName, buffer))
				{
					trap_SendServerCommand(ent-g_entities, va("print \"%s (%i)\n\"", itemLookupTable[i].displayName, i));
					return;
				}
			}
		}
	}
	trap_SendServerCommand(ent-g_entities, va("print \"%s refers to an item that does not exist!\n\"", buffer));
}


/*
==================
JKG_BuyItem_f

==================
*/
void JKG_BuyItem_f(gentity_t *ent)
{
	int i;
	int entities[MAX_GENTITIES];
	int numEnts = 0;//trap_EntitiesInBox(ent->r.absmin, ent->r.absmax, entities, MAX_GENTITIES);

	//if (numEnts <= 1)
	{// UQ1: Non-Trigger Vendors...
		vec3_t mins;
		vec3_t maxs;
		int e = 0;

		for ( e = 0 ; e < 3 ; e++ ) 
		{
			if (e == 2)
			{// Up/Down axis is smaller so people can't buy from another level of the building...
				mins[e] = ent->r.currentOrigin[e] - 48;
				maxs[e] = ent->r.currentOrigin[e] + 48;
			}
			else
			{
				mins[e] = ent->r.currentOrigin[e] - 192;
				maxs[e] = ent->r.currentOrigin[e] + 192;
			}
		}

		numEnts = trap_EntitiesInBox(mins, maxs, entities, MAX_GENTITIES);
	}

	if(trap_Argc() < 1)
	{
		trap_SendServerCommand(ent->s.number, "print \"Not enough args.\n\"");
		return;
	}

	if(numEnts < 1)
	{
		trap_SendServerCommand(ent->s.number, "print \"You are not at a vendor.\n\"");
		return;
	}
	else
	{
		for(i = 0; i < numEnts; i++)
		{
			if(!strncmp(g_entities[entities[i]].classname, "trigger_", 8))
			{
				//This ent is a trigger. OH SNAP.
				gentity_t *pointingAt = G_Find(NULL, FOFS(targetname), g_entities[entities[i]].target);
				if(!pointingAt || !pointingAt->inuse)
				{
					continue;	//No target
				}
				else if(!Q_stricmp(pointingAt->classname, "jkg_target_vendor"))
				{
					//This is a vendor. Buy it!
					char buffer[16];
					trap_Argv(1, buffer, sizeof(buffer));
					JKG_Vendor_Buy(ent, pointingAt, atoi(buffer));
					return;
				}
			}
			else
			{// UQ1: Actual NPC (non trigger) usage...
				gentity_t *pointingAt = &g_entities[entities[i]];

				if(!pointingAt || !pointingAt->inuse || !pointingAt->client || pointingAt->s.eType != ET_NPC)
				{
					continue;	//No target
				}

				switch (pointingAt->client->NPC_class)
				{// UQ1: Need to change these in the actual NPC script files...
				case CLASS_GENERAL_VENDOR:
				case CLASS_WEAPONS_VENDOR:
				case CLASS_ARMOR_VENDOR:
				case CLASS_SUPPLIES_VENDOR:
				case CLASS_FOOD_VENDOR:
				case CLASS_MEDICAL_VENDOR:
				case CLASS_GAMBLER_VENDOR:
				case CLASS_TRADE_VENDOR:
				case CLASS_ODDITIES_VENDOR:
				case CLASS_DRUG_VENDOR:
				case CLASS_TRAVELLING_VENDOR:
					{//This is a vendor. Buy it!
						char buffer[16];
						trap_Argv(1, buffer, sizeof(buffer));
						JKG_Vendor_Buy(ent, pointingAt, atoi(buffer));
						return;
					}
					break;
				default:
					break;
				}
			}
		}
	}

	trap_SendServerCommand(ent->s.number, "print \"You need to be at a vendor to purchase items.\n\"");
}
/*
==================
Cmd_Score_f

Request current scoreboard information
==================
*/
void Cmd_Score_f( gentity_t *ent ) {
	DeathmatchScoreboardMessage( ent );
}



/*
==================
CheatsOk
==================
*/
qboolean	CheatsOk( gentity_t *ent ) {
	if (ent->client->sess.adminRank >= ADMRANK_DEVELOPER)
	{
		return qtrue;
	}
	if ( !g_cheats.integer ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOCHEATS")));
		return qfalse;
	}
	if ( ent->health <= 0 || (ent->client && ent->client->deathcamTime) ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "MUSTBEALIVE")));
		return qfalse;
	}
	return qtrue;
}


/*
==================
ConcatArgs
==================
*/
char	*ConcatArgs( int start ) {
	int		i, c, tlen;
	static char	line[MAX_STRING_CHARS];
	int		len;
	char	arg[MAX_STRING_CHARS];

	len = 0;
	c = trap_Argc();
	for ( i = start ; i < c ; i++ ) {
		trap_Argv( i, arg, sizeof( arg ) );
		tlen = strlen( arg );
		if ( len + tlen >= MAX_STRING_CHARS - 1 ) {
			break;
		}
		memcpy( line + len, arg, tlen );
		len += tlen;
		if ( i != c - 1 ) {
			line[len] = ' ';
			len++;
		}
	}

	line[len] = 0;

	return line;
}

/*
==================
JKG_CheckIfNumber

Loops through a string and returns the following:
0 if neither decimal nor alpha
1 if alpha
2 if decimal
==================
*/
typedef enum
{
	JKGSTR_NONE,
	JKGSTR_ALPHA,
	JKGSTR_DECIMAL
} JKGStringType_t;

JKGStringType_t JKG_CheckIfNumber(const char *string)
{
	int i = 0;
	while(string[i] != '\0')
	{
		if(isalpha((int)string[i]))
			return JKGSTR_ALPHA;
		else if(!isdigit((int)string[i]))
			return JKGSTR_NONE;
		i++;
	}
	return JKGSTR_DECIMAL;
}

/*
==================
SanitizeString

Remove case and control characters
==================
*/
void SanitizeString( char *in, char *out ) {
	while ( *in ) {
		if ( *in == 27 ) {
			in += 2;		// skip color code
			continue;
		}
		if ( *in < 32 ) {
			in++;
			continue;
		}
		*out++ = tolower( (unsigned char) *in++ );
	}

	*out = 0;
}

/*
==================
ClientNumberFromString

Returns a player number for either a number or name string
Returns -1 if invalid
==================
*/
int ClientNumberFromString( gentity_t *to, char *s ) {
	gclient_t	*cl;
	int			idnum;
	char		s2[MAX_STRING_CHARS];
	char		n2[MAX_STRING_CHARS];

	// numeric values are just slot numbers
	if (s[0] >= '0' && s[0] <= '9') {
		idnum = atoi( s );
		if ( idnum < 0 || idnum >= level.maxclients ) {
			trap_SendServerCommand( to-g_entities, va("print \"Bad client slot: %i\n\"", idnum));
			return -1;
		}

		cl = &level.clients[idnum];
		if ( cl->pers.connected != CON_CONNECTED ) {
			trap_SendServerCommand( to-g_entities, va("print \"Client %i is not active\n\"", idnum));
			return -1;
		}
		return idnum;
	}

	// check for a name match
	SanitizeString( s, s2 );
	for ( idnum=0,cl=level.clients ; idnum < level.maxclients ; idnum++,cl++ ) {
		if ( cl->pers.connected != CON_CONNECTED ) {
			continue;
		}
		SanitizeString( cl->pers.netname, n2 );
		if ( !strcmp( n2, s2 ) ) {
			return idnum;
		}
	}

	trap_SendServerCommand( to-g_entities, va("print \"User %s is not on the server\n\"", s));
	return -1;
}

int G_ClientNumberFromStrippedSubstring ( const char* name, qboolean checkAll );
int G_ClientNumberFromArg ( const char* name)
{
	int client_id = 0;
	char *cp;
	
	cp = (char *)name;
	while (*cp)
	{
		if ( *cp >= '0' && *cp <= '9' ) cp++;
		else
		{
			client_id = -1; //mark as alphanumeric
			break;
		}
	}

	if ( client_id == 0 )
	{ // arg is assumed to be client number
		client_id = atoi(name);
	}
	// arg is client name
	if ( client_id == -1 )
	{
		client_id = G_ClientNumberFromStrippedSubstring(name, qfalse);
	}
	return client_id;
}

/*
==================
Cmd_Give_f

Give items to a client
==================
*/
extern void JKG_A_RollItem( unsigned int itemIndex, int qualityOverride, inv_t *inventory );
extern void JKG_A_GiveEntItem( unsigned int itemIndex, int qualityOverride, inv_t *inventory, gclient_t *owner );
extern void JKG_A_GiveEntItemForcedToACI( unsigned int itemIndex, int qualityOverride, inv_t *inventory, gclient_t *owner, unsigned int ACIslot );
void Cmd_Give_f (gentity_t *cmdent, int baseArg)
{
	char		name[MAX_TOKEN_CHARS];
	gentity_t	*ent;
	gitem_t		*it;
	int			i;
	qboolean	give_all;
	gentity_t		*it_ent;
	trace_t		trace;
	char		arg[MAX_TOKEN_CHARS];

	if ( !CheatsOk( cmdent ) ) {
		return;
	}

	if (baseArg)
	{
		char otherindex[MAX_TOKEN_CHARS];

		trap_Argv( 1, otherindex, sizeof( otherindex ) );

		if (!otherindex[0])
		{
			Com_Printf("giveother requires that the second argument be a client index number.\n");
			return;
		}

		i = atoi(otherindex);

		if (i < 0 || i >= MAX_CLIENTS)
		{
			Com_Printf("%i is not a client index\n", i);
			return;
		}

		ent = &g_entities[i];

		if (!ent->inuse || !ent->client)
		{
			Com_Printf("%i is not an active client\n", i);
			return;
		}
	}
	else
	{
		ent = cmdent;
	}

	trap_Argv( 1+baseArg, name, sizeof( name ) );

	if (Q_stricmp(name, "all") == 0)
		give_all = qtrue;
	else
		give_all = qfalse;

	if (give_all)
	{
		i = 0;
		while (i < HI_NUM_HOLDABLE)
		{
			ent->client->ps.stats[STAT_HOLDABLE_ITEMS] |= (1 << i);
			i++;
		}
		i = 0;
	}

	// Pretty much to test out the awesome seeker upgrades.
	if ( Q_stricmp( name, "seeker" ) == 0 )
	{
		char time[1024];
		trap_Argv( 2 + baseArg, time, sizeof( time ));
		ItemUse_Seeker( cmdent );
		if ( time[0] ) cmdent->client->ps.droneExistTime = level.time + ( atoi( time ) * 1000 );
		else cmdent->client->ps.droneExistTime = level.time + 60000;
	}

	if (give_all || Q_stricmp( name, "health") == 0)
	{
		if (trap_Argc() == 3+baseArg) {
			trap_Argv( 2+baseArg, arg, sizeof( arg ) );
			ent->health = atoi(arg);
			if (ent->health > ent->client->ps.stats[STAT_MAX_HEALTH]) {
				ent->health = ent->client->ps.stats[STAT_MAX_HEALTH];
			}
		}
		else {
			ent->health = ent->client->ps.stats[STAT_MAX_HEALTH];
		}
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "weapons") == 0)
	{
		ent->client->ps.stats[STAT_WEAPONS] = (1 << (LAST_USEABLE_WEAPON+1))  - ( 1 << WP_NONE );
		if (!give_all)
			return;
	}
	
	if ( !give_all && Q_stricmp(name, "weaponnum") == 0 )
	{
		trap_Argv( 2+baseArg, arg, sizeof( arg ) );
		ent->client->ps.stats[STAT_WEAPONS] |= (1 << atoi(arg));
		return;
	}
	
	if ( !give_all && Q_stricmp (name, "weapon") == 0 )
	{
	    weaponData_t *weapon;
	    
	    trap_Argv (2 + baseArg, arg, sizeof (arg));
	    weapon = BG_GetWeaponByClassName (arg);
	    if ( weapon )
	    {
	        int i = 0;
			int itemID;
	        
			if ( ent->client->coreStats.weight >= MAX_INVENTORY_WEIGHT)
	        {
	            trap_SendServerCommand (ent->s.number, "print \"Your inventory is full. No more items can be added.\n\"");
	            return;
	        }

			//FIXME: The below assumes that there is a valid weapon item
			itemID = JKG_GetItemByWeaponIndex(BG_GetWeaponIndex((unsigned int)weapon->weaponBaseIndex, (unsigned int)weapon->weaponModIndex))->itemID;

	        //while ( i < MAX_INVENTORY_ITEMS && cmdent->inventory[i].id )
			JKG_A_GiveEntItem(itemID, IQUAL_NORMAL, ent->inventory, ent->client);
			trap_SendServerCommand (ent->s.number, va ("print \"'%s' was added to your inventory.\n\"", itemLookupTable[itemID].displayName));
	    }
	    else
	    {
	        trap_SendServerCommand (ent->s.number, va ("print \"'%s' does not exist.\n\"", arg));
	    }
	    return;
	}

	if (give_all || Q_stricmp(name, "ammo") == 0)
	{
		int num = 999;
		if (trap_Argc() == 3+baseArg) {
			trap_Argv( 2+baseArg, arg, sizeof( arg ) );
			num = atoi(arg);
		}
		for ( i = 0 ; i < JKG_MAX_AMMO_INDICES ; i++ ) {
			ent->client->ammoTable[i] = num;		//FIXME: copy to proper ammo array
		}
		for ( i = 0; i <= 255; i++ )
		{
			int weapVar, weapBase;
			if(!BG_GetWeaponByIndex(i, &weapBase, &weapVar))
			{
				break;
			}
			ent->client->clipammo[i] = GetWeaponAmmoClip (weapBase, weapVar);
		}
		ent->client->ps.ammo = num;
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "armor") == 0)
	{
		if (trap_Argc() == 3+baseArg) {
			trap_Argv( 2+baseArg, arg, sizeof( arg ) );
			ent->client->ps.stats[STAT_ARMOR] = atoi(arg);
		} else {
			ent->client->ps.stats[STAT_ARMOR] = ent->client->ps.stats[STAT_MAX_ARMOR];
		}

		if (!give_all)
			return;
	}

	//Inventory items -- eezstreet/JKG
	if(!give_all && Q_stricmp(name, "itemNew") == 0)
	{
		int itemID = 0, j = 0;
		qboolean inventoryFull = qtrue;

		trap_Argv(2+baseArg, arg, sizeof( arg ) );
		itemID = atoi(arg);

		if(itemID)
		{
			if(!itemLookupTable[itemID].itemID)
			{
				trap_SendServerCommand(ent - g_entities, va("print \"%i refers to an item that does not exist\n\"", itemID));
				return;
			}
		}
		else
		{
			//Find us the correct item.
			for(j = 0; j < MAX_ITEM_TABLE_SIZE; j++)
			{
				if(itemLookupTable[j].itemID)
				{
					if(!Q_stricmp(itemLookupTable[j].internalName, ConcatArgs(2+baseArg)))
					{
						itemID = j;
						break;
					}
				}
			}
			if(!itemID)
			{
				trap_SendServerCommand(ent - g_entities, va("print \"%s refers to an item that does not exist\n\"", arg));
				return;
			}
		}
		/*for(i = 0; i < MAX_INVENTORY_ITEMS; i++)
		{
			if(!ent->inventory[i].id)
				break;
			else
			{
				if(!ent->inventory[i].id->itemID)
				{
					inventoryFull = qfalse;
					break;
				}
			}
		}*/
		//JKG_A_RollItem(itemID, IQUAL_NORMAL, ent->inventory);
		JKG_A_GiveEntItem(itemID, IQUAL_NORMAL, ent->inventory, ent->client);
		return;
	}

	if (Q_stricmp(name, "excellent") == 0) {
		ent->client->ps.persistant[PERS_EXCELLENT_COUNT]++;
		return;
	}
	if (Q_stricmp(name, "credits") == 0) {
		int creditAmount;
		trap_Argv(2+baseArg, arg, sizeof( arg ) );

		creditAmount = atoi(arg);
		ent->client->ps.persistant[PERS_CREDITS] += creditAmount;
		trap_SendServerCommand( ent->client->ps.clientNum, va("print \"Your new balance is: %i credits\n\"", ent->client->ps.persistant[PERS_CREDITS]) );
		return;
	}
	if (Q_stricmp(name, "gauntletaward") == 0) {
		ent->client->ps.persistant[PERS_GAUNTLET_FRAG_COUNT]++;
		return;
	}
	if (Q_stricmp(name, "defend") == 0) {
		ent->client->ps.persistant[PERS_DEFEND_COUNT]++;
		return;
	}
	if (Q_stricmp(name, "assist") == 0) {
		ent->client->ps.persistant[PERS_ASSIST_COUNT]++;
		return;
	}

	if ( give_all )
	{
		ent->client->ps.cloakFuel	= 100;
		ent->client->ps.jetpackFuel	= 100;
	}

	// spawn a specific item right on the player
	if ( !give_all ) {
		it = BG_FindItem (name);
		if (!it) {
			return;
		}

		it_ent = G_Spawn();
		VectorCopy( ent->r.currentOrigin, it_ent->s.origin );
		it_ent->classname = it->classname;
		G_SpawnItem (it_ent, it);
		FinishSpawningItem(it_ent );
		memset( &trace, 0, sizeof( trace ) );
		Touch_Item (it_ent, ent, &trace);
		if (it_ent->inuse) {
			G_FreeEntity( it_ent );
		}
	}
}

/*
==================
Cmd_God_f

Sets client to godmode

argv(0) god
==================
*/
void Cmd_God_f (gentity_t *ent)
{
	char	*msg;

	if ( !CheatsOk( ent ) ) {
		return;
	}

	ent->flags ^= FL_GODMODE;
	if (!(ent->flags & FL_GODMODE) )
		msg = "godmode OFF\n";
	else
		msg = "godmode ON\n";

	trap_SendServerCommand( ent-g_entities, va("print \"%s\"", msg));
}


/*
==================
Cmd_Notarget_f

Sets client to notarget

argv(0) notarget
==================
*/
void Cmd_Notarget_f( gentity_t *ent ) {
	char	*msg;

	if ( !CheatsOk( ent ) ) {
		return;
	}

	ent->flags ^= FL_NOTARGET;
	if (!(ent->flags & FL_NOTARGET) )
		msg = "notarget OFF\n";
	else
		msg = "notarget ON\n";

	trap_SendServerCommand( ent-g_entities, va("print \"%s\"", msg));
}


/*
==================
Cmd_Noclip_f

argv(0) noclip
==================
*/
void Cmd_Noclip_f( gentity_t *ent ) {
	char	*msg;

	if ( !CheatsOk( ent ) ) {
		return;
	}

	if ( ent->client->noclip ) {
		msg = "noclip OFF\n";
	} else {
		msg = "noclip ON\n";
	}
	ent->client->noclip = !ent->client->noclip;

	trap_SendServerCommand( ent-g_entities, va("print \"%s\"", msg));
}


/*
==================
Cmd_LevelShot_f

This is just to help generate the level pictures
for the menus.  It goes to the intermission immediately
and sends over a command to the client to resize the view,
hide the scoreboard, and take a special screenshot
==================
*/
void Cmd_LevelShot_f( gentity_t *ent ) {
	if ( !CheatsOk( ent ) ) {
		return;
	}

	// doesn't work in single player
	if ( g_gametype.integer != 0 ) {
		trap_SendServerCommand( ent-g_entities, 
			"print \"Must be in g_gametype 0 for levelshot\n\"" );
		return;
	}

	BeginIntermission();
	trap_SendServerCommand( ent-g_entities, "clientLevelShot" );
}


/*
==================
Cmd_TeamTask_f

From TA.
==================
*/
void Cmd_TeamTask_f( gentity_t *ent ) {
	char userinfo[MAX_INFO_STRING];
	char		arg[MAX_TOKEN_CHARS];
	int task;
	int client = ent->client - level.clients;

	if ( trap_Argc() != 2 ) {
		return;
	}
	trap_Argv( 1, arg, sizeof( arg ) );
	task = atoi( arg );

	trap_GetUserinfo(client, userinfo, sizeof(userinfo));
	Info_SetValueForKey(userinfo, "teamtask", va("%d", task));
	trap_SetUserinfo(client, userinfo);
	ClientUserinfoChanged(client);
}



/*
=================
Cmd_Kill_f
=================
*/
void Cmd_Kill_f( gentity_t *ent ) {
	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		return;
	}
	if ( ent->client->tempSpectate > level.time) {
		return;		// Cant /kill in tempspec
	}

	if (ent->health <= 0) {
		return;
	}

	if ((g_gametype.integer == GT_DUEL || g_gametype.integer == GT_POWERDUEL) &&
		level.numPlayingClients > 1 && !level.warmupTime)
	{
		if (!g_allowDuelSuicide.integer)
		{
			trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "ATTEMPTDUELKILL")) );
			return;
		}
	}

	ent->flags &= ~FL_GODMODE;
	ent->client->ps.stats[STAT_HEALTH] = ent->health = -999;
	player_die (ent, ent, ent, 100000, MOD_SUICIDE);
}

gentity_t *G_GetDuelWinner(gclient_t *client)
{
	gclient_t *wCl;
	int i;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		wCl = &level.clients[i];
		
		if (wCl && wCl != client && /*wCl->ps.clientNum != client->ps.clientNum &&*/
			wCl->pers.connected == CON_CONNECTED && wCl->sess.sessionTeam != TEAM_SPECTATOR)
		{
			return &g_entities[wCl->ps.clientNum];
		}
	}

	return NULL;
}

/*
=================
BroadCastTeamChange

Let everyone know about a team change
=================
*/
void BroadcastTeamChange( gclient_t *client, int oldTeam )
{
	client->ps.fd.forceDoInit = 1; //every time we change teams make sure our force powers are set right

	if ( client->sess.sessionTeam == TEAM_RED ) {
		trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " %s\n\"",
			client->pers.netname, G_GetStringEdString2(bgGangWarsTeams[level.redTeam].joinstring)) );
			//client->pers.netname, G_GetStringEdString("MP_SVGAME", "JOINEDTHEREDTEAM")) );
	} else if ( client->sess.sessionTeam == TEAM_BLUE ) {
		trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " %s\n\"",
			client->pers.netname, G_GetStringEdString2(bgGangWarsTeams[level.blueTeam].joinstring)) );
		//client->pers.netname, G_GetStringEdString("MP_SVGAME", "JOINEDTHEBLUETEAM")));
	} else if ( client->sess.sessionTeam == TEAM_SPECTATOR && oldTeam != TEAM_SPECTATOR ) {
		trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " %s\n\"",
		client->pers.netname, G_GetStringEdString("MP_SVGAME", "JOINEDTHESPECTATORS")));
	} else if ( client->sess.sessionTeam == TEAM_FREE ) {
		if (g_gametype.integer == GT_DUEL || g_gametype.integer == GT_POWERDUEL)
		{
			/*
			gentity_t *currentWinner = G_GetDuelWinner(client);

			if (currentWinner && currentWinner->client)
			{
				trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " %s %s\n\"",
				currentWinner->client->pers.netname, G_GetStringEdString("MP_SVGAME", "VERSUS"), client->pers.netname));
			}
			else
			{
				trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " %s\n\"",
				client->pers.netname, G_GetStringEdString("MP_SVGAME", "JOINEDTHEBATTLE")));
			}
			*/
			//NOTE: Just doing a vs. once it counts two players up
		}
		else
		{
			trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " %s\n\"",
			client->pers.netname, G_GetStringEdString("MP_SVGAME", "JOINEDTHEBATTLE")));
		}
	}

	G_LogPrintf ( "setteam:  %i %s %s\n",
				  client - &level.clients[0],
				  TeamName ( oldTeam ),
				  TeamName ( client->sess.sessionTeam ) );
}

qboolean G_PowerDuelCheckFail(gentity_t *ent)
{
	int			loners = 0;
	int			doubles = 0;

	if (!ent->client || ent->client->sess.duelTeam == DUELTEAM_FREE)
	{
		return qtrue;
	}

	G_PowerDuelCount(&loners, &doubles, qfalse);

	if (ent->client->sess.duelTeam == DUELTEAM_LONE && loners >= 1)
	{
		return qtrue;
	}

	if (ent->client->sess.duelTeam == DUELTEAM_DOUBLE && doubles >= 2)
	{
		return qtrue;
	}

	return qfalse;
}

/*
=================
SetTeam
=================
*/
qboolean g_dontPenalizeTeam = qfalse;
qboolean g_preventTeamBegin = qfalse;
void SetTeam( gentity_t *ent, char *s ) {
	int					team, oldTeam;
	gclient_t			*client;
	int					clientNum;
	spectatorState_t	specState;
	int					specClient;
	int					teamLeader;

	//
	// see what change is requested
	//
	client = ent->client;

	clientNum = client - level.clients;
	specClient = 0;
	specState = SPECTATOR_NOT;
	if ( !Q_stricmp( s, "scoreboard" ) || !Q_stricmp( s, "score" )  ) {
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_SCOREBOARD;
	} else if ( !Q_stricmp( s, "follow1" ) ) {
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_FOLLOW;
		specClient = -1;
	} else if ( !Q_stricmp( s, "follow2" ) ) {
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_FOLLOW;
		specClient = -2;
	} else if ( !Q_stricmp( s, "spectator" ) || !Q_stricmp( s, "s" ) ) {
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_FREE;
	} else if ( g_gametype.integer >= GT_TEAM ) {
		// if running a team game, assign player to one of the teams
		specState = SPECTATOR_NOT;
		if ( !Q_stricmp( s, "red" ) || !Q_stricmp( s, "r" ) ) {
			team = TEAM_RED;
		} else if ( !Q_stricmp( s, "blue" ) || !Q_stricmp( s, "b" ) ) {
			team = TEAM_BLUE;
		} else {
			// pick the team with the least number of players
			//For now, don't do this. The legalize function will set powers properly now.
			/*
			if (g_forceBasedTeams.integer)
			{
				if (ent->client->ps.fd.forceSide == FORCE_LIGHTSIDE)
				{
					team = TEAM_BLUE;
				}
				else
				{
					team = TEAM_RED;
				}
			}
			else
			{
			*/
				team = PickTeam( clientNum );
			//}
		}

#ifdef __JKG_NINELIVES__
#ifdef __JKG_TICKETING__
#ifdef __JKG_ROUNDBASED__
		// Can't switch teams to anything but Spectator if the round is not ready
		if(level.time - level.gamestartTime > 300000 &&
			g_gametype.integer >= GT_LMS_NINELIVES &&
			g_gametype.integer <= GT_LMS_ROUNDS)
		{
			// Restrict joining the game if the game is beyond 5 minutes time
			if( team != TEAM_SPECTATOR )
			{
				return;
			}
		}
#endif
#endif
#endif

		if ( g_teamForceBalance.integer && !g_trueJedi.integer ) {
			int		counts[TEAM_NUM_TEAMS];

			counts[TEAM_BLUE] = TeamCount( ent->client->ps.clientNum, TEAM_BLUE );
			counts[TEAM_RED] = TeamCount( ent->client->ps.clientNum, TEAM_RED );

			// We allow a spread of two
			if ( team == TEAM_RED && counts[TEAM_RED] - counts[TEAM_BLUE] > 1 ) {
				//For now, don't do this. The legalize function will set powers properly now.
				/*
				if (g_forceBasedTeams.integer && ent->client->ps.fd.forceSide == FORCE_DARKSIDE)
				{
					trap_SendServerCommand( ent->client->ps.clientNum, 
						va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "TOOMANYRED_SWITCH")) );
				}
				else
				*/
				{
					//trap_SendServerCommand( ent->client->ps.clientNum, 
					//	va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "TOOMANYRED")) );
					trap_SendServerCommand( ent->client->ps.clientNum, 
						va("print \"%s\n\"", G_GetStringEdString2(bgGangWarsTeams[level.redTeam].toomanystring)) );
				}
				return; // ignore the request
			}
			if ( team == TEAM_BLUE && counts[TEAM_BLUE] - counts[TEAM_RED] > 1 ) {
				//For now, don't do this. The legalize function will set powers properly now.
				/*
				if (g_forceBasedTeams.integer && ent->client->ps.fd.forceSide == FORCE_LIGHTSIDE)
				{
					trap_SendServerCommand( ent->client->ps.clientNum, 
						va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "TOOMANYBLUE_SWITCH")) );
				}
				else
				*/
				{
					//trap_SendServerCommand( ent->client->ps.clientNum, 
					//	va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "TOOMANYBLUE")) );
					trap_SendServerCommand( ent->client->ps.clientNum, 
						va("print \"%s\n\"", G_GetStringEdString2(bgGangWarsTeams[level.blueTeam].toomanystring)) );
				}
				return; // ignore the request
			}

			// It's ok, the team we are switching to has less or same number of players
		}

		//For now, don't do this. The legalize function will set powers properly now.
		/*
		if (g_forceBasedTeams.integer)
		{
			if (team == TEAM_BLUE && ent->client->ps.fd.forceSide != FORCE_LIGHTSIDE)
			{
				trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "MUSTBELIGHT")) );
				return;
			}
			if (team == TEAM_RED && ent->client->ps.fd.forceSide != FORCE_DARKSIDE)
			{
				trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "MUSTBEDARK")) );
				return;
			}
		}
		*/

	} else {
		// force them to spectators if there aren't any spots free
		team = TEAM_FREE;
	}

	// override decision if limiting the players
	if ( (g_gametype.integer == GT_DUEL)
		&& level.numNonSpectatorClients >= 2 )
	{
		team = TEAM_SPECTATOR;
	}
	else if ( (g_gametype.integer == GT_POWERDUEL)
		&& (level.numPlayingClients >= 3 || G_PowerDuelCheckFail(ent)) )
	{
		team = TEAM_SPECTATOR;
	}
	else if ( g_maxGameClients.integer > 0 && 
		level.numNonSpectatorClients >= g_maxGameClients.integer )
	{
		team = TEAM_SPECTATOR;
	}

	//
	// decide if we will allow the change
	//
	oldTeam = client->sess.sessionTeam;
	if ( team == oldTeam && team != TEAM_SPECTATOR ) {
		return;
	}

	//
	// execute the team change
	//

	// if the player was dead leave the body
	if ( client->ps.stats[STAT_HEALTH] <= 0 && client->sess.sessionTeam != TEAM_SPECTATOR ) {
		MaintainBodyQueue(ent);
	}

	// he starts at 'base'
	client->pers.teamState.state = TEAM_BEGIN;
	if ( oldTeam != TEAM_SPECTATOR ) {
		// Kill him (makes sure he loses flags, etc)
		ent->flags &= ~FL_GODMODE;
		ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
		g_dontPenalizeTeam = qtrue;
		player_die (ent, ent, ent, 100000, MOD_SUICIDE);
		g_dontPenalizeTeam = qfalse;

	}
	// they go to the end of the line for tournements
	if ( team == TEAM_SPECTATOR ) {
		if ( (g_gametype.integer != GT_DUEL) || (oldTeam != TEAM_SPECTATOR) )	{//so you don't get dropped to the bottom of the queue for changing skins, etc.
			client->sess.spectatorTime = level.time;
		}
	}

	client->sess.sessionTeam = team;
	client->sess.spectatorState = specState;
	client->sess.spectatorClient = specClient;

	client->sess.teamLeader = qfalse;
	if ( team == TEAM_RED || team == TEAM_BLUE ) {
		teamLeader = TeamLeader( team );
		// if there is no team leader or the team leader is a bot and this client is not a bot
		if ( teamLeader == -1 || ( !(g_entities[clientNum].r.svFlags & SVF_BOT) && (g_entities[teamLeader].r.svFlags & SVF_BOT) ) ) {
			//SetLeader( team, clientNum );
		}
	}
	// make sure there is a team leader on the team the player came from
	if ( oldTeam == TEAM_RED || oldTeam == TEAM_BLUE ) {
		CheckTeamLeader( oldTeam );
	}

	BroadcastTeamChange( client, oldTeam );

	//make a disappearing effect where they were before teleporting them to the appropriate spawn point,
	//if we were not on the spec team
	if (oldTeam != TEAM_SPECTATOR)
	{
		gentity_t *tent = G_TempEntity( client->ps.origin, EV_PLAYER_TELEPORT_OUT );
		tent->s.clientNum = clientNum;
	}

	// get and distribute relevent paramters
	ClientUserinfoChanged( clientNum );

	if (!g_preventTeamBegin)
	{
		ClientBegin( clientNum, qfalse );
	}
}

/*
=================
StopFollowing

If the client being followed leaves the game, or you just want to drop
to free floating spectator mode
=================
*/
void StopFollowing( gentity_t *ent ) {
	ent->client->ps.persistant[ PERS_TEAM ] = TEAM_SPECTATOR;	
	ent->client->sess.sessionTeam = TEAM_SPECTATOR;	
	ent->client->sess.spectatorState = SPECTATOR_FREE;
	ent->client->ps.pm_flags &= ~PMF_FOLLOW;
	ent->r.svFlags &= ~SVF_BOT;
	ent->client->ps.clientNum = ent - g_entities;
	ent->client->ps.weapon = WP_NONE;
	ent->client->ps.weaponVariation = 0;
	ent->client->ps.m_iVehicleNum = 0;
	ent->client->ps.viewangles[ROLL] = 0.0f;
	ent->client->ps.forceHandExtend = HANDEXTEND_NONE;
	ent->client->ps.forceHandExtendTime = 0;
	ent->client->ps.zoomMode = 0;
	ent->client->ps.zoomLocked = 0;
	ent->client->ps.zoomLockTime = 0;
	ent->client->ps.legsAnim = 0;
	ent->client->ps.legsTimer = 0;
	ent->client->ps.torsoAnim = 0;
	ent->client->ps.torsoTimer = 0;
}

/*
=================
Cmd_Team_f
=================
*/
void Cmd_Team_f( gentity_t *ent ) {
	int			oldTeam;
	char		s[MAX_TOKEN_CHARS];

	if ( trap_Argc() != 2 ) {
		oldTeam = ent->client->sess.sessionTeam;
		switch ( oldTeam ) {
		case TEAM_BLUE:
			trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", bgGangWarsTeams[level.blueTeam].longname) );
			break;
		case TEAM_RED:
			trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", bgGangWarsTeams[level.redTeam].longname) );
			break;
		case TEAM_FREE:
			trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "PRINTFREETEAM")) );
			break;
		case TEAM_SPECTATOR:
			trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "PRINTSPECTEAM")) );
			break;
		}
		return;
	}

	if ( ent->client->switchTeamTime > level.time ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOSWITCH")) );
		return;
	}

	if (gEscaping)
	{
		return;
	}

	// if they are playing a tournement game, count as a loss
	if ( g_gametype.integer == GT_DUEL
		&& ent->client->sess.sessionTeam == TEAM_FREE ) {//in a tournament game
		//disallow changing teams
		trap_SendServerCommand( ent-g_entities, "print \"Cannot switch teams in Duel\n\"" );
		return;
		//FIXME: why should this be a loss???
		//ent->client->sess.losses++;
	}

	if (g_gametype.integer == GT_POWERDUEL)
	{ //don't let clients change teams manually at all in powerduel, it will be taken care of through automated stuff
		trap_SendServerCommand( ent-g_entities, "print \"Cannot switch teams in Power Duel\n\"" );
		return;
	}

	trap_Argv( 1, s, sizeof( s ) );

	SetTeam( ent, s );

	ent->client->switchTeamTime = level.time + 5000;
}



//==========================================================
// INVENTORY RELATED COMMANDS
//==========================================================

void JKG_CompactInventory(inventory_t inventory)
{
	int i;
	// We can make this even more efficient, but not really much point.
	for(i = 0; i < MAX_INVENTORY_ITEMS; i++)
	{
	    int j;
	    
	    if ( inventory[i].id )
	    {
	        continue;
	    }
	    
	    for ( j = i + 1; j < MAX_INVENTORY_ITEMS; j++ )
	    {
	        if ( inventory[j].id )
	        {
	            inventory[i] = inventory[j];
	            memset (&inventory[j], 0, sizeof (inventory[j]));
	            
	            break;
	        }
	    }
	    
	    if ( j == MAX_INVENTORY_ITEMS )
	    {
	        // Current slot is null, but can find no other items above this one.
	        // Must be the end of the list.
	        break;
	    }
	}
}

extern void JKG_Easy_RemoveItemFromInventory(int number, itemInstance_t **inventory, gentity_t *owner, qboolean NPC);
/*
=================
JKG_Cmd_Loot_f
=================
*/
void JKG_Cmd_Loot_f(gentity_t *ent, int otherNum, int lootID, qboolean trade)
{
    int i;
	gentity_t *other = &g_entities[otherNum];

	//Check whether we're trying to loot ourselves.
	if(ent->s.number == other->s.number){
		trap_SendServerCommand( ent-g_entities, "print \"You cannot loot yourself.\n\"" );
		return;
	}

	//Check whether our target is a player
	if(other->client && !other->client->NPC_class){
		trap_SendServerCommand( ent-g_entities, "print \"You cannot loot other players.\n\"" );
		return;
	}
	
	//Check and see if we've got a valid loot ID
	if( lootID < 0 ){
		trap_SendServerCommand( ent-g_entities, "print \"Invalid loot ID\n\"" );
		return;
	}

	//check to make sure this object is even lootable
	//if( !other->inventory[lootID].id ){
	if( !other->inventory->items[lootID].id ) {
		trap_SendServerCommand( ent-g_entities, "print \"Invalid target or loot ID.\n\"" );
		return;
	}

	//check to make sure we can carry this item
	//if( (ent->client->coreStats.weight - other->inventory[lootID].id->weight) < 0 ){
	if ( (signed int)( ent->client->coreStats.weight - other->inventory->items[lootID].id->weight ) < 0 ) {
		trap_SendServerCommand( ent-g_entities, "print \"You can't carry any more.\n\"" );
		return;
	}

	//check if we've got enough open slots
	/*for(i = 0; i < MAX_INVENTORY_ITEMS; i++)
	{
		if(!ent->inventory[i].id)
		{
			break;
		}
	}
	if(i >= MAX_INVENTORY_ITEMS)
	{
		trap_SendServerCommand( ent-g_entities, "print \"You are out of space.\n\"" );
		return;
	}*/
	i = 0;
	while ( 1 )
	{
		if(!ent->inventory->items[i].id)
			break;
	}

	//find the distance between the two targets
	if ( DistanceSquared (other->s.origin, ent->client->ps.origin) >= MAX_LOOT_DISTANCE * MAX_LOOT_DISTANCE ) {
		trap_SendServerCommand( ent-g_entities, "print \"You are too far away to loot that target.\n\"" );
		return;
	}

	//We can carry this item. Super. Now let's find our first available inventory space.
	//eezstreet note: WHY THE HECK DID WE EVEN LOOP LIKE THIS TO BEGIN WITH?! ABORT! ABORT!
	/*for ( i = 0; i < MAX_INVENTORY_ITEMS; i++ )
	{
		if( !ent->inventory[i].id )
		{
			ent->inventory[i] = other->inventory[lootID];
#ifdef _DEBUG
			trap_SendServerCommand( ent-g_entities, va("print \"loot: %s\n\"", ent->inventory[i].id->displayName) );
#endif
			trap_SendServerCommand( ent-g_entities, va("pInv add %i %i %i %i %i %i %i %i %i %i %i %i %i %i",
				i,
				ent->inventory[i].id->itemID, 
				ent->inventory[i].itemQuality,
				ent->inventory[i].amount[0],
				ent->inventory[i].amount[1],
				ent->inventory[i].amount[2],
				ent->inventory[i].amount[3],
				ent->inventory[i].amount[4],
				ent->inventory[i].amount[5],
				ent->inventory[i].amount[6],
				ent->inventory[i].amount[7],
				ent->inventory[i].amount[8],
				ent->inventory[i].amount[9],
				ent->inventory[i].equipped));
			ent->client->coreStats.weight += other->inventory[lootID].id->weight;
			memset(&other->inventory[lootID], 0, sizeof(itemInstance_t));
			
			JKG_CompactInventory (other->inventory);
			return;
		}
	}*/
#ifdef _DEBUG
	trap_SendServerCommand( ent-g_entities, va("print \"loot: %s\n\"", ent->inventory->items[i].id->displayName));
#endif
	trap_SendServerCommand( ent-g_entities, va("pInv add %i %i %i %i %i %i %i %i %i %i %i %i %i %i",
		i,
		ent->inventory->items[i].id->itemID,
		ent->inventory->items[i].amount[0],
		ent->inventory->items[i].amount[1],
		ent->inventory->items[i].amount[2],
		ent->inventory->items[i].amount[3],
		ent->inventory->items[i].amount[4],
		ent->inventory->items[i].amount[5],
		ent->inventory->items[i].amount[6],
		ent->inventory->items[i].amount[7],
		ent->inventory->items[i].amount[8],
		ent->inventory->items[i].amount[9],
		ent->inventory->items[i].equipped));
	ent->client->coreStats.weight += other->inventory->items[lootID].id->weight;
	JKG_Easy_RemoveItemFromInventory(lootID, &other->inventory->items, other, qtrue);
}

/*
=================
JKG_Cmd_ItemAction_f
=================
*/
extern void JKG_Easy_RemoveItemFromInventory(int number, itemInstance_t **inventory, gentity_t *owner, qboolean NPC);
extern void JKG_Easy_DIMA_Remove(inv_t *inventory, unsigned int invID);
void JKG_Cmd_ItemAction_f (gentity_t *ent, int itemNum)
{
	itemInstance_t *itemInUse;
	int i;
	if(!ent->client)
	{
		return; //NOTENOTE: NPCs can perform item actions. Nifty, eh?
	}

	if(ent->client->ps.stats[STAT_HEALTH] <= 0 ||
		ent->client->ps.pm_type == PM_DEAD)
	{
		// Bugfix: Can no longer use items when dead --eez
		Com_Printf("Can't use items while dead!\n");
		return;
	}
	
	if ( itemNum < 0 || itemNum >= MAX_INVENTORY_ITEMS )
	{
	    return;
	}
	if( itemNum > ent->inventory->elements )
	{
		//Nope.
		return;
	}

	//itemInUse = &ent->inventory[itemNum];
	itemInUse = &ent->inventory->items[itemNum];
	if ( !itemInUse->id )
	{
#ifdef _DEBUG
	    trap_SendServerCommand (ent->s.number, "print \"No item with that number.\n\"");
#endif
	    return;
	}
	
	for(i = 0; i < MAX_PSPELLS; i++){
		switch(itemInUse->id->pSpell[i])
		{
			case -1:
			case PSPELL_NONE:
				return;
				break;
			case PSPELL_ADD_PS:
				ent->client->ps.stats[itemInUse->id->affector[i]] += (int)itemInUse->amount[i];
				break;
			case PSPELL_SUB_PS:
				ent->client->ps.stats[itemInUse->id->affector[i]] -= (int)itemInUse->amount[i];
				break;
			case PSPELL_MUL_PS:
				ent->client->ps.stats[itemInUse->id->affector[i]] *= (int)itemInUse->amount[i];
				break;
			case PSPELL_DIV_PS:
				ent->client->ps.stats[itemInUse->id->affector[i]] /= (int)itemInUse->amount[i];
				break;
			case PSPELL_EXP_PS:
				ent->client->ps.stats[itemInUse->id->affector[i]] = (int)pow(ent->client->ps.stats[itemInUse->id->affector[i]], (double)itemInUse->amount[i]);
				break;
			case PSPELL_SQT_PS:
				ent->client->ps.stats[itemInUse->id->affector[i]] = (int)sqrt(ent->client->ps.stats[itemInUse->id->affector[i]]);
				break;
			case PSPELL_SEEKER:
				if(ent->client->ps.eFlags & EF_SEEKERDRONE)
				{
					trap_SendServerCommand(ent->client->ps.clientNum, "print \"You cannot have more than one seeker drone at a time.\n\"");
					return;
				}
				ItemUse_Seeker(ent);
				break;
			case PSPELL_PLAYSOUND:
				G_Sound( ent, CHAN_AUTO, itemInUse->id->affector[i] );
				break;
			case PSPELL_DESTROYITEM:
				JKG_Easy_RemoveItemFromInventory(itemNum, &ent->inventory->items, ent, qfalse);
				return;
				break;
			case PSPELL_CND_GT_PS:
				{
					if(ent->client->ps.stats[itemInUse->id->affector[i]] > (int)itemInUse->amount[i])
					{
						return;
					}
				}
				break;
			case PSPELL_CND_EQ_PS:
				{
					if(ent->client->ps.stats[itemInUse->id->affector[i]] == (int)itemInUse->amount[i])
					{
						return;
					}
				}
				break;
			case PSPELL_CND_LT_PS:
				{
					if(ent->client->ps.stats[itemInUse->id->affector[i]] < (int)itemInUse->amount[i])
					{
						return;
					}
				}
				break;
			case PSPELL_DO_EVENT:
				G_AddEvent(ent, itemInUse->id->affector[i], (int)itemInUse->amount[i]);
				break;
			default:
				ent->client->ps.stats[itemInUse->id->affector[i]] += (int)itemInUse->amount[i];
				break;
		}
		if(itemInUse->id->affector[i] == STAT_HEALTH && itemInUse->id->pSpell[i] < PSPELL_SEEKER)
			ent->health = ent->client->ps.stats[STAT_HEALTH];
	}
}

/*
=================
JKG_Cmd_ShowInv_f
=================
*/
void JKG_Cmd_ShowInv_f(gentity_t *ent)
{
    char buffer[MAX_STRING_CHARS] = { 0 };
	int i = 0;
	if(!ent->client)
		return;

	Q_strncpyz (buffer, "Inventory ID | Item Num | Instance Name                       | Weight\n", sizeof (buffer));
	Q_strcat (buffer, sizeof (buffer), "-------------+----------+-----------------------------------------------+--------\n");

	for ( i = 0; i < ent->inventory->elements; i++ )
	{
	        
		if(!ent->inventory->items[i].equipped)
			Q_strcat (buffer, sizeof(buffer), va(S_COLOR_WHITE "%12i | %8i | %-45s | %i\n", i, ent->inventory->items[i].id->itemID, ent->inventory->items[i].id->displayName, ent->inventory->items[i].id->weight));
			//Q_strcat (buffer, sizeof (buffer), va (S_COLOR_WHITE "%12i | %8i | %-45s | %i\n", i, ent->inventory[i].id->itemID, ent->inventory[i].id->displayName, ent->inventory[i].id->weight));
		else
			Q_strcat (buffer, sizeof(buffer), va(S_COLOR_YELLOW "%12i | %8i | %-45s | %i\n", i, ent->inventory->items[i].id->itemID, va ("%s (equipped)", ent->inventory->items[i].id->displayName), ent->inventory->items[i].id->weight));
			//Q_strcat (buffer, sizeof (buffer), va (S_COLOR_YELLOW "%12i | %8i | %-45s | %i\n", i, ent->inventory[i].id->itemID, va ("%s (equipped)", ent->inventory[i].id->displayName), ent->inventory[i].id->weight));
	}
	
	Q_strcat (buffer, sizeof (buffer), va( "%i total items, %i weight left.", i, MAX_INVENTORY_WEIGHT - ent->client->coreStats.weight));
	
	trap_SendServerCommand (ent->s.number, va ("print \"%s\n\"", buffer));
}

/*
=========================================
JKG_Cmd_EquipToACI_f / JKG_Cmd_Unequip_f
=========================================
*/

extern void JKG_EquipItem(gentity_t *ent, int iNum);
extern void JKG_UnequipItem(gentity_t *ent, int iNum);
void JKG_Cmd_EquipItem_f(gentity_t *ent)
{
	char arg[6];
	if(trap_Argc() != 2)
	{
		trap_SendServerCommand( ent->s.number, "print \"Usage: /equip <item num>\n\"" );
		return;
	}
	
	trap_Argv (1, arg, sizeof(arg));

	JKG_EquipItem (ent, atoi (arg));
}

void JKG_Cmd_UnequipItem_f(gentity_t *ent)
{
    char arg[6];
	if(trap_Argc() != 2)
	{
		trap_SendServerCommand( ent->s.number, "print \"Usage: /unequip <item num>\n\"" );
		return;
	}
	
	trap_Argv (1, arg, sizeof(arg));

	JKG_UnequipItem (ent, atoi (arg));
}

/*
==================
JKG_Cmd_DestroyItem_f
Destroys an item from your inventory
==================
*/
void JKG_Cmd_DestroyItem_f(gentity_t *ent)
{
	char arg[64];
	int i, inventoryID;
	itemInstance_t destroyedItem;
	trap_Argv(1, arg, sizeof(arg));

	memset(&destroyedItem, 0, sizeof(itemInstance_t));

	if(trap_Argc() < 2)
	{
		trap_SendServerCommand(ent->client->ps.clientNum, "print \"Syntax: /inventoryDestroy <inventory ID or internal>\n\"");
		return;
	}

	if(JKG_CheckIfNumber(arg) == JKGSTR_DECIMAL)
	{
		//if(!ent->inventory[atoi(arg)].id || atoi(arg) < 0)
		if(!ent->inventory->items[atoi(arg)].id || atoi(arg) < 0)
		{
			trap_SendServerCommand(ent->client->ps.clientNum, "print \"^1Invalid inventory ID.\n\"");
			return;
		}
		//destroyedItem = ent->inventory[atoi(arg)];
		destroyedItem = ent->inventory->items[atoi(arg)];
		inventoryID = atoi(arg);
		if(inventoryID < 0 || inventoryID >= ent->inventory->elements)
		{
			//No way.
			trap_SendServerCommand(ent->s.number, "print \"You cannot destroy/sell an item on an index that is out of bounds.\n\"");
			return;
		}
		JKG_Easy_RemoveItemFromInventory(atoi(arg), (itemInstance_t **)ent->inventory, ent, qfalse);
	}
	else
	{
		/*for(i = 0; i < MAX_INVENTORY_ITEMS; i++)
		{
			if(ent->inventory[i].id)
			{
				if(!Q_stricmp(ent->inventory[i].id->internalName, ConcatArgs(1)))
				{
					destroyedItem = ent->inventory[i];
					inventoryID = i;
					JKG_Easy_RemoveItemFromInventory(i, (itemInstance_t **)ent->inventory);
					break;
				}
			}
			else
			{
				trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^1Could not find item with internal %s\n\"", ConcatArgs(1)));
				return;
			}
		}*/
		for(i = 0; i < ent->inventory->elements; i++)
		{
			if(ent->inventory->items[i].id)
			{
				if(!Q_stricmp(ent->inventory->items[i].id->internalName, ConcatArgs(1)))
				{
					destroyedItem = ent->inventory->items[i];
					inventoryID = i;
					JKG_Easy_RemoveItemFromInventory(i, (itemInstance_t **)ent->inventory->items, ent, qfalse);
					break;
				}
				else
					continue;
			}
		}
		if(!destroyedItem.id)
		{
			trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^1Could not find item with internal %s\n\"", ConcatArgs(1)));
			return;
		}

	}
	trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^2DEBUG: %s (internal %s, invID %i) destroyed.\n\"", destroyedItem.id->displayName,
		destroyedItem.id->internalName, inventoryID));
}

void JKG_Cmd_SellItem_f(gentity_t *ent)
{
	// TODO: put proper pricing here
	char arg[64];
	trap_Argv(1, arg, sizeof(arg));
	if(trap_Argc() < 2)
	{
		trap_SendServerCommand(ent->client->ps.clientNum, "print \"Syntax: /inventorySell <inventory ID or internal>\n\"");
		return;
	}
	if(JKG_CheckIfNumber(arg) == JKGSTR_DECIMAL)
	{
		int numbah = atoi(arg);
		if(numbah >= ent->inventory->elements)
		{
			return;
		}
		if(!ent->inventory->items[numbah].id)
		{
			return;
		}
		// DO NOT ALLOW SELLING OF STARTER WEAPONS! (unless you already have another gun in your inventory)
		if(ent->inventory->items[numbah].id->itemType == ITEM_WEAPON)
		{
			if(!Q_stricmp(GetWeaponData(ent->inventory->items[numbah].id->weapon, ent->inventory->items[numbah].id->variation)->displayName,
				level.startingWeapon))
			{
				if(ent->inventory->elements < 2)
				{
					trap_SendServerCommand(ent->client->ps.clientNum, "print \"You cannot sell your starter gun unless you have another item in your inventory.\n\"");
					return;
				}
				ent->inventory->items[numbah].id->baseCost = 2;	// hackery. Starting weapon sells for 1 credit.
			}
		}
		ent->client->ps.persistant[PERS_CREDITS] += (ent->inventory->items[numbah].id->baseCost)/2;
		JKG_Cmd_DestroyItem_f(ent);
	}
	else
	{
		int i, inventoryID;
		itemInstance_t destroyedItem;
		for(i = 0; i < ent->inventory->elements; i++)
		{
			if(ent->inventory->items[i].id)
			{
				if(!Q_stricmp(ent->inventory->items[i].id->internalName, ConcatArgs(1)))
				{
					destroyedItem = ent->inventory->items[i];
					inventoryID = i;

					// DO NOT ALLOW DESTROYING OF STARTER ITEMS!
					if(ent->inventory->items[i].id->itemType == ITEM_WEAPON)
					{
						if(!Q_stricmp(GetWeaponData(ent->inventory->items[i].id->weapon, ent->inventory->items[i].id->variation)->displayName,
							level.startingWeapon))
						{
							if(ent->inventory->elements < 2)
							{
								trap_SendServerCommand(ent->client->ps.clientNum, "print \"You cannot sell your starter gun unless you have another item in your inventory.\n\"");
								return;
							}
						}
					}

					ent->client->ps.persistant[PERS_CREDITS] += (ent->inventory->items[i].id->baseCost)/2;
					JKG_Easy_RemoveItemFromInventory(i, (itemInstance_t **)ent->inventory->items, ent, qfalse);
					break;
				}
				else
					continue;
			}
		}
		if(!destroyedItem.id)
		{
			trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^1Could not find item with internal %s\n\"", ConcatArgs(1)));
			return;
		}
	}
	trap_SendServerCommand(ent->s.number, va("inventory_update %i", ent->client->ps.persistant[PERS_CREDITS]));
}
/*
=================
Cmd_DuelTeam_f
=================
*/
void Cmd_DuelTeam_f(gentity_t *ent)
{
	int			oldTeam;
	char		s[MAX_TOKEN_CHARS];

	if (g_gametype.integer != GT_POWERDUEL)
	{ //don't bother doing anything if this is not power duel
		return;
	}

	/*
	if (ent->client->sess.sessionTeam != TEAM_SPECTATOR)
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You cannot change your duel team unless you are a spectator.\n\""));
		return;
	}
	*/

	if ( trap_Argc() != 2 )
	{ //No arg so tell what team we're currently on.
		oldTeam = ent->client->sess.duelTeam;
		switch ( oldTeam )
		{
		case DUELTEAM_FREE:
			trap_SendServerCommand( ent-g_entities, va("print \"None\n\"") );
			break;
		case DUELTEAM_LONE:
			trap_SendServerCommand( ent-g_entities, va("print \"Single\n\"") );
			break;
		case DUELTEAM_DOUBLE:
			trap_SendServerCommand( ent-g_entities, va("print \"Double\n\"") );
			break;
		default:
			break;
		}
		return;
	}

	if ( ent->client->switchDuelTeamTime > level.time )
	{ //debounce for changing
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOSWITCH")) );
		return;
	}

	trap_Argv( 1, s, sizeof( s ) );

	oldTeam = ent->client->sess.duelTeam;

	if (!Q_stricmp(s, "free"))
	{
		ent->client->sess.duelTeam = DUELTEAM_FREE;
	}
	else if (!Q_stricmp(s, "single"))
	{
		ent->client->sess.duelTeam = DUELTEAM_LONE;
	}
	else if (!Q_stricmp(s, "double"))
	{
		ent->client->sess.duelTeam = DUELTEAM_DOUBLE;
	}
	else
	{
		trap_SendServerCommand( ent-g_entities, va("print \"'%s' not a valid duel team.\n\"", s) );
	}

	if (oldTeam == ent->client->sess.duelTeam)
	{ //didn't actually change, so don't care.
		return;
	}

	if (ent->client->sess.sessionTeam != TEAM_SPECTATOR)
	{ //ok..die
		int curTeam = ent->client->sess.duelTeam;
		ent->client->sess.duelTeam = oldTeam;
		G_Damage(ent, ent, ent, NULL, ent->client->ps.origin, 99999, DAMAGE_NO_PROTECTION, MOD_SUICIDE);
		ent->client->sess.duelTeam = curTeam;
	}
	//reset wins and losses
	ent->client->sess.wins = 0;
	ent->client->sess.losses = 0;

	//get and distribute relevent paramters
	ClientUserinfoChanged( ent->s.number );

	ent->client->switchDuelTeamTime = level.time + 5000;
}

/*
=================
Cmd_ForceChanged_f
=================
*/
void Cmd_ForceChanged_f( gentity_t *ent )
{
	char fpChStr[1024];
	const char *buf;
//	Cmd_Kill_f(ent);
	if (ent->client->sess.sessionTeam == TEAM_SPECTATOR)
	{ //if it's a spec, just make the changes now
		//trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "FORCEAPPLIED")) );
		//No longer print it, as the UI calls this a lot.
		WP_InitForcePowers( ent );
		goto argCheck;
	}

	buf = G_GetStringEdString("MP_SVGAME", "FORCEPOWERCHANGED");

	strcpy(fpChStr, buf);

	trap_SendServerCommand( ent-g_entities, va("print \"%s%s\n\n\"", S_COLOR_GREEN, fpChStr) );

	ent->client->ps.fd.forceDoInit = 1;
argCheck:
	if (g_gametype.integer == GT_DUEL || g_gametype.integer == GT_POWERDUEL)
	{ //If this is duel, don't even bother changing team in relation to this.
		return;
	}

	if (trap_Argc() > 1)
	{
		char	arg[MAX_TOKEN_CHARS];

		trap_Argv( 1, arg, sizeof( arg ) );

		if (arg && arg[0])
		{ //if there's an arg, assume it's a combo team command from the UI.
			Cmd_Team_f(ent);
		}
	}
}

extern qboolean WP_SaberStyleValidForSaber( saberInfo_t *saber1, saberInfo_t *saber2, int saberHolstered, int saberAnimLevel );
extern qboolean WP_UseFirstValidSaberStyle( saberInfo_t *saber1, saberInfo_t *saber2, int saberHolstered, int *saberAnimLevel );
qboolean G_SetSaber(gentity_t *ent, int saberNum, char *saberName, qboolean siegeOverride)
{
	char truncSaberName[64];
	int i = 0;

	while (saberName[i] && i < 64-1)
	{
        truncSaberName[i] = saberName[i];
		i++;
	}
	truncSaberName[i] = 0;

	if ( saberNum == 0 && (Q_stricmp( "none", truncSaberName ) == 0 || Q_stricmp( "remove", truncSaberName ) == 0) )
	{ //can't remove saber 0 like this
        strcpy(truncSaberName, "Kyle");
	}

	//Set the saber with the arg given. If the arg is
	//not a valid sabername defaults will be used.
	WP_SetSaber(ent->s.number, ent->client->saber, saberNum, truncSaberName);

	if (!ent->client->saber[0].model[0])
	{
		assert(0); //should never happen!
		strcpy(ent->client->sess.saberType, "none");
	}
	else
	{
		strcpy(ent->client->sess.saberType, ent->client->saber[0].name);
	}

	if (!ent->client->saber[1].model[0])
	{
		strcpy(ent->client->sess.saber2Type, "none");
	}
	else
	{
		strcpy(ent->client->sess.saber2Type, ent->client->saber[1].name);
	}

	if ( !WP_SaberStyleValidForSaber( &ent->client->saber[0], &ent->client->saber[1], ent->client->ps.saberHolstered, ent->client->ps.fd.saberAnimLevel ) )
	{
		WP_UseFirstValidSaberStyle( &ent->client->saber[0], &ent->client->saber[1], ent->client->ps.saberHolstered, &ent->client->ps.fd.saberAnimLevel );
		ent->client->ps.fd.saberAnimLevelBase = ent->client->saberCycleQueue = ent->client->ps.fd.saberAnimLevel;
	}

	return qtrue;
}

/*
=================
Cmd_Follow_f
=================
*/
void Cmd_Follow_f( gentity_t *ent ) {
	int		i;
	char	arg[MAX_TOKEN_CHARS];

	if ( trap_Argc() != 2 ) {
		if ( ent->client->sess.spectatorState == SPECTATOR_FOLLOW ) {
			StopFollowing( ent );
		}
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	i = ClientNumberFromString( ent, arg );
	if ( i == -1 ) {
		return;
	}

	// can't follow self
	if ( &level.clients[ i ] == ent->client ) {
		return;
	}

	// can't follow another spectator
	if ( level.clients[ i ].sess.sessionTeam == TEAM_SPECTATOR ) {
		return;
	}

	// JKG - can't follow another spectator
	if ( level.clients[ i ].tempSpectate >= level.time ) {
		return;
	}

	// if they are playing a tournement game, count as a loss
	if ( (g_gametype.integer == GT_DUEL || g_gametype.integer == GT_POWERDUEL)
		&& ent->client->sess.sessionTeam == TEAM_FREE ) {
		//WTF???
		ent->client->sess.losses++;
	}

	// first set them to spectator
	if ( ent->client->sess.sessionTeam != TEAM_SPECTATOR ) {
		SetTeam( ent, "spectator" );
	}

	ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
	ent->client->sess.spectatorClient = i;
}

/*
=================
Cmd_FollowCycle_f
=================
*/
void Cmd_FollowCycle_f( gentity_t *ent, int dir ) {
	int		clientnum;
	int		original;

	// if they are playing a tournement game, count as a loss
	if ( (g_gametype.integer == GT_DUEL || g_gametype.integer == GT_POWERDUEL)
		&& ent->client->sess.sessionTeam == TEAM_FREE ) {\
		//WTF???
		ent->client->sess.losses++;
	}
	// first set them to spectator
	if ( ent->client->sess.spectatorState == SPECTATOR_NOT ) {
		SetTeam( ent, "spectator" );
	}

	if ( dir != 1 && dir != -1 ) {
		G_Error( "Cmd_FollowCycle_f: bad dir %i", dir );
	}

	clientnum = ent->client->sess.spectatorClient;
	original = clientnum;
	do {
		clientnum += dir;
		if ( clientnum >= level.maxclients ) {
			clientnum = 0;
		}
		if ( clientnum < 0 ) {
			clientnum = level.maxclients - 1;
		}

		// can only follow connected clients
		if ( level.clients[ clientnum ].pers.connected != CON_CONNECTED ) {
			continue;
		}

		// can't follow another spectator
		if ( level.clients[ clientnum ].sess.sessionTeam == TEAM_SPECTATOR ) {
			continue;
		}

		// JKG - can't follow another spectator
		if ( level.clients[ clientnum ].tempSpectate >= level.time ) {
			return;
		}

		// this is good, we can use it
		ent->client->sess.spectatorClient = clientnum;
		ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
		return;
	} while ( clientnum != original );

	// leave it where it was
}


/*
==================
G_Say
==================
*/

static void G_SayTo( gentity_t *ent, gentity_t *other, int mode, int color, const char *name, const char *message, char *locMsg, int fadeLevel )
{
	if (!other) {
		return;
	}
	if (!other->inuse) {
		return;
	}
	if (!other->client) {
		return;
	}
	if ( other->client->pers.connected != CON_CONNECTED ) {
		return;
	}
	if ( mode == SAY_TEAM  && !OnSameTeam(ent, other) ) {
		return;
	}
	/*
	// no chatting to players in tournements
	if ( (g_gametype.integer == GT_DUEL || g_gametype.integer == GT_POWERDUEL)
		&& other->client->sess.sessionTeam == TEAM_FREE
		&& ent->client->sess.sessionTeam != TEAM_FREE ) {
		//Hmm, maybe some option to do so if allowed?  Or at least in developer mode...
		return;
	}
	*/
	//They've requested I take this out.

	if (locMsg)
	{
		trap_SendServerCommand( other-g_entities, va("%s %i \"%s\" \"%s\" \"%c\" \"%s\"", 
			mode == SAY_TEAM ? "ltchat" : "lchat",
			fadeLevel, name, locMsg, color, message));
	}
	else
	{
		trap_SendServerCommand( other-g_entities, va("%s %i \"%s%c%c%s\"", 
			mode == SAY_TEAM ? "tchat" : "chat",
			fadeLevel, name, Q_COLOR_ESCAPE, color, message));
	}
}

#define EC		"\x19"

// Escape % and ", so they can be sent along properly
static const char *ChatBox_UnescapeChat(const char *message) {
	static char buff[1024] = {0};
	char *s, *t;
	char *cutoff = &buff[1023];
	s = &buff[0];
	t = (char *)message;
	while (*t && s != cutoff) {
		/*if (*t == 0x18) {
			*s = '%';
		} else*/ if (*t == 0x17) {
			*s = '"';
		} else {
			*s = *t;
		}
		t++; s++;
	}
	*s = 0;
	return &buff[0];
}

// Determine the level of fade to use for the chat message
int G_Say_GetFadeLevel(int distance, int range) {
	// Outer 20% of the range triggers the fade out
	// Minimal intensity is 15, (so text is still rendered)
	float cutoff = (float)range*0.75f;
	float cutoffrange = (float)range *0.25f;
	float cutoffarea;
	float fadeLevel;

	if (distance > range) {
		// Shouldn't happen, but if so, minimal level
		return 15;
	}
	if (distance < cutoff) {
		// Closer than cutoff, dont bother doin math, (s)he's well in range
		return 100;
	}
	// Hmk, math time! >:O!
	cutoffarea = distance - cutoff;
	fadeLevel = 1 - (cutoffarea/cutoffrange);
	if (fadeLevel < 0.15f) {
		fadeLevel = 0.15f;
	} else if (fadeLevel > 1) {
		fadeLevel = 1;		// Failsafe, this should never happen
	}

	return fadeLevel*100;

}

extern vmCvar_t		jkg_chatFloodProtect;

qboolean CCmd_Execute(int clientNum, const char *command);
void G_Say( gentity_t *ent, gentity_t *target, int mode, const char *chatText ) {
	int			j;
	gentity_t	*other;
	int			color;
	char		name[64];
	// don't let text be too long for malicious reasons
	char		text[MAX_SAY_TEXT];
	char		location[64];
	char		*locMsg = NULL;

	int			normalRadius = 0; // The radius of this message
	int			pvsRadius = 0; // The radius of this message if we go through pvs
	vec3_t		tempVec;

	int			fadeLevel = 100;

	if (!chatText || !chatText[0]) {
		return;
	}

	// Check for validity of targets
	if (!ent || !ent->client || (target && !target->client)) {
		return;
	}

	// Check for flood
	// If the target is the same as the sender, then we're sending a message to ourselves (because
	// of private chat).
	if (ent != target && (ent->client->lastChatMessage > (level.time - jkg_chatFloodProtect.integer))) {
		return;
	}

	ent->client->lastChatMessage = level.time;

	
	if ( g_gametype.integer < GT_TEAM && mode == SAY_TEAM ) {
		mode = SAY_YELL;
	}

	if (strlen(chatText) > 511) {
		// Malicious use, ignore
		trap_SendServerCommand( ent-g_entities, "chat 100 \"^7System: ^1Malicious use of chat detected, message blocked\"");
		return;
	}

	// First, filter out any unauthorized characters (newlines)
	{
		char *s = (char *)chatText;
		while (*s) {
			if (*s == '\n' || *s == '\r') {
				*s = ' ';
			}
			s++;
		}
	}




	if (chatText[0] == '/' || chatText[0] == '\\') {
		// Command, special treatment
		if (!CCmd_Execute(ent-g_entities, &chatText[1])) {
			trap_SendServerCommand( ent-g_entities, "chat 100 \"^7System: ^1Unknown chat command\"");
		}
		return;
	}

	if (ent->client->pers.silenced) {
		// Player is silenced, ignore all his/her chat messages
		return;
	}

	// Check if Lua wants to block this message
	if (GLua_Hook_PlayerSay(ent, target, mode, chatText))
		return;

	switch ( mode ) {
	default:
	case SAY_ALL:		// Local area chat
		G_LogPrintf( "say: %s: %s\n", ent->client->pers.netname, ChatBox_UnescapeChat(chatText) );
		Com_sprintf (name, sizeof(name), "%s%c%c"EC": ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
		color = COLOR_GREEN;
		normalRadius = 1280;
		pvsRadius = 0;		// Cant go through pvs
		break;
	case SAY_TEAM:
		G_LogPrintf( "sayteam: %s: %s\n", ent->client->pers.netname, ChatBox_UnescapeChat(chatText) );
		if (Team_GetLocationMsg(ent, location, sizeof(location)))
		{
			Com_sprintf (name, sizeof(name), EC"(%s%c%c"EC")"EC": ", 
				ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
			locMsg = location;
		}
		else
		{
			Com_sprintf (name, sizeof(name), EC"(%s%c%c"EC")"EC": ", 
				ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
		}
		color = COLOR_CYAN;
		break;
	case SAY_TELL:
		if (target && g_gametype.integer >= GT_TEAM &&
			target->client->sess.sessionTeam == ent->client->sess.sessionTeam &&
			Team_GetLocationMsg(ent, location, sizeof(location)))
		{
			Com_sprintf (name, sizeof(name), EC"[%s%c%c"EC"]"EC": ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
			locMsg = location;
		}
		else
		{
			Com_sprintf (name, sizeof(name), EC"[%s%c%c"EC"]"EC": ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
		}
		color = COLOR_MAGENTA;
		break;
	case SAY_ACT:
		G_LogPrintf( "sayact: %s: %s\n", ent->client->pers.netname, ChatBox_UnescapeChat(chatText) );
		Com_sprintf (name, sizeof(name), "*%s%c%c"EC" ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
		normalRadius = 1280;
		pvsRadius = 0;		// Cant go through pvs
		color = COLOR_WHITE;
		break;
	case SAY_GLOBAL:
		G_LogPrintf( "sayglobal: %s: %s\n", ent->client->pers.netname, ChatBox_UnescapeChat(chatText) );
		Com_sprintf (name, sizeof(name), "^7[^5Global^7] %s%c%c"EC": ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
		color = COLOR_GREEN;
		break;
	case SAY_YELL:
		G_LogPrintf( "sayyell: %s: %s\n", ent->client->pers.netname, ChatBox_UnescapeChat(chatText) );
		Com_sprintf (name, sizeof(name), "^7[^3YELL^7] %s%c%c"EC": ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
		color = COLOR_YELLOW;
		normalRadius = 3500;
		pvsRadius = 2000;
		break;
	case SAY_WHISPER:
		G_LogPrintf( "saywhisper: %s: %s\n", ent->client->pers.netname, ChatBox_UnescapeChat(chatText) );
		Com_sprintf (name, sizeof(name), "^7[^5Whisper^7] %s%c%c"EC": ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
		color = COLOR_CYAN;
		normalRadius = 150;
		pvsRadius = 0;
		break;
	}

	Q_strncpyz( text, chatText, sizeof(text) );

	if ( target ) {
		G_SayTo( ent, target, mode, color, name, text, locMsg, fadeLevel );
		return;
	}

	// echo the text to the console
	if ( g_dedicated.integer ) {
		G_Printf( "%s%s\n", name, text);
	}

	// send it to all the apropriate clients
	// First check if we got a radius or not
	if (!pvsRadius && !normalRadius) {
		// No radius on either, send to all
		for (j = 0; j < level.maxclients; j++) {
			other = &g_entities[j];
			G_SayTo( ent, other, mode, color, name, text, locMsg, fadeLevel );
		}
	} else {
		// We got a radius here, so filter
		for (j = 0; j < level.maxclients; j++) {
			other = &g_entities[j];
			if (!other->inuse) {
				continue;
			}
			if (!other->client) {
				continue;
			}
			if ( other->client->pers.connected != CON_CONNECTED ) {
				continue;
			}
			if ( other->client->tempSpectate || other->client->sess.sessionTeam == TEAM_SPECTATOR  ) {
				// spectating folks cant see local area chat
				continue;
			}
			if (!trap_InPVS(ent->client->ps.origin, other->client->ps.origin)) {
				// Not in PVS
				if (!pvsRadius) {
					continue;
				} else {
					VectorSubtract(ent->client->ps.origin, other->client->ps.origin, tempVec);
					if (VectorLength(tempVec) > pvsRadius) {
						continue;
					}
					fadeLevel = G_Say_GetFadeLevel(VectorLength(tempVec), pvsRadius);
				}
			} else {
				// In PVS
				VectorSubtract(ent->client->ps.origin, other->client->ps.origin, tempVec);
				if (VectorLength(tempVec) > normalRadius) {
					continue;
				}
				fadeLevel = G_Say_GetFadeLevel(VectorLength(tempVec), normalRadius);
			}
			G_SayTo( ent, other, mode, color, name, text, locMsg, fadeLevel );
		}
	}
}


/*
==================
Cmd_Say_f
==================
*/
static void Cmd_Say_f( gentity_t *ent, int mode, qboolean arg0 ) {
	char		*p;

	if ( trap_Argc () < 2 && !arg0 ) {
		return;
	}

	if (arg0)
	{
		p = ConcatArgs( 0 );
	}
	else
	{
		p = ConcatArgs( 1 );
	}

	G_Say( ent, NULL, mode, p );
}

/*
==================
Cmd_SayAct_f - Actions
==================
*/
static void Cmd_SayAct_f( gentity_t *ent ) {
	char		*p;

	if ( trap_Argc () < 2 ) {
		return;
	}

	p = ConcatArgs( 1 );

	G_Say( ent, NULL, SAY_ACT, p );
}

/*
==================
Cmd_SayGlobal_f - Server-wide
==================
*/
static void Cmd_SayGlobal_f( gentity_t *ent ) {
	char		*p;

	if ( trap_Argc () < 2 ) {
		return;
	}

	p = ConcatArgs( 1 );

	G_Say( ent, NULL, SAY_GLOBAL, p );
}

/*
==================
Cmd_SayYell_f - Yell
==================
*/
static void Cmd_SayYell_f( gentity_t *ent ) {
	char		*p;

	if ( trap_Argc () < 2 ) {
		return;
	}

	p = ConcatArgs( 1 );

	G_Say( ent, NULL, SAY_YELL, p );
}

/*
==================
Cmd_SayTeam_f - Team Chat
VERSUS ONLY
==================
*/
static void Cmd_SayTeam_f( gentity_t *ent ) {
	char		*p;

	if ( trap_Argc () < 2 ) {
		return;
	}

	p = ConcatArgs( 1 );

	G_Say( ent, NULL, SAY_TEAM, p );
}

/*
==================
Cmd_SayWhisper_f - Yell
==================
*/
static void Cmd_SayWhisper_f( gentity_t *ent ) {
	char		*p;

	if ( trap_Argc () < 2 ) {
		return;
	}

	p = ConcatArgs( 1 );

	G_Say( ent, NULL, SAY_WHISPER, p );
}

/*
==================
Cmd_Tell_f
==================
*/

static void Cmd_Tell_f( gentity_t *ent ) {
	int			targetNum;
	gentity_t	*target;
	char		*p;
	char		arg[MAX_TOKEN_CHARS];

	if ( trap_Argc () < 2 ) {
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	targetNum = G_ClientNumberFromArg( arg );
	if ( targetNum < 0 || targetNum >= level.maxclients ) {
		return;
	}

	target = &g_entities[targetNum];
	if ( !target || !target->inuse || !target->client ) {
		return;
	}

	p = ConcatArgs( 2 );

	if (strlen(p) > 511) {
		// Malicious use, ignore
		trap_SendServerCommand( ent-g_entities, "chat 100 \"^7System: ^1Malicious use of chat detected, message blocked\"");
		return;
	}

	G_LogPrintf( "tell: %s to %s: %s\n", ent->client->pers.netname, target->client->pers.netname, p );
	G_Say( ent, target, SAY_TELL, p );
	// don't tell to the player self if it was already directed to this player
	// also don't send the chat back to a bot
	if ( ent != target && !(ent->r.svFlags & SVF_BOT)) {
		G_Say( ent, ent, SAY_TELL, p );
	}
}

//siege voice command
static void Cmd_VoiceCommand_f(gentity_t *ent)
{
	gentity_t *te;
	char arg[MAX_TOKEN_CHARS];
	char *s;
	int i = 0;

	if (g_gametype.integer < GT_TEAM)
	{
		return;
	}

	if (trap_Argc() < 2)
	{
		return;
	}

	if (ent->client->sess.sessionTeam == TEAM_SPECTATOR ||
		ent->client->tempSpectate >= level.time)
	{
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOVOICECHATASSPEC")) );
		return;
	}

	trap_Argv(1, arg, sizeof(arg));

	if (arg[0] == '*')
	{ //hmm.. don't expect a * to be prepended already. maybe someone is trying to be sneaky.
		return;
	}

	s = va("*%s", arg);

	//now, make sure it's a valid sound to be playing like this.. so people can't go around
	//screaming out death sounds or whatever.
	while (i < MAX_CUSTOM_SIEGE_SOUNDS)
	{
		if (!bg_customSiegeSoundNames[i])
		{
			break;
		}
		if (!Q_stricmp(bg_customSiegeSoundNames[i], s))
		{ //it matches this one, so it's ok
			break;
		}
		i++;
	}

	if (i == MAX_CUSTOM_SIEGE_SOUNDS || !bg_customSiegeSoundNames[i])
	{ //didn't find it in the list
		return;
	}

	te = G_TempEntity(vec3_origin, EV_VOICECMD_SOUND);
	te->s.groundEntityNum = ent->s.number;
	te->s.eventParm = G_SoundIndex((char *)bg_customSiegeSoundNames[i]);
	te->r.svFlags |= SVF_BROADCAST;
}


static char	*gc_orders[] = {
	"hold your position",
	"hold this position",
	"come here",
	"cover me",
	"guard location",
	"search and destroy",
	"report"
};

void Cmd_GameCommand_f( gentity_t *ent ) {
	int		player;
	int		order;
	char	str[MAX_TOKEN_CHARS];

	trap_Argv( 1, str, sizeof( str ) );
	player = atoi( str );
	trap_Argv( 2, str, sizeof( str ) );
	order = atoi( str );

	if ( player < 0 || player >= MAX_CLIENTS ) {
		return;
	}
	if ( order < 0 || order > sizeof(gc_orders)/sizeof(char *) ) {
		return;
	}
	G_Say( ent, &g_entities[player], SAY_TELL, gc_orders[order] );
	G_Say( ent, ent, SAY_TELL, gc_orders[order] );
}

/*
==================
Cmd_Where_f
==================
*/
void Cmd_Where_f( gentity_t *ent ) {
	if(ent->client && ent->client->sess.sessionTeam != TEAM_SPECTATOR )
	{//active players use currentOrigin
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", vtos( ent->r.currentOrigin ) ) );
	}
	else
	{
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", vtos( ent->s.origin ) ) );
	}
}

static const char *gameNames[] = {
	"Free For All",
	"Holocron FFA",
	"Jedi Master",
	"Duel",
	"Power Duel",
	"Single Player",
	"Team FFA",
	"Siege",
	"Capture the Flag",
	"Capture the Ysalamiri",
	"Warzone",
};

/*
==================
G_ClientNumberFromName

Finds the client number of the client with the given name
==================
*/
int G_ClientNumberFromName ( const char* name )
{
	char		s2[MAX_STRING_CHARS];
	char		n2[MAX_STRING_CHARS];
	int			i;
	gclient_t*	cl;

	// check for a name match
	SanitizeString( (char*)name, s2 );
	for ( i=0 ; i < level.numConnectedClients ; i++ ) 
	{
		cl = &level.clients[level.sortedClients[i]];
		SanitizeString( cl->pers.netname, n2 );
		if ( !strcmp( n2, s2 ) ) 
		{
			return level.sortedClients[i];
		}
	}

	return -1;
}

/*
==================
SanitizeString2

Rich's revised version of SanitizeString
==================
*/
void SanitizeString2( char *in, char *out )
{
	int i = 0;
	int r = 0;

	while (in[i])
	{
		if (i >= MAX_NAME_LENGTH-1)
		{ //the ui truncates the name here..
			break;
		}

		if (in[i] == '^')
		{
			if (in[i+1] >= 48 && //'0'
				in[i+1] <= 57) //'9'
			{ //only skip it if there's a number after it for the color
				i += 2;
				continue;
			}
			else
			{ //just skip the ^
				i++;
				continue;
			}
		}

		if (in[i] < 32)
		{
			i++;
			continue;
		}

		out[r] = in[i];
		r++;
		i++;
	}
	out[r] = 0;
}

/*
==================
G_ClientNumberFromStrippedName

Same as above, but strips special characters out of the names before comparing.
Jedi Knight Galaxies - Fixed the code to return the correct client ID
==================
*/
int G_ClientNumberFromStrippedName ( const char* name )
{
	char		s2[MAX_STRING_CHARS];
	char		n2[MAX_STRING_CHARS];
	int			i;
	gclient_t*	cl;

	// check for a name match
	SanitizeString2( (char*)name, s2 );
	Q_strlwr(s2);
	for ( i=0; i < level.numConnectedClients ; i++ ) 
	{
		cl = &level.clients[level.sortedClients[i]];
		SanitizeString2( cl->pers.netname, n2 );
		Q_strlwr(n2);
		if ( !strcmp( n2, s2 ) ) 
		{
			return level.sortedClients[i];
		}
	}

	return -1;
}

/*
==================
G_ClientNumberFromStrippedSubstring

Same as above, but strips special characters out of the names before comparing.
Checks substrings rather than the full string and returns -2 on multiple matches (if specified to do so)
==================
*/

int G_ClientNumberFromStrippedSubstring ( const char* name, qboolean checkAll )
{
	char		s2[MAX_STRING_CHARS];
	char		n2[MAX_STRING_CHARS];
	int			i, match = -1;
	gclient_t	*cl;

	// check for a name match
	SanitizeString2( (char*)name, s2 );
	Q_strlwr(s2);
	for ( i=0 ; i < level.numConnectedClients ; i++ ) 
	{
		cl = &level.clients[level.sortedClients[i]];
		SanitizeString2( cl->pers.netname, n2 );
		Q_strlwr(n2);
		if ( strstr( n2, s2 ) ) 
		{
			if( match != -1 )
			{ //found more than one match
				return -2;
			}
			match = level.sortedClients[i];
			if (!checkAll) {
				// Don't continue checking
				return match;
			}
		}
	}

	return match;
}

#ifdef __SECONDARY_NETWORK__
extern void jkg_netserverbegin();
extern void jkg_netservershutdown();
#endif //__SECONDARY_NETWORK__

/*
==================
Cmd_CallVote_f
==================
*/
const char *G_GetArenaInfoByMap( const char *map );
void Cmd_CallVote_f( gentity_t *ent ) {
	int		i;
	/* BUGFIX: Limit the buffer size, as 1024 is too big to fit in cvars */
	char	arg1[96];
	char	arg2[96];
//	int		n = 0;
//	char*	type = NULL;
	char*		mapName = 0;
	const char*	arenaInfo;

	/*if ( !g_allowVote.integer )*/ {
		// NO voting in JKG --eez
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOVOTE")) );
		return;
	}

	if ( level.voteTime || level.voteExecuteTime >= level.time ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "VOTEINPROGRESS")) );
		return;
	}
	if ( ent->client->pers.voteCount >= MAX_VOTE_COUNT ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "MAXVOTES")) );
		return;
	}

	if (g_gametype.integer != GT_DUEL &&
		g_gametype.integer != GT_POWERDUEL)
	{
		if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
			trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOSPECVOTE")) );
			return;
		}
	}

	// make sure it is a valid command to vote on
	trap_Argv( 1, arg1, sizeof( arg1 ) );
	trap_Argv( 2, arg2, sizeof( arg2 ) );

	if( strchr( arg1, ';' ) || strchr( arg2, ';' ) ) {
		trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
		return;
	}
	if( strchr( arg1, '\n' ) || strchr( arg1, '\r' ) || strchr( arg2, '\n' ) || strchr( arg2, '\r' )) {
		// Only a hack can put those in there, so this is an exploit attempt
		trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
		return;
	}

	if ( !Q_stricmp( arg1, "map_restart" ) ) {
	} else if ( !Q_stricmp( arg1, "nextmap" ) ) {
	} else if ( !Q_stricmp( arg1, "map" ) ) {
	} else if ( !Q_stricmp( arg1, "g_gametype" ) ) {
	} else if ( !Q_stricmp( arg1, "kick" ) ) {
	} else if ( !Q_stricmp( arg1, "clientkick" ) ) {
	} else if ( !Q_stricmp( arg1, "g_doWarmup" ) ) {
	} else if ( !Q_stricmp( arg1, "timelimit" ) ) {
	} else if ( !Q_stricmp( arg1, "fraglimit" ) ) {
	} else {
		trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
		trap_SendServerCommand( ent-g_entities, "print \"Vote commands are: map_restart, nextmap, map <mapname>, g_gametype <n>, kick <player>, clientkick <clientnum>, g_doWarmup, timelimit <time>, fraglimit <frags>.\n\"" );
		return;
	}

	// if there is still a vote to be executed
	if ( level.voteExecuteTime ) {
		level.voteExecuteTime = 0;
		trap_SendConsoleCommand( EXEC_APPEND, va("%s\n", level.voteString ) );
	}

	// special case for g_gametype, check for bad values
	if ( !Q_stricmp( arg1, "g_gametype" ) )
	{
		i = atoi( arg2 );
		if( i == GT_SINGLE_PLAYER || i < GT_FFA || i >= GT_MAX_GAME_TYPE) {
			trap_SendServerCommand( ent-g_entities, "print \"Invalid gametype.\n\"" );
			return;
		}

		level.votingGametype = qtrue;
		level.votingGametypeTo = i;

		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %d", arg1, i );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s %s", arg1, gameNames[i] );
	}
	else if ( !Q_stricmp( arg1, "map" ) ) 
	{
		// special case for map changes, we want to reset the nextmap setting
		// this allows a player to change maps, but not upset the map rotation
		char	s[MAX_STRING_CHARS];

		if (!G_DoesMapSupportGametype(arg2, trap_Cvar_VariableIntegerValue("g_gametype")))
		{
			//trap_SendServerCommand( ent-g_entities, "print \"You can't vote for this map, it isn't supported by the current gametype.\n\"" );
			trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOVOTE_MAPNOTSUPPORTEDBYGAME")) );
			return;
		}

#ifdef __SECONDARY_NETWORK__
		jkg_netservershutdown();
#endif //__SECONDARY_NETWORK__

		trap_Cvar_VariableStringBuffer( "nextmap", s, sizeof(s) );
		if (*s) {
			Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %s; set nextmap \"%s\"", arg1, arg2, s );
		} else {
			Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %s", arg1, arg2 );
		}
		
		arenaInfo	= G_GetArenaInfoByMap(arg2);
		if (arenaInfo)
		{
			mapName = Info_ValueForKey(arenaInfo, "longname");
		}

		if (!mapName || !mapName[0])
		{
			mapName = "ERROR";
		}

		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "map %s", mapName);
	}
	else if ( !Q_stricmp ( arg1, "clientkick" ) )
	{
		int n = atoi ( arg2 );

		if ( n < 0 || n >= MAX_CLIENTS )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"invalid client number %d.\n\"", n ) );
			return;
		}

		if ( g_entities[n].client->pers.connected == CON_DISCONNECTED )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"there is no client with the client number %d.\n\"", n ) );
			return;
		}
			
		Com_sprintf ( level.voteString, sizeof(level.voteString ), "%s %s", arg1, arg2 );
		Com_sprintf ( level.voteDisplayString, sizeof(level.voteDisplayString), "kick %s", g_entities[n].client->pers.netname );
	}
	else if ( !Q_stricmp ( arg1, "kick" ) )
	{
		int clientid = G_ClientNumberFromName ( arg2 );

		if ( clientid == -1 )
		{
			clientid = G_ClientNumberFromStrippedName(arg2);

			if (clientid == -1)
			{
				trap_SendServerCommand( ent-g_entities, va("print \"there is no client named '%s' currently on the server.\n\"", arg2 ) );
				return;
			}
		}

		Com_sprintf ( level.voteString, sizeof(level.voteString ), "clientkick %d", clientid );
		Com_sprintf ( level.voteDisplayString, sizeof(level.voteDisplayString), "kick %s", g_entities[clientid].client->pers.netname );
	}
	else if ( !Q_stricmp( arg1, "nextmap" ) ) 
	{
		char	s[MAX_STRING_CHARS];

		trap_Cvar_VariableStringBuffer( "nextmap", s, sizeof(s) );
		if (!*s) {
			trap_SendServerCommand( ent-g_entities, "print \"nextmap not set.\n\"" );
			return;
		}
		Com_sprintf( level.voteString, sizeof( level.voteString ), "vstr nextmap");
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", level.voteString );
	} 
	else
	{
		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s \"%s\"", arg1, arg2 );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", level.voteString );
	}

	trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", ent->client->pers.netname, G_GetStringEdString("MP_SVGAME", "PLCALLEDVOTE") ) );

	// start the voting, the caller autoamtically votes yes
	level.voteTime = level.time;
	level.voteYes = 1;
	level.voteNo = 0;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		level.clients[i].mGameFlags &= ~PSG_VOTED;
	}
	ent->client->mGameFlags |= PSG_VOTED;

	trap_SetConfigstring( CS_VOTE_TIME, va("%i", level.voteTime ) );
	trap_SetConfigstring( CS_VOTE_STRING, level.voteDisplayString );	
	trap_SetConfigstring( CS_VOTE_YES, va("%i", level.voteYes ) );
	trap_SetConfigstring( CS_VOTE_NO, va("%i", level.voteNo ) );	
}

/*
==================
Cmd_Vote_f
==================
*/
void Cmd_Vote_f( gentity_t *ent ) {
	char		msg[64];

	if ( !level.voteTime ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOVOTEINPROG")) );
		return;
	}
	if ( ent->client->mGameFlags & PSG_VOTED ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "VOTEALREADY")) );
		return;
	}
	if (g_gametype.integer != GT_DUEL &&
		g_gametype.integer != GT_POWERDUEL)
	{
		if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
			trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOVOTEASSPEC")) );
			return;
		}
	}

	trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "PLVOTECAST")) );

	ent->client->mGameFlags |= PSG_VOTED;

	trap_Argv( 1, msg, sizeof( msg ) );

	if ( msg[0] == 'y' || msg[1] == 'Y' || msg[1] == '1' ) {
		level.voteYes++;
		trap_SetConfigstring( CS_VOTE_YES, va("%i", level.voteYes ) );
	} else {
		level.voteNo++;
		trap_SetConfigstring( CS_VOTE_NO, va("%i", level.voteNo ) );	
	}

	// a majority will be determined in CheckVote, which will also account
	// for players entering or leaving
}

/*
==================
Cmd_CallTeamVote_f
==================
*/
void Cmd_CallTeamVote_f( gentity_t *ent ) {
	int		i, team, cs_offset;
	char	arg1[MAX_STRING_TOKENS];
	char	arg2[MAX_STRING_TOKENS];

	team = ent->client->sess.sessionTeam;
	if ( team == TEAM_RED )
		cs_offset = 0;
	else if ( team == TEAM_BLUE )
		cs_offset = 1;
	else
		return;

	if ( !g_allowVote.integer ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOVOTE")) );
		return;
	}

	if ( level.teamVoteTime[cs_offset] ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "TEAMVOTEALREADY")) );
		return;
	}
	if ( ent->client->pers.teamVoteCount >= MAX_VOTE_COUNT ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "MAXTEAMVOTES")) );
		return;
	}
	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOSPECVOTE")) );
		return;
	}

	// make sure it is a valid command to vote on
	trap_Argv( 1, arg1, sizeof( arg1 ) );
	arg2[0] = '\0';
	for ( i = 2; i < trap_Argc(); i++ ) {
		if (i > 2)
			strcat(arg2, " ");
		trap_Argv( i, &arg2[strlen(arg2)], sizeof( arg2 ) - strlen(arg2) );
	}

	if( strchr( arg1, ';' ) || strchr( arg2, ';' ) ) {
		trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
		return;
	}

	if ( !Q_stricmp( arg1, "leader" ) ) {
		char netname[MAX_NETNAME], leader[MAX_NETNAME];

		if ( !arg2[0] ) {
			i = ent->client->ps.clientNum;
		}
		else {
			// numeric values are just slot numbers
			for (i = 0; i < 3; i++) {
				if ( !arg2[i] || arg2[i] < '0' || arg2[i] > '9' )
					break;
			}
			if ( i >= 3 || !arg2[i]) {
				i = atoi( arg2 );
				if ( i < 0 || i >= level.maxclients ) {
					trap_SendServerCommand( ent-g_entities, va("print \"Bad client slot: %i\n\"", i) );
					return;
				}

				if ( !g_entities[i].inuse ) {
					trap_SendServerCommand( ent-g_entities, va("print \"Client %i is not active\n\"", i) );
					return;
				}
			}
			else {
				Q_strncpyz(leader, arg2, sizeof(leader));
				Q_CleanStr(leader);
				for ( i = 0 ; i < level.maxclients ; i++ ) {
					if ( level.clients[i].pers.connected == CON_DISCONNECTED )
						continue;
					if (level.clients[i].sess.sessionTeam != team)
						continue;
					Q_strncpyz(netname, level.clients[i].pers.netname, sizeof(netname));
					Q_CleanStr(netname);
					if ( !Q_stricmp(netname, leader) ) {
						break;
					}
				}
				if ( i >= level.maxclients ) {
					trap_SendServerCommand( ent-g_entities, va("print \"%s is not a valid player on your team.\n\"", arg2) );
					return;
				}
			}
		}
		Com_sprintf(arg2, sizeof(arg2), "%d", i);
	} else {
		trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
		trap_SendServerCommand( ent-g_entities, "print \"Team vote commands are: leader <player>.\n\"" );
		return;
	}

	Com_sprintf( level.teamVoteString[cs_offset], sizeof( level.teamVoteString[cs_offset] ), "%s %s", arg1, arg2 );

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( level.clients[i].pers.connected == CON_DISCONNECTED )
			continue;
		if (level.clients[i].sess.sessionTeam == team)
			trap_SendServerCommand( i, va("print \"%s called a team vote.\n\"", ent->client->pers.netname ) );
	}

	// start the voting, the caller autoamtically votes yes
	level.teamVoteTime[cs_offset] = level.time;
	level.teamVoteYes[cs_offset] = 1;
	level.teamVoteNo[cs_offset] = 0;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if (level.clients[i].sess.sessionTeam == team)
			level.clients[i].mGameFlags &= ~PSG_TEAMVOTED;
	}
	ent->client->mGameFlags |= PSG_TEAMVOTED;

	trap_SetConfigstring( CS_TEAMVOTE_TIME + cs_offset, va("%i", level.teamVoteTime[cs_offset] ) );
	trap_SetConfigstring( CS_TEAMVOTE_STRING + cs_offset, level.teamVoteString[cs_offset] );
	trap_SetConfigstring( CS_TEAMVOTE_YES + cs_offset, va("%i", level.teamVoteYes[cs_offset] ) );
	trap_SetConfigstring( CS_TEAMVOTE_NO + cs_offset, va("%i", level.teamVoteNo[cs_offset] ) );
}

/*
==================
Cmd_TeamVote_f
==================
*/
void Cmd_TeamVote_f( gentity_t *ent ) {
	int			team, cs_offset;
	char		msg[64];

	team = ent->client->sess.sessionTeam;
	if ( team == TEAM_RED )
		cs_offset = 0;
	else if ( team == TEAM_BLUE )
		cs_offset = 1;
	else
		return;

	if ( !level.teamVoteTime[cs_offset] ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOTEAMVOTEINPROG")) );
		return;
	}
	if ( ent->client->mGameFlags & PSG_TEAMVOTED ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "TEAMVOTEALREADYCAST")) );
		return;
	}
	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOVOTEASSPEC")) );
		return;
	}

	trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "PLTEAMVOTECAST")) );

	ent->client->mGameFlags |= PSG_TEAMVOTED;

	trap_Argv( 1, msg, sizeof( msg ) );

	if ( msg[0] == 'y' || msg[1] == 'Y' || msg[1] == '1' ) {
		level.teamVoteYes[cs_offset]++;
		trap_SetConfigstring( CS_TEAMVOTE_YES + cs_offset, va("%i", level.teamVoteYes[cs_offset] ) );
	} else {
		level.teamVoteNo[cs_offset]++;
		trap_SetConfigstring( CS_TEAMVOTE_NO + cs_offset, va("%i", level.teamVoteNo[cs_offset] ) );	
	}

	// a majority will be determined in TeamCheckVote, which will also account
	// for players entering or leaving
}

/*
=================
Cmd_Reload_f
=================
*/
qboolean PM_InKnockDown( playerState_t *ps );
void Cmd_Reload_f( gentity_t *ent ) {
	int weapon;
	int ammotoadd;
	int weaponIndex;
	gentity_t *evt;

	weapon = ent->client->ps.weapon;

    #ifdef _DEBUG
    //trap_SendServerCommand (-1, va ("print \"Weapon %s/%d has clip size of %d.\n\"", WPTable[weapon + 1], ent->client->ps.weaponVariation, GetWeaponAmmoClip(weapon, ent->client->ps.weaponVariation)));
    #endif
	if ( GetWeaponAmmoClip( weapon, ent->client->ps.weaponVariation ) == -1 )
	{
		// Current weapon does not use a clip, bail
		return;
	}

	if (ent->client->ps.weaponTime > 0 && ent->client->ps.weaponstate != WEAPON_READY) {
		// Weapon is busy, cannot reload at this moment
		return;
	}

	if ( ent->client->ps.forceHandExtend != HANDEXTEND_NONE && !PM_InKnockDown (&ent->client->ps) )
	{//We're in a knockdown, or something ugly like that...DO NOT WANT!
		return;
	}
	
	/* No more ammo left to place in the clip */
	if ( ent->client->ps.ammo <= 0 )
	{
		return;
	}
	
	// Can't reload while sprinting
	if ( BG_IsSprinting (&ent->client->ps, &ent->client->pers.cmd, &ent->client->ns) )
	{
	    return;
	}
	
    // Can't reload while charging the weapon
	if ( ent->client->ps.weaponstate == WEAPON_CHARGING || ent->client->ps.weaponstate == WEAPON_CHARGING_ALT )
	{
		return;
	}

	ammotoadd = GetWeaponAmmoClip( weapon, ent->client->ps.weaponVariation );
	weaponIndex = BG_GetWeaponIndex(weapon, ent->client->ps.weaponVariation);
	
	if (ent->client->clipammo[weaponIndex] >= ammotoadd)	{
		// Current clip is full
		return;
	}
	
	ammotoadd -= ent->client->clipammo[weaponIndex];

    #ifdef _DEBUG
    //trap_SendServerCommand (-1, va ("print \"Reloading weapon %s.\n\"", WPTable[weapon + 1]));
    #endif
    
	evt						= G_TempEntity( ent->s.pos.trBase, EV_RELOAD );
	evt->s.time				= ent->s.number;
	evt->s.eventParm		= weapon;
	evt->s.weaponVariation	= ent->client->ps.weaponVariation;

	ent->client->ps.weaponstate = WEAPON_RELOADING;
	
	// We're in a zoom and start a reload, remove the zoom!
	if ( ent->client->ps.zoomMode )
	{
		ent->client->ps.zoomMode = 0;
	}

	// TODO: Reload animation
	G_SetAnim(ent, NULL, SETANIM_TORSO, GetWeaponData (weapon, ent->client->ps.weaponVariation)->anims.reload.torsoAnim, SETANIM_FLAG_OVERRIDE,0);
	ent->client->ps.weaponTime = GetWeaponData( weapon, ent->client->ps.weaponVariation )->weaponReloadTime;
	// TODO: Add sound

	// Check if we have enough ammo remaining
	ammotoadd = min (ammotoadd, ent->client->ps.ammo);

	//Remove the ammo from 'bag'
	ent->client->ps.ammo -= ammotoadd;
	ent->client->ammoTable[GetWeaponAmmoIndex(ent->client->ps.weapon, ent->client->ps.weaponVariation)] -= ammotoadd;

	//Add the ammo to weapon
	ent->client->clipammo[weaponIndex] += ammotoadd;

	//Take us out of ironsights
	//ent->client->ns.ironsightsTime &= ~IRONSIGHTS_MSB;
	ent->client->ns.ironsightsDebounceStart = level.time + ent->client->ps.weaponTime;
}



/*
=================
Cmd_SetViewpos_f
=================
*/
void Cmd_SetViewpos_f( gentity_t *ent ) {
	vec3_t		origin, angles;
	char		buffer[MAX_TOKEN_CHARS];
	int			i;

	if ( !g_cheats.integer ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOCHEATS")));
		return;
	}
	if ( trap_Argc() != 5 ) {
		trap_SendServerCommand( ent-g_entities, va("print \"usage: setviewpos x y z yaw\n\""));
		return;
	}

	VectorClear( angles );
	for ( i = 0 ; i < 3 ; i++ ) {
		trap_Argv( i + 1, buffer, sizeof( buffer ) );
		origin[i] = atof( buffer );
	}

	trap_Argv( 4, buffer, sizeof( buffer ) );
	angles[YAW] = atof( buffer );

	TeleportPlayer( ent, origin, angles );
}



/*
=================
Cmd_Stats_f
=================
*/
void Cmd_Stats_f( gentity_t *ent ) {
/*
	int max, n, i;

	max = trap_AAS_PointReachabilityAreaIndex( NULL );

	n = 0;
	for ( i = 0; i < max; i++ ) {
		if ( ent->client->areabits[i >> 3] & (1 << (i & 7)) )
			n++;
	}

	//trap_SendServerCommand( ent-g_entities, va("print \"visited %d of %d areas\n\"", n, max));
	trap_SendServerCommand( ent-g_entities, va("print \"%d%% level coverage\n\"", n * 100 / max));
*/
}

int G_ItemUsable(playerState_t *ps, int forcedUse)
{
	vec3_t fwd, fwdorg, dest, pos;
	vec3_t yawonly;
	vec3_t mins, maxs;
	vec3_t trtest;
	trace_t tr;

	if (ps->m_iVehicleNum)
	{
		return 0;
	}
	
	if (ps->pm_flags & PMF_USE_ITEM_HELD)
	{ //force to let go first
		return 0;
	}

	if (!forcedUse)
	{
		forcedUse = bg_itemlist[ps->stats[STAT_HOLDABLE_ITEM]].giTag;
	}

	if (!BG_IsItemSelectable(ps, forcedUse))
	{
		return 0;
	}

	switch (forcedUse)
	{
	case HI_MEDPAC:
	case HI_MEDPAC_BIG:
		if (ps->stats[STAT_HEALTH] >= ps->stats[STAT_MAX_HEALTH])
		{
			return 0;
		}

		if (ps->stats[STAT_HEALTH] <= 0)
		{
			return 0;
		}

		return 1;
	case HI_SEEKER:
		if (ps->eFlags & EF_SEEKERDRONE)
		{
			G_AddEvent(&g_entities[ps->clientNum], EV_ITEMUSEFAIL, SEEKER_ALREADYDEPLOYED);
			return 0;
		}

		return 1;
	case HI_SENTRY_GUN:
		if (ps->fd.sentryDeployed)
		{
			G_AddEvent(&g_entities[ps->clientNum], EV_ITEMUSEFAIL, SENTRY_ALREADYPLACED);
			return 0;
		}

		yawonly[ROLL] = 0;
		yawonly[PITCH] = 0;
		yawonly[YAW] = ps->viewangles[YAW];

		VectorSet( mins, -8, -8, 0 );
		VectorSet( maxs, 8, 8, 24 );

		AngleVectors(yawonly, fwd, NULL, NULL);

		fwdorg[0] = ps->origin[0] + fwd[0]*64;
		fwdorg[1] = ps->origin[1] + fwd[1]*64;
		fwdorg[2] = ps->origin[2] + fwd[2]*64;

		trtest[0] = fwdorg[0] + fwd[0]*16;
		trtest[1] = fwdorg[1] + fwd[1]*16;
		trtest[2] = fwdorg[2] + fwd[2]*16;

		trap_Trace(&tr, ps->origin, mins, maxs, trtest, ps->clientNum, MASK_PLAYERSOLID);

		if ((tr.fraction != 1 && tr.entityNum != ps->clientNum) || tr.startsolid || tr.allsolid)
		{
			G_AddEvent(&g_entities[ps->clientNum], EV_ITEMUSEFAIL, SENTRY_NOROOM);
			return 0;
		}

		return 1;
	case HI_SHIELD:
		mins[0] = -8;
		mins[1] = -8;
		mins[2] = 0;

		maxs[0] = 8;
		maxs[1] = 8;
		maxs[2] = 8;

		AngleVectors (ps->viewangles, fwd, NULL, NULL);
		fwd[2] = 0;
		VectorMA(ps->origin, 64, fwd, dest);
		trap_Trace(&tr, ps->origin, mins, maxs, dest, ps->clientNum, MASK_SHOT );
		if (tr.fraction > 0.9f && !tr.startsolid && !tr.allsolid)
		{
			VectorCopy(tr.endpos, pos);
			VectorSet( dest, pos[0], pos[1], pos[2] - 4096 );
			trap_Trace( &tr, pos, mins, maxs, dest, ps->clientNum, MASK_SOLID );
			if ( !tr.startsolid && !tr.allsolid )
			{
				return 1;
			}
		}
		G_AddEvent(&g_entities[ps->clientNum], EV_ITEMUSEFAIL, SHIELD_NOROOM);
		return 0;
	case HI_JETPACK: //do something?
		return 1;
	case HI_HEALTHDISP:
		return 1;
	case HI_AMMODISP:
		return 1;
	case HI_EWEB:
		return 1;
	case HI_CLOAK:
		return 1;
	default:
		return 1;
	}
}

void saberKnockDown(gentity_t *saberent, gentity_t *saberOwner, gentity_t *other);

void Cmd_ToggleSaber_f(gentity_t *ent)
{
	if (ent->client->ps.fd.forceGripCripple)
	{ //if they are being gripped, don't let them unholster their saber
		if (ent->client->ps.saberHolstered)
		{
			return;
		}
	}

	if (ent->client->ps.saberInFlight)
	{
		if (ent->client->ps.saberEntityNum)
		{ //turn it off in midair
			saberKnockDown(&g_entities[ent->client->ps.saberEntityNum], ent, ent);
		}
		return;
	}

	if (ent->client->ps.forceHandExtend != HANDEXTEND_NONE)
	{
		return;
	}

	if (ent->client->ps.weapon != WP_SABER)
	{
		return;
	}

//	if (ent->client->ps.duelInProgress && !ent->client->ps.saberHolstered)
//	{
//		return;
//	}

	if (ent->client->ps.duelTime >= level.time)
	{
		return;
	}

	if (ent->client->ps.saberLockTime >= level.time)
	{
		return;
	}

	if (ent->client && ent->client->ps.weaponTime < 1)
	{
		if (ent->client->ps.saberHolstered == 2)
		{
			ent->client->ps.saberHolstered = 0;

			if (ent->client->saber[0].soundOn)
			{
				G_Sound(ent, CHAN_AUTO, ent->client->saber[0].soundOn);
			}
			if (ent->client->saber[1].soundOn)
			{
				G_Sound(ent, CHAN_AUTO, ent->client->saber[1].soundOn);
			}
		}
		else
		{
			ent->client->ps.saberHolstered = 2;
			if (ent->client->saber[0].soundOff)
			{
				G_Sound(ent, CHAN_AUTO, ent->client->saber[0].soundOff);
			}
			if (ent->client->saber[1].soundOff &&
				ent->client->saber[1].model[0])
			{
				G_Sound(ent, CHAN_AUTO, ent->client->saber[1].soundOff);
			}
			//prevent anything from being done for 400ms after holster
			ent->client->ps.weaponTime = 400;
		}
	}
}

extern vmCvar_t		d_saberStanceDebug;

extern qboolean WP_SaberCanTurnOffSomeBlades( saberInfo_t *saber );
void Cmd_SaberAttackCycle_f(gentity_t *ent)
{
	int selectLevel = 0;
	qboolean usingSiegeStyle = qfalse;
	
	if ( !ent || !ent->client )
	{
		return;
	}

	if( ent->client->saberStanceDebounce > level.time)
	{
		// No. Don't want spamming of firing mode change or stance change --eez
		return;
	}

	if( ent->client->ps.weaponTime > 0 )
	{
		// Don't use in the middle of a burst or in a saber move or while reloading --eez
		return;
	}
	/*
	if (ent->client->ps.weaponTime > 0)
	{ //no switching attack level when busy
		return;
	}
	*/

	// Jedi Knight Galaxies, we cant swich saber style if we're not using a saber...
	if (ent->client->ps.weapon != WP_SABER)
	{
		// But we can change firing modes!
		int previousFiringMode = ent->client->ps.firingMode;
		weaponData_t *wp = GetWeaponData( ent->client->ps.weapon, ent->client->ps.weaponVariation );
		ent->client->ps.firingMode++;
		if(ent->client->ps.firingMode >= wp->numFiringModes)
		{
			ent->client->ps.firingMode = 0;
		}
		ent->client->saberStanceDebounce = level.time + 400;	// changed to 400 cuz 1000 was way too slow
		trap_SendServerCommand(ent->s.number, va("fmrefresh %i", previousFiringMode));
		ent->client->firingModes[ent->client->ps.weaponId] = ent->client->ps.firingMode;
		return;
	}
	else
	{
		// Change saber style then, if permitted
		if( ent->client->ps.saberInFlight )
		{
			//Jedi Knight Galaxies, cannot switch saber style if throwing a saber
			return;
		}

		ent->client->saberStanceDebounce = level.time + 1000;

		if (ent->client->saber[0].model[0] && ent->client->saber[1].model[0])
		{ //no cycling for akimbo
			if ( WP_SaberCanTurnOffSomeBlades( &ent->client->saber[1] ) )
			{//can turn second saber off 
				if ( ent->client->ps.saberHolstered == 1 )
				{//have one holstered
					//unholster it
					G_Sound(ent, CHAN_AUTO, ent->client->saber[1].soundOn);
					ent->client->ps.saberHolstered = 0;
					//g_active should take care of this, but...
					ent->client->ps.fd.saberAnimLevel = SS_DUAL;
				}
				else if ( ent->client->ps.saberHolstered == 0 )
				{//have none holstered
					if ( (ent->client->saber[1].saberFlags2&SFL2_NO_MANUAL_DEACTIVATE) )
					{//can't turn it off manually
					}
					else if ( ent->client->saber[1].bladeStyle2Start > 0
						&& (ent->client->saber[1].saberFlags2&SFL2_NO_MANUAL_DEACTIVATE2) )
					{//can't turn it off manually
					}
					else
					{
						//turn it off
						G_Sound(ent, CHAN_AUTO, ent->client->saber[1].soundOff);
						ent->client->ps.saberHolstered = 1;
						//g_active should take care of this, but...
						ent->client->ps.fd.saberAnimLevel = SS_FAST;
					}
				}

				if (d_saberStanceDebug.integer)
				{
					trap_SendServerCommand( ent-g_entities, va("print \"SABERSTANCEDEBUG: Attempted to toggle dual saber blade.\n\"") );
				}
				return;
			}
		}
		else if (ent->client->saber[0].numBlades > 1
			&& WP_SaberCanTurnOffSomeBlades( &ent->client->saber[0] ) )
		{ //use staff stance then.
			if ( ent->client->ps.saberHolstered == 1 )
			{//second blade off
				if ( ent->client->ps.saberInFlight )
				{//can't turn second blade back on if it's in the air, you naughty boy!
					if (d_saberStanceDebug.integer)
					{
						trap_SendServerCommand( ent-g_entities, va("print \"SABERSTANCEDEBUG: Attempted to toggle staff blade in air.\n\"") );
					}
					return;
				}
				//turn it on
				G_Sound(ent, CHAN_AUTO, ent->client->saber[0].soundOn);
				ent->client->ps.saberHolstered = 0;
				//g_active should take care of this, but...
				if ( ent->client->saber[0].stylesForbidden )
				{//have a style we have to use
					WP_UseFirstValidSaberStyle( &ent->client->saber[0], &ent->client->saber[1], ent->client->ps.saberHolstered, &selectLevel );
					if ( ent->client->ps.weaponTime <= 0 )
					{ //not busy, set it now
						ent->client->ps.fd.saberAnimLevel = selectLevel;
					}
					else
					{ //can't set it now or we might cause unexpected chaining, so queue it
						ent->client->saberCycleQueue = selectLevel;
					}
				}
			}
			else if ( ent->client->ps.saberHolstered == 0 )
			{//both blades on
				if ( (ent->client->saber[0].saberFlags2&SFL2_NO_MANUAL_DEACTIVATE) )
				{//can't turn it off manually
				}
				else if ( ent->client->saber[0].bladeStyle2Start > 0
					&& (ent->client->saber[0].saberFlags2&SFL2_NO_MANUAL_DEACTIVATE2) )
				{//can't turn it off manually
				}
				else
				{
					//turn second one off
					G_Sound(ent, CHAN_AUTO, ent->client->saber[0].soundOff);
					ent->client->ps.saberHolstered = 1;
					//g_active should take care of this, but...
					if ( ent->client->saber[0].singleBladeStyle != SS_NONE )
					{
						if ( ent->client->ps.weaponTime <= 0 )
						{ //not busy, set it now
							ent->client->ps.fd.saberAnimLevel = ent->client->saber[0].singleBladeStyle;
						}
						else
						{ //can't set it now or we might cause unexpected chaining, so queue it
							ent->client->saberCycleQueue = ent->client->saber[0].singleBladeStyle;
						}
					}
				}
			}
			if (d_saberStanceDebug.integer)
			{
				trap_SendServerCommand( ent-g_entities, va("print \"SABERSTANCEDEBUG: Attempted to toggle staff blade.\n\"") );
			}
			return;
		}

		if (ent->client->saberCycleQueue)
		{ //resume off of the queue if we haven't gotten a chance to update it yet
			selectLevel = ent->client->saberCycleQueue;
		}
		else
		{
			selectLevel = ent->client->ps.fd.saberAnimLevel;
		}

		selectLevel++;
		if ( selectLevel > ent->client->ps.fd.forcePowerLevel[FP_SABER_OFFENSE] )
		{
			selectLevel = FORCE_LEVEL_1;
		}
		if (d_saberStanceDebug.integer)
		{
			trap_SendServerCommand( ent-g_entities, va("print \"SABERSTANCEDEBUG: Attempted to cycle stance normally.\n\"") );
		}
	/*
	#ifndef FINAL_BUILD
		switch ( selectLevel )
		{
		case FORCE_LEVEL_1:
			trap_SendServerCommand( ent-g_entities, va("print \"Lightsaber Combat Style: %sfast\n\"", S_COLOR_BLUE) );
			break;
		case FORCE_LEVEL_2:
			trap_SendServerCommand( ent-g_entities, va("print \"Lightsaber Combat Style: %smedium\n\"", S_COLOR_YELLOW) );
			break;
		case FORCE_LEVEL_3:
			trap_SendServerCommand( ent-g_entities, va("print \"Lightsaber Combat Style: %sstrong\n\"", S_COLOR_RED) );
			break;
		}
	#endif
	*/
		if ( !usingSiegeStyle )
		{
			//make sure it's valid, change it if not
			WP_UseFirstValidSaberStyle( &ent->client->saber[0], &ent->client->saber[1], ent->client->ps.saberHolstered, &selectLevel );
		}

		if (ent->client->ps.weaponTime <= 0)
		{ //not busy, set it now
			ent->client->ps.fd.saberAnimLevelBase = ent->client->ps.fd.saberAnimLevel = selectLevel;
		}
		else
		{ //can't set it now or we might cause unexpected chaining, so queue it
			ent->client->ps.fd.saberAnimLevelBase = ent->client->saberCycleQueue = selectLevel;
		}
	}
}

qboolean G_OtherPlayersDueling(void)
{
	int i = 0;
	gentity_t *ent;

	while (i < MAX_CLIENTS)
	{
		ent = &g_entities[i];

		if (ent && ent->inuse && ent->client && ent->client->ps.duelInProgress)
		{
			return qtrue;
		}
		i++;
	}

	return qfalse;
}

void Cmd_EngageDuel_f(gentity_t *ent)
{
	trace_t tr;
	vec3_t forward, fwdOrg;

	if (!g_privateDuel.integer)
	{
		return;
	}

	if (g_gametype.integer == GT_DUEL || g_gametype.integer == GT_POWERDUEL)
	{ //rather pointless in this mode..
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NODUEL_GAMETYPE")) );
		return;
	}

	//if (g_gametype.integer >= GT_TEAM && g_gametype.integer != GT_SIEGE)
	if (g_gametype.integer >= GT_TEAM)
	{ //no private dueling in team modes
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NODUEL_GAMETYPE")) );
		return;
	}

	if (ent->client->ps.duelTime >= level.time)
	{
		return;
	}

	if (ent->client->ps.weapon != WP_SABER)
	{
		return;
	}

	/*
	if (!ent->client->ps.saberHolstered)
	{ //must have saber holstered at the start of the duel
		return;
	}
	*/
	//NOTE: No longer doing this..

	if (ent->client->ps.saberInFlight)
	{
		return;
	}

	if (ent->client->ps.duelInProgress)
	{
		return;
	}

	//New: Don't let a player duel if he just did and hasn't waited 10 seconds yet (note: If someone challenges him, his duel timer will reset so he can accept)
	if (ent->client->ps.fd.privateDuelTime > level.time)
	{
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "CANTDUEL_JUSTDID")) );
		return;
	}

	if (G_OtherPlayersDueling())
	{
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "CANTDUEL_BUSY")) );
		return;
	}

	AngleVectors( ent->client->ps.viewangles, forward, NULL, NULL );

	fwdOrg[0] = ent->client->ps.origin[0] + forward[0]*256;
	fwdOrg[1] = ent->client->ps.origin[1] + forward[1]*256;
	fwdOrg[2] = (ent->client->ps.origin[2]+ent->client->ps.viewheight) + forward[2]*256;

	trap_Trace(&tr, ent->client->ps.origin, NULL, NULL, fwdOrg, ent->s.number, MASK_PLAYERSOLID);

	if (tr.fraction != 1 && tr.entityNum < MAX_CLIENTS)
	{
		gentity_t *challenged = &g_entities[tr.entityNum];

		if (!challenged || !challenged->client || !challenged->inuse ||
			challenged->health < 1 || challenged->client->ps.stats[STAT_HEALTH] < 1 ||
			challenged->client->ps.weapon != WP_SABER || challenged->client->ps.duelInProgress ||
			challenged->client->ps.saberInFlight)
		{
			return;
		}

		if (g_gametype.integer >= GT_TEAM && OnSameTeam(ent, challenged))
		{
			return;
		}

		if (challenged->client->ps.duelIndex == ent->s.number && challenged->client->ps.duelTime >= level.time)
		{
			trap_SendServerCommand( /*challenged-g_entities*/-1, va("print \"%s %s %s!\n\"", challenged->client->pers.netname, G_GetStringEdString("MP_SVGAME", "PLDUELACCEPT"), ent->client->pers.netname) );

			ent->client->ps.duelInProgress = qtrue;
			challenged->client->ps.duelInProgress = qtrue;

			ent->client->ps.duelTime = level.time + 2000;
			challenged->client->ps.duelTime = level.time + 2000;

			G_AddEvent(ent, EV_PRIVATE_DUEL, 1);
			G_AddEvent(challenged, EV_PRIVATE_DUEL, 1);

			//Holster their sabers now, until the duel starts (then they'll get auto-turned on to look cool)

			if (!ent->client->ps.saberHolstered)
			{
				if (ent->client->saber[0].soundOff)
				{
					G_Sound(ent, CHAN_AUTO, ent->client->saber[0].soundOff);
				}
				if (ent->client->saber[1].soundOff &&
					ent->client->saber[1].model[0])
				{
					G_Sound(ent, CHAN_AUTO, ent->client->saber[1].soundOff);
				}
				ent->client->ps.weaponTime = 400;
				ent->client->ps.saberHolstered = 2;
			}
			if (!challenged->client->ps.saberHolstered)
			{
				if (challenged->client->saber[0].soundOff)
				{
					G_Sound(challenged, CHAN_AUTO, challenged->client->saber[0].soundOff);
				}
				if (challenged->client->saber[1].soundOff &&
					challenged->client->saber[1].model[0])
				{
					G_Sound(challenged, CHAN_AUTO, challenged->client->saber[1].soundOff);
				}
				challenged->client->ps.weaponTime = 400;
				challenged->client->ps.saberHolstered = 2;
			}
		}
		else
		{
			//Print the message that a player has been challenged in private, only announce the actual duel initiation in private
			trap_SendServerCommand( challenged-g_entities, va("cp \"%s %s\n\"", ent->client->pers.netname, G_GetStringEdString("MP_SVGAME", "PLDUELCHALLENGE")) );
			trap_SendServerCommand( ent-g_entities, va("cp \"%s %s\n\"", G_GetStringEdString("MP_SVGAME", "PLDUELCHALLENGED"), challenged->client->pers.netname) );
		}

		challenged->client->ps.fd.privateDuelTime = 0; //reset the timer in case this player just got out of a duel. He should still be able to accept the challenge.

		ent->client->ps.forceHandExtend = HANDEXTEND_DUELCHALLENGE;
		ent->client->ps.forceHandExtendTime = level.time + 1000;

		ent->client->ps.duelIndex = challenged->s.number;
		ent->client->ps.duelTime = level.time + 5000;
	}
}

#ifndef FINAL_BUILD
extern stringID_table_t animTable[MAX_ANIMATIONS+1];

void Cmd_DebugSetSaberMove_f(gentity_t *self)
{
	int argNum = trap_Argc();
	char arg[MAX_STRING_CHARS];

	if (argNum < 2)
	{
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );

	if (!arg[0])
	{
		return;
	}

	self->client->ps.saberMove = atoi(arg);
	self->client->ps.saberBlocked = BLOCKED_BOUNCE_MOVE;

	if (self->client->ps.saberMove >= LS_MOVE_MAX)
	{
		self->client->ps.saberMove = LS_MOVE_MAX-1;
	}

	Com_Printf("Anim for move: %s\n", animTable[saberMoveData[self->client->ps.saberMove].animToUse].name);
}

void Cmd_DebugSetBodyAnim_f(gentity_t *self, int flags)
{
	int argNum = trap_Argc();
	char arg[MAX_STRING_CHARS];
	int i = 0;

	if (argNum < 2)
	{
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );

	if (!arg[0])
	{
		return;
	}

	while (i < MAX_ANIMATIONS)
	{
		if (!Q_stricmp(arg, animTable[i].name))
		{
			break;
		}
		i++;
	}

	if (i == MAX_ANIMATIONS)
	{
		Com_Printf("Animation '%s' does not exist\n", arg);
		return;
	}

	G_SetAnim(self, NULL, SETANIM_BOTH, i, flags, 0);

	Com_Printf("Set body anim to %s\n", arg);
}
#endif

void StandardSetBodyAnim(gentity_t *self, int anim, int flags)
{
	G_SetAnim(self, NULL, SETANIM_BOTH, anim, flags, 0);
}

void DismembermentTest(gentity_t *self);

void Bot_SetForcedMovement(int bot, int forward, int right, int up);

#ifndef FINAL_BUILD
extern void DismembermentByNum(gentity_t *self, int num);
extern void G_SetVehDamageFlags( gentity_t *veh, int shipSurf, int damageLevel );
#endif

static int G_ClientNumFromNetname(char *name)
{
	int i = 0;
	gentity_t *ent;

	while (i < MAX_CLIENTS)
	{
		ent = &g_entities[i];

		if (ent->inuse && ent->client &&
			!Q_stricmp(ent->client->pers.netname, name))
		{
			return ent->s.number;
		}
		i++;
	}

	return -1;
}

qboolean TryGrapple(gentity_t *ent)
{
	if (ent->client->ps.weaponTime > 0)
	{ //weapon busy
		return qfalse;
	}
	if (ent->client->ps.forceHandExtend != HANDEXTEND_NONE)
	{ //force power or knockdown or something
		return qfalse;
	}
	if (ent->client->grappleState)
	{ //already grappling? but weapontime should be > 0 then..
		return qfalse;
	}

	if (ent->client->ps.weapon != WP_SABER && ent->client->ps.weapon != WP_MELEE)
	{
		return qfalse;
	}

	if (ent->client->ps.weapon == WP_SABER && !ent->client->ps.saberHolstered)
	{
		Cmd_ToggleSaber_f(ent);
		if (!ent->client->ps.saberHolstered)
		{ //must have saber holstered
			return qfalse;
		}
	}

	//G_SetAnim(ent, &ent->client->pers.cmd, SETANIM_BOTH, BOTH_KYLE_PA_1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD, 0);
	G_SetAnim(ent, &ent->client->pers.cmd, SETANIM_BOTH, BOTH_KYLE_GRAB, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD, 0);
	if (ent->client->ps.torsoAnim == BOTH_KYLE_GRAB)
	{ //providing the anim set succeeded..
		ent->client->ps.torsoTimer += 500; //make the hand stick out a little longer than it normally would
		if (ent->client->ps.legsAnim == ent->client->ps.torsoAnim)
		{
			ent->client->ps.legsTimer = ent->client->ps.torsoTimer;
		}
		ent->client->ps.weaponTime = ent->client->ps.torsoTimer;
		return qtrue;
	}

	return qfalse;
}

#ifndef FINAL_BUILD
qboolean saberKnockOutOfHand(gentity_t *saberent, gentity_t *saberOwner, vec3_t velocity);
#endif

#if 0
static void Cmd_WeaponVariation_f ( gentity_t *ent )
{
    char buffer[MAX_TOKEN_CHARS];
    int variation;
    
    if ( trap_Argc() != 2 )
    {
        trap_SendServerCommand (ent->s.number, "print \"usage: weaponvariation <variation>\n");
        return;
    }
    
    trap_Argv (1, buffer, sizeof (buffer));
    if ( buffer[0] < '0' || buffer[0] > '9' )
    {
        trap_SendServerCommand (ent->s.number, "print \"usage: weaponvariation <variation>\n");
        return;
    }
    
    variation = atoi (buffer);
    if ( variation == ent->client->ps.weaponVariation )
    {
        // No need to change
        return;
    }
    
    if ( !BG_WeaponVariationExists (ent->client->ps.weapon, variation) )
    {
        trap_SendServerCommand (ent->s.number, va ("print \"Variation %d does not exist for the current weapon.\n", variation));
        return;
    }
    
    ent->client->ps.weaponVariation = variation;
    ent->client->ps.weaponVariationChanged = 1;
    trap_SendServerCommand (ent->s.number, "print \"Weapon variation updated.\n");
}

static void Cmd_NextWeaponVariation_f ( gentity_t *ent )
{
    int variation = ent->client->ps.weaponVariation + 1;
    if ( !BG_WeaponVariationExists (ent->client->ps.weapon, variation) )
    {
        variation = 0;
    }
    
    if ( variation == ent->client->ps.weaponVariation )
    {
        return;
    }
    
    ent->client->ps.weaponVariation = variation;
    ent->client->ps.weaponVariationChanged = 1;
    trap_SendServerCommand (ent->s.number, "print \"Weapon variation updated.\n");
}

static void Cmd_PrevWeaponVariation_f ( gentity_t *ent )
{
    int variation = ent->client->ps.weaponVariation - 1;
    if ( variation < 0 )
    {
        variation = BG_NumberOfWeaponVariations (ent->client->ps.weapon) - 1;
    }
    
    if ( !BG_WeaponVariationExists (ent->client->ps.weapon, variation) )
    {
        variation = 0;
    }
    
    ent->client->ps.weaponVariation = variation;
    ent->client->ps.weaponVariationChanged = 1;
    trap_SendServerCommand (ent->s.number, "print \"Weapon variation updated.\n");
}
#endif

/*
=================
ClientCommand
=================
*/

#include "jkg_threading.h"

extern lootTable_t lootLookupTable[MAX_LOOT_TABLE_SIZE];
void ClientCommand( int clientNum ) {
	gentity_t *ent;
	char	cmd[MAX_TOKEN_CHARS];

	ent = g_entities + clientNum;
	if ( !ent->client ) {
		return;		// not fully in game yet
	}


	trap_Argv( 0, cmd, sizeof( cmd ) );

	//rww - redirect bot commands
	if (strstr(cmd, "bot_") && AcceptBotCommand(cmd, ent))
	{
		return;
	}
	//end rww

	// IMPORTANT!!
	// DO NOT LET CONNECTING CLIENTS GO BEYOND THIS POINT!
	if (level.clients[clientNum].pers.connected != CON_CONNECTED)
		return;

	// Check for admin commands
	if (JKG_Admin_Execute(cmd, ent)) return;

	// Check for Glua bound commands
	if (GLua_Command(clientNum, cmd)) return;

	// Check team commands.
	if ( TeamCommand( clientNum, cmd, NULL )) return;

	if (!Q_stricmp( cmd, "taskreport" )) {
		JKG_PrintTasksTable( clientNum );
		return;
	}

	/*if (!Q_stricmp (cmd, "fieldlight")) {
		if (ent->playerState->eFlags & EF_FIELDLIGHT) {
			ent->playerState->eFlags &= ~EF_FIELDLIGHT;
			trap_SendServerCommand( clientNum, "print \"Fieldlight disabled\n\"" );
		} else {
			ent->playerState->eFlags |= EF_FIELDLIGHT;
			trap_SendServerCommand( clientNum, "print \"Fieldlight enabled\n\"" );
		}
		return;
	}*/

	if (!Q_stricmp(cmd, "reload")) {
		Cmd_Reload_f(ent);
		return;
	}

	if (Q_stricmp (cmd, "say") == 0) {
		Cmd_Say_f (ent, SAY_ALL, qfalse);
		return;
	}


	if (Q_stricmp (cmd, "sayact") == 0) {
		Cmd_SayAct_f (ent);
		return;
	}

	if (Q_stricmp (cmd, "sayglobal") == 0) {
		Cmd_SayGlobal_f (ent);
		return;
	}

	if (Q_stricmp (cmd, "sayyell") == 0) {
		Cmd_SayYell_f (ent);
		return;
	}

	if (Q_stricmp (cmd, "saywhisper") == 0) {
		Cmd_SayWhisper_f (ent);
		return;
	}

	if (Q_stricmp (cmd, "say_team") == 0) {
		if (g_gametype.integer < GT_TEAM)
		{ //not a team game, just refer to regular say.
			Cmd_Say_f (ent, SAY_ALL, qfalse);
		}
		else
		{
			Cmd_Say_f (ent, SAY_TEAM, qfalse);
		}
		return;
	}
	if (Q_stricmp (cmd, "tell") == 0) {
		Cmd_Tell_f ( ent );
		return;
	}
	if (Q_stricmp (cmd, "togglesaber") == 0)
	{
		Cmd_ToggleSaber_f(ent);
		return;
	}

	if (Q_stricmp(cmd, "voice_cmd") == 0)
	{
		Cmd_VoiceCommand_f(ent);
		return;
	}

	if (Q_stricmp (cmd, "score") == 0) {
		Cmd_Score_f (ent);
		return;
	}

	//eezstreet add
	if(Q_stricmp(cmd, "itemLookup") == 0) {
		JKG_ItemLookup_f(ent);
		return;
	}
	if(Q_stricmp(cmd, "itemCheck") == 0) {
		JKG_ItemCheck_f(ent);
		return;
	}

	// ignore all other commands when at intermission
	if (level.intermissiontime)
	{
		qboolean giveError = qfalse;
		//rwwFIXMEFIXME: This is terrible, write it differently

		if (!Q_stricmp(cmd, "give"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "giveother"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "god"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "notarget"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "noclip"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "kill"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "teamtask"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "levelshot"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "follow"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "follownext"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "followprev"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "team"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "duelteam"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "siegeclass"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "forcechanged"))
		{ //special case: still update force change
			Cmd_ForceChanged_f (ent);
			return;
		}
		else if (!Q_stricmp(cmd, "where"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "callvote"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "vote"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "callteamvote"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "teamvote"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "gc"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "setviewpos"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "stats"))
		{
			giveError = qtrue;
		}
		// Jedi Knight Galaxies begin
		else if (!Q_stricmp(cmd, "buyVendor"))
		{
			giveError = qtrue;
		}
		// Jedi Knight Galaxies end

		if (giveError)
		{
			trap_SendServerCommand( clientNum, va("print \"%s (%s) \n\"", G_GetStringEdString("MP_SVGAME", "CANNOT_TASK_INTERMISSION"), cmd ) );
		}
		else
		{
			Cmd_Say_f (ent, qfalse, qtrue);
		}
		return;
	}

	if (Q_stricmp (cmd, "give") == 0)
	{
		Cmd_Give_f (ent, 0);
	}
	else if (Q_stricmp (cmd, "giveother") == 0)
	{ //for debugging pretty much
		Cmd_Give_f (ent, 1);
	}
	// Jedi Knight Galaxies begin
	else if (Q_stricmp (cmd, "credits") == 0)
	{
		//DEBUG: Show how many credits you have
		trap_SendServerCommand( clientNum, va("print \"You have %i credits, %s.\n\"", ent->client->ps.persistant[PERS_CREDITS], ent->client->pers.netname) );
		return;
	}
	else if (!Q_stricmp(cmd, "buyVendor"))
	{
		JKG_BuyItem_f(ent);
		return;
	}
	// Jedi Knight Galaxies end
	else if (Q_stricmp (cmd, "t_use") == 0 && CheatsOk(ent))
	{ //debug use map object
		if (trap_Argc() > 1)
		{
			char sArg[MAX_STRING_CHARS];
			gentity_t *targ;

			trap_Argv( 1, sArg, sizeof( sArg ) );
			targ = G_Find( NULL, FOFS(targetname), sArg );

			while (targ)
			{
				if (targ->use)
				{
					targ->use(targ, ent, ent);
				}
				targ = G_Find( targ, FOFS(targetname), sArg );
			}
		}
	}
	else if (Q_stricmp (cmd, "god") == 0)
		Cmd_God_f (ent);
	else if (Q_stricmp (cmd, "notarget") == 0)
		Cmd_Notarget_f (ent);
	else if (Q_stricmp (cmd, "noclip") == 0)
		Cmd_Noclip_f (ent);
	else if ( Q_stricmp( cmd, "NPC" ) == 0 && CheatsOk(ent) )
	{
		Cmd_NPC_f( ent );
	}
	else if (Q_stricmp (cmd, "kill") == 0)
		Cmd_Kill_f (ent);
	else if (Q_stricmp (cmd, "teamtask") == 0)
		Cmd_TeamTask_f (ent);
	#ifdef _DEBUG
	else if (Q_stricmp (cmd, "levelshot") == 0)
		Cmd_LevelShot_f (ent);
	#endif
	else if (Q_stricmp (cmd, "follow") == 0)
		Cmd_Follow_f (ent);
	else if (Q_stricmp (cmd, "follownext") == 0)
		Cmd_FollowCycle_f (ent, 1);
	else if (Q_stricmp (cmd, "followprev") == 0)
		Cmd_FollowCycle_f (ent, -1);
	else if (Q_stricmp (cmd, "team") == 0)
		Cmd_Team_f (ent);
	else if (Q_stricmp (cmd, "duelteam") == 0)
		Cmd_DuelTeam_f (ent);
	else if (Q_stricmp (cmd, "forcechanged") == 0)
		Cmd_ForceChanged_f (ent);
	else if (Q_stricmp (cmd, "where") == 0)
		Cmd_Where_f (ent);
	else if (Q_stricmp (cmd, "callvote") == 0)
		Cmd_CallVote_f (ent);
	else if (Q_stricmp (cmd, "vote") == 0)
		Cmd_Vote_f (ent);
	else if (Q_stricmp (cmd, "callteamvote") == 0)
		Cmd_CallTeamVote_f (ent);
	else if (Q_stricmp (cmd, "teamvote") == 0)
		Cmd_TeamVote_f (ent);
	else if (Q_stricmp (cmd, "gc") == 0)
		Cmd_GameCommand_f( ent );
	else if (Q_stricmp (cmd, "setviewpos") == 0)
		Cmd_SetViewpos_f( ent );
	else if (Q_stricmp (cmd, "stats") == 0)
		Cmd_Stats_f( ent );
#ifdef __UNUSED__
	else if ( Q_stricmp (cmd, "nav_generate") == 0 )
	{
	    char mapname[MAX_STRING_CHARS] = { 0 };
	    trap_Cvar_VariableStringBuffer ("mapname", mapname, sizeof (mapname));
	    JKG_Nav_CreateNavMesh (va ("maps/%s.bsp", mapname));
	}
#endif //__UNUSED__
#ifdef __AUTOWAYPOINT__ // __DOMINANCE_NPC__
	else if (Q_stricmp (cmd, "checkbotreach") == 0 && !Q_strncmp(ent->client->sess.IP, "127.0.0.1:",10))	{
		AIMod_CheckMapPaths( ent );
	}
	else if (Q_stricmp (cmd, "checkobjectivesreach") == 0 && !Q_strncmp(ent->client->sess.IP, "127.0.0.1:",10))	{
		AIMod_CheckObjectivePaths( ent );
	}
	else if (Q_stricmp (cmd, "closeentities") == 0 && !Q_strncmp(ent->client->sess.IP, "127.0.0.1:",10))	{
		int i = 0;

		G_Printf("----------------------------------------------\n");
		G_Printf("Entities Within Range.\n");
		G_Printf("----------------------------------------------\n");
		
		for (i = MAX_CLIENTS; i < MAX_GENTITIES; i++)
		{
			gentity_t *item = &g_entities[i];
			float dist = 0.0f;

			if (!item) continue;
			if (!item->classname || !item->classname[0]) continue;

			dist = Distance(ent->r.currentOrigin, item->s.origin);

			if (dist > 256) continue;

			G_Printf("Entity %i", i);

			switch ( item->s.eType ) {
			case ET_GENERAL:
				G_Printf("[ET_GENERAL         ]");
				break;
			case ET_PLAYER:
				G_Printf("[ET_PLAYER          ]");
				break;
			case ET_ITEM:
				G_Printf("[ET_ITEM            ]");
				break;
			case ET_MISSILE:
				G_Printf("[ET_MISSILE         ]");
				break;
			case ET_MOVER:
				G_Printf("[ET_MOVER           ]");
				break;
			case ET_BEAM:
				G_Printf("[ET_BEAM            ]");
				break;
			case ET_PORTAL:
				G_Printf("[ET_PORTAL          ]");
				break;
			case ET_SPEAKER:
				G_Printf("[ET_SPEAKER         ]");
				break;
			case ET_PUSH_TRIGGER:
				G_Printf("[ET_PUSH_TRIGGER    ]");
				break;
			case ET_TELEPORT_TRIGGER:
				G_Printf("[ET_TELEPORT_TRIGGER]");
				break;
			case ET_INVISIBLE:
				G_Printf("[ET_INVISIBLE       ]");
				break;
			case ET_NPC:
				G_Printf("[ET_NPC             ]");
				break;
			default:
				G_Printf("[%3i                ]", item->s.eType);
				break;
			}

			G_Printf(" [%s] at a distance of %f.\n", item->classname, dist);
		}

		G_Printf("----------------------------------------------\n");
	}
#endif //__AUTOWAYPOINT__ // __DOMINANCE_NPC__
	/*
	else if (Q_stricmp (cmd, "kylesmash") == 0)
	{
		TryGrapple(ent);
	}
	*/
	//for convenient powerduel testing in release
	else if (Q_stricmp(cmd, "killother") == 0 && CheatsOk( ent ))
	{
		if (trap_Argc() > 1)
		{
			char sArg[MAX_STRING_CHARS];
			int entNum = 0;

			trap_Argv( 1, sArg, sizeof( sArg ) );

			entNum = G_ClientNumFromNetname(sArg);

			if (entNum >= 0 && entNum < MAX_GENTITIES)
			{
				gentity_t *kEnt = &g_entities[entNum];

				if (kEnt->inuse && kEnt->client)
				{
					kEnt->flags &= ~FL_GODMODE;
					kEnt->client->ps.stats[STAT_HEALTH] = kEnt->health = -999;
					player_die (kEnt, kEnt, kEnt, 100000, MOD_SUICIDE);
				}
			}
		}
	}
#ifdef _DEBUG
	else if (Q_stricmp(cmd, "relax") == 0 && CheatsOk( ent ))
	{
		if (ent->client->ps.eFlags & EF_RAG)
		{
			ent->client->ps.eFlags &= ~EF_RAG;
		}
		else
		{
			ent->client->ps.eFlags |= EF_RAG;
		}
	}
	else if (Q_stricmp(cmd, "holdme") == 0 && CheatsOk( ent ))
	{
		if (trap_Argc() > 1)
		{
			char sArg[MAX_STRING_CHARS];
			int entNum = 0;

			trap_Argv( 1, sArg, sizeof( sArg ) );

			entNum = atoi(sArg);

			if (entNum >= 0 &&
				entNum < MAX_GENTITIES)
			{
				gentity_t *grabber = &g_entities[entNum];

				if (grabber->inuse && grabber->client && grabber->ghoul2)
				{
					if (!grabber->s.number)
					{ //switch cl 0 and entitynum_none, so we can operate on the "if non-0" concept
						ent->client->ps.ragAttach = ENTITYNUM_NONE;
					}
					else
					{
						ent->client->ps.ragAttach = grabber->s.number;
					}
				}
			}
		}
		else
		{
			ent->client->ps.ragAttach = 0;
		}
	}
	else if (Q_stricmp(cmd, "limb_break") == 0 && CheatsOk( ent ))
	{
		if (trap_Argc() > 1)
		{
			char sArg[MAX_STRING_CHARS];
			int breakLimb = 0;

			trap_Argv( 1, sArg, sizeof( sArg ) );
			if (!Q_stricmp(sArg, "right"))
			{
				breakLimb = BROKENLIMB_RARM;
			}
			else if (!Q_stricmp(sArg, "left"))
			{
				breakLimb = BROKENLIMB_LARM;
			}

			G_BreakArm(ent, breakLimb);
		}
	}
	else if (Q_stricmp(cmd, "headexplodey") == 0 && CheatsOk( ent ))
	{
		Cmd_Kill_f (ent);
		if (ent->health < 1)
		{
			DismembermentTest(ent);
		}
	}
	else if (Q_stricmp(cmd, "debugstupidthing") == 0 && CheatsOk( ent ))
	{
		int i = 0;
		gentity_t *blah;
		while (i < MAX_GENTITIES)
		{
			blah = &g_entities[i];
			if (blah->inuse && blah->classname && blah->classname[0] && !Q_stricmp(blah->classname, "NPC_Vehicle"))
			{
				Com_Printf("Found it.\n");
			}
			i++;
		}
	}
	else if (Q_stricmp(cmd, "arbitraryprint") == 0 && CheatsOk( ent ))
	{
		trap_SendServerCommand( -1, va("cp \"Blah blah blah\n\""));
	}
	else if (Q_stricmp(cmd, "handcut") == 0 && CheatsOk( ent ))
	{
		int bCl = 0;
		char sarg[MAX_STRING_CHARS];

		if (trap_Argc() > 1)
		{
			trap_Argv( 1, sarg, sizeof( sarg ) );

			if (sarg[0])
			{
				bCl = atoi(sarg);

				if (bCl >= 0 && bCl < MAX_GENTITIES)
				{
					gentity_t *hEnt = &g_entities[bCl];

					if (hEnt->client)
					{
						if (hEnt->health > 0)
						{
							gGAvoidDismember = 1;
							hEnt->flags &= ~FL_GODMODE;
							hEnt->client->ps.stats[STAT_HEALTH] = hEnt->health = -999;
							player_die (hEnt, hEnt, hEnt, 100000, MOD_SUICIDE);
						}
						gGAvoidDismember = 2;
						G_CheckForDismemberment(hEnt, ent, hEnt->client->ps.origin, 999, hEnt->client->ps.legsAnim, qfalse);
						gGAvoidDismember = 0;
					}
				}
			}
		}
	}
	else if (Q_stricmp(cmd, "loveandpeace") == 0 && CheatsOk( ent ))
	{
		trace_t tr;
		vec3_t fPos;

		AngleVectors(ent->client->ps.viewangles, fPos, 0, 0);

		fPos[0] = ent->client->ps.origin[0] + fPos[0]*40;
		fPos[1] = ent->client->ps.origin[1] + fPos[1]*40;
		fPos[2] = ent->client->ps.origin[2] + fPos[2]*40;

		trap_Trace(&tr, ent->client->ps.origin, 0, 0, fPos, ent->s.number, ent->clipmask);

		if (tr.entityNum < MAX_CLIENTS && tr.entityNum != ent->s.number)
		{
			gentity_t *other = &g_entities[tr.entityNum];

			if (other && other->inuse && other->client)
			{
				vec3_t entDir;
				vec3_t otherDir;
				vec3_t entAngles;
				vec3_t otherAngles;

				if (ent->client->ps.weapon == WP_SABER && !ent->client->ps.saberHolstered)
				{
					Cmd_ToggleSaber_f(ent);
				}

				if (other->client->ps.weapon == WP_SABER && !other->client->ps.saberHolstered)
				{
					Cmd_ToggleSaber_f(other);
				}

				if ((ent->client->ps.weapon != WP_SABER || ent->client->ps.saberHolstered) &&
					(other->client->ps.weapon != WP_SABER || other->client->ps.saberHolstered))
				{
					VectorSubtract( other->client->ps.origin, ent->client->ps.origin, otherDir );
					VectorCopy( ent->client->ps.viewangles, entAngles );
					entAngles[YAW] = vectoyaw( otherDir );
					SetClientViewAngle( ent, entAngles );

					StandardSetBodyAnim(ent, /*BOTH_KISSER1LOOP*/BOTH_STAND1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS);
					ent->client->ps.saberMove = LS_NONE;
					ent->client->ps.saberBlocked = 0;
					ent->client->ps.saberBlocking = 0;

					VectorSubtract( ent->client->ps.origin, other->client->ps.origin, entDir );
					VectorCopy( other->client->ps.viewangles, otherAngles );
					otherAngles[YAW] = vectoyaw( entDir );
					SetClientViewAngle( other, otherAngles );

					StandardSetBodyAnim(other, /*BOTH_KISSEE1LOOP*/BOTH_STAND1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS);
					other->client->ps.saberMove = LS_NONE;
					other->client->ps.saberBlocked = 0;
					other->client->ps.saberBlocking = 0;
				}
			}
		}
	}
#endif
	else if (Q_stricmp(cmd, "thedestroyer") == 0 && CheatsOk( ent ) && ent && ent->client && ent->client->ps.saberHolstered && ent->client->ps.weapon == WP_SABER)
	{
		Cmd_ToggleSaber_f(ent);

		if (!ent->client->ps.saberHolstered)
		{
		}
	}
	//begin bot debug cmds
	else if (Q_stricmp(cmd, "debugBMove_Forward") == 0 && CheatsOk(ent))
	{
		int arg = 4000;
		int bCl = 0;
		char sarg[MAX_STRING_CHARS];

		assert(trap_Argc() > 1);
		trap_Argv( 1, sarg, sizeof( sarg ) );

		assert(sarg[0]);
		bCl = atoi(sarg);
		Bot_SetForcedMovement(bCl, arg, -1, -1);
	}
	else if (Q_stricmp(cmd, "debugBMove_Back") == 0 && CheatsOk(ent))
	{
		int arg = -4000;
		int bCl = 0;
		char sarg[MAX_STRING_CHARS];

		assert(trap_Argc() > 1);
		trap_Argv( 1, sarg, sizeof( sarg ) );

		assert(sarg[0]);
		bCl = atoi(sarg);
		Bot_SetForcedMovement(bCl, arg, -1, -1);
	}
	else if (Q_stricmp(cmd, "debugBMove_Right") == 0 && CheatsOk(ent))
	{
		int arg = 4000;
		int bCl = 0;
		char sarg[MAX_STRING_CHARS];

		assert(trap_Argc() > 1);
		trap_Argv( 1, sarg, sizeof( sarg ) );

		assert(sarg[0]);
		bCl = atoi(sarg);
		Bot_SetForcedMovement(bCl, -1, arg, -1);
	}
	else if (Q_stricmp(cmd, "debugBMove_Left") == 0 && CheatsOk(ent))
	{
		int arg = -4000;
		int bCl = 0;
		char sarg[MAX_STRING_CHARS];

		assert(trap_Argc() > 1);
		trap_Argv( 1, sarg, sizeof( sarg ) );

		assert(sarg[0]);
		bCl = atoi(sarg);
		Bot_SetForcedMovement(bCl, -1, arg, -1);
	}
	else if (Q_stricmp(cmd, "debugBMove_Up") == 0 && CheatsOk(ent))
	{
		int arg = 4000;
		int bCl = 0;
		char sarg[MAX_STRING_CHARS];

		assert(trap_Argc() > 1);
		trap_Argv( 1, sarg, sizeof( sarg ) );

		assert(sarg[0]);
		bCl = atoi(sarg);
		Bot_SetForcedMovement(bCl, -1, -1, arg);
	}
	//end bot debug cmds
/*#ifndef FINAL_BUILD
	else if (Q_stricmp(cmd, "debugSetSaberMove") == 0)
	{
		Cmd_DebugSetSaberMove_f(ent);
	}
	else if (Q_stricmp(cmd, "debugSetBodyAnim") == 0)
	{
		Cmd_DebugSetBodyAnim_f(ent, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
	}
	else if (Q_stricmp(cmd, "debugDismemberment") == 0)
	{
		Cmd_Kill_f (ent);
		if (ent->health < 1)
		{
			char	arg[MAX_STRING_CHARS];
			int		iArg = 0;

			if (trap_Argc() > 1)
			{
				trap_Argv( 1, arg, sizeof( arg ) );

				if (arg[0])
				{
					iArg = atoi(arg);
				}
			}

			DismembermentByNum(ent, iArg);
		}
	}
	else if (Q_stricmp(cmd, "debugDropSaber") == 0)
	{
		if (ent->client->ps.weapon == WP_SABER &&
			ent->client->ps.saberEntityNum &&
			!ent->client->ps.saberInFlight)
		{
			saberKnockOutOfHand(&g_entities[ent->client->ps.saberEntityNum], ent, vec3_origin);
		}
	}
	else if (Q_stricmp(cmd, "debugKnockMeDown") == 0)
	{
		if (BG_KnockDownable(&ent->client->ps))
		{
			ent->client->ps.forceHandExtend = HANDEXTEND_KNOCKDOWN;
			ent->client->ps.forceDodgeAnim = 0;
			if (trap_Argc() > 1)
			{
				ent->client->ps.forceHandExtendTime = level.time + 1100;
				ent->client->ps.quickerGetup = qfalse;
			}
			else
			{
				ent->client->ps.forceHandExtendTime = level.time + 700;
				ent->client->ps.quickerGetup = qtrue;
			}
		}
	}
	else if (Q_stricmp(cmd, "debugSaberSwitch") == 0)
	{
		gentity_t *targ = NULL;

		if (trap_Argc() > 1)
		{
			char	arg[MAX_STRING_CHARS];

			trap_Argv( 1, arg, sizeof( arg ) );

			if (arg[0])
			{
				int x = atoi(arg);
				
				if (x >= 0 && x < MAX_CLIENTS)
				{
					targ = &g_entities[x];
				}
			}
		}

		if (targ && targ->inuse && targ->client)
		{
			Cmd_ToggleSaber_f(targ);
		}
	}
	else if (Q_stricmp(cmd, "debugIKGrab") == 0)
	{
		gentity_t *targ = NULL;

		if (trap_Argc() > 1)
		{
			char	arg[MAX_STRING_CHARS];

			trap_Argv( 1, arg, sizeof( arg ) );

			if (arg[0])
			{
				int x = atoi(arg);
				
				if (x >= 0 && x < MAX_CLIENTS)
				{
					targ = &g_entities[x];
				}
			}
		}

		if (targ && targ->inuse && targ->client && ent->s.number != targ->s.number)
		{
			targ->client->ps.heldByClient = ent->s.number+1;
		}
	}
	else if (Q_stricmp(cmd, "debugIKBeGrabbedBy") == 0)
	{
		gentity_t *targ = NULL;

		if (trap_Argc() > 1)
		{
			char	arg[MAX_STRING_CHARS];

			trap_Argv( 1, arg, sizeof( arg ) );

			if (arg[0])
			{
				int x = atoi(arg);
				
				if (x >= 0 && x < MAX_CLIENTS)
				{
					targ = &g_entities[x];
				}
			}
		}

		if (targ && targ->inuse && targ->client && ent->s.number != targ->s.number)
		{
			ent->client->ps.heldByClient = targ->s.number+1;
		}
	}
	else if (Q_stricmp(cmd, "debugIKRelease") == 0)
	{
		gentity_t *targ = NULL;

		if (trap_Argc() > 1)
		{
			char	arg[MAX_STRING_CHARS];

			trap_Argv( 1, arg, sizeof( arg ) );

			if (arg[0])
			{
				int x = atoi(arg);
				
				if (x >= 0 && x < MAX_CLIENTS)
				{
					targ = &g_entities[x];
				}
			}
		}

		if (targ && targ->inuse && targ->client)
		{
			targ->client->ps.heldByClient = 0;
		}
	}
	else if (Q_stricmp(cmd, "debugThrow") == 0)
	{
		trace_t tr;
		vec3_t tTo, fwd;

		if (ent->client->ps.weaponTime > 0 || ent->client->ps.forceHandExtend != HANDEXTEND_NONE ||
			ent->client->ps.groundEntityNum == ENTITYNUM_NONE || ent->health < 1)
		{
			return;
		}

		AngleVectors(ent->client->ps.viewangles, fwd, 0, 0);
		tTo[0] = ent->client->ps.origin[0] + fwd[0]*32;
		tTo[1] = ent->client->ps.origin[1] + fwd[1]*32;
		tTo[2] = ent->client->ps.origin[2] + fwd[2]*32;

		trap_Trace(&tr, ent->client->ps.origin, 0, 0, tTo, ent->s.number, MASK_PLAYERSOLID);

		if (tr.fraction != 1)
		{
			gentity_t *other = &g_entities[tr.entityNum];

			if (other->inuse && other->client && other->client->ps.forceHandExtend == HANDEXTEND_NONE &&
				other->client->ps.groundEntityNum != ENTITYNUM_NONE && other->health > 0 &&
				(int)ent->client->ps.origin[2] == (int)other->client->ps.origin[2])
			{
				float pDif = 40.0f;
				vec3_t entAngles, entDir;
				vec3_t otherAngles, otherDir;
				vec3_t intendedOrigin;
				vec3_t boltOrg, pBoltOrg;
				vec3_t tAngles, vDif;
				vec3_t fwd, right;
				trace_t tr;
				trace_t tr2;

				VectorSubtract( other->client->ps.origin, ent->client->ps.origin, otherDir );
				VectorCopy( ent->client->ps.viewangles, entAngles );
				entAngles[YAW] = vectoyaw( otherDir );
				SetClientViewAngle( ent, entAngles );

				ent->client->ps.forceHandExtend = HANDEXTEND_PRETHROW;
				ent->client->ps.forceHandExtendTime = level.time + 5000;

				ent->client->throwingIndex = other->s.number;
				ent->client->doingThrow = level.time + 5000;
				ent->client->beingThrown = 0;

				VectorSubtract( ent->client->ps.origin, other->client->ps.origin, entDir );
				VectorCopy( other->client->ps.viewangles, otherAngles );
				otherAngles[YAW] = vectoyaw( entDir );
				SetClientViewAngle( other, otherAngles );

				other->client->ps.forceHandExtend = HANDEXTEND_PRETHROWN;
				other->client->ps.forceHandExtendTime = level.time + 5000;

				other->client->throwingIndex = ent->s.number;
				other->client->beingThrown = level.time + 5000;
				other->client->doingThrow = 0;

				//Doing this now at a stage in the throw, isntead of initially.
				//other->client->ps.heldByClient = ent->s.number+1;

				G_EntitySound( other, CHAN_VOICE, G_SoundIndex("*pain100.wav") );
				G_EntitySound( ent, CHAN_VOICE, G_SoundIndex("*jump1.wav") );
				G_Sound(other, CHAN_AUTO, G_SoundIndex( "sound/movers/objects/objectHit.wav" ));

				//see if we can move to be next to the hand.. if it's not clear, break the throw.
				VectorClear(tAngles);
				tAngles[YAW] = ent->client->ps.viewangles[YAW];
				VectorCopy(ent->client->ps.origin, pBoltOrg);
				AngleVectors(tAngles, fwd, right, 0);
				boltOrg[0] = pBoltOrg[0] + fwd[0]*8 + right[0]*pDif;
				boltOrg[1] = pBoltOrg[1] + fwd[1]*8 + right[1]*pDif;
				boltOrg[2] = pBoltOrg[2];

				VectorSubtract(boltOrg, pBoltOrg, vDif);
				VectorNormalize(vDif);

				VectorClear(other->client->ps.velocity);
				intendedOrigin[0] = pBoltOrg[0] + vDif[0]*pDif;
				intendedOrigin[1] = pBoltOrg[1] + vDif[1]*pDif;
				intendedOrigin[2] = other->client->ps.origin[2];

				trap_Trace(&tr, intendedOrigin, other->r.mins, other->r.maxs, intendedOrigin, other->s.number, other->clipmask);
				trap_Trace(&tr2, ent->client->ps.origin, ent->r.mins, ent->r.maxs, intendedOrigin, ent->s.number, CONTENTS_SOLID);

				if (tr.fraction == 1.0f && !tr.startsolid && tr2.fraction == 1.0f && !tr2.startsolid)
				{
					VectorCopy(intendedOrigin, other->client->ps.origin);
				}
				else
				{ //if the guy can't be put here then it's time to break the throw off.
					vec3_t oppDir;
					int strength = 4;

					other->client->ps.heldByClient = 0;
					other->client->beingThrown = 0;
					ent->client->doingThrow = 0;

					ent->client->ps.forceHandExtend = HANDEXTEND_NONE;
					G_EntitySound( ent, CHAN_VOICE, G_SoundIndex("*pain25.wav") );

					other->client->ps.forceHandExtend = HANDEXTEND_NONE;
					VectorSubtract(other->client->ps.origin, ent->client->ps.origin, oppDir);
					VectorNormalize(oppDir);
					other->client->ps.velocity[0] = oppDir[0]*(strength*40);
					other->client->ps.velocity[1] = oppDir[1]*(strength*40);
					other->client->ps.velocity[2] = 150;

					VectorSubtract(ent->client->ps.origin, other->client->ps.origin, oppDir);
					VectorNormalize(oppDir);
					ent->client->ps.velocity[0] = oppDir[0]*(strength*40);
					ent->client->ps.velocity[1] = oppDir[1]*(strength*40);
					ent->client->ps.velocity[2] = 150;
				}
			}
		}
	}
#endif
#ifdef VM_MEMALLOC_DEBUG
	else if (Q_stricmp(cmd, "debugTestAlloc") == 0)
	{ //rww - small routine to stress the malloc trap stuff and make sure nothing bad is happening.
		char *blah;
		int i = 1;
		int x;

		//stress it. Yes, this will take a while. If it doesn't explode miserably in the process.
		while (i < 32768)
		{
			x = 0;

			trap_TrueMalloc((void **)&blah, i);
			if (!blah)
			{ //pointer is returned null if allocation failed
				trap_SendServerCommand( -1, va("print \"Failed to alloc at %i!\n\"", i));
				break;
			}
			while (x < i)
			{ //fill the allocated memory up to the edge
				if (x+1 == i)
				{
					blah[x] = 0;
				}
				else
				{
					blah[x] = 'A';
				}
				x++;
			}
			trap_TrueFree((void **)&blah);
			if (blah)
			{ //should be nullified in the engine after being freed
				trap_SendServerCommand( -1, va("print \"Failed to free at %i!\n\"", i));
				break;
			}

			i++;
		}

		trap_SendServerCommand( -1, "print \"Finished allocation test\n\"");
	}
#endif
#ifndef FINAL_BUILD
	else if (Q_stricmp(cmd, "debugShipDamage") == 0)
	{
		char	arg[MAX_STRING_CHARS];
		char	arg2[MAX_STRING_CHARS];
		int		shipSurf, damageLevel;

		trap_Argv( 1, arg, sizeof( arg ) );
		trap_Argv( 2, arg2, sizeof( arg2 ) );
		shipSurf = SHIPSURF_FRONT+atoi(arg);
		damageLevel = atoi(arg2);

		G_SetVehDamageFlags( &g_entities[ent->s.m_iVehicleNum], shipSurf, damageLevel );
	}
	else if (Q_stricmp(cmd, "debugHealth") == 0)
	{
		trap_SendServerCommand( clientNum, va("print \"%i \n\"", ent->client->ps.stats[STAT_HEALTH]));
		return;
	}
	else if (Q_stricmp(cmd, "debugInventory") == 0)
	{
		JKG_Cmd_ShowInv_f(ent);
		return;
	}
#endif
	else if (Q_stricmp(cmd, "bodyLoot") == 0)
	{
		char arg[MAX_STRING_CHARS];
		char arg2[MAX_STRING_CHARS];
		if(trap_Argc() < 3) //not enough args
		{
			trap_SendServerCommand( clientNum, "print \"Syntax: /bodyLoot <entity num> <loot ID>\n\"" );
			return;
		}
		trap_Argv( 1, arg, sizeof ( arg ) );
		trap_Argv( 2, arg2, sizeof ( arg2 ) );
		JKG_Cmd_Loot_f( ent, atoi( arg ), atoi( arg2 ), qfalse );
		return;
	}*/
	else if (Q_stricmp(cmd, "inventoryUse") == 0)
	{
		char arg[MAX_STRING_CHARS];
		if(trap_Argc() < 2)
		{
			trap_SendServerCommand( clientNum, "print \"Syntax: /inventoryUse <item num>\n\"" );
			return;
		}
		trap_Argv( 1, arg, sizeof(arg) );
		JKG_Cmd_ItemAction_f( ent, atoi( arg ) );
		return;
	}
	else if (Q_stricmp(cmd, "equip") == 0)
	{
		JKG_Cmd_EquipItem_f (ent);
	}
	else if (Q_stricmp(cmd, "unequip") == 0)
	{
		JKG_Cmd_UnequipItem_f (ent);
	}
	else if (Q_stricmp(cmd, "inventoryDestroy") == 0)
	{
		JKG_Cmd_DestroyItem_f(ent);
	}
	else if (Q_stricmp(cmd, "inventorySell") == 0)
	{
		JKG_Cmd_SellItem_f(ent);
	}
	//
	// BEGIN - Warzone Gametype Editing...
	//
	else if(!Q_stricmp (cmd, "warzone_savegameinfo") && CheatsOk( ent )) 
	{
		WARZONE_SaveGameInfo();
	} 
	else if(!Q_stricmp (cmd, "warzone_loadgameinfo") && CheatsOk( ent )) 
	{
		WARZONE_LoadGameInfo();
	} 
	else if(!Q_stricmp (cmd, "warzone_addhealthcrate") && CheatsOk( ent )) 
	{
		WARZONE_AddHealthCrate( ent->r.currentOrigin, ent->r.currentAngles );
	} 
	else if(!Q_stricmp (cmd, "warzone_addammocrate") && CheatsOk( ent )) 
	{
		WARZONE_AddAmmoCrate( ent->r.currentOrigin, ent->r.currentAngles );
#ifdef __VEHICLES__
	} 
	else if(!Q_stricmp (cmd, "warzone_addvehicle") && CheatsOk( ent )) 
	{
		int  vehicleType;
		char str[MAX_TOKEN_CHARS];

		trap_Argv(1, str, sizeof(str));

		if (!Q_stricmp(str,"heavy")) 
		{
			vehicleType = VEHICLE_CLASS_HEAVY_TANK;
		} 
		else if (!Q_stricmp(str,"medium")) 
		{
			vehicleType = VEHICLE_CLASS_MEDIUM_TANK;
		} 
		else if (!Q_stricmp(str,"flame")) 
		{
			vehicleType = VEHICLE_CLASS_FLAMETANK;
		} 
		else 
		{
			vehicleType = VEHICLE_CLASS_LIGHT_TANK;
		}

		WARZONE_AddVehicle( ent->r.currentOrigin, ent->r.currentAngles, vehicleType );
#endif //__VEHICLES__
	} 
	else if(!Q_stricmp (cmd, "warzone_addflag") && CheatsOk( ent )) 
	{
		int  team;
		char str[MAX_TOKEN_CHARS];

		trap_Argv(1, str, sizeof(str));

		if (!Q_stricmp(str,"red")) 
		{
			team = TEAM_RED;
		} 
		else if (!Q_stricmp(str,"blue")) 
		{
			team = TEAM_BLUE;
		} 
		else 
		{
			team = TEAM_FREE;
		}

		WARZONE_AddFlag( ent->r.currentOrigin, team );
	} 
	else if(!Q_stricmp (cmd, "warzone_changeflagteam") && CheatsOk( ent )) 
	{
		int  team = 0, flag_number = 0;
		char str[MAX_TOKEN_CHARS];

		trap_Argv(1, str, sizeof(str));

		if (!str || !str[0])
		{
			G_Printf("Format: /warzone_changeflagteam <flag #> <team name>\n");
			WARZONE_ShowInfo();
			return;
		}

		flag_number = atoi(str);

		trap_Argv(2, str, sizeof(str));

		if (!Q_stricmp(str,"red")) 
		{
			team = TEAM_RED;
		} 
		else if (!Q_stricmp(str,"blue")) 
		{
			team = TEAM_BLUE;
		} 
		else 
		{
			team = TEAM_FREE;
		}
		
		WARZONE_ChangeFlagTeam( flag_number, team );
	} 
	else if(!Q_stricmp (cmd, "warzone_raiseents") && CheatsOk( ent )) 
	{
		WARZONE_RaiseAllWarzoneEnts( -1 );
	} 
	else if(!Q_stricmp (cmd, "warzone_removeents") && CheatsOk( ent )) 
	{
		WARZONE_RemoveAllWarzoneEnts();
	} 
	else if(!Q_stricmp (cmd, "warzone_removeammocrate") && CheatsOk( ent )) 
	{
		WARZONE_RemoveAmmoCrate();
	} 
	else if(!Q_stricmp (cmd, "warzone_removehealthcrate") && CheatsOk( ent )) 
	{
		WARZONE_RemoveHealthCrate();
	} 
	else if(!Q_stricmp (cmd, "warzone_showinfo") && CheatsOk( ent )) 
	{
		WARZONE_ShowInfo();
	}
	//
	// END - Warzone Gametype Editing...
	//
	else
	{
		if (Q_stricmp(cmd, "addbot") == 0)
		{ //because addbot isn't a recognized command unless you're the server, but it is in the menus regardless
//			trap_SendServerCommand( clientNum, va("print \"You can only add bots as the server.\n\"" ) );
			trap_SendServerCommand( clientNum, va("print \"%s.\n\"", G_GetStringEdString("MP_SVGAME", "ONLY_ADD_BOTS_AS_SERVER")));
		}
		else
		{
			trap_SendServerCommand( clientNum, va("print \"unknown cmd %s\n\"", cmd ) );
		}
	}
}
