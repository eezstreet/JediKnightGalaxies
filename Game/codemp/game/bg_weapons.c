// Copyright (C) 2001-2002 Raven Software
//
// bg_weapons.c -- part of bg_pmove functionality

#include "q_shared.h"
#include "bg_public.h"
#include "bg_local.h"
#include "bg_strap.h"

weaponAmmo_t xweaponAmmo [] =
{
	/* Ammo Index		  Cost	  Size	  Max */
	{ AMMO_NONE			, -1	, -1	, -1	},
	{ AMMO_FORCE		, 0		, 0		, 1000	},
	{ AMMO_BLASTER		, 0		, 60	, 3000	},
	{ AMMO_POWERCELL	, 0		, 50	, 3000	},
	{ AMMO_METAL_BOLTS	, 0		, 150	, 3000	},
	{ AMMO_CONCUSSION   , 0     , 10    , 500   },
	{ AMMO_ROCKETS		, 0		, 1		, 25	},
	{ AMMO_EMPLACED		, 0		, 0		, 800	},
	{ AMMO_THERMAL		, 0		, 0		, 10	},
	{ AMMO_TRIPMINE		, 0		, 0		, 10	},
	{ AMMO_DETPACK		, 0		, 0		, 10	}
};

/*
Non-Ranged Weapons
00) Weapon None (Coding Purposes Only)
01) Melee (Unarmed, Spiked Knuckles)(Secondary Weapon)
02) Blades (Lightsaber, Vibroblade)(Primary/Secondary Weapon)
03) Stun Weapons (Stun Baton)(Secondary Weapon)

Ranged Energy Weapons
04) Blaster Pistols (Pistol)(Secondary Weapon)
05) Blaster Rifles & Carbines (E11)(Primary/Secondary Weapon)
06) Repeating Blasters (Repeater, T-21)(Primary Weapon)
07) Ion/Sonic Guns (DEMP2)(Primary Weapon)
08) Disruptor Weapons (Disruptor, MSD-32 Disruptor Pistol)(Primary/Secondary Weapon)

Ranged Projectile Weapons
09) Slugthrower Pistol (.48 Enforcer Pistol)(Secondary Weapon)
10) Slugthrower Rifle (KiSteer 1284 Projectile Rifle)(Primary Weapon)
11) Fletchette Launchers (FC-1 Flechette Launcher)(Primary Weapon)
12) Crossbows (Bowcaster)(Primary Weapon)

Grenade/Explosives
13) Grenades (Frag Grenade, Stun Grenade, Thermal Detonator)(Tertiary Weapon)
14) Explosive (Tripmine, Detpack)(Tertiary Weapon)
15) Explosive Systems (Concussion, Rocket Launcher)(Primary Weapon)
*/

// Slugs pierce armor?
// Fixxy stand still + walk

// This too. Will contain all effects for each gun eventually, after hooking the first bits up.
// Ghoul instances will be done from here too so muzzles always are correct.



/**************************************************
* weaponData_t
*
* This is the weapon data table, the heart and soul
* of all the weapons in the game. Variations and base
* weapons can be made using this table.
**************************************************/
#define MAX_WEAPON_TABLE_SIZE (64)
static weaponData_t weaponDataTable[MAX_WEAPON_TABLE_SIZE];
static unsigned int numLoadedWeapons;
static unsigned int numWeapons[MAX_WEAPONS];

unsigned int BG_NumberOfLoadedWeapons ( void )
{
    return numLoadedWeapons;
}

unsigned int BG_NumberOfWeaponVariations ( unsigned char weaponId )
{
    return numWeapons[weaponId];
}

