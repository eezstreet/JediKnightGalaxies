// Copyright (C) 2001-2002 Raven Software
//
// bg_weapons.c -- part of bg_pmove functionality

#include "q_shared.h"
#include "bg_public.h"
#include "bg_local.h"
#include "bg_ammo.h"

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

weaponData_t *BG_GetWeaponDataByIndex( int index )
{
	if(index >= 0 && index < numLoadedWeapons)
	{
		return &weaponDataTable[index];
	}
	return NULL;
}

//Xycaleth/eezstreet add
qboolean BG_GetWeaponByIndex ( int index, int *weapon, int *variation ) { 
	weaponData_t *weaponData = NULL;

	if(index >= MAX_WEAPON_TABLE_SIZE || index < 0)
		return qfalse;
		
	weaponData = &weaponDataTable[index]; 

	*weapon = weaponData->weaponBaseIndex; 
	*variation = weaponData->weaponModIndex; 
	return qtrue;
}
//Xycaleth end

int BG_GetWeaponIndexFromClass ( int weapon, int variation )
{
    static int lastWeapon = 0;
    static int lastVariation = 0;
    static int lastIndex = -1;
    
    if ( lastIndex != -1 && weapon == lastWeapon && variation == lastVariation )
    {
        return lastIndex;
    }
    else
    {
        int i = 0;
        for ( i = 0; i < numLoadedWeapons; i++ )
        {
            if ( weapon == weaponDataTable[i].weaponBaseIndex &&
                variation == weaponDataTable[i].weaponModIndex )
            {
                lastIndex = i;
                lastWeapon = weapon;
                lastVariation = variation;
                
                return i;
            }
        }
    }
    
    return -1;
}

weaponData_t *BG_GetWeaponByClassName ( const char *className )
{
    static char lastClassName[MAX_QPATH] = { 0 };
    static weaponData_t *lastWeaponData = NULL;
    
    if ( lastWeaponData != NULL && Q_stricmp (className, lastClassName) == 0 )
    {
        return lastWeaponData;
    }
    else
    {
        int i = 0;
        for ( i = 0; i < numLoadedWeapons; i++ )
        {
            if ( Q_stricmp (weaponDataTable[i].classname, className) == 0 )
            {
                lastWeaponData = &weaponDataTable[i];
                Q_strncpyz (lastClassName, className, sizeof (lastClassName));
                
                return &weaponDataTable[i];
            }
        }
    }
    
    return NULL;
}

int BG_GetWeaponIndex ( unsigned int weapon, unsigned int variation )
{
    int i;
    for ( i = 0; i < numLoadedWeapons; i++ )
    {
        if ( weaponDataTable[i].weaponBaseIndex == weapon && weaponDataTable[i].weaponModIndex == variation )
        {
            break;
        }
    }
    
    return i;
}

qboolean BG_WeaponVariationExists ( unsigned int weaponId, unsigned int variation )
{
    return BG_GetWeaponIndex (weaponId, variation) != numLoadedWeapons;
}

weaponData_t *GetWeaponDataUnsafe ( unsigned char weapon, unsigned char variation )
{
    static weaponData_t *lastWeapon = NULL;
	unsigned int i;

	/* Remember the last weapon to cut down in the number of lookups required */
	if ( lastWeapon && lastWeapon->weaponBaseIndex == weapon && lastWeapon->weaponModIndex == variation )
	{
		return lastWeapon;
	}

	/* Check the table to see if we can find a match, remember it and return it! */
	for ( i = 0; i < numLoadedWeapons; i++ )
	{
		if ( weaponDataTable[i].weaponBaseIndex == weapon && weaponDataTable[i].weaponModIndex == variation )
		{
			lastWeapon = &weaponDataTable[i];
			return lastWeapon;
		}
	}
	
	return NULL;
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
	weaponData_t *weapon = GetWeaponDataUnsafe (baseIndex, modIndex);
	if ( weapon )
	{
	    return weapon;
	}

	/* This is a serious error, this is a weapon we don't know! */
	Com_Error( ERR_DISCONNECT, va( "No weapon with base %i and variation %i could be found.", baseIndex, modIndex ));
	return NULL;
}

unsigned char GetWeaponAmmoIndex ( unsigned char baseIndex, unsigned char modIndex )
{
    return GetWeaponData( baseIndex, modIndex )->ammoIndex;
}

short GetWeaponAmmoClip ( unsigned char baseIndex, unsigned char modIndex )
{
    return GetWeaponData( baseIndex, modIndex )->clipSize;
}

short GetWeaponAmmoMax ( unsigned char baseIndex, unsigned char modIndex )
{
    return ammoTable[GetWeaponData( baseIndex, modIndex )->ammoIndex].ammoMax;
}

short GetAmmoMax ( unsigned char ammoIndex )
{
    return ammoTable[ammoIndex].ammoMax;
}

