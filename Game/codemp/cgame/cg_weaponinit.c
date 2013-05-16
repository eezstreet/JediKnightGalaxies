//
// cg_weaponinit.c -- events and effects dealing with weapons
#include "cg_local.h"
#include "fx_local.h"
#include "bg_local.h"

#include "cg_weapons.h"

//=========================================================
// Description:
// Returns true if modelPath is a GHOUL2 model file.
//=========================================================
static qboolean CG_IsGhoul2Model ( const char *modelPath )
{
    size_t len = strlen (modelPath);
    if ( len <= 4 )
    {
        return qfalse;
    }
    
    if ( Q_stricmp (modelPath + len - 4, ".glm") == 0 )
    {
        return qtrue;
    }
    
    return qfalse;
}

#define MAX_WEAPON_ANIMFILE_LENGTH (4096)

const stringID_table_t weaponAnimTable[MAX_WEAPON_ANIMATIONS + 1] =
{
    ENUM2STRING (ABASE),
    ENUM2STRING (E11_IDLE),
    ENUM2STRING (WPROOT),
    
    { NULL, -1 }
};

//=========================================================
// Description:
// Loads an animation config file for a view weapon.
//=========================================================
static qboolean CG_LoadViewWeaponAnimations ( weaponInfo_t *weapon, const char *animationPath )
{
    fileHandle_t f;
    char weaponFileData[MAX_WEAPON_ANIMFILE_LENGTH];
    int fileLength;
    int numAnims;
    
    fileLength = trap_FS_FOpenFile (animationPath, &f, FS_READ);
    if ( !f || fileLength == -1 )
    {
        CG_Printf (S_COLOR_YELLOW "Unable to load weapon animation config file %s\n", animationPath);
        return qfalse;
    }
    
    if ( (fileLength + 1) >= MAX_WEAPON_ANIMFILE_LENGTH )
    {
        CG_Printf (S_COLOR_YELLOW "Weapon animation config file %s too long (%d characters, max %d", animationPath, fileLength, MAX_WEAPON_ANIMFILE_LENGTH - 1);
        trap_FS_FCloseFile (f);
        
        return qfalse;
    }
    
    trap_FS_Read (weaponFileData, fileLength, f);
    weaponFileData[fileLength] = '\0';
    
    trap_FS_FCloseFile (f);
    
    numAnims = BG_ParseGenericAnimationFile (weapon->viewModelAnims, MAX_WEAPON_ANIMATIONS, weaponAnimTable, animationPath, weaponFileData);
    
#ifdef _DEBUG
    CG_Printf ("%s: %d animations loaded\n", animationPath, numAnims);
#endif

    return qtrue;
}

//=========================================================
// Description:
// Loads a view weapon.
//=========================================================
static void CG_LoadViewWeapon ( weaponInfo_t *weapon, const char *modelPath )
{
    char file[MAX_QPATH];
    char *slash;
    //int root;
    //int model;
    
    Q_strncpyz (file, modelPath, sizeof (file));
    slash = Q_strrchr (file, '/');
    
    Q_strncpyz (slash, "/model_default.skin", sizeof (file) - (slash - file));  
    weapon->viewModelSkin = trap_R_RegisterSkin (file);
    
    trap_G2API_InitGhoul2Model (&weapon->g2ViewModel, modelPath, 0, weapon->viewModelSkin, 0, 0, 0);
    if ( !trap_G2_HaveWeGhoul2Models (weapon->g2ViewModel) )
    {
        return;
    }
    
    /*root = trap_G2API_AddBolt (weapon->g2ViewModel, 0, "model_root");
    
    model = trap_G2API_InitGhoul2Model (&weapon->g2ViewModel, "models/weapons2/test3_r_hand/model.glm", 1, 0, trap_R_RegisterSkin ("models/weapons2/test3_r_hand/model_default.skin"), 0, 0);
    if ( !trap_G2API_HasGhoul2ModelOnIndex (&weapon->g2ViewModel, 1) )
    {
        CG_Printf (S_COLOR_YELLOW "WARNING: Failed to load right hand model.\n");
    }
    
    trap_G2API_SetBoltInfo (weapon->g2ViewModel, 0, root);
    
    trap_G2API_InitGhoul2Model (&weapon->g2ViewModel, "models/weapons2/test3_l_hand/model.glm", 2, 0, trap_R_RegisterSkin ("models/weapons2/test3_l_hand/model_default.skin"), 0, 0);
    if ( !trap_G2API_HasGhoul2ModelOnIndex (&weapon->g2ViewModel, 2) )
    {
        CG_Printf (S_COLOR_YELLOW "WARNING: Failed to load left hand model.\n");
    }
    
    trap_G2API_SetBoltInfo (weapon->g2ViewModel, 0, root);*/
    
    memset (weapon->viewModelAnims, 0, sizeof (weapon->viewModelAnims));
    trap_G2API_GetGLAName (weapon->g2ViewModel, 0, file);
    if ( !file[0] )
    {
        return;
    }
    
    slash = Q_strrchr (file, '/');
    
    Q_strncpyz (slash, "/animation.cfg", sizeof (file) - (slash - file));
    CG_LoadViewWeaponAnimations (weapon, file);
}