/*static weaponData_t xweaponData[] = 
{
	// ADDITIONS:
	// - Change the repeater secondary fire to use a grenade (since it is basically a 203).
	// - Dropped weapons with variations, a way to detect them and have the proper models loaded.
	// - Grenade effects (Close Range Disruptify, Short Range Dismemberment, Medium Range Light Dismemberment)
	// - Handle the created [Weapon Variation] tags and verify NPC's featuring weapons with the new properties.
	// - Model list with each effect per weapon for base weapons and its variations, use this instead of event/slot-based.

	// BUGS:
	// - Bowcaster must have a single zoom, it cannot zoom any further.

	// REQUIREMENTS:
	// - Normalize values using XLS calculations (new calculation set with all the added values).

	// TODO:
	// - Finish 'Requirements'
	// - Hook up Tripmine/Detpack logic.
	// - Remove contact grenades, they don't magically know they hit a person.
	// - IsBlockable (for jedi block) set the mask appropriatly (for instance, rockets are a no-block)
	// - Player rendering limit (distance for all, not in FOV for NPC only)		
	// - fix use_eweb and /place emplaced_gun 0

	/* Weapon Base Index      Mod Reload	  Weapon Slot		  Ammo Index		  Cookable	  KnockBack	  Roll		  Secondary	  Zoom		  Speed,    Reload, { DMG	  AG  BC  DW  SC  Box	  CMax	  CMul	CTime	  Cost	  Delay	  Range	  DRange  Recoil  Spread  Speed		  Weapon Classname		  Weapon Direct MOD		  Weapon Splash MOD			}  { DMG  AG  BC  DW  SC  Box	  CMax	  CMul	  CTime	  Cost	  Delay	  Range	  DRange  Recoil  Spread   Speed		  Weapon Classname			  Weapon Direct MOD		  Weapon Splash MOD			} *
	{ WP_NONE               , 0	, 0			, WPS_NONE			, AMMO_NONE			, qfalse	, qfalse	, qtrue		, qfalse	, qfalse	, 1.0f,     1.0f,   { 0		, 0	, 0	, 0 , 0 , 0		, 0		, 0		, 0		, 0		, 0		, 0		, 0		, 0		, 0		, 0			, ""					, 0						, 0							}, { 0	, 0	, 0 , 0 , 0	, 0		, 0		, 0		, 0		, 0		, 0		, 0		, 0		, 0		, 0		 , 0			, ""						, 0						, 0							}},
	{ WP_STUN_BATON			, 0	, 0			, WPS_NONE			, AMMO_NONE			, qfalse	, qfalse	, qtrue		, qfalse	, qfalse	, 1.0f,     1.0f,   { 20	, 0	, 0	, 0 , 0 , 0		, 0		, 0		, 0		, 0		, 400	, 8		, 0		, 0		, 0		, 0			, ""					, MOD_STUN_BATON		, 0							}, { 0	, 0	, 0 , 0 , 0	, 0		, 0		, 0		, 0		, 0		, 0		, 0		, 0		, 0		, 0		 , 0			, ""						, MOD_STUN_BATON		, 0							}},
	{ WP_MELEE				, 0	, 0			, WPS_NONE			, AMMO_NONE			, qfalse	, qfalse	, qtrue		, qfalse	, qfalse	, 1.0f,     1.0f,   { 10	, 0	, 0	, 0 , 0 , 0		, 0		, 0		, 0		, 0		, 400	, 8		, 0		, 0		, 0		, 0			, ""					, MOD_MELEE				, 0							}, { 0	, 0	, 0 , 0 , 0	, 0		, 0		, 0		, 0		, 0		, 0		, 0		, 0		, 0		, 0		 , 0			, ""						, MOD_MELEE				, 0							}},
	{ WP_SABER				, 0	, 0			, WPS_NONE			, AMMO_NONE			, qfalse	, qfalse	, qtrue		, qtrue		, qfalse	, 1.0f,     1.0f,   { 0		, 0	, 0	, 0 , 0 , 0		, 0		, 0		, 0		, 0		, 0		, 0		, 0		, 0		, 0		, 0			, ""					, MOD_SABER				, 0							}, { 0	, 0	, 0 , 0 , 0	, 0		, 0		, 0		, 0		, 0		, 0		, 0		, 0		, 0		, 0		 , 0			, ""						, MOD_SABER				, 0							}},

	/* Weapon Base Index      Mod Reload	  Weapon Slot		  Ammo Index		  Cookable	  KnockBack	  Roll		  Secondary	  Zoom		  Speed,    Reload, { DMG	  AG  BC  DW  SC  Box	  CMax	  CMul	CTime	  Cost	  Delay	  Range	  DRange  Recoil  Spread  Speed		  Weapon Classname		  Weapon Direct MOD		  Weapon Splash MOD			}  { DMG  AG  BC  DW  SC  Box	  CMax	  CMul	  CTime	  Cost	  Delay	  Range	  DRange  Recoil  Spread   Speed		  Weapon Classname			  Weapon Direct MOD		  Weapon Splash MOD			} *
	{ WP_BRYAR_PISTOL		, 0	, 750		, WPS_SECONDARY		, AMMO_BLASTER		, qfalse	, qfalse	, qtrue		, qtrue		, qfalse	, 1.0f,     0.7f,   { 60	, 0	, 0	, 0 , 0 , 0		, 0		, 0		, 0		, 2		, 500	, WPR_S	, 0		, 2.50f	, 0		, 0			, "bryar_proj"			, MOD_BRYAR_PISTOL		, 0							}, { 22	, 0	, 0 , 0 , 0	, 0		, 1500	, 1.5f	, 300	, 3		, 800	, WPR_S	, 0		, 3.50f	, 0		 , 0			, "bryar_proj"				, MOD_BRYAR_PISTOL_ALT	, 0							}},
	{ WP_BLASTER			, 0	, 1500		, WPS_SECONDARY		, AMMO_BLASTER		, qfalse	, qfalse	, qtrue		, qtrue		, qfalse	, 1.0f,     0.7f,   { 21	, 0	, 0	, 0 , 0 , 0		, 0		, 0		, 0		, 1		, 300	, 0		, 0		, 0.80f	, 1.00f	, 0			, "blaster_proj"		, MOD_BLASTER			, 0							}, { 15	, 0	, 0 , 0 , 0	, 0		, 0		, 0		, 0		, 1		, 100	, 0		, 0		, 0.80f	, 1.00f	 , 0			, "blaster_proj"			, MOD_BLASTER			, 0							}},

	/* Weapon Base Index      Mod Reload	  Weapon Slot		  Ammo Index		  Cookable	  KnockBack	  Roll		  Secondary	  Zoom		  Speed,    Reload, { DMG	  AG  BC  DW  SC  Box	  CMax	  CMul	CTime	  Cost	  Delay	  Range	  DRange  Recoil  Spread  Speed		  Weapon Classname		  Weapon Direct MOD		  Weapon Splash MOD			}  { DMG  AG  BC  DW  SC  Box	  CMax	  CMul	  CTime	  Cost	  Delay	  Range	  DRange  Recoil  Spread   Speed		  Weapon Classname			  Weapon Direct MOD		  Weapon Splash MOD			} *
	{ WP_BOWCASTER			, 0	, 1000		, WPS_PRIMARY		, AMMO_POWERCELL	, qfalse	, qfalse	, qfalse	, qfalse	, qtrue		, 1.0f,     0.7f,   { 26	, 0	, 0	, 0 , 0 , 2.0f	, 0		, 0		, 0		, 1		, 300	, 0		, 0		, 0.85f	, 0.85f	, 0			, "bowcaster_proj"		, MOD_BOWCASTER			, 0							}, { -1	, 0	, 0 , 0 , -1, -1	, -1	, -1	, -1	, -1	, -1	, -1	, 0		, -1	, -1	 , -1			, "bowcaster_alt_proj"		, MOD_BOWCASTER			, 0							}},
	{ WP_DEMP2				, 0	, 750		, WPS_PRIMARY		, AMMO_POWERCELL	, qfalse	, qfalse	, qfalse	, qfalse	, qfalse	, 0.95f,    0.5f,   { 30	, 0	, 0	, 0 , 0 , 0		, 0		, 0		, 0		, 1		, 200	, 0		, 0		, 0.85f	, 0.85f	, 0			, "demp2_proj"			, MOD_DEMP2				, 0							}, { -1	, 0	, 0 , 0 , -1, -1	, -1	, -1	, -1	, -1	, -1	, -1	, 0		, -1	, -1	 , -1			, "demp2_alt_proj"			, MOD_DEMP2_ALT			, 0							}},
	{ WP_DISRUPTOR			, 0	, 1500		, WPS_PRIMARY		, AMMO_POWERCELL	, qfalse	, qfalse	, qfalse	, qtrue		, qtrue		, 1.0f,     0.7f,   { 45	, 0	, 0	, 1 , 0 , 0		, 0		, 0		, 0		, 4		, 1000	, WPR_L	, 0		, 2.00f	, 0		, 0			, ""					, MOD_DISRUPTOR			, MOD_DISRUPTOR_SPLASH		}, { 18	, 0	, 0 , 1 , 0	, 0		, 2000	, 1.2f	, 200	, 1		, 1000	, WPR_L	, 0		, 2.50f	, 0		 , 0			, ""						, MOD_DISRUPTOR_SNIPER	, MOD_DISRUPTOR_SPLASH		}},
	{ WP_FLECHETTE			, 0	, 2000		, WPS_PRIMARY		, AMMO_METAL_BOLTS	, qfalse	, qfalse	, qfalse	, qfalse	, qfalse	, 0.95f,    0.6f,   { 10	, 1	, 0	, 0 , 10, 0		, 0		, 0		, 0		, 10	, 400	, WPR_I	, 0		, 5.00f	, 3.5f	, 0			, "flech_proj"			, MOD_FLECHETTE			, 0							}, { -1	, 0	, 0 , 0 , -1, -1	, -1	, -1	, -1	, -1	, -1	, -1	, 0		, -1	, -1	 , -1			, "flech_alt"				, 0						, MOD_FLECHETTE_ALT_SPLASH	}},

	/* Weapon Base Index      Mod Reload	  Weapon Slot		  Ammo Index		  Cookable	  KnockBack	  Roll		  Secondary	  Zoom		  Speed,    Reload, { DMG	  AG  BC  DW  SC  Box	  CMax	  CMul	CTime	  Cost	  Delay	  Range	  DRange  Recoil  Spread  Speed		  Weapon Classname		  Weapon Direct MOD		  Weapon Splash MOD			}  { DMG  AG  BC  DW  SC  Box	  CMax	  CMul	  CTime	  Cost	  Delay	  Range	  DRange  Recoil  Spread   Speed		  Weapon Classname			  Weapon Direct MOD		  Weapon Splash MOD			} *
	{ WP_CONCUSSION			, 0	, 3000		, WPS_PRIMARY		, AMMO_CONCUSSION	, qfalse	, qtrue		, qfalse	, qfalse	, qfalse	, 0.8f,     0.4f,   { 150	, 0	, 0	, 0 , 0 , 3.0f	, 0		, 0		, 0		, 1 	, 4000	, WPR_L	, 300	, 9.00f	, 1.15f	, 3000.0f	, "conc_proj"			, MOD_CONC				, MOD_CONC					}, { -1	, -1, -1, -1, -1, -1	, -1	, -1	, -1	, -1	, -1	, -1	, -1	, -1	, -1	 , -1			, ""        				, MOD_CONC_ALT			, 0							}},
	{ WP_REPEATER			, 0	, 1500		, WPS_PRIMARY		, AMMO_METAL_BOLTS	, qfalse	, qfalse	, qfalse	, qtrue		, qfalse	, 0.9f,     0.7f,   { 12	, 0	, 0	, 0 , 0 , 0		, 0		, 0		, 0		, 1		, 75	, 0		, 0		, 0.65f	, 1.05f	, 0			, "repeater_proj"		, MOD_REPEATER			, 0							}, { 100, 1	, 0 , 0 , 0	, 0		, 0		, 0		, 0		, 50	, 1000	, WPR_L	, 64	,9.00f	, 1.15f	 , 2750.0f		, "repeater_alt_proj"		, MOD_REPEATER_ALT		, MOD_REPEATER_ALT_SPLASH	}},
	{ WP_ROCKET_LAUNCHER	, 0	, 4000		, WPS_PRIMARY		, AMMO_ROCKETS		, qfalse	, qtrue		, qfalse	, qfalse	, qfalse	, 0.5f,     0.4f,   { 250	, 1	, 0	, 0 , 0 , 3.0f	, 0		, 0		, 0		, 1		, 1000	, WPR_L	, 256	, 9.00f	, 1.15f	, 3000.0f	, "rocket_proj"			, MOD_ROCKET			, MOD_ROCKET_SPLASH			}, { -1	, 0	, 0 , 0 , -1, -1	, -1	, -1	, -1	, -1	, -1	, -1	, 0		, -1	, -1	 , -1			, "rocket_proj"				, MOD_ROCKET_HOMING		, MOD_ROCKET_HOMING_SPLASH	}},

	/* Weapon Base Index      Mod Reload	  Weapon Slot		  Ammo Index		  Cookable	  KnockBack	  Roll		  Secondary	  Zoom		  Speed,    Reload, { DMG	  AG  BC  DW  SC  Box	  CMax	  CMul	CTime	  Cost	  Delay	  Range	  DRange  Recoil  Spread  Speed		  Weapon Classname		  Weapon Direct MOD		  Weapon Splash MOD			}  { DMG  AG  BC  DW  SC  Box	  CMax	  CMul	  CTime	  Cost	  Delay	  Range	  DRange  Recoil  Spread   Speed		  Weapon Classname			  Weapon Direct MOD		  Weapon Splash MOD			} *
	{ WP_THERMAL			, 0	, 3000		, WPS_GRENADE		, AMMO_THERMAL		, qtrue		, qfalse	, qtrue		, qtrue		, qfalse	, 1.0f,     1.0f,   { 350	, 1	, -5, 0 , 0 , 3.0f	, 3000	, 0		, 3000	, 1		, 1000	, 0		, 386	, 0		, 0		, 900.0f	, "thermal_detonator"	, MOD_THERMAL			, MOD_THERMAL_SPLASH		}, { 250, 1	, -5, 0 , 0 , 3.0f	, 3000	, 0		, 3000	, 1		, 1000	, 0		, 386	, 0		, 0		 , 900.0f		, "thermal_detonator"		, MOD_THERMAL			, MOD_THERMAL_SPLASH		}},
	{ WP_TRIP_MINE			, 0	, 0			, WPS_GRENADE		, AMMO_TRIPMINE		, qfalse	, qfalse	, qtrue		, qtrue		, qfalse	, 1.0f,     1.0f,   { 150	, 0	, 0	, 0 , 0 , 0		, 0		, 0		, 0		, 1		, 1000	, 0		, 384	, 0		, 0		, 0			, "laserTrap"			, 0						, MOD_TRIP_MINE_SPLASH		}, { 150, 0	, 0 , 0 , 0	, 0		, 0		, 0		, 0		, 1		, 1000	, 0		, 384	, 0		, 0		 , 0			, "laserTrap"				, 0						, MOD_TRIP_MINE_SPLASH		}},
	{ WP_DET_PACK			, 0	, 0			, WPS_GRENADE		, AMMO_DETPACK		, qfalse	, qfalse	, qtrue		, qtrue		, qfalse	, 1.0f,     1.0f,   { 200	, 0	, 0	, 0 , 0 , 0		, 0		, 0		, 0		, 1		, 1000	, 0		, 512	, 0		, 0		, 0			, "detpack"				, 0						, MOD_DET_PACK_SPLASH		}, { -1	, -1, -1, -1, -1, -1	, -1	, -1	, -1	, -1	, -1	, -1	, -1	, -1	, -1	 , -1			, "detpack"					, 0						, 0							}},

	/* Weapon Base Index      Mod Reload	  Weapon Slot		  Ammo Index		  Cookable	  KnockBack	  Roll		  Secondary	  Zoom		  Speed,    Reload, { DMG	  AG  BC  DW  SC  Box	  CMax	  CMul	CTime	  Cost	  Delay	  Range	  DRange  Recoil  Spread  Speed		  Weapon Classname		  Weapon Direct MOD		  Weapon Splash MOD			}  { DMG  AG  BC  DW  SC  Box	  CMax	  CMul	  CTime	  Cost	  Delay	  Range	  DRange  Recoil  Spread   Speed		  Weapon Classname			  Weapon Direct MOD		  Weapon Splash MOD			} *
	{ WP_BRYAR_OLD			, 0	, 750		, WPS_SECONDARY		, AMMO_BLASTER		, qfalse	, qfalse	, qtrue		, qtrue		, qfalse	, 1.0f,     1.0f,   { 60	, 0	, 0	, 0 , 0 , 0		, 0		, 0		, 0		, 2		, 500	, WPR_S	, 0		, 2.50f	, 0		, 0			, "bryar_proj"			, MOD_BRYAR_PISTOL		, 0							}, { 22	, 0	, 0 , 0 , 0	, 0		, 1500	, 1.5f	, 300	, 3		, 800	, WPR_S	, 0		, 3.50f	, 0		 , 0			, "bryar_proj"				, MOD_BRYAR_PISTOL_ALT	, 0							}},	// Pistol
	{ WP_EMPLACED_GUN		, 0	, 1500		, WPS_SECONDARY		, AMMO_BLASTER		, qfalse	, qfalse	, qtrue		, qtrue		, qfalse	, 1.0f,     1.0f,   { 25	, 0	, 0	, 0 , 0 , 0		, 0		, 0		, 0		, 1		, 100	, WPR_L	, 0		, 0.80f	, 1.00f	, 0			, "emplaced_gun_proj"	, MOD_TURBLAST			, 0							}, { -1	, -1, -1, -1, -1, -1	, -1	, -1	, -1	, -1	, -1	, -1	, -1	, -1	, -1	 , -1			, "detpack"					, 0						, 0							}}, // E-11 Alt
	{ WP_TURRET				, 0	, 1500		, WPS_SECONDARY		, AMMO_BLASTER		, qfalse	, qfalse	, qtrue		, qtrue		, qfalse	, 1.0f,     1.0f,   { 25	, 0	, 0	, 0 , 0 , 0		, 0		, 0		, 0		, 1		, 100	, WPR_L	, 0		, 0.80f	, 1.00f	, 0			, ""					, MOD_TURBLAST			, 0							}, { -1	, -1, -1, -1, -1, -1	, -1	, -1	, -1	, -1	, -1	, -1	, -1	, -1	, -1	 , -1			, "detpack"					, 0						, 0							}}, // E-11 Alt

};*/