// Just in case we need to initialize the struct any other way,
// it's easier to have it's own function.
void BG_InitializeWeaponData ( weaponData_t *weaponData )
{
    memset (weaponData, 0, sizeof (*weaponData));
    weaponData->speedModifier = 1.0f;
	weaponData->anims.ready.torsoAnim = BOTH_STAND1;
	weaponData->anims.ready.legsAnim = BOTH_STAND1;

	weaponData->anims.forwardWalk.legsAnim = BOTH_WALK1;
	weaponData->anims.crouchWalkBack.legsAnim = BOTH_CROUCH1WALKBACK;
	weaponData->anims.crouchWalk.legsAnim = BOTH_CROUCH1WALK;
	weaponData->anims.jump.legsAnim = weaponData->anims.jump.torsoAnim = BOTH_JUMP1;
	weaponData->anims.land.legsAnim = weaponData->anims.land.torsoAnim = BOTH_LAND1;
	weaponData->anims.run.legsAnim = BOTH_RUN1;
	weaponData->anims.sprint.legsAnim = weaponData->anims.sprint.torsoAnim = BOTH_SPRINT;
}

void BG_AddWeaponData ( weaponData_t *weaponData )
{
    if ( numLoadedWeapons >= MAX_WEAPON_TABLE_SIZE )
    {
        Com_Printf (S_COLOR_RED "ERROR: Too many weapons trying to be loaded. %s was not loaded.", weaponData->classname);
        return;
    }
    
    weaponDataTable[numLoadedWeapons] = *weaponData;
    
    numLoadedWeapons++;
    numWeapons[weaponData->weaponBaseIndex]++;
}

void BG_InitializeWeapons ( void )
{
    weaponData_t predefinedWeapons;
    
    //BG_InitializeAmmo();
    numLoadedWeapons = 0;

    memset (numWeapons, 0, sizeof (numWeapons));
    memset (weaponDataTable, 0, sizeof (weaponDataTable));
    
    BG_InitializeWeaponData (&predefinedWeapons);
    
    predefinedWeapons.weaponBaseIndex = WP_NONE;
    BG_AddWeaponData (&predefinedWeapons);
    
    predefinedWeapons.weaponBaseIndex = WP_EMPLACED_GUN;
    BG_AddWeaponData (&predefinedWeapons);
    
    predefinedWeapons.weaponBaseIndex = WP_TURRET;
    BG_AddWeaponData (&predefinedWeapons);
    
    if ( !BG_LoadWeapons (weaponDataTable, &numLoadedWeapons, numWeapons) )
    {
        #ifdef _DEBUG
        Com_Printf (S_COLOR_RED "No weapons were loaded.\n");
        #endif
        return;
    }
}

#ifdef CGAME
qboolean BG_DumpWeaponList ( const char *filename )
{
    char buffer[8192] = { 0 };
    char *classnames[MAX_WEAPON_TABLE_SIZE] = { NULL };
    int i;
    fileHandle_t f;
    
    Com_sprintf (buffer, sizeof (buffer), va ("%-64s | %s\n", "Display Name", "Class Name"));
    Q_strcat (buffer, sizeof (buffer), "-----------------------------------------------------------------+----------------------------------\n");
    for ( i = 0; i < numLoadedWeapons; i++ )
    {
        weaponData_t *w = &weaponDataTable[i];
        Q_strcat (buffer, sizeof (buffer), va ("%-64s | %s\n", w->displayName, w->classname));
        classnames[i] = w->classname;
    }
    
    Q_strcat (buffer, sizeof (buffer), "\n");
    //qsort (classnames, numLoadedWeapons, sizeof (char *), strcmp);
    i = 0;
    while ( i < numLoadedWeapons )
    {
        int duplicates = 0;
        int j = i + 1;
        while ( j < numLoadedWeapons && strcmp (classnames[i], classnames[j]) == 0 )
        {
            duplicates++;
            j++;
        }
        
        if ( duplicates > 0 )
        {
            Q_strcat (buffer, sizeof (buffer), va ("%s has %d duplicates.\n", classnames[i], duplicates));
        }
        
        i = j;
    }
    
    trap_FS_FOpenFile (filename, &f, FS_WRITE);
    if ( f )
    {
        trap_FS_Write (buffer, strlen (buffer), f);
        trap_FS_FCloseFile (f);
        
        return qtrue;
    }
    
    return qfalse;
}

void BG_PrintWeaponList( void )
{
	int i = 0;
	Com_Printf("----------------------------------------------------------------------------------------------------\n");
	for ( i = 0; i < numLoadedWeapons; i++ )
    {
        weaponData_t *w = &weaponDataTable[i];
		Com_Printf("%-64s | %s\n", w->displayName, w->classname);
    }
	Com_Printf("----------------------------------------------------------------------------------------------------\n");
}
#endif

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