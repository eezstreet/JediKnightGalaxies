//       ____ ___________________   ___           ____  __ _______   ___  ________  ___ ______________
//      |    |\_   _____/\______ \ |   |         |    |/ _|\      \ |   |/  _____/ /   |   \__    ___/
//      |    | |    __)_  |    |  \|   |         |      <  /   |   \|   /   \  ___/    ~    \|    |   
//  /\__|    | |        \ |    `   \   |         |    |  \/    |    \   \    \_\  \    Y    /|    |   
//  \________|/_______  //_______  /___|         |____|__ \____|__  /___|\______  /\___|_  / |____|   
//                    \/         \/                      \/       \/            \/       \/           
//                         ________    _____   ____       _____  ____  ___ ______________ _________   
//                        /  _____/   /  _  \ |    |     /  _  \ \   \/  /|   \_   _____//   _____/   
//                       /   \  ___  /  /_\  \|    |    /  /_\  \ \     / |   ||    __)_ \_____  \    
//                       \    \_\  \/    |    \    |___/    |    \/     \ |   ||        \/        \   
//                        \______  /\____|__  /_______ \____|__  /___/\  \|___/_______  /_______  /   
//                               \/         \/        \/	   \/	   \_/			  \/        \/ (c)
// bg_weapons.h
// This has been modified beyond it's original purpose. In the past, weapons were hardcoded
// with a weaponData_t struct that barely contained any useful information about the weapon,
// not enough damages, which were in a define. However, most if not all of the weapon data
// has been appropriately moved over to weaponData_t, which is filled by the .wpn files.
// (c) 2013 Jedi Knight Galaxies

#ifndef BG_WEAPONS_H
#define BG_WEAPONS_H

#include "q_shared.h"
#include "bg_ammo.h"

#define MAX_WEAPON_TABLE_SIZE (255)
#define MAX_FIREMODES (16)

typedef enum 
{

	AMMO_NONE,
	AMMO_FORCE,
	AMMO_BLASTER,
	AMMO_POWERCELL,
	AMMO_METAL_BOLTS,
	AMMO_CONCUSSION,
	AMMO_ROCKETS,
	AMMO_EMPLACED,
	AMMO_THERMAL,
	AMMO_TRIPMINE,
	AMMO_DETPACK,
	AMMO_MAX

} ammoType_t;

typedef enum
{

	WP_NONE,
	WP_STUN_BATON,
	WP_MELEE,
	WP_SABER,
	WP_BRYAR_PISTOL,
	WP_BLASTER,
	WP_DISRUPTOR,
	WP_BOWCASTER,
	WP_REPEATER,
	WP_DEMP2,
	WP_FLECHETTE,
	WP_CONCUSSION,
	WP_ROCKET_LAUNCHER,
	WP_THERMAL,
	WP_TRIP_MINE,
	WP_DET_PACK,
	WP_BRYAR_OLD,
	WP_EMPLACED_GUN,
	WP_TURRET,
	WP_NUM_WEAPONS

} weapon_t;

typedef struct
{

	unsigned char ammoIndex;
	short ammoClipCost;
	short ammoClipSize;
	short ammoMax;

} weaponAmmo_t;

typedef enum
{
    ZOOM_NONE,
    ZOOM_CONTINUOUS,
    ZOOM_TOGGLE
} zoomMode_t;

typedef enum firingType_e
{
    FT_AUTOMATIC,
    FT_SEMI,
    FT_BURST
} firingType_t;

typedef enum
{
	SPRINTSTYLE_CUSTOM = -1,
	SPRINTSTYLE_NONE,
	SPRINTSTYLE_LOWERED,
	SPRINTSTYLE_LOWERED_SLIGHT,
	SPRINTSTYLE_LOWERED_HEAVY,
	SPRINTSTYLE_SIDE,
	SPRINTSTYLE_SIDE_SLIGHT,
	SPRINTSTYLE_SIDE_HEAVY,
	SPRINTSTYLE_RAISED,
	SPRINTSTYLE_RAISED_SLIGHT,
	SPRINTSTYLE_RAISED_HEAVY,
	SPRINTSTYLE_ANGLED_DOWN,
	SPRINTSTYLE_ANGLED_DOWN_SLIGHT,			// copy bob from raised
	SPRINTSTYLE_ANGLED_DOWN_HEAVY,			// copy bob from raised
	SPRINTSTYLE_SIDE_UP,
	SPRINTSTYLE_SIDE_UP_SLIGHT,				// copy bob from side
	SPRINTSTYLE_SIDE_UP_HEAVY,				// copy bob from side
	SPRINTSTYLE_SIDE_MEDIUM,
	SPRINTSTYLE_SIDE_MEDIUM_SLIGHT,			// copy bob from side
	SPRINTSTYLE_SIDE_MEDIUM_HEAVY,			// copy bob from side
} sprintStyles_t;