//static const unsigned int numWeapons = sizeof (xweaponData) / sizeof (xweaponData[0]);


/**************************************************
* GetWeaponInitialization
*
* Initialization routine for the weapon table. This
* will verify weapon variations and will copy base
* stats when a field is marked as no change (-1). It
* will also handle non-secondary zoom-able weapons,
* which need the data of the primary fire.
**************************************************/

/*void GetWeaponInitialization( void )
{
	weaponData_t	*baseWeapon;
	unsigned int				 i = 0;

	for ( i = 0; i < numWeapons; i++ )
	{
		/* This weapon is a variation on a base weapon, check it's parameters or copy them from the base weapon. *
		if ( xweaponData[i].weaponModIndex )
		{
			/* Get the base weapon of this variation, this is mod index 0 as always *
			baseWeapon									= GetWeaponData( xweaponData[i].weaponBaseIndex, 0 );

			/* Check the primary fire parameters, anything marked as unchanged will be copied of the master *
			xweaponData[i].primary.baseDamage			= ( xweaponData[i].primary.baseDamage == -1 )			? baseWeapon->secondary.baseDamage			: xweaponData[i].primary.baseDamage;
			xweaponData[i].primary.applyGravity			= ( xweaponData[i].primary.applyGravity == -1 )			? baseWeapon->secondary.applyGravity		: xweaponData[i].primary.applyGravity;
			xweaponData[i].primary.bounceCount			= ( xweaponData[i].primary.bounceCount == -1 )			? baseWeapon->secondary.bounceCount			: xweaponData[i].primary.bounceCount;
			xweaponData[i].primary.disruptionWeapon		= ( xweaponData[i].primary.disruptionWeapon == -1 )		? baseWeapon->secondary.disruptionWeapon	: xweaponData[i].primary.disruptionWeapon;
			xweaponData[i].primary.shotCount			= ( xweaponData[i].primary.shotCount == -1 )			? baseWeapon->secondary.shotCount			: xweaponData[i].primary.shotCount;
			xweaponData[i].primary.boxSize				= ( xweaponData[i].primary.boxSize == -1 )				? baseWeapon->secondary.boxSize				: xweaponData[i].primary.boxSize;
			xweaponData[i].primary.chargeMaximum		= ( xweaponData[i].primary.chargeMaximum == -1 )		? baseWeapon->secondary.chargeMaximum		: xweaponData[i].primary.chargeMaximum;
			xweaponData[i].primary.chargeMultiplier		= ( xweaponData[i].primary.chargeMultiplier == -1 )		? baseWeapon->secondary.chargeMultiplier	: xweaponData[i].primary.chargeMultiplier;
			xweaponData[i].primary.chargeTime			= ( xweaponData[i].primary.chargeTime == -1 )			? baseWeapon->secondary.chargeTime			: xweaponData[i].primary.chargeTime;
			xweaponData[i].primary.cost					= ( xweaponData[i].primary.cost == -1 )					? baseWeapon->secondary.cost				: xweaponData[i].primary.cost;
			xweaponData[i].primary.delay				= ( xweaponData[i].primary.delay == -1 )				? baseWeapon->secondary.delay				: xweaponData[i].primary.delay;
			xweaponData[i].primary.range				= ( xweaponData[i].primary.range == -1 )				? baseWeapon->secondary.range				: xweaponData[i].primary.range;
			xweaponData[i].primary.rangeSplash			= ( xweaponData[i].primary.rangeSplash == -1 )			? baseWeapon->secondary.rangeSplash			: xweaponData[i].primary.rangeSplash;
			xweaponData[i].primary.recoil				= ( xweaponData[i].primary.recoil == -1 )				? baseWeapon->secondary.recoil				: xweaponData[i].primary.recoil;
			xweaponData[i].primary.spread				= ( xweaponData[i].primary.spread == -1 )				? baseWeapon->secondary.spread				: xweaponData[i].primary.spread;
			xweaponData[i].primary.speed				= ( xweaponData[i].primary.speed == -1 )				? baseWeapon->secondary.speed				: xweaponData[i].primary.speed;
			if ( !xweaponData[i].primary.weaponClass[0] )
			{
			    Q_strncpyz (xweaponData[i].primary.weaponClass, baseWeapon->secondary.weaponClass, sizeof (xweaponData[i].primary.weaponClass));
			}
			xweaponData[i].primary.weaponMOD			= ( xweaponData[i].primary.weaponMOD == -1 )			? baseWeapon->secondary.weaponMOD			: xweaponData[i].primary.weaponMOD;
			xweaponData[i].primary.weaponSplashMOD		= ( xweaponData[i].primary.weaponSplashMOD == -1 )		? baseWeapon->secondary.weaponSplashMOD		: xweaponData[i].primary.weaponSplashMOD;

			/* Check the primary fire parameters, anything marked as unchanged will be copied of the master *
			xweaponData[i].secondary.baseDamage			= ( xweaponData[i].secondary.baseDamage == -1 )			? baseWeapon->primary.baseDamage			: xweaponData[i].secondary.baseDamage;
			xweaponData[i].secondary.applyGravity		= ( xweaponData[i].secondary.applyGravity == -1 )		? baseWeapon->primary.applyGravity			: xweaponData[i].secondary.applyGravity;
			xweaponData[i].secondary.bounceCount		= ( xweaponData[i].secondary.bounceCount == -1 )		? baseWeapon->primary.bounceCount			: xweaponData[i].secondary.bounceCount;
			xweaponData[i].secondary.disruptionWeapon	= ( xweaponData[i].secondary.disruptionWeapon == -1 )	? baseWeapon->primary.disruptionWeapon		: xweaponData[i].secondary.disruptionWeapon;
			xweaponData[i].secondary.shotCount			= ( xweaponData[i].secondary.shotCount == -1 )			? baseWeapon->primary.shotCount				: xweaponData[i].secondary.shotCount;
			xweaponData[i].secondary.boxSize			= ( xweaponData[i].secondary.boxSize == -1 )			? baseWeapon->primary.boxSize				: xweaponData[i].secondary.boxSize;
			xweaponData[i].secondary.chargeMaximum		= ( xweaponData[i].secondary.chargeMaximum == -1 )		? baseWeapon->primary.chargeMaximum			: xweaponData[i].secondary.chargeMaximum;
			xweaponData[i].secondary.chargeMultiplier	= ( xweaponData[i].secondary.chargeMultiplier == -1 )	? baseWeapon->primary.chargeMultiplier		: xweaponData[i].secondary.chargeMultiplier;
			xweaponData[i].secondary.chargeTime			= ( xweaponData[i].secondary.chargeTime == -1 )			? baseWeapon->primary.chargeTime			: xweaponData[i].secondary.chargeTime;
			xweaponData[i].secondary.cost				= ( xweaponData[i].secondary.cost == -1 )				? baseWeapon->primary.cost					: xweaponData[i].secondary.cost;
			xweaponData[i].secondary.delay				= ( xweaponData[i].secondary.delay == -1 )				? baseWeapon->primary.delay					: xweaponData[i].secondary.delay;
			xweaponData[i].secondary.range				= ( xweaponData[i].secondary.range == -1 )				? baseWeapon->primary.range					: xweaponData[i].secondary.range;
			xweaponData[i].secondary.rangeSplash		= ( xweaponData[i].secondary.rangeSplash == -1 )		? baseWeapon->primary.rangeSplash			: xweaponData[i].secondary.rangeSplash;
			xweaponData[i].secondary.recoil				= ( xweaponData[i].secondary.recoil == -1 )				? baseWeapon->primary.recoil				: xweaponData[i].secondary.recoil;
			xweaponData[i].secondary.spread				= ( xweaponData[i].secondary.spread == -1 )				? baseWeapon->primary.spread				: xweaponData[i].secondary.spread;
			xweaponData[i].secondary.speed				= ( xweaponData[i].secondary.speed == -1 )				? baseWeapon->primary.speed					: xweaponData[i].secondary.speed;
			if ( !xweaponData[i].secondary.weaponClass[0] )
			{
			    Q_strncpyz (xweaponData[i].secondary.weaponClass, baseWeapon->primary.weaponClass, sizeof (xweaponData[i].secondary.weaponClass));
			}
			xweaponData[i].secondary.weaponMOD			= ( xweaponData[i].secondary.weaponMOD == -1 )			? baseWeapon->primary.weaponMOD				: xweaponData[i].secondary.weaponMOD;
			xweaponData[i].secondary.weaponSplashMOD	= ( xweaponData[i].secondary.weaponSplashMOD == -1 )	? baseWeapon->primary.weaponSplashMOD		: xweaponData[i].secondary.weaponSplashMOD;
		}

		/* This weapon has no secondary but has a zoom mode. Due to design, the trigger will still use secondary, so copy the weapon stats *
		if ( !xweaponData[i].hasSecondary && xweaponData[i].hasZoomMode )
		{
			memcpy( &xweaponData[i].secondary, &xweaponData[i].primary, sizeof( xweaponData[i].secondary ));
		}

		/* Verify the shot count for each weapon, if the shot count is zero we still want to fire at least one, so set it *
		xweaponData[i].primary.shotCount	= ( xweaponData[i].primary.shotCount ) ? abs( xweaponData[i].primary.shotCount ) : 1;
		xweaponData[i].secondary.shotCount	= ( xweaponData[i].secondary.shotCount ) ? abs( xweaponData[i].secondary.shotCount ) : 1;
	}
}*/