static void CG_LoadG2ModelWithSkin ( const char *modelPath, void **g2ModelPtr )
{
    void *g2Model = NULL;

    trap_G2API_InitGhoul2Model (g2ModelPtr, modelPath, 0, 0, 0, 0, 0);
    
    g2Model = *g2ModelPtr;
    
    if ( trap_G2_HaveWeGhoul2Models (g2Model) && trap_G2API_SkinlessModel (g2Model, 0) )
    {
        char skinName[MAX_QPATH];
        char *slash = NULL;
        int skinLen = 0;
        qhandle_t skin;
        
        Q_strncpyz (skinName, modelPath, sizeof (skinName));
        skinLen = strlen (skinName);
        
        slash = Q_strrchr (skinName, '/');
        Q_strncpyz (slash, "/model_default.skin", sizeof (skinName) - (slash - skinName));  
        
        skin = trap_R_RegisterSkin (skinName);
        trap_G2API_SetSkin (g2Model, 0, skin, skin);
    }
}

static void ReadColor ( const char *vectorString, vec3_t out )
{
    int vector[3];
    sscanf (vectorString, "%d %d %d", &vector[0], &vector[1], &vector[2]);
    
    out[0] = vector[0] / 255.0f;
    out[1] = vector[1] / 255.0f;
    out[2] = vector[2] / 255.0f;
}

static void ReadVector ( const char *vectorString, vec3_t out )
{
    sscanf (vectorString, "%f %f %f", &out[0], &out[1], &out[2]);
}