typedef enum
{
	FMANIM_NONE,							// no change in animation
	FMANIM_RAISED,							// rasied, used for grenade launcher and flechette gun mines
	FMANIM_TILTED,							// tilted to the right a little, for the anti armor attachment on clone rifle
} firingModeAnim_t;

typedef enum
{
	FMTRANS_RAISED_NONE,					// had to fix this because this was causing issues --eez
	FMTRANS_NONE_NONE,
	FMTRANS_NONE_RAISED,
	FMTRANS_NONE_TILTED,
	FMTRANS_RAISED_RAISED,
	FMTRANS_RAISED_TILTED,
	FMTRANS_TILTED_NONE,
	FMTRANS_TILTED_RAISED,
	FMTRANS_TILTED_TILTED,
} firingModeTransitions_t;

typedef struct weaponFireModeStats_s
{
    const ammo_t *ammo;             // Ammo
									// Charge Base Damage: baseDamage * (( chargeTime / chargeDrain ) * chargeMuliplier )
	short       baseDamage;		    // The amount of base damage this weapon does, goes through modifiers for skills.
	qhandle_t   damageTypeHandle;   // For use with the more complex damage type system.
	qhandle_t   secondaryDmgHandle; // For secondary damage..
	char		applyGravity;		// If true, bolt is affected by gravity and will act accordingly. Speed will decide the forward thrust.
	char		bounceCount;		// The amount of bounces this weapon has, if any.
	char		hitscan;        	// Is this weapon hitscan?
	char		shotCount;			// The number of shots this weapon makes per fire (Flechette shoots more for instance)
	float		boxSize;			// The box size for a shot. By default 1.0 (like a blaster) and can increase for charged weapons.
	short		chargeMaximum;		// The maximum amount of time charged.
	float		chargeMultiplier;	// The multiplier to apply on charged shot damage calculation.
	short		chargeTime;		    // The time before substracting a drain.
	char		cost;				// The ammo cost to shoot the weapon.
	short		delay;				// The delay between each shot/throw/burstfire/whatever.
	float		range;				// The maximum amount of range this weapon/mode can reach.
	float		rangeSplash;		// The possible splash damage range. Damage calculation is done based on point of impact to end of radius.
	float		recoil;			    // The weapon recoil to smash into the camera, repeaters for instance have low but much recoil.
	float		spread;			    // The amount of spread this weapon mode has, if any. Goes through multiplers!
	float		speed;				// The speed override, set different then 0 to avoid using global speed.
	firingType_t    firingType;     // Firing type (auto, semi, burst)
	char        shotsPerBurst;      // Shots per burst
	short       burstFireDelay;     // Delay between firing in a burst.
	char	    weaponClass[32];	// The projectile class name for server reference and information.
	int		    weaponMOD;			// The MOD (Means of Death) for this mode and weapon for direct contact.
	int			weaponSplashMOD;	// The MOD (Means of Death) for this mode and weapon for splash damage.
	qboolean    isGrenade;          // Is this firemode a grenade?
	qboolean	grenadeBounces;		// Does this grenade bounce off of enemies, or does it explode on impact? (true for bounces on people)
	int			grenadeBounceDMG;	// Determines the amount of damage to do when bouncing off of an enemy.
} weaponFireModeStats_t;

typedef enum indicatorType_e
{
    IND_NONE,
    IND_NORMAL,
    IND_GRENADE
} indicatorType_t;