qboolean BG_WeaponVariationExists ( unsigned int weaponId, unsigned int variation )
{
    int i;
    for ( i = 0; i < numLoadedWeapons; i++ )
    {
        if ( weaponDataTable[i].weaponBaseIndex == weaponId && weaponDataTable[i].weaponModIndex == variation )
        {
            return qtrue;
        }
    }
    
    return qfalse;
}

/**************************************************
* GetWeaponData
*
* This is the main routine used in the new weapon
* table. It will search for the provided base weapon
* and its possible variation (note that the variation
* field might be bitwise). Upon failure, the client
* is dropped.
**************************************************/

weaponData_t *GetWeaponData( unsigned char baseIndex, unsigned char modIndex )
{
	static weaponData_t *lastWeapon = NULL;
	unsigned int i;

	/* Remember the last weapon to cut down in the number of lookups required */
	if ( lastWeapon && lastWeapon->weaponBaseIndex == baseIndex && lastWeapon->weaponModIndex == modIndex )
	{
		return lastWeapon;
	}

	/* Check the table to see if we can find a match, remember it and return it! */
	for ( i = 0; i < numLoadedWeapons; i++ )
	{
		if ( weaponDataTable[i].weaponBaseIndex == baseIndex && weaponDataTable[i].weaponModIndex == modIndex )
		{
			lastWeapon = &weaponDataTable[i];
			return lastWeapon;
		}
	}

	/* This is a serious error, this is a weapon we don't know! */
	Com_Error( ERR_DISCONNECT, va( "Illegal weapon %i with variation %i!", baseIndex, modIndex ));
	return NULL;
}