static void JKG_LoadFireModeAssets ( weaponDrawData_t *drawData, const weaponFireModeStats_t* fireMode, const weaponVisualFireMode_t *fireModeVisuals )
{
    //qboolean isGrenade = (qboolean)(Q_stricmp (fireModeVisuals->type, "grenade") == 0);
    //qboolean isBlaster = (qboolean)(Q_stricmp (fireModeVisuals->type, "blaster") == 0);
    qboolean isTripmine = (qboolean)(Q_stricmp (fireModeVisuals->type, "tripmine") == 0);
    qboolean isDetpack = (qboolean)(Q_stricmp (fireModeVisuals->type, "detpack") == 0);
    
    int i;

    // Weapon render
    if ( fireModeVisuals->weaponRender.generic.chargingEffect[0] )
        drawData->weaponRender.generic.chargingEffect = trap_FX_RegisterEffect (fireModeVisuals->weaponRender.generic.chargingEffect);
    
    if ( fireModeVisuals->weaponRender.generic.muzzleEffect[0] )
        drawData->weaponRender.generic.muzzleEffect = trap_FX_RegisterEffect (fireModeVisuals->weaponRender.generic.muzzleEffect);
        
    ReadColor (fireModeVisuals->weaponRender.generic.muzzleLightColor, drawData->weaponRender.generic.muzzleLightColor);
    drawData->weaponRender.generic.muzzleLightIntensity = fireModeVisuals->weaponRender.generic.muzzleLightIntensity;
    
    // Weapon fire
    for ( i = 0; i < 8; i++ )
    {
        if ( fireModeVisuals->weaponFire.generic.fireSound[i][0] )
        {
            drawData->weaponFire.generic.fireSound[i] = trap_S_RegisterSound (fireModeVisuals->weaponFire.generic.fireSound[i]);
        }
        else
        {
            break;
        }
    }
    drawData->weaponFire.generic.fireSoundCount = i;
    
    // Hitscan render
    drawData->tracelineRender.generic.lifeTime = fireModeVisuals->tracelineRender.generic.lifeTime;
    drawData->tracelineRender.generic.maxSize = fireModeVisuals->tracelineRender.generic.maxSize;
    drawData->tracelineRender.generic.minSize = fireModeVisuals->tracelineRender.generic.minSize;
    if ( fireModeVisuals->tracelineRender.generic.tracelineShader[0] )
        drawData->tracelineRender.generic.tracelineShader = trap_R_RegisterShader (fireModeVisuals->tracelineRender.generic.tracelineShader);
    
    // Weapon charge
    if ( fireModeVisuals->weaponCharge.chargingSound[0] )
        drawData->weaponCharge.chargingSound = trap_S_RegisterSound (fireModeVisuals->weaponCharge.chargingSound);
    
    // Projectile render
    ReadColor (fireModeVisuals->projectileRender.generic.lightColor, drawData->projectileRender.generic.lightColor);
    drawData->projectileRender.generic.lightIntensity = fireModeVisuals->projectileRender.generic.lightIntensity;
    
    if ( fireModeVisuals->projectileRender.generic.projectileEffect[0] )
        drawData->projectileRender.generic.projectileEffect = trap_FX_RegisterEffect (fireModeVisuals->projectileRender.generic.projectileEffect);
    else if ( fireMode->ammo && fireMode->ammo->fx )
        drawData->projectileRender.generic.projectileEffect = fireMode->ammo->fx;
        
    if ( fireModeVisuals->projectileRender.generic.projectileModel[0] )
        drawData->projectileRender.generic.projectileModel = trap_R_RegisterModel (fireModeVisuals->projectileRender.generic.projectileModel);
    else if ( fireMode->ammo && fireMode->ammo->model )
        drawData->projectileRender.generic.projectileModel = fireMode->ammo->model;
        
    if ( fireModeVisuals->projectileRender.generic.runSound[0] )
        drawData->projectileRender.generic.runSound = trap_S_RegisterSound (fireModeVisuals->projectileRender.generic.runSound);
        
    if ( fireModeVisuals->projectileRender.generic.deathEffect[0] )
        drawData->projectileRender.generic.deathEffect = trap_FX_RegisterEffect (fireModeVisuals->projectileRender.generic.deathEffect);
    else if ( fireMode->ammo && fireMode->ammo->deathFx )
        drawData->projectileRender.generic.deathEffect = fireMode->ammo->deathFx;
    
    // Projectile miss event
    if ( isTripmine || isDetpack )
    {
        if ( fireModeVisuals->projectileMiss.explosive.stickSound[0] )
            drawData->projectileMiss.explosive.stickSound = trap_S_RegisterSound (fireModeVisuals->projectileMiss.explosive.stickSound);
    }
    else
    {
        if ( fireModeVisuals->projectileMiss.generic.impactEffect[0] )
            drawData->projectileMiss.generic.impactEffect = trap_FX_RegisterEffect (fireModeVisuals->projectileMiss.generic.impactEffect);
        else if ( fireMode->ammo && fireMode->ammo->missFx )
            drawData->projectileMiss.generic.impactEffect = fireMode->ammo->missFx;
            
        if ( fireModeVisuals->projectileMiss.grenade.shockwaveEffect[0] )
            drawData->projectileMiss.grenade.shockwaveEffect = trap_FX_RegisterEffect (fireModeVisuals->projectileMiss.grenade.shockwaveEffect);
    }
    
    // Projectile hit event
    if ( fireModeVisuals->projectileHitPlayer.generic.impactEffect[0] )
        drawData->projectileHitPlayer.generic.impactEffect = trap_FX_RegisterEffect (fireModeVisuals->projectileHitPlayer.generic.impactEffect);
    else if ( fireMode->ammo && fireMode->ammo->hitFx )
        drawData->projectileHitPlayer.generic.impactEffect = fireMode->ammo->hitFx;
        
    if ( fireModeVisuals->projectileHitPlayer.grenade.shockwaveEffect[0] )
        drawData->projectileHitPlayer.grenade.shockwaveEffect = trap_FX_RegisterEffect (fireModeVisuals->projectileHitPlayer.grenade.shockwaveEffect);
    
    // Projectile deflected event
    if ( fireModeVisuals->projectileDeflected.generic.deflectEffect[0] )
        drawData->projectileDeflected.generic.deflectEffect = trap_FX_RegisterEffect (fireModeVisuals->projectileDeflected.generic.deflectEffect);
    
    // Grenade bounce event
    if ( fireModeVisuals->grenadeBounce.grenade.bounceSound[0][0] )
        drawData->grenadeBounce.grenade.bounceSound[0] = trap_S_RegisterSound (fireModeVisuals->grenadeBounce.grenade.bounceSound[0]);
        
    if ( fireModeVisuals->grenadeBounce.grenade.bounceSound[1][0] )
        drawData->grenadeBounce.grenade.bounceSound[1] = trap_S_RegisterSound (fireModeVisuals->grenadeBounce.grenade.bounceSound[1]);
    
    // Explosive render
    if ( fireModeVisuals->explosiveRender.detpack.g2Model[0] )
        CG_LoadG2ModelWithSkin (fireModeVisuals->explosiveRender.detpack.g2Model, &drawData->explosiveRender.detpack.g2Model);
        
    drawData->explosiveRender.detpack.g2Radius = fireModeVisuals->explosiveRender.detpack.g2Radius;
    
    if ( fireModeVisuals->explosiveRender.tripmine.lineEffect[0] )
        drawData->explosiveRender.tripmine.lineEffect = trap_FX_RegisterEffect (fireModeVisuals->explosiveRender.tripmine.lineEffect);
    
    // Explosive blow event
    if ( fireModeVisuals->explosiveBlow.generic.explodeEffect[0] )
        drawData->explosiveBlow.generic.explodeEffect = trap_FX_RegisterEffect (fireModeVisuals->explosiveBlow.generic.explodeEffect);
    
    // Explosive armed event
    if ( fireModeVisuals->explosiveArm.armSound[0] )
        drawData->explosiveArm.armSound = trap_S_RegisterSound (fireModeVisuals->explosiveArm.armSound);
}