// This should roughly match up with weaponDrawData_t in
// cg_weapons.h.
typedef struct weaponVisualFireMode_s
{
    char type[16];
	char				displayName[128];	// what to display as the name of it ("Burst", "Grenade Launcher", etc)
	firingModeAnim_t	animType;			// which special anim to use
	char				switchToSound[MAX_QPATH];
	char				crosshairShader[MAX_QPATH];	// what to change the crosshair to (if applicable)
	short				overrideIndicatorFrame;	 // frame to override the firing mode shader with (since these aren't always in order)

    union
    {
        struct
        {
            float muzzleLightIntensity;
            char muzzleLightColor[16];
            char chargingEffect[MAX_QPATH];
            char muzzleEffect[MAX_QPATH];
        } generic;
    } weaponRender;
    
    union
    {
        struct
        {
            char fireSound[8][MAX_QPATH];
        } generic;
    } weaponFire;
    
    union
    {
        struct
        {
            char tracelineShader[MAX_QPATH];
            float minSize;
            float maxSize;
            int lifeTime;
        } generic;
    } tracelineRender;
    
    union
    {
        char chargingSound[MAX_QPATH];
    } weaponCharge;
    
    union
    {
        struct
        {
            char projectileModel[MAX_QPATH];
            char projectileEffect[MAX_QPATH];
            char runSound[MAX_QPATH];
            
            float lightIntensity;
            char lightColor[16];
            
            char deathEffect[MAX_QPATH];
        } generic;
    } projectileRender;
    
    union
    {
        struct
        {
            char impactEffect[MAX_QPATH];
        } generic;
        
        struct
        {
            char stickSound[MAX_QPATH];
        } explosive;
        
        struct
        {
            char impactEffect[MAX_QPATH];
            char shockwaveEffect[MAX_QPATH];
        } grenade;
    } projectileMiss;
    
    union
    {
        struct
        {
            char impactEffect[MAX_QPATH];
        } generic;
        
        struct
        {
            char impactEffect[MAX_QPATH];
            char shockwaveEffect[MAX_QPATH];
        } grenade;
    } projectileHitPlayer;
    
    union
    {
        struct
        {
            char deflectEffect[MAX_QPATH];
        } generic;
    } projectileDeflected;
    
    union
    {
        struct
        {
            char bounceSound[2][MAX_QPATH];
        } grenade;
    } grenadeBounce;
    
    union
    {
        struct
        {
            char g2Model[MAX_QPATH];
            float g2Radius;
        } detpack;
        
        struct
        {
            char g2Model[MAX_QPATH];
            float g2Radius;
            char lineEffect[MAX_QPATH];
        } tripmine;
    } explosiveRender;
    
    union
    {
        struct
        {
            char explodeEffect[MAX_QPATH];
        } generic;
    } explosiveBlow;
    
    union
    {
        char armSound[MAX_QPATH];
    } explosiveArm;
} weaponVisualFireMode_t;

typedef struct weaponVisual_s
{
	char description[512];			// The description of this weapon to display in UI.
	
	char world_model[MAX_QPATH];	// The model used for 3D rendering.
	char view_model[MAX_QPATH];		// The model used when in first person mode.
	
	char icon[MAX_QPATH];		    // The icon of this weapon to be used in the HUD.
	char icon_na[MAX_QPATH];        // Not available icon
	
	char selectSound[MAX_QPATH];
	
	char firemodeIndicatorShader[MAX_QPATH];
	char groupedIndicatorShaders[3][MAX_QPATH];
	indicatorType_t indicatorType;
	
	char gunPosition[16];
	char ironsightsPosition[16];
	float ironsightsFov;
	
	char scopeShader[MAX_QPATH];
	char scopeStartSound[MAX_QPATH];
    char scopeStopSound[MAX_QPATH];
    int scopeSoundLoopTime;
    char scopeLoopSound[MAX_QPATH];

	//eezstreet add: Variable crosshairs
	int crosshairValue;

	weaponVisualFireMode_t visualFireModes[MAX_FIREMODES];
} weaponVisual_t;

typedef struct
{
	int torsoAnim;
	int legsAnim;
} weaponAnimationSet_t;

typedef struct
{
	weaponAnimationSet_t firing;
	weaponAnimationSet_t ready;
	weaponAnimationSet_t reload;
	weaponAnimationSet_t forwardWalk;
	weaponAnimationSet_t backwardWalk;
	weaponAnimationSet_t crouchWalk;
	weaponAnimationSet_t crouchWalkBack;
	weaponAnimationSet_t jump;
	weaponAnimationSet_t land;
	weaponAnimationSet_t run;
	weaponAnimationSet_t sprint;
} weaponAnimationReplacements_t;

typedef struct
{
    char            classname[MAX_QPATH];   // Class name..

	unsigned char	weaponBaseIndex;		// Base index, determines the type of weapon this is.
	unsigned char	weaponModIndex;			// Mod index, determines which alternate version of the base weapon this is.
	unsigned short	weaponReloadTime;		// The time required to reload the weapon (or the time till she blows, for grenades).
	unsigned char	weaponSlot;				// The slot this weapon uses, i.e. main weapon, secondary weapon or grenade.
	
	unsigned char	ammoIndex;				// Ammo index, determines the type of weapon clips used.
	unsigned int	ammoOnSpawn;			// Ammo given upon spawning with this weapon.
	unsigned int	ammoOnPickup;			// Ammo given upon picking up a dropped weapon
	unsigned int	clipSize;				// Number of shots in one clip

	unsigned char	numFiringModes;			// Number of firing modes on the gun.

	unsigned char	hasCookAbility;			// Determines whether or not this weapon can be cooked (grenades only).
	unsigned char	hasKnockBack;			// The amount of damage determines the knockback of the weapon.
	unsigned char	hasRollAbility;			// Determines whether or not you can roll with this weapon.
	
	unsigned char	zoomType;		    	// Determines whether or not the weapon has the ability to zoom.
	float           startZoomFov;           // Starting FOV when zooming
	float           endZoomFov;             // Max FOV when zooming
	unsigned int    zoomTime;               // Time in milliseconds it takes to zoom all the way in
	
	float           speedModifier;          // Modifier to apply to player's speed when weapon in use. 1.0f for default speed.
	float           reloadModifier;         // Modifier to apply to player's speed when reloading. 1.0f for no change.
	                                        // The modifier is applied to the player's speed after the speed has been modified.

	weaponAnimationReplacements_t anims;

	int firstPersonSprintStyle;				// Handles the way sprinting is done.
											// -1 = custom animation defined in MD3
											// 0 = no animation
											// 1 = lowered weapon
											// 2 = lowered weapon, slight bob
											// 3 = lowered weapon, heavy bob
											// 4 = side weapon
											// 5 = side weapon, slight bob
											// 6 = side weapon, heavy bob
											// 7 = lowered, angled weapon
											// 8 = lowered, angled weapon, slight bob
											// 9 = lowered, angled weapon, heavy bob
											// 10 = raised
											// 11 = raised, slight bob
											// 12 = raised, heavy bob

	weaponFireModeStats_t firemodes[MAX_FIREMODES];
    
    char displayName[64];			// The name which is to be displayed on the HUD.
    
#ifdef CGAME
    weaponVisual_t visuals;
#endif

} weaponData_t;