unsigned char GetWeaponAmmoIndex ( unsigned char baseIndex, unsigned char modIndex )
{
    return GetWeaponData( baseIndex, modIndex )->ammoIndex;
}

short GetWeaponAmmoClip ( unsigned char baseIndex, unsigned char modIndex )
{
    return xweaponAmmo[GetWeaponData( baseIndex, modIndex )->ammoIndex].ammoClipSize;
}

short GetWeaponAmmoMax ( unsigned char baseIndex, unsigned char modIndex )
{
    return xweaponAmmo[GetWeaponData( baseIndex, modIndex )->ammoIndex].ammoMax;
}

short GetAmmoMax ( unsigned char ammoIndex )
{
    return xweaponAmmo[ammoIndex].ammoMax;
}

#include <json/cJSON.h>
#include "../cgame/animtable.h"

static void BG_AddWeaponData ( weaponData_t *weaponData )
{
    memcpy (&weaponDataTable[numLoadedWeapons], weaponData, sizeof (weaponDataTable[0]));
    
    numLoadedWeapons++;
    numWeapons[weaponData->weaponBaseIndex]++;
}

// Just in case we need to initialize the struct any other way,
// it's easier to have it's own function.
static ID_INLINE void BG_InitializeWeaponData ( weaponData_t *weaponData )
{
    memset (weaponData, 0, sizeof (*weaponData));
}

static ID_INLINE void BG_ParseWeaponStatsFlags ( weaponData_t *weaponData, const char *flagStr )
{
    if ( Q_stricmp (flagStr, "cookable") == 0 )
    {
        weaponData->hasCookAbility = qtrue;
    }
    else if ( Q_stricmp (flagStr, "knockback") == 0 )
    {
        weaponData->hasKnockBack = qtrue;
    }
    else if ( Q_stricmp (flagStr, "roll") == 0 )
    {
        weaponData->hasRollAbility = qtrue;
    }
    else if ( Q_stricmp (flagStr, "zoom") == 0 )
    {
        weaponData->zoomType = ZOOM_CONTINUOUS;
    }
    else if ( Q_stricmp (flagStr, "togglezoom") == 0 )
    {
        weaponData->zoomType = ZOOM_TOGGLE;
    }
}

static const stringID_table_t MODTable[] =
{
	ENUM2STRING (MOD_STUN_BATON),
	ENUM2STRING (MOD_MELEE),
	ENUM2STRING (MOD_SABER),
	ENUM2STRING (MOD_BRYAR_PISTOL),
	ENUM2STRING (MOD_BRYAR_PISTOL_ALT),
	ENUM2STRING (MOD_BLASTER),
	ENUM2STRING (MOD_TURBLAST),
	ENUM2STRING (MOD_DISRUPTOR),
	ENUM2STRING (MOD_DISRUPTOR_SPLASH),
	ENUM2STRING (MOD_DISRUPTOR_SNIPER),
	ENUM2STRING (MOD_BOWCASTER),
	ENUM2STRING (MOD_REPEATER),
	ENUM2STRING (MOD_REPEATER_ALT),
	ENUM2STRING (MOD_REPEATER_ALT_SPLASH),
	ENUM2STRING (MOD_DEMP2),
	ENUM2STRING (MOD_DEMP2_ALT),
	ENUM2STRING (MOD_FLECHETTE),
	ENUM2STRING (MOD_FLECHETTE_ALT_SPLASH),
	ENUM2STRING (MOD_ROCKET),
	ENUM2STRING (MOD_ROCKET_SPLASH),
	ENUM2STRING (MOD_ROCKET_HOMING),
	ENUM2STRING (MOD_ROCKET_HOMING_SPLASH),
	ENUM2STRING (MOD_THERMAL),
	ENUM2STRING (MOD_THERMAL_SPLASH),
	ENUM2STRING (MOD_TRIP_MINE_SPLASH),
	ENUM2STRING (MOD_TIMED_MINE_SPLASH),
	ENUM2STRING (MOD_DET_PACK_SPLASH),
	ENUM2STRING (MOD_VEHICLE),
	ENUM2STRING (MOD_CONC),
	ENUM2STRING (MOD_CONC_ALT),
	ENUM2STRING (MOD_FORCE_DARK),
	ENUM2STRING (MOD_SENTRY),
	ENUM2STRING (MOD_WATER),
	ENUM2STRING (MOD_SLIME),
	ENUM2STRING (MOD_LAVA),
	ENUM2STRING (MOD_CRUSH),
	ENUM2STRING (MOD_TELEFRAG),
	ENUM2STRING (MOD_FALLING),
	ENUM2STRING (MOD_SUICIDE),
	ENUM2STRING (MOD_TARGET_LASER),
	ENUM2STRING (MOD_TRIGGER_HURT),
	ENUM2STRING (MOD_TEAM_CHANGE),
	
	{ NULL, -1 }
};

static const stringID_table_t AmmoTable[] =
{
    ENUM2STRING (AMMO_NONE),
	ENUM2STRING (AMMO_FORCE),
	ENUM2STRING (AMMO_BLASTER),
	ENUM2STRING (AMMO_POWERCELL),
	ENUM2STRING (AMMO_METAL_BOLTS),
	ENUM2STRING (AMMO_CONCUSSION),
	ENUM2STRING (AMMO_ROCKETS),
	ENUM2STRING (AMMO_EMPLACED),
	ENUM2STRING (AMMO_THERMAL),
	ENUM2STRING (AMMO_TRIPMINE),
	ENUM2STRING (AMMO_DETPACK),
	ENUM2STRING (AMMO_MAX),
	
	{ NULL, -1 }
};

static void BG_ParseWeaponFireMode ( weaponFireModeStats_t *fireModeStats, cJSON *fireModeNode )
{
    cJSON *node;
    const char *str = NULL;
    
    if ( fireModeNode == NULL )
    {
        return;
    }
    
    /*node = cJSON_GetObjectItem (fireModeNode, "ammo");
    str = cJSON_ToString (node);
    fireModeStats->ammo = BG_GetAmmo (str);*/

    node = cJSON_GetObjectItem (fireModeNode, "damage");
    fireModeStats->baseDamage = (short)cJSON_ToIntegerOpt (node, 0);
    
    node = cJSON_GetObjectItem (fireModeNode, "ballistic");
    fireModeStats->applyGravity = (char)cJSON_ToBooleanOpt (node, 0);
    
    node = cJSON_GetObjectItem (fireModeNode, "bounces");
    fireModeStats->bounceCount = (char)cJSON_ToIntegerOpt (node, 0);
    
    node = cJSON_GetObjectItem (fireModeNode, "disruptor");
    fireModeStats->disruptionWeapon = (char)cJSON_ToBooleanOpt (node, 0);
    
    node = cJSON_GetObjectItem (fireModeNode, "projectiles");
    fireModeStats->shotCount = (char)cJSON_ToIntegerOpt (node, 0);
    
    node = cJSON_GetObjectItem (fireModeNode, "collisionsize");
    fireModeStats->boxSize = (float)cJSON_ToNumberOpt (node, 0.0);
    
    node = cJSON_GetObjectItem (fireModeNode, "maxchargetime");
    fireModeStats->chargeMaximum = (short)cJSON_ToIntegerOpt (node, 0);
    
    node = cJSON_GetObjectItem (fireModeNode, "chargedamage");
    fireModeStats->chargeMultiplier = (float)cJSON_ToNumberOpt (node, 0);
    
    node = cJSON_GetObjectItem (fireModeNode, "chargedelay");
    fireModeStats->chargeTime = (short)cJSON_ToIntegerOpt (node, 0);
    
    node = cJSON_GetObjectItem (fireModeNode, "ammocost");
    fireModeStats->cost = (char)cJSON_ToIntegerOpt (node, 0);
    
    node = cJSON_GetObjectItem (fireModeNode, "firedelay");
    fireModeStats->delay = (short)cJSON_ToIntegerOpt (node, 0);
    
    node = cJSON_GetObjectItem (fireModeNode, "range");
    fireModeStats->range = (float)cJSON_ToIntegerOpt (node, WPR_M);
    
    node = cJSON_GetObjectItem (fireModeNode, "splashrange");
    fireModeStats->rangeSplash = (float)cJSON_ToNumberOpt (node, 0.0);
    
    node = cJSON_GetObjectItem (fireModeNode, "recoil");
    fireModeStats->recoil = (float)cJSON_ToNumberOpt (node, 0.0);
    
    node = cJSON_GetObjectItem (fireModeNode, "spread");
    fireModeStats->spread = (float)cJSON_ToNumberOpt (node, 0.0);
    
    node = cJSON_GetObjectItem (fireModeNode, "projectilespeed");
    fireModeStats->speed = (float)cJSON_ToNumberOpt (node, 0.0);
    
    node = cJSON_GetObjectItem (fireModeNode, "projectileclass");
    str = cJSON_ToStringOpt (node, "unknown_proj");
    Q_strncpyz (fireModeStats->weaponClass, str, sizeof (fireModeStats->weaponClass));
    
    node = cJSON_GetObjectItem (fireModeNode, "meansofdeath");
    str = cJSON_ToStringOpt (node, "MOD_UNKNOWN");
    fireModeStats->weaponMOD = GetIDForString (MODTable, str);
    
    node = cJSON_GetObjectItem (fireModeNode, "splashmeansofdeath");
    str = cJSON_ToStringOpt (node, "MOD_UNKNOWN");
    fireModeStats->weaponSplashMOD = GetIDForString (MODTable, str);
}