qhandle_t CG_FindShader(const char *name);
void JKG_LoadWeaponAssets ( weaponInfo_t *weaponInfo, const weaponData_t *weaponData )
{
    static const char *barrelSuffixes[] = {
        "_barrel.md3",
        "_barrel2.md3",
        "_barrel3.md3",
        "_barrel4.md3"
    };
    int i;
    char extensionlessModel[MAX_QPATH];
    const weaponVisual_t *weaponVisuals = &weaponData->visuals;
    
    weaponInfo->indicatorType = weaponVisuals->indicatorType;
    for ( i = 0; i < 3; i++ )
    {
        if ( weaponVisuals->groupedIndicatorShaders[i][0] )
        {
            weaponInfo->groupedIndicators[i] = CG_FindShader (weaponVisuals->groupedIndicatorShaders[i]);
        }
    }
    
    if ( weaponVisuals->firemodeIndicatorShader[0] )
    {
        weaponInfo->fireModeIndicator = CG_FindShader (weaponVisuals->firemodeIndicatorShader);
    }
    
    VectorClear (weaponInfo->gunPosition);
    if ( weaponVisuals->gunPosition[0] )
    {
        ReadVector (weaponVisuals->gunPosition, weaponInfo->gunPosition);
    }
    
    VectorClear (weaponInfo->ironsightsPosition);
    if ( weaponVisuals->ironsightsPosition[0] )
    {
        ReadVector (weaponVisuals->ironsightsPosition, weaponInfo->ironsightsPosition);
    }
    else
    {
        VectorCopy (weaponInfo->gunPosition, weaponInfo->ironsightsPosition);
    }
    weaponInfo->ironsightsFov = weaponVisuals->ironsightsFov;
    
    if ( CG_IsGhoul2Model (weaponVisuals->world_model) )
    {
        trap_G2API_InitGhoul2Model (&weaponInfo->g2WorldModel, weaponVisuals->world_model, 0, 0, 0, 0, 0);
        if ( !trap_G2_HaveWeGhoul2Models (weaponInfo->g2WorldModel) )
        {
            weaponInfo->g2WorldModel = NULL;
        }
    }
    memset (weaponInfo->barrelModels, NULL_HANDLE, sizeof (weaponInfo->barrelModels));
    COM_StripExtension (weaponVisuals->view_model, extensionlessModel);
    
    for ( i = 0; i < 4; i++ )
    {
        const char *barrelModel = va ("%s%s", extensionlessModel, barrelSuffixes[i]);
        int len = strlen (barrelModel);
        qhandle_t barrel;
        
        if ( (len + 1) > MAX_QPATH )
        {
            CG_Printf (S_COLOR_YELLOW "Warning: barrel model path %s is too long (%d chars). Max length is 63.\n", barrelModel, len);
            break;
        }
        
        barrel = trap_R_RegisterModel (barrelModel);
        if ( barrel == NULL_HANDLE )
        {
            break;
        }
        
        weaponInfo->barrelModels[i] = barrel;
    }
    
    // Scope render
    if ( weaponVisuals->scopeShader[0] )
        weaponInfo->scopeShader = trap_R_RegisterShader (weaponVisuals->scopeShader);
    
    // Scope toggle
    if ( weaponVisuals->scopeStartSound[0] )
        weaponInfo->scopeStartSound = trap_S_RegisterSound (weaponVisuals->scopeStartSound);
        
    if ( weaponVisuals->scopeStopSound[0] )
        weaponInfo->scopeStopSound = trap_S_RegisterSound (weaponVisuals->scopeStopSound);
    
    // Scope zoom
    if ( weaponVisuals->scopeLoopSound[0] )
        weaponInfo->scopeLoopSound = trap_S_RegisterSound (weaponVisuals->scopeLoopSound);
        
    weaponInfo->scopeSoundLoopTime = weaponVisuals->scopeSoundLoopTime;

    JKG_LoadFireModeAssets (&weaponInfo->primDrawData, &weaponData->firemodes[0], &weaponVisuals->primary);
    JKG_LoadFireModeAssets (&weaponInfo->altDrawData, &weaponData->firemodes[1], &weaponVisuals->secondary);
}