typedef enum
{

	WPS_NONE = 0,
	WPS_GRENADE,
	WPS_SECONDARY,
	WPS_PRIMARY

} weaponSlot_t;

/* These shouldn't be used since they don't take variations into account (execeptions only) */
#define GetWeaponAmmoIndexSingle( a )		GetWeaponAmmoIndex( a, 0 )				// Replenish items. Allowed because we assume variations use the same ammo type (otherwise, bad luck?)
#define GetWeaponAmmoMaxSingle( a )			GetWeaponAmmoMax( a, 0 )				// Replenish items. Allowed because we assume variations use the same ammo type (otherwise, bad luck?)
#define GetWeaponPrimaryCostSingle( a )		GetWeaponData( a, 0 )->firemodes[0].cost		// CG_WeaponCheck for weapon select drawing (Trash the function?)!
#define GetWeaponSecondaryCostSingle( a )	GetWeaponData( a, 0 )->firemodes[1].cost	// CG_WeaponCheck for weapon select drawing (Trash the function?)!

/* This is the main function to get weapon data for each variation */
void            BG_InitializeWeapons ( void );
void            BG_InitializeWeaponData ( weaponData_t *weaponData );
qboolean        BG_WeaponVariationExists ( unsigned int weaponId, unsigned int variation );
int             BG_GetWeaponIndex ( unsigned int weapon, unsigned int variation );
qboolean        BG_GetWeaponByIndex ( int index, int *weapon, int *variation );
weaponData_t   *BG_GetWeaponByClassName ( const char *className );
int             BG_GetWeaponIndexFromClass ( int weapon, int variation );

unsigned int BG_NumberOfLoadedWeapons ( void );
unsigned int BG_NumberOfWeaponVariations ( unsigned char weaponId );
weaponData_t *BG_GetWeaponDataByIndex( int index );

//void			GetWeaponInitialization( void );
weaponData_t   *GetWeaponData( unsigned char baseIndex, unsigned char modIndex );
weaponData_t   *GetWeaponDataUnsafe ( unsigned char weapon, unsigned char variation );
unsigned char   GetWeaponAmmoIndex ( unsigned char baseIndex, unsigned char modIndex );
short           GetWeaponAmmoClip ( unsigned char baseIndex, unsigned char modIndex );
short           GetWeaponAmmoMax ( unsigned char baseIndex, unsigned char modIndex );
short           GetAmmoMax ( unsigned char ammoIndex );

#ifdef CGAME
qboolean BG_DumpWeaponList ( const char *filename );
void BG_PrintWeaponList( void );
#endif

/* Original definitions used for weapon switching and such, we won't use it eventually */
#define LAST_USEABLE_WEAPON			WP_BRYAR_OLD		// anything > this will be considered not player useable
#define FIRST_WEAPON				WP_BRYAR_PISTOL		// this is the first weapon for next and prev weapon switching
#define MAX_PLAYER_WEAPONS			WP_NUM_WEAPONS - 1	// this is the max you can switch to and get with the give all.

/* These are the weapon ranges to be used by any weapon. Excepts are possible, as always */
#define WPR_S						3072.0f		/* Short Range */
#define WPR_M						3584.0f		/* Medium Range */
#define WPR_L						4096.0f		/* Long Range */
#define WPR_I						  -1.0f		/* Infinite Range */

/* These externals contain all the data we want and need! */
extern	weaponAmmo_t				xweaponAmmo[];

typedef struct
{
	union {
		char *a;
		int i;
		float f;
		unsigned char uc;
	} data;
	//void *data;
	qboolean isFloat;
	qboolean isString;
	int byteCount;
} weaponDataGrab_t;
#endif