//=========================================================
// BG_ParseWeaponStats
//---------------------------------------------------------
// Description:
// Parses the stats block of a weapons file.
//=========================================================
static void BG_ParseWeaponStats ( weaponData_t *weaponData, cJSON *statsNode )
{
    cJSON *node;
    const char *flags[4];
    const char *ammo;
    
    if ( statsNode == NULL )
    {
        return;
    }

    node = cJSON_GetObjectItem (statsNode, "slot");
    weaponData->weaponSlot = (unsigned char)cJSON_ToIntegerOpt (node, 0);

    node = cJSON_GetObjectItem (statsNode, "reloadtime");
    weaponData->weaponReloadTime = (unsigned short)cJSON_ToIntegerOpt (node, 0);

    node = cJSON_GetObjectItem (statsNode, "ammo");
    ammo = cJSON_ToStringOpt (node, "AMMO_NONE");
    weaponData->ammoIndex = GetIDForString (AmmoTable, ammo);

    node = cJSON_GetObjectItem (statsNode, "flags");
    if ( node != NULL )
    {
        int numFlags = cJSON_ReadStringArray (node, 4, flags);
        int i;

        for ( i = 0; i < numFlags; i++ )
        {
            BG_ParseWeaponStatsFlags (weaponData, flags[i]);
        }
    }

    node = cJSON_GetObjectItem (statsNode, "speed");
    weaponData->speedModifier = (float)cJSON_ToNumberOpt (node, 1.0);
        
    node = cJSON_GetObjectItem (statsNode, "reloadmodifier");
    weaponData->reloadModifier = (float)cJSON_ToNumberOpt (node, 1.0);
    
    node = cJSON_GetObjectItem (statsNode, "startzoomfov");
    weaponData->startZoomFov = (float)cJSON_ToNumberOpt (node, 50.0);
    
    node = cJSON_GetObjectItem (statsNode, "endzoomfov");
    weaponData->endZoomFov = (float)cJSON_ToNumberOpt (node, weaponData->zoomType == ZOOM_TOGGLE ? weaponData->startZoomFov : 5.0);
    
    node = cJSON_GetObjectItem (statsNode, "zoomtime");
    weaponData->zoomTime = cJSON_ToIntegerOpt (node, 2000);
}

static void BG_ParseAnimationObject ( cJSON *animationNode, int *torsoAnimation, int *legsAnimation )
{
    cJSON *node = cJSON_GetObjectItem (animationNode, "torso");
    const char *anim = cJSON_ToStringOpt (node, "BOTH_STAND1");
    
    *torsoAnimation = GetIDForString (animTable, anim);
    
    node = cJSON_GetObjectItem (animationNode, "legs");
    if ( node == NULL )
    {
        *legsAnimation = *torsoAnimation;
        return;
    }
    
    anim = cJSON_ToString (node);
    *legsAnimation = GetIDForString (animTable, anim);
}

static void BG_ParseWeaponPlayerAnimations ( weaponData_t *weaponData, cJSON *playerAnimNode )
{
    cJSON *node;
    
    node = cJSON_GetObjectItem (playerAnimNode, "ready");
    BG_ParseAnimationObject (node, &weaponData->torsoReadyAnimation, &weaponData->legsReadyAnimation);
    
    node = cJSON_GetObjectItem (playerAnimNode, "firing");
    BG_ParseAnimationObject (node, &weaponData->torsoFiringAnimation, &weaponData->legsFiringAnimation);
    
    /*node = cJSON_GetObjectItem (playerAnimNode, "idle");
    BG_ParseAnimationObject (node, &weaponData->torsoFiringAnimation, &weaponData->legsFiringAnimation);
    
    node = cJSON_GetObjectItem (playerAnimNode, "drop");
    BG_ParseAnimationObject (node, &weaponData->torsoFiringAnimation, &weaponData->legsFiringAnimation);
    
    node = cJSON_GetObjectItem (playerAnimNode, "raise");
    BG_ParseAnimationObject (node, &weaponData->torsoFiringAnimation, &weaponData->legsFiringAnimation);*/
    
    node = cJSON_GetObjectItem (playerAnimNode, "reload");
    BG_ParseAnimationObject (node, &weaponData->torsoReloadAnimation, &weaponData->legsReloadAnimation);
}

#ifdef CGAME
static void ReadString ( cJSON *parent, const char *field, char *dest, size_t destSize )
{
    cJSON *node = NULL;
    const char *str = NULL;
    
    node = cJSON_GetObjectItem (parent, field);
    str = cJSON_ToString (node);
    if ( str ) Q_strncpyz (dest, str, destSize);
}