/*
=================
CG_RegisterWeapon

The server says this item is used on this level
=================
*/
void CG_RegisterWeapon( int weaponNum, int variation ) {
	weaponInfo_t	*weaponInfo;
	const weaponData_t *weaponData;
	gitem_t			*item/*, *ammo*/;
	char			path[MAX_QPATH];
	vec3_t			mins, maxs;
	int				i;

	if ( weaponNum == 0 ) {
		return;
	}
	
	weaponInfo = CG_WeaponInfoUnsafe (weaponNum, variation);
	if ( weaponInfo )
	{
	    // Non-null means the weapon exists.
	    return;
	}

    weaponInfo = CG_NextFreeWeaponInfo();
	if ( weaponInfo == NULL ) {
	    CG_Error ("Max weapon info slots exceeded.\n");
		return;
	}
	
	#ifdef _DEBUG
	CG_Printf ("Registering weapon %d variation %d\n", weaponNum, variation);
	#endif

	memset( weaponInfo, 0, sizeof( *weaponInfo ) );
	weaponInfo->registered = qtrue;
	weaponInfo->weaponNum = weaponNum;
	weaponInfo->variation = variation;

	for ( item = bg_itemlist + 1 ; item->classname ; item++ ) {
		if ( item->giType == IT_WEAPON && item->giTag == weaponNum ) {
			weaponInfo->item = item;
			break;
		}
	}
	if ( !item->classname ) {
		CG_Error( "Couldn't find weapon %i", weaponNum );
	}
	CG_RegisterItemVisuals( item - bg_itemlist );
	
	weaponData = GetWeaponData (weaponNum, variation);

	// load cmodel before model so filecache works
	weaponInfo->weaponModel = trap_R_RegisterModel( weaponData->visuals.world_model );
	// load in-view model also
	weaponInfo->viewModel = NULL_HANDLE;
	if ( weaponInfo->g2ViewModel )
	{
	    trap_G2API_CleanGhoul2Models (&weaponInfo->g2ViewModel);
	    weaponInfo->g2ViewModel = NULL;
	}
		
	if ( CG_IsGhoul2Model (weaponData->visuals.view_model) )
	{
	    CG_LoadViewWeapon (weaponInfo, weaponData->visuals.view_model);
	}
	else
	{
	    weaponInfo->viewModel = trap_R_RegisterModel(weaponData->visuals.view_model);
	}

	// calc midpoint for rotation
	trap_R_ModelBounds( weaponInfo->weaponModel, mins, maxs );
	for ( i = 0 ; i < 3 ; i++ ) {
		weaponInfo->weaponMidpoint[i] = mins[i] + 0.5 * ( maxs[i] - mins[i] );
	}

	weaponInfo->hudIcon = trap_R_RegisterShaderNoMip (weaponData->visuals.icon);
	weaponInfo->hudNAIcon = trap_R_RegisterShaderNoMip (weaponData->visuals.icon_na);

	/* Xycaleth: wtf was this for?
	for ( ammo = bg_itemlist + 1 ; ammo->classname ; ammo++ ) {
		if ( ammo->giType == IT_AMMO && ammo->giTag == weaponNum ) {
			break;
		}
	}
	if ( ammo->classname && ammo->world_model[0] ) {
		weaponInfo->ammoModel = trap_R_RegisterModel( ammo->world_model[0] );
	}*/

//	strcpy( path, item->view_model );
//	COM_StripExtension( path, path );
//	strcat( path, "_flash.md3" );
	weaponInfo->flashModel = 0;//trap_R_RegisterModel( path );

	/*if (weaponNum == WP_DISRUPTOR ||
		weaponNum == WP_FLECHETTE ||
		weaponNum == WP_REPEATER ||
		weaponNum == WP_ROCKET_LAUNCHER)
	{
		strcpy( path, weaponData->visuals.view_model );
		COM_StripExtension( path, path );
		strcat( path, "_barrel.md3" );
		weaponInfo->barrelModel = trap_R_RegisterModel( path );
	}
	else if (weaponNum == WP_STUN_BATON)
	{ //only weapon with more than 1 barrel..
		trap_R_RegisterModel("models/weapons2/stun_baton/baton_barrel.md3");
		trap_R_RegisterModel("models/weapons2/stun_baton/baton_barrel2.md3");
		trap_R_RegisterModel("models/weapons2/stun_baton/baton_barrel3.md3");
	}
	else
	{
		weaponInfo->barrelModel = 0;
	}*/

	if (weaponNum != WP_SABER)
	{
		strcpy( path, weaponData->visuals.view_model );
		COM_StripExtension( path, path );
		strcat( path, "_hand.md3" );
		weaponInfo->handsModel = trap_R_RegisterModel( path );
	}
	else
	{
		weaponInfo->handsModel = 0;
	}

//	if ( !weaponInfo->handsModel ) {
//		weaponInfo->handsModel = trap_R_RegisterModel( "models/weapons2/shotgun/shotgun_hand.md3" );
//	}

    JKG_SetWeaponEventsHandler (weaponInfo, weaponData->visuals.primary.type, weaponData->visuals.secondary.type);
    JKG_LoadWeaponAssets (weaponInfo, weaponData);

	switch ( weaponNum ) {
	case WP_STUN_BATON:
	case WP_MELEE:
/*		MAKERGB( weaponInfo->flashDlightColor, 0.6f, 0.6f, 1.0f );
		weaponInfo->firingSound = trap_S_RegisterSound( "sound/weapons/saber/saberhum.wav" );
//		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/melee/fstatck.wav" );
*/
		//trap_R_RegisterShader( "gfx/effects/stunPass" );
		trap_FX_RegisterEffect( "stunBaton/flesh_impact" );

		if (weaponNum == WP_STUN_BATON)
		{
			trap_S_RegisterSound( "sound/weapons/baton/idle.wav" );
			weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/baton/fire.mp3" );
			weaponInfo->altFlashSound[0] = trap_S_RegisterSound( "sound/weapons/baton/fire.mp3" );
		}
		else
		{
			/*
			int j = 0;

			while (j < 4)
			{
				weaponInfo->flashSound[j] = trap_S_RegisterSound( va("sound/weapons/melee/swing%i", j+1) );
				weaponInfo->altFlashSound[j] = weaponInfo->flashSound[j];
				j++;
			}
			*/
			//No longer needed, animsound config plays them for us
		}
		break;
	case WP_SABER:
		MAKERGB( weaponInfo->flashDlightColor, 0.6f, 0.6f, 1.0f );
		weaponInfo->firingSound = trap_S_RegisterSound( "sound/weapons/saber/saberhum1.wav" );
		weaponInfo->missileModel		= trap_R_RegisterModel( "models/weapons2/saber/saber_w.glm" );
		break;

	case WP_TURRET:
		weaponInfo->flashSound[0]		= NULL_SOUND;
		weaponInfo->firingSound			= NULL_SOUND;
		weaponInfo->chargeSound			= NULL_SOUND;
		weaponInfo->muzzleEffect		= NULL_HANDLE;
		weaponInfo->missileModel		= NULL_HANDLE;
		weaponInfo->missileSound		= NULL_SOUND;
		weaponInfo->missileDlight		= 0;
		weaponInfo->missileHitSound		= NULL_SOUND;
		weaponInfo->missileTrailFunc	= FX_TurretProjectileThink;

		trap_FX_RegisterEffect("effects/blaster/wall_impact.efx");
		trap_FX_RegisterEffect("effects/blaster/flesh_impact.efx");
		break;

	 default:
		MAKERGB( weaponInfo->flashDlightColor, 1, 1, 1 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/rocket/rocklf1a.wav" );
		break;
	}
}
