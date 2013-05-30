#ifndef TEAMS_H
#define TEAMS_H
#ifndef __linux__
typedef enum //# team_e
#else
enum
#endif
{
	NPCTEAM_FREE,			// also TEAM_FREE - caution, some code checks a team_t via "if (!team_t_varname)" so I guess this should stay as entry 0, great or what? -slc
	NPCTEAM_ENEMY,			// also TEAM_RED
	NPCTEAM_PLAYER,			// also TEAM_BLUE
	NPCTEAM_NEUTRAL,		// also TEAM_SPECTATOR - most droids are team_neutral, there are some exceptions like Probe,Seeker,Interrogator

	//# #eol
	NPCTEAM_NUM_TEAMS
};
typedef int npcteam_t;

// This list is made up from the model directories, this MUST be in the same order as the ClassNames array in NPC_stats.cpp
typedef enum 
{
	CLASS_NONE,				// hopefully this will never be used by an npc, just covering all bases
	CLASS_ATST,				// technically droid...
	CLASS_BARTENDER,
	CLASS_BESPIN_COP,		
	CLASS_CLAW,
	CLASS_COMMANDO,
	CLASS_DESANN,			
	CLASS_FISH,
	CLASS_FLIER2,
	CLASS_GALAK,
	CLASS_GLIDER,
	CLASS_GONK,				// droid
	CLASS_GRAN,
	CLASS_HOWLER,
	CLASS_IMPERIAL,
	CLASS_IMPWORKER,
	CLASS_INTERROGATOR,		// droid 
	CLASS_JAN,				
	CLASS_JEDI,
	CLASS_KYLE,				
	CLASS_LANDO,			
	CLASS_LIZARD,
	CLASS_LUKE,				
	CLASS_MARK1,			// droid
	CLASS_MARK2,			// droid
	CLASS_GALAKMECH,		// droid
	CLASS_MINEMONSTER,
	CLASS_MONMOTHA,			
	CLASS_MORGANKATARN,
	CLASS_MOUSE,			// droid
	CLASS_MURJJ,
	CLASS_PRISONER,
	CLASS_PROBE,			// droid
	CLASS_PROTOCOL,			// droid
	CLASS_R2D2,				// droid
	CLASS_R5D2,				// droid
	CLASS_REBEL,
	CLASS_REBORN,
	CLASS_REELO,
	CLASS_REMOTE,
	CLASS_RODIAN,
	CLASS_SEEKER,			// droid
	CLASS_SENTRY,
	CLASS_SHADOWTROOPER,
	CLASS_STORMTROOPER,
	CLASS_MERC,//Stoiss add merc class
	CLASS_SWAMP,
	CLASS_SWAMPTROOPER,
	CLASS_TAVION,
	CLASS_TRANDOSHAN,
	CLASS_UGNAUGHT,
	CLASS_JAWA,
	CLASS_WEEQUAY,
	CLASS_BOBAFETT,
	CLASS_VEHICLE,
	CLASS_RANCOR,
	CLASS_WAMPA,
	CLASS_CIVILIAN,			// UQ1: Random civilian NPCs...
	CLASS_GENERAL_VENDOR,
	CLASS_WEAPONS_VENDOR,
	CLASS_ARMOR_VENDOR,
	CLASS_SUPPLIES_VENDOR,
	CLASS_FOOD_VENDOR,
	CLASS_MEDICAL_VENDOR,
	CLASS_GAMBLER_VENDOR,
	CLASS_TRADE_VENDOR,
	CLASS_ODDITIES_VENDOR,
	CLASS_DRUG_VENDOR,
	CLASS_TRAVELLING_VENDOR,
	//Stoiss add: FAQ Npc class
	CLASS_JKG_FAQ_IMP_DROID,
	CLASS_JKG_FAQ_ALLIANCE_DROID,
	CLASS_JKG_FAQ_SPY_DROID,
	CLASS_JKG_FAQ_CRAFTER_DROID,
	CLASS_JKG_FAQ_MERC_DROID,
	CLASS_JKG_FAQ_JEDI_MENTOR,
	CLASS_JKF_FAQ_SITH_MENTOR,
	//Stoiss end
	CLASS_BOT_FAKE_NPC,
	
	CLASS_NUM_CLASSES
} class_t;

#endif	// #ifndef TEAMS_H