static void BG_ParseVisualsFireMode ( weaponVisualFireMode_t *fireMode, cJSON *fireModeNode )
{
    cJSON *node = NULL;
    cJSON *child = NULL;
    qboolean isGrenade = qfalse;
    qboolean isBlaster = qfalse;
    qboolean isTripmine = qfalse;
    qboolean isDetpack = qfalse;

    ReadString (fireModeNode, "type", fireMode->type, sizeof (fireMode->type));
    isGrenade = (qboolean)(Q_stricmp (fireMode->type, "grenade") == 0);
    isBlaster = (qboolean)(Q_stricmp (fireMode->type, "blaster") == 0);
    isTripmine = (qboolean)(Q_stricmp (fireMode->type, "tripmine") == 0);
    isDetpack = (qboolean)(Q_stricmp (fireMode->type, "detpack") == 0);
    
    // TODO: Need to tie this to the table in cg_weapons.c somehow...
    // Weapon Render
    node = cJSON_GetObjectItem (fireModeNode, "muzzlelightintensity");
    fireMode->weaponRender.generic.muzzleLightIntensity = (float)cJSON_ToNumber (node);
    
    ReadString (fireModeNode, "muzzlelightcolor", fireMode->weaponRender.generic.muzzleLightColor, sizeof (fireMode->weaponRender.generic.muzzleLightColor));
    ReadString (fireModeNode, "chargingfx", fireMode->weaponRender.generic.chargingEffect, sizeof (fireMode->weaponRender.generic.chargingEffect));    
    ReadString (fireModeNode, "muzzlefx", fireMode->weaponRender.generic.muzzleEffect, sizeof (fireMode->weaponRender.generic.muzzleEffect));
    
    // Weapon Fire
    child = cJSON_GetObjectItem (fireModeNode, "firesound");
    if ( child )
    {
        if ( cJSON_IsArray (child) )
        {
            const char *fireSounds[8];
            int numFireSounds = cJSON_ReadStringArray (child, 8, fireSounds);
            int i;
            
            for ( i = 0; i < numFireSounds; i++ )
            {
                Q_strncpyz (fireMode->weaponFire.generic.fireSound[i], fireSounds[i], sizeof (fireMode->weaponFire.generic.fireSound[i]));
            }
        }
        else
        {
            const char *s = cJSON_ToString (child);
            if ( s ) Q_strncpyz (fireMode->weaponFire.generic.fireSound[0], s, sizeof (fireMode->weaponFire.generic.fireSound[0]));
        }
    }
    
    // Traceline Render
    ReadString (fireModeNode, "tracelineshader", fireMode->tracelineRender.generic.tracelineShader, sizeof (fireMode->tracelineRender.generic.tracelineShader));
    
    node = cJSON_GetObjectItem (fireModeNode, "minsize");
    fireMode->tracelineRender.generic.minSize = (float)cJSON_ToNumber (node);
    
    node = cJSON_GetObjectItem (fireModeNode, "maxsize");
    fireMode->tracelineRender.generic.maxSize = (float)cJSON_ToNumber (node);
    
    node = cJSON_GetObjectItem (fireModeNode, "lifetime");
    fireMode->tracelineRender.generic.lifeTime = cJSON_ToInteger (node);
    
    // Weapon Charge
    ReadString (fireModeNode, "chargingsound", fireMode->weaponCharge.chargingSound, sizeof (fireMode->weaponCharge.chargingSound));
    
    // Projectile render
    ReadString (fireModeNode, "projectilemodel", fireMode->projectileRender.generic.projectileModel, sizeof (fireMode->projectileRender.generic.projectileModel));
    ReadString (fireModeNode, "projectilefx", fireMode->projectileRender.generic.projectileEffect, sizeof (fireMode->projectileRender.generic.projectileEffect));
    ReadString (fireModeNode, "runsound", fireMode->projectileRender.generic.runSound, sizeof (fireMode->projectileRender.generic.runSound));
    
    node = cJSON_GetObjectItem (fireModeNode, "lightintensity");
    fireMode->projectileRender.generic.lightIntensity = (float)cJSON_ToNumber (node);
    
    ReadString (fireModeNode, "lightcolor", fireMode->projectileRender.generic.lightColor, sizeof (fireMode->projectileRender.generic.lightColor));
  
    ReadString (fireModeNode, "deathfx", fireMode->projectileRender.generic.deathEffect, sizeof (fireMode->projectileRender.generic.deathEffect));
    
    // Projectile miss event
    child = cJSON_GetObjectItem (fireModeNode, "miss");
    if ( isTripmine || isDetpack )
    {
        ReadString (child, "sticksound", fireMode->projectileMiss.explosive.stickSound, sizeof (fireMode->projectileMiss.explosive.stickSound));
    }
    else
    {
        ReadString (child, "impactfx", fireMode->projectileMiss.generic.impactEffect, sizeof (fireMode->projectileMiss.generic.impactEffect));
        ReadString (child, "shockwavefx", fireMode->projectileMiss.grenade.shockwaveEffect, sizeof (fireMode->projectileMiss.grenade.shockwaveEffect));
    }
    
    // Projectile hit event
    child = cJSON_GetObjectItem (fireModeNode, "hit");
    ReadString (child, "impactfx", fireMode->projectileHitPlayer.generic.impactEffect, sizeof (fireMode->projectileHitPlayer.generic.impactEffect));
    ReadString (child, "shockwavefx", fireMode->projectileHitPlayer.grenade.shockwaveEffect, sizeof (fireMode->projectileHitPlayer.grenade.shockwaveEffect));
    
    // Projectile deflected event
    ReadString (fireModeNode, "deflectedfx", fireMode->projectileDeflected.generic.deflectEffect, sizeof (fireMode->projectileDeflected.generic.deflectEffect));
    
    // Grenade bounce event
    child = cJSON_GetObjectItem (fireModeNode, "bouncesound");
    if ( child )
    {
        const char *bounceSounds[2];
        int numBounceSounds = cJSON_ReadStringArray (child, 2, bounceSounds);
        int i;
        
        for ( i = 0; i < numBounceSounds; i++ )
        {
            Q_strncpyz (fireMode->grenadeBounce.grenade.bounceSound[i], bounceSounds[i], sizeof (fireMode->grenadeBounce.grenade.bounceSound[i]));
        }
    }
    
    // Explosive render
    ReadString (fireModeNode, "g2model", fireMode->explosiveRender.detpack.g2Model, sizeof (fireMode->explosiveRender.detpack.g2Model));
    
    node = cJSON_GetObjectItem (fireModeNode, "g2radius");
    fireMode->explosiveRender.detpack.g2Radius = (float)cJSON_ToNumber (node);
    
    ReadString (fireModeNode, "linefx", fireMode->explosiveRender.tripmine.lineEffect, sizeof (fireMode->explosiveRender.tripmine.lineEffect));
    
    // Explosive blow event
    ReadString (fireModeNode, "explodefx", fireMode->explosiveBlow.generic.explodeEffect, sizeof (fireMode->explosiveBlow.generic.explodeEffect));
    
    // Explosive armed event
    ReadString (fireModeNode, "armsound", fireMode->explosiveArm.armSound, sizeof (fireMode->explosiveArm.armSound));
}

static void BG_ParseVisuals ( weaponData_t *weaponData, cJSON *visualsNode )
{
    cJSON *node = NULL;
    cJSON *child = NULL;
    weaponVisual_t *weaponVisuals = &weaponData->visuals;

    ReadString (visualsNode, "worldmodel", weaponVisuals->world_model, sizeof (weaponVisuals->world_model));
    ReadString (visualsNode, "viewmodel", weaponVisuals->view_model, sizeof (weaponVisuals->view_model));
    ReadString (visualsNode, "hudicon", weaponVisuals->icon, sizeof (weaponVisuals->icon));
    ReadString (visualsNode, "hudnaicon", weaponVisuals->icon_na, sizeof (weaponVisuals->icon_na));
    ReadString (visualsNode, "selectsound", weaponVisuals->selectSound, sizeof (weaponVisuals->selectSound));
    
    child = cJSON_GetObjectItem (visualsNode, "indicators");
    if ( child )
    {
        int numGroupIndicators = 0;
        int i;
        char *shaders[3];
        cJSON *subchild = cJSON_GetObjectItem (child, "ammo");
        if ( subchild )
        {
            weaponVisuals->indicatorType = IND_NORMAL;
            numGroupIndicators = cJSON_ReadStringArray (subchild, 3, shaders);
            for ( i = 0; i < numGroupIndicators; i++ )
            {
                Q_strncpyz (weaponVisuals->groupedIndicatorShaders[i], shaders[i], sizeof (weaponVisuals->groupedIndicatorShaders[i]));
            }
        }
        else if ( subchild = cJSON_GetObjectItem (child, "leds") )
        {
            weaponVisuals->indicatorType = IND_GRENADE;
            numGroupIndicators = cJSON_ReadStringArray (subchild, 3, shaders);
            for ( i = 0; i < numGroupIndicators; i++ )
            {
                Q_strncpyz (weaponVisuals->groupedIndicatorShaders[i], shaders[i], sizeof (weaponVisuals->groupedIndicatorShaders[i]));
            }
        }
        
        ReadString (child, "firemode", weaponVisuals->firemodeIndicatorShader, sizeof (weaponVisuals->firemodeIndicatorShader));
    }
    
    ReadString (visualsNode, "gunposition", weaponVisuals->gunPosition, sizeof (weaponVisuals->gunPosition));
    ReadString (visualsNode, "ironsightsPosition", weaponVisuals->ironsightsPosition, sizeof (weaponVisuals->ironsightsPosition));
    
    node = cJSON_GetObjectItem (visualsNode, "ironsightsFov");
    weaponVisuals->ironsightsFov = (float)cJSON_ToNumberOpt (node, 0.0);
    
    // Scope toggle
    child = cJSON_GetObjectItem (visualsNode, "scope");
    ReadString (child, "startsound", weaponVisuals->scopeStartSound, sizeof (weaponVisuals->scopeStartSound));
    ReadString (child, "stopsound", weaponVisuals->scopeStopSound, sizeof (weaponVisuals->scopeStopSound));
    
    // Scope zoom
    node = cJSON_GetObjectItem (child, "looptime");
    weaponVisuals->scopeSoundLoopTime = cJSON_ToInteger (node);
    
    ReadString (child, "loopsound", weaponVisuals->scopeLoopSound, sizeof (weaponVisuals->scopeLoopSound));
    
    // Scope render
    ReadString (child, "mask", weaponVisuals->scopeShader, sizeof (weaponVisuals->scopeShader));
    
    node = cJSON_GetObjectItem (visualsNode, "primary");
    BG_ParseVisualsFireMode (&weaponVisuals->primary, node);
    
    node = cJSON_GetObjectItem (visualsNode, "secondary");
    BG_ParseVisualsFireMode (&weaponVisuals->secondary, node);
}
#endif

stringID_table_t WPTable[]; // From bg_saga.c

#define MAX_WEAPON_FILE_LENGTH (16384) // 16kb should be enough, 4kb apparently wasn't!
static qboolean BG_ParseWeaponFile ( const char *weaponFilePath )
{
    cJSON *json = NULL;
    cJSON *jsonNode = NULL;
    char error[MAX_STRING_CHARS];
    const char *str = NULL;
    int weapon;
    
    char weaponFileData[MAX_WEAPON_FILE_LENGTH];
    fileHandle_t f;
    int fileLen = strap_FS_FOpenFile (weaponFilePath, &f, FS_READ);
    
    weaponData_t weaponData;
    
    if ( !f || fileLen == -1 )
    {
        Com_Printf (S_COLOR_RED "%s: failed to read the weapon file. File is unreadable or is empty.\n", weaponFilePath);
        return qfalse;
    }
    
    if ( (fileLen + 1) >= MAX_WEAPON_FILE_LENGTH )
    {
        trap_FS_FCloseFile (f);
        Com_Printf (S_COLOR_RED "%s: file too big (%d bytes, maximum is %d).\n", weaponFilePath, fileLen, MAX_WEAPON_FILE_LENGTH - 1);
        
        return qfalse;
    }
    
    strap_FS_Read (&weaponFileData, fileLen, f);
    weaponFileData[fileLen] = '\0';
    
    strap_FS_FCloseFile (f);
    
    json = cJSON_ParsePooled (weaponFileData, error, sizeof (error));
    if ( json == NULL )
    {
        Com_Printf (S_COLOR_RED "%s: %s\n", weaponFilePath, error);
        
        return qfalse;
    }
    
    BG_InitializeWeaponData (&weaponData);
    
    jsonNode = cJSON_GetObjectItem (json, "type");
    str = cJSON_ToString (jsonNode);
    weapon = GetIDForString (WPTable, str);
    weaponData.weaponBaseIndex = weapon;
    
    jsonNode = cJSON_GetObjectItem (json, "variation");
    weapon = cJSON_ToNumber (jsonNode);
    weaponData.weaponModIndex = weapon;

    jsonNode = cJSON_GetObjectItem (json, "stats");
    BG_ParseWeaponStats (&weaponData, jsonNode);
    
    jsonNode = cJSON_GetObjectItem (json, "primaryattack");
    BG_ParseWeaponFireMode (&weaponData.primary, jsonNode);
    
    jsonNode = cJSON_GetObjectItem (json, "secondaryattack");
    if ( jsonNode != NULL )
    {
        weaponData.hasSecondary = 1;
        BG_ParseWeaponFireMode (&weaponData.secondary, jsonNode);
    }
    
    jsonNode = cJSON_GetObjectItem (json, "playeranims");
    BG_ParseWeaponPlayerAnimations (&weaponData, jsonNode);
    
#ifdef CGAME
    // TODO: Maybe we can turn this into a loop somehow? It's just turning into a stupidly long list.

    jsonNode = cJSON_GetObjectItem (json, "weaponanims");
    // TODO

    jsonNode = cJSON_GetObjectItem (json, "name");
    str = cJSON_ToString (jsonNode);
    Q_strncpyz (weaponData.visuals.displayName, str, sizeof (weaponData.visuals.displayName));
    
    jsonNode = cJSON_GetObjectItem (json, "description");
    str = cJSON_ToString (jsonNode);
    Q_strncpyz (weaponData.visuals.description, str, sizeof (weaponData.visuals.description));
    
    jsonNode = cJSON_GetObjectItem (json, "visual");
    BG_ParseVisuals (&weaponData, jsonNode);
#endif
    
    if ( /*!weaponData.hasSecondary && */weaponData.zoomType != ZOOM_NONE )
    {
        // If we have zoom mode, then copy over the data from the primary to the secondary
        // so it's as if we haven't changed fire modes at all! Ingenious! (And also temporary)
        memcpy (&weaponData.secondary, &weaponData.primary, sizeof (weaponData.secondary));
    }
    
    BG_AddWeaponData (&weaponData);
    
    cJSON_Delete (json);
    
    return qtrue;
}

static qboolean BG_LoadWeapons ( void )
{
    int i;
    char weaponFiles[8192];
    int numFiles = strap_FS_GetFileList ("ext_data/weapons", ".wpn", weaponFiles, sizeof (weaponFiles));
    const char *weaponFile = weaponFiles;
    int successful = 0;
    int failed = 0;
    
    Com_Printf ("------- Weapon Initialization -------\n");
    
    for ( i = 0; i < numFiles; i++ )
    {
        if ( BG_ParseWeaponFile (va ("ext_data/weapons/%s", weaponFile)) )
        {
            successful++;
        }
        else
        {
            failed++;
        }
        
        weaponFile += strlen (weaponFile) + 1;
    }
    
    Com_Printf ("Successfully loaded %d weapons, failed to load %d weapons.\n", successful, failed);
    Com_Printf ("-------------------------------------\n");
    
    return (qboolean)(numFiles > 0);
}

void BG_InitializeWeapons ( void )
{
    weaponData_t predefinedWeapons;
    
    //BG_InitializeAmmo();

    memset (numWeapons, 0, sizeof (numWeapons));
    memset (weaponDataTable, 0, sizeof (weaponDataTable));
    memset (&predefinedWeapons, 0, sizeof (predefinedWeapons));
    
    predefinedWeapons.weaponBaseIndex = WP_NONE;
    predefinedWeapons.torsoReadyAnimation = BOTH_STAND1;
    predefinedWeapons.legsReadyAnimation = BOTH_STAND1;
    BG_AddWeaponData (&predefinedWeapons);
    
    predefinedWeapons.weaponBaseIndex = WP_EMPLACED_GUN;
    predefinedWeapons.torsoReadyAnimation = BOTH_STAND1;
    predefinedWeapons.legsReadyAnimation = BOTH_STAND1;
    BG_AddWeaponData (&predefinedWeapons);
    
    predefinedWeapons.weaponBaseIndex = WP_TURRET;
    predefinedWeapons.torsoReadyAnimation = BOTH_STAND1;
    predefinedWeapons.legsReadyAnimation = BOTH_STAND1;
    BG_AddWeaponData (&predefinedWeapons);
    
    if ( !BG_LoadWeapons() )
    {
        Com_Error (ERR_DISCONNECT, "No weapon files parsed.");
        return;
    }
}

/*
// Muzzle point table...
vec3_t WP_MuzzlePoint[WP_NUM_WEAPONS] = 
{//	Fwd,	right,	up.
	{0,		0,		0	},	// WP_NONE,
	{0	,	8,		0	},	// WP_STUN_BATON,
	{0	,	8,		0	},	// WP_MELEE,
	{8	,	16,		0	},	// WP_SABER,				 
	{12,	6,		-6	},	// WP_BRYAR_PISTOL,
	{12,	6,		-6	},	// WP_BLASTER,
	{12,	6,		-6	},	// WP_DISRUPTOR,
	{12,	2,		-6	},	// WP_BOWCASTER,
	{12,	4.5,	-6	},	// WP_REPEATER,
	{12,	6,		-6	},	// WP_DEMP2,
	{12,	6,		-6	},	// WP_FLECHETTE,
	{12,	8,		-4	},	// WP_ROCKET_LAUNCHER,
	{12,	0,		-4	},	// WP_THERMAL,
	{12,	0,		-10	},	// WP_TRIP_MINE,
	{12,	0,		-4	},	// WP_DET_PACK,
	{12,	6,		-6	},	// WP_CONCUSSION
	{12,	6,		-6	},	// WP_BRYAR_OLD,
};
*/