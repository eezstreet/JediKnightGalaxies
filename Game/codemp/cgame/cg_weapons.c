// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_weapons.c -- events and effects dealing with weapons
#include <math.h>
#include <float.h>

#include "cg_local.h"
#include "fx_local.h"
#include "cg_weapons.h"

#include "ghoul2/g2.h"

extern vec4_t	bluehudtint;
extern vec4_t	redhudtint;
extern float	*hudTintColor;

static weaponInfo_t cg_weapons[MAX_WEAPON_INFO_SLOTS];

void CG_InitWeapons ( void )
{
	memset (cg_weapons, 0, sizeof (cg_weapons));
}

/*
Ghoul2 Insert Start
*/
// set up the appropriate ghoul2 info to a refent
void CG_SetGhoul2InfoRef( refEntity_t *ent, refEntity_t	*s1)
{
	ent->ghoul2 = s1->ghoul2;
	VectorCopy( s1->modelScale, ent->modelScale);
	ent->radius = s1->radius;
	VectorCopy( s1->angles, ent->angles);
}

/*
Ghoul2 Insert End
*/

/*
=================
CG_RegisterItemVisuals

The server says this item is used on this level
=================
*/
void CG_RegisterItemVisuals( int itemNum ) {
	itemInfo_t		*itemInfo;
	gitem_t			*item;
	int				handle;

	if ( itemNum < 0 || itemNum >= bg_numItems ) {
		CG_Error( "CG_RegisterItemVisuals: itemNum %d out of range [0-%d]", itemNum, bg_numItems-1 );
	}

	itemInfo = &cg_items[ itemNum ];
	if ( itemInfo->registered ) {
		return;
	}

	item = &bg_itemlist[ itemNum ];

	memset( itemInfo, 0, sizeof( &itemInfo ) );
	itemInfo->registered = qtrue;

	if (item->giType == IT_TEAM &&
		(item->giTag == PW_REDFLAG || item->giTag == PW_BLUEFLAG) &&
		cgs.gametype == GT_CTY)
	{ //in CTY the flag model is different
		itemInfo->models[0] = trap_R_RegisterModel( item->world_model[1] );
	}
	else if (item->giType == IT_WEAPON &&
		(item->giTag == WP_THERMAL || item->giTag == WP_TRIP_MINE || item->giTag == WP_DET_PACK))
	{
		itemInfo->models[0] = trap_R_RegisterModel( item->world_model[1] );
	}
	else
	{
		itemInfo->models[0] = trap_R_RegisterModel( item->world_model[0] );
	}
/*
Ghoul2 Insert Start
*/
	if (!Q_stricmp(&item->world_model[0][strlen(item->world_model[0]) - 4], ".glm"))
	{
		handle = trap_G2API_InitGhoul2Model(&itemInfo->g2Models[0], item->world_model[0], 0 , 0, 0, 0, 0);
		if (handle<0)
		{
			itemInfo->g2Models[0] = NULL;
		}
		else
		{
			itemInfo->radius[0] = 60;
		}
	}
/*
Ghoul2 Insert End
*/
	if (item->icon)
	{
		if (item->giType == IT_HEALTH)
		{ //medpack gets nomip'd by the ui or something I guess.
			itemInfo->icon = trap_R_RegisterShaderNoMip( item->icon );
		}
		else
		{
			itemInfo->icon = trap_R_RegisterShader( item->icon );
		}
	}
	else
	{
		itemInfo->icon = 0;
	}

	if ( item->giType == IT_WEAPON ) {
	    unsigned int numVariations = BG_NumberOfWeaponVariations (item->giTag);
	    unsigned int i;
	    for ( i = 0; i < numVariations; i++ )
	    {
		    CG_RegisterWeapon( item->giTag, i );
		}
	}

	//
	// powerups have an accompanying ring or sphere
	//
	if ( item->giType == IT_POWERUP || item->giType == IT_HEALTH || 
		item->giType == IT_ARMOR || item->giType == IT_HOLDABLE ) {
		if ( item->world_model[1] ) {
			itemInfo->models[1] = trap_R_RegisterModel( item->world_model[1] );
		}
	}
}


/*
========================================================================================

VIEW WEAPON

========================================================================================
*/

#define WEAPON_FORCE_BUSY_HOLSTER

#ifdef WEAPON_FORCE_BUSY_HOLSTER
//rww - this was done as a last resort. Forgive me.
static int cgWeapFrame = 0;
static int cgWeapFrameTime = 0;
#endif

/*
=================
CG_MapTorsoToWeaponFrame

=================
*/
static int CG_MapTorsoToWeaponFrame( const clientInfo_t *ci, int frame, int animNum ) {
	animation_t *animations = bgHumanoidAnimations;
#ifdef WEAPON_FORCE_BUSY_HOLSTER
	if (cg.snap->ps.forceHandExtend != HANDEXTEND_NONE || cgWeapFrameTime > cg.time)
	{ //the reason for the after delay is so that it doesn't snap the weapon frame to the "idle" (0) frame
		//for a very quick moment
		if (cgWeapFrame < 6)
		{
			cgWeapFrame = 6;
			cgWeapFrameTime = cg.time + 10;
		}

		if (cgWeapFrameTime < cg.time && cgWeapFrame < 10)
		{
			cgWeapFrame++;
			cgWeapFrameTime = cg.time + 10;
		}

		if (cg.snap->ps.forceHandExtend != HANDEXTEND_NONE &&
			cgWeapFrame == 10)
		{
			cgWeapFrameTime = cg.time + 100;
		}

		return cgWeapFrame;
	}
	else
	{
		cgWeapFrame = 0;
		cgWeapFrameTime = 0;
	}
#endif

    // TODO: Fix this properly. This code sort of works, but sort of doesn't. For now the
    // reload animations have just been changed back to base.
#if 0
    if ( (cg.reloadTimeStart + cg.reloadTimeDuration) >= cg.time )
    {
        // So much hax...must do this a better way.
        int animFrame = frame - animations[animNum].firstFrame;
        
        if ( animFrame >= (animations[animNum].numFrames - 3) )
        {
            animFrame -= animations[animNum].numFrames - 3;
            animFrame += 10;
        }
        else
        {
            animFrame = min (animFrame, 4);
            animFrame += 6;
        }

		return animFrame;
    }
#endif

	switch( animNum )
	{
	case TORSO_DROPWEAP1:
		if ( frame >= animations[animNum].firstFrame && frame < animations[animNum].firstFrame + 5 ) 
		{
			return frame - animations[animNum].firstFrame + 6;
		}
		break;

	case TORSO_RAISEWEAP1:
		if ( frame >= animations[animNum].firstFrame && frame < animations[animNum].firstFrame + 4 ) 
		{
			return frame - animations[animNum].firstFrame + 6 + 4;
		}
		break;
	case BOTH_ATTACK1:
	case BOTH_ATTACK2:
	case BOTH_ATTACK3:
	case BOTH_ATTACK4:
	case BOTH_ATTACK10:
	case BOTH_THERMAL_THROW:
		if ( frame >= animations[animNum].firstFrame && frame < animations[animNum].firstFrame + 6 ) 
		{
			return 1 + ( frame - animations[animNum].firstFrame );
		}

		break;
	}	
	return -1;
}

//=========================================================
// Description:
// Updates the animations for the current view model.
//=========================================================
void CG_AnimateViewWeapon ( const playerState_t *ps )
{
    weaponInfo_t *weapon = NULL;
    
	weapon = CG_WeaponInfo (ps->weapon, ps->weaponVariation);
	
	if ( ps->persistant[PERS_TEAM] == TEAM_SPECTATOR )
	{
	    return;
	}
	
	if ( ps->pm_type == PM_INTERMISSION )
	{
	    return;
	}
	
	if ( !weapon->g2ViewModel )
	{
	    return;
	}
	
	if ( cg.viewWeaponAnimation != NULL )
	{
	    animation_t *anim = cg.viewWeaponAnimation;
	    trap_G2API_SetBoneAnim (weapon->g2ViewModel, 0, "model_root", anim->firstFrame, anim->numFrames, BONE_ANIM_OVERRIDE | BONE_ANIM_BLEND, 50.0f / anim->frameLerp, cg.time, anim->firstFrame, 150);
	    
	    cg.viewWeaponAnimation = NULL;
	}
}


/*
==============
CG_CalculateWeaponPosition
==============
*/
static void CG_CalculateWeaponPosition( vec3_t origin, vec3_t angles ) {
	float	scale;
	int		delta;
	float	fracsin;

	VectorCopy( cg.refdef.vieworg, origin );
	VectorCopy( cg.refdef.viewangles, angles );

	// on odd legs, invert some angles
	if ( cg.bobcycle & 1 ) {
		scale = -cg.xyspeed;
	} else {
		scale = cg.xyspeed;
	}

	// gun angles from bobbing
	angles[ROLL] += scale * cg.bobfracsin * 0.005;
	angles[YAW] += scale * cg.bobfracsin * 0.01;
	angles[PITCH] += cg.xyspeed * cg.bobfracsin * 0.005;

	// drop the weapon when landing
	delta = cg.time - cg.landTime;
	if ( delta < LAND_DEFLECT_TIME ) {
		origin[2] += cg.landChange*0.25 * delta / LAND_DEFLECT_TIME;
	} else if ( delta < LAND_DEFLECT_TIME + LAND_RETURN_TIME ) {
		origin[2] += cg.landChange*0.25 * 
			(LAND_DEFLECT_TIME + LAND_RETURN_TIME - delta) / LAND_RETURN_TIME;
	}

#if 0
	// drop the weapon when stair climbing
	delta = cg.time - cg.stepTime;
	if ( delta < STEP_TIME/2 ) {
		origin[2] -= cg.stepChange*0.25 * delta / (STEP_TIME/2);
	} else if ( delta < STEP_TIME ) {
		origin[2] -= cg.stepChange*0.25 * (STEP_TIME - delta) / (STEP_TIME/2);
	}
#endif

	// idle drift
	scale = cg.xyspeed + 40;
	fracsin = sin( cg.time * 0.001 );
	angles[ROLL] += scale * fracsin * 0.01;
	angles[YAW] += scale * fracsin * 0.01;
	angles[PITCH] += scale * fracsin * 0.01;
}


/*
===============
CG_LightningBolt

Origin will be the exact tag point, which is slightly
different than the muzzle point used for determining hits.
The cent should be the non-predicted cent if it is from the player,
so the endpoint will reflect the simulated strike (lagging the predicted
angle)
===============
*/
static void CG_LightningBolt( centity_t *cent, vec3_t origin ) {
//	trace_t  trace;
	refEntity_t  beam;
//	vec3_t   forward;
//	vec3_t   muzzlePoint, endPoint;

	//Must be a durational weapon that continuously generates an effect.
	if ( cent->currentState.weapon == WP_DEMP2 && cent->currentState.eFlags & EF_ALT_FIRING ) 
	{ /*nothing*/ }
	else
	{
		return;
	}

	memset( &beam, 0, sizeof( beam ) );

	// NOTENOTE No lightning gun-ish stuff yet.
/*
	// CPMA  "true" lightning
	if ((cent->currentState.number == cg.predictedPlayerState.clientNum) && (cg_trueLightning.value != 0)) {
		vec3_t angle;
		int i;

		for (i = 0; i < 3; i++) {
			float a = cent->lerpAngles[i] - cg.refdef.viewangles[i];
			if (a > 180) {
				a -= 360;
			}
			if (a < -180) {
				a += 360;
			}

			angle[i] = cg.refdef.viewangles[i] + a * (1.0 - cg_trueLightning.value);
			if (angle[i] < 0) {
				angle[i] += 360;
			}
			if (angle[i] > 360) {
				angle[i] -= 360;
			}
		}

		AngleVectors(angle, forward, NULL, NULL );
		VectorCopy(cent->lerpOrigin, muzzlePoint );
//		VectorCopy(cg.refdef.vieworg, muzzlePoint );
	} else {
		// !CPMA
		AngleVectors( cent->lerpAngles, forward, NULL, NULL );
		VectorCopy(cent->lerpOrigin, muzzlePoint );
	}

	// FIXME: crouch
	muzzlePoint[2] += DEFAULT_VIEWHEIGHT;

	VectorMA( muzzlePoint, 14, forward, muzzlePoint );

	// project forward by the lightning range
	VectorMA( muzzlePoint, LIGHTNING_RANGE, forward, endPoint );

	// see if it hit a wall
	CG_Trace( &trace, muzzlePoint, vec3_origin, vec3_origin, endPoint, 
		cent->currentState.number, MASK_SHOT );

	// this is the endpoint
	VectorCopy( trace.endpos, beam.oldorigin );

	// use the provided origin, even though it may be slightly
	// different than the muzzle origin
	VectorCopy( origin, beam.origin );

	beam.reType = RT_LIGHTNING;
	beam.customShader = cgs.media.lightningShader;
	trap_R_AddRefEntityToScene( &beam );
*/

	// NOTENOTE No lightning gun-ish stuff yet.
/*
	// add the impact flare if it hit something
	if ( trace.fraction < 1.0 ) {
		vec3_t	angles;
		vec3_t	dir;

		VectorSubtract( beam.oldorigin, beam.origin, dir );
		VectorNormalize( dir );

		memset( &beam, 0, sizeof( beam ) );
		beam.hModel = cgs.media.lightningExplosionModel;

		VectorMA( trace.endpos, -16, dir, beam.origin );

		// make a random orientation
		angles[0] = rand() % 360;
		angles[1] = rand() % 360;
		angles[2] = rand() % 360;
		AnglesToAxis( angles, beam.axis );
		trap_R_AddRefEntityToScene( &beam );
	}
*/
}


/*
========================
CG_AddWeaponWithPowerups
========================
*/
static void CG_AddWeaponWithPowerups( refEntity_t *gun, int powerups ) {
	// add powerup effects
	trap_R_AddRefEntityToScene( gun );

	if (cg.predictedPlayerState.electrifyTime > cg.time)
	{ //add electrocution shell
		int preShader = gun->customShader;
		if ( rand() & 1 )
		{
			gun->customShader = cgs.media.electricBodyShader;	
		}
		else
		{
			gun->customShader = cgs.media.electricBody2Shader;
		}
		trap_R_AddRefEntityToScene( gun );
		gun->customShader = preShader; //set back just to be safe
	}
}


/*
=============
CG_AddPlayerWeapon

Used for both the view weapon (ps is valid) and the world modelother character models (ps is NULL)
The main player will have this called for BOTH cases, so effects like light and
sound should only be done on the world model case.
=============
*/

// JKG - Weapon indicators
void JKG_WeaponIndicators_Update(const centity_t *cent, const playerState_t *ps);
extern vec3_t cg_crosshairPos;

void CG_AddPlayerWeapon( refEntity_t *parent, playerState_t *ps, centity_t *cent, int team, vec3_t newAngles, qboolean thirdPerson ) {
	refEntity_t	gun;
	refEntity_t	barrel;
	vec3_t		angles;
	weapon_t	weaponNum;
	weaponInfo_t	*weapon;
	centity_t	*nonPredictedCent;
	refEntity_t	flash;

	weaponNum = cent->currentState.weapon;

	if (cent->currentState.weapon == WP_EMPLACED_GUN)
	{
		return;
	}

	if (cg.predictedPlayerState.pm_type == PM_SPECTATOR &&
		cent->currentState.number == cg.predictedPlayerState.clientNum)
	{ //spectator mode, don't draw it...
		return;
	}

	// JKG - Update weapon indicators
	JKG_WeaponIndicators_Update(cent, ps);

	weapon = CG_WeaponInfo (weaponNum, cent->currentState.weaponVariation);
/*
Ghoul2 Insert Start
*/

	memset( &gun, 0, sizeof( gun ) );

	// only do this if we are in first person, since world weapons are now handled on the server by Ghoul2
	if (!thirdPerson)
	{

		// add the weapon
		VectorCopy( parent->lightingOrigin, gun.lightingOrigin );
		gun.shadowPlane = parent->shadowPlane;
		gun.renderfx = parent->renderfx;

		if (ps)
		{	// this player, in first person view
		    if ( weapon->g2ViewModel )
		    {
			    gun.ghoul2 = weapon->g2ViewModel;
			    gun.radius = 32.0f;
			    if ( !gun.ghoul2 )
			    {
			        return;
			    }
			}
			else
			{
			    gun.hModel = weapon->viewModel;
			    if ( !gun.hModel )
			    {
			        return;
			    }
			}
		}
		else
		{
			gun.hModel = weapon->weaponModel;
			
			if (!gun.hModel) {
			    return;
		    }
		}

		if ( !ps ) {
			// add weapon ready sound
			cent->pe.lightningFiring = qfalse;
			if ( ( cent->currentState.eFlags & EF_FIRING ) && weapon->firingSound ) {
				// lightning gun and guantlet make a different sound when fire is held down
				trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin, weapon->firingSound );
				cent->pe.lightningFiring = qtrue;
			} else if ( weapon->readySound ) {
				trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin, weapon->readySound );
			}
		}

		CG_PositionEntityOnTag( &gun, parent, parent->hModel, "tag_weapon");

		if (!CG_IsMindTricked(cent->currentState.trickedentindex,
			cent->currentState.trickedentindex2,
			cent->currentState.trickedentindex3,
			cent->currentState.trickedentindex4,
			cg.snap->ps.clientNum))
		{
			CG_AddWeaponWithPowerups( &gun, cent->currentState.powerups ); //don't draw the weapon if the player is invisible
			/*
			if ( weaponNum == WP_STUN_BATON )
			{
				gun.shaderRGBA[0] = gun.shaderRGBA[1] = gun.shaderRGBA[2] = 25;
	
				gun.customShader = trap_R_RegisterShader( "gfx/effects/stunPass" );
				gun.renderfx = RF_RGB_TINT | RF_FIRST_PERSON | RF_DEPTHHACK;
				trap_R_AddRefEntityToScene( &gun );
			}
			*/
		}

		if (weaponNum == WP_STUN_BATON)
		{
			int i = 0;

			while (i < 3)
			{
				memset( &barrel, 0, sizeof( barrel ) );
				VectorCopy( parent->lightingOrigin, barrel.lightingOrigin );
				barrel.shadowPlane = parent->shadowPlane;
				barrel.renderfx = parent->renderfx;

				if (i == 0)
				{
					barrel.hModel = trap_R_RegisterModel("models/weapons2/stun_baton/baton_barrel.md3");
				}
				else if (i == 1)
				{
					barrel.hModel = trap_R_RegisterModel("models/weapons2/stun_baton/baton_barrel2.md3");
				}
				else
				{
					barrel.hModel = trap_R_RegisterModel("models/weapons2/stun_baton/baton_barrel3.md3");
				}
				angles[YAW] = 0;
				angles[PITCH] = 0;
				angles[ROLL] = 0;

				AnglesToAxis( angles, barrel.axis );

				if (i == 0)
				{
					CG_PositionRotatedEntityOnTag( &barrel, parent/*&gun*/, /*weapon->weaponModel*/weapon->handsModel, "tag_barrel" );
				}
				else if (i == 1)
				{
					CG_PositionRotatedEntityOnTag( &barrel, parent/*&gun*/, /*weapon->weaponModel*/weapon->handsModel, "tag_barrel2" );
				}
				else
				{
					CG_PositionRotatedEntityOnTag( &barrel, parent/*&gun*/, /*weapon->weaponModel*/weapon->handsModel, "tag_barrel3" );
				}
				CG_AddWeaponWithPowerups( &barrel, cent->currentState.powerups );

				i++;
			}
		}
		else
		{
            #if 0
			// add the spinning barrel
			if ( weapon->barrelModel ) {
				memset( &barrel, 0, sizeof( barrel ) );
				VectorCopy( parent->lightingOrigin, barrel.lightingOrigin );
				barrel.shadowPlane = parent->shadowPlane;
				barrel.renderfx = parent->renderfx;

				barrel.hModel = weapon->barrelModel;
				angles[YAW] = 0;
				angles[PITCH] = 0;
				angles[ROLL] = 0;

				AnglesToAxis( angles, barrel.axis );

				CG_PositionRotatedEntityOnTag( &barrel, parent/*&gun*/, /*weapon->weaponModel*/weapon->handsModel, "tag_barrel" );

				CG_AddWeaponWithPowerups( &barrel, cent->currentState.powerups );
			}
			#endif
		}
	}
/*
Ghoul2 Insert End
*/

	
	// E-11 Laser sight (WIP)
	/*
	if (weaponNum  == WP_BLASTER) {
		refEntity_t tmp;
		trace_t tr;
		vec3_t org;
		vec3_t dest;
		vec3_t delta;
		vec3_t dir;
		memset( &tmp, 0, sizeof( tmp ) );
		CG_PositionEntityOnTag( &tmp, &gun, gun.hModel, "tag_laser");
		if (thirdPerson || !ps) {	
			VectorCopy(tmp.origin, dest);
			VectorCopy(tmp.axis[0], dir);
			VectorMA(dest, 256, dir, org);
			CG_G2Trace(&tr, dest, NULL, NULL, org, cent->currentState.number, MASK_PLAYERSOLID);
			VectorCopy(tr.endpos, org);
			VectorSubtract(dest, org, delta);
			vectoangles(delta, dir);
			AngleVectors(dir, dir, NULL, NULL);
		} else {
			VectorCopy(tmp.origin, dest);
			VectorCopy(cg_crosshairPos, org);
			VectorSubtract(dest, org, delta);
			vectoangles(delta, dir);
			AngleVectors(dir, dir, NULL, NULL);
		}

		trap_FX_PlayEffectID(trap_FX_RegisterEffect("blaster/laser.efx"), org, dir, -1, -1);				
	}
	*/

	memset (&flash, 0, sizeof(flash));
	CG_PositionEntityOnTag( &flash, &gun, gun.hModel, "tag_flash");

	VectorCopy(flash.origin, cg.lastFPFlashPoint);

	// Do special charge bits
	//-----------------------
	//[TrueView]
	//Make the guns do their charging visual in True View.
	if ( (ps || cg.renderingThirdPerson || cg.predictedPlayerState.clientNum != cent->currentState.number || cg_trueguns.integer) &&
	//if ( (ps || cg.renderingThirdPerson || cg.predictedPlayerState.clientNum != cent->currentState.number) &&
	//[/TrueView]
		( ( cent->currentState.modelindex2 == WEAPON_CHARGING_ALT && cent->currentState.weapon == WP_BRYAR_PISTOL ) ||
		  ( cent->currentState.modelindex2 == WEAPON_CHARGING_ALT && cent->currentState.weapon == WP_BRYAR_OLD ) ||
		  ( cent->currentState.weapon == WP_BOWCASTER && cent->currentState.modelindex2 == WEAPON_CHARGING ) ||
		  ( cent->currentState.weapon == WP_DEMP2 && cent->currentState.modelindex2 == WEAPON_CHARGING_ALT) ) )
	{
		int		shader = 0;
		float	val = 0.0f;
		float	scale = 1.0f;
		addspriteArgStruct_t fxSArgs;
		vec3_t flashorigin, flashdir;

		if (!thirdPerson)
		{
			VectorCopy(flash.origin, flashorigin);
			VectorCopy(flash.axis[0], flashdir);
		}
		else
		{
			mdxaBone_t 		boltMatrix;

			if (!trap_G2API_HasGhoul2ModelOnIndex(&(cent->ghoul2), 1))
			{ //it's quite possible that we may have have no weapon model and be in a valid state, so return here if this is the case
				return;
			}

			// go away and get me the bolt position for this frame please
 			if (!(trap_G2API_GetBoltMatrix(cent->ghoul2, 1, 0, &boltMatrix, newAngles, cent->lerpOrigin, cg.time, cgs.gameModels, cent->modelScale)))
			{	// Couldn't find bolt point.
				return;
			}
			
			BG_GiveMeVectorFromMatrix(&boltMatrix, ORIGIN, flashorigin);
			BG_GiveMeVectorFromMatrix(&boltMatrix, POSITIVE_X, flashdir);
		}

		if ( cent->currentState.weapon == WP_BRYAR_PISTOL ||
			cent->currentState.weapon == WP_BRYAR_OLD)
		{
			// Hardcoded max charge time of 1 second
			val = ( cg.time - cent->currentState.constantLight ) * 0.001f;
			shader = cgs.media.bryarFrontFlash;
		}
		else if ( cent->currentState.weapon == WP_BOWCASTER )
		{
			// Hardcoded max charge time of 1 second
			val = ( cg.time - cent->currentState.constantLight ) * 0.001f;
			shader = cgs.media.greenFrontFlash;
		}
		else if ( cent->currentState.weapon == WP_DEMP2 )
		{
			val = ( cg.time - cent->currentState.constantLight ) * 0.001f;
			shader = cgs.media.lightningFlash;
			scale = 1.75f;
		}

		if ( val < 0.0f )
		{
			val = 0.0f;
		}
		else if ( val > 1.0f )
		{
			val = 1.0f;
			if (ps && cent->currentState.number == ps->clientNum)
			{
				CGCam_Shake( /*0.1f*/0.2f, 100 );
			}
		}
		else
		{
			if (ps && cent->currentState.number == ps->clientNum)
			{
				CGCam_Shake( val * val * /*0.3f*/0.6f, 100 );
			}
		}

		val += random() * 0.5f;

		VectorCopy(flashorigin, fxSArgs.origin);
		VectorClear(fxSArgs.vel);
		VectorClear(fxSArgs.accel);
		fxSArgs.scale = 3.0f*val*scale;
		fxSArgs.dscale = 0.0f;
		fxSArgs.sAlpha = 0.7f;
		fxSArgs.eAlpha = 0.7f;
		fxSArgs.rotation = random()*360;
		fxSArgs.bounce = 0.0f;
		fxSArgs.life = 1.0f;
		fxSArgs.shader = shader;
		fxSArgs.flags = 0x08000000;

		//FX_AddSprite( flash.origin, NULL, NULL, 3.0f * val, 0.0f, 0.7f, 0.7f, WHITE, WHITE, random() * 360, 0.0f, 1.0f, shader, FX_USE_ALPHA );
		trap_FX_AddSprite(&fxSArgs);
	}

	// make sure we aren't looking at cg.predictedPlayerEntity for LG
	nonPredictedCent = &cg_entities[cent->currentState.clientNum];

	// if the index of the nonPredictedCent is not the same as the clientNum
	// then this is a fake player (like on teh single player podiums), so
	// go ahead and use the cent
	if( ( nonPredictedCent - cg_entities ) != cent->currentState.clientNum ) {
		nonPredictedCent = cent;
	}

	// add the flash
	if ( ( weaponNum == WP_DEMP2)
		&& ( nonPredictedCent->currentState.eFlags & EF_FIRING ) ) 
	{
		// continuous flash
	} else {
		// impulse flash
		if ( cg.time - cent->muzzleFlashTime > MUZZLE_FLASH_TIME) {
			return;
		}
	}

	//[TrueView]
	if ( ps || cg.renderingThirdPerson || cg_trueguns.integer 
		|| cent->currentState.number != cg.predictedPlayerState.clientNum ) 
	//if ( ps || cg.renderingThirdPerson ||
	//		cent->currentState.number != cg.predictedPlayerState.clientNum ) 
	//[/TrueView] 
	{	// Make sure we don't do the thirdperson model effects for the local player if we're in first person
		vec3_t flashorigin, flashdir;
		refEntity_t	flash;

		memset (&flash, 0, sizeof(flash));

		if (!thirdPerson)
		{
			CG_PositionEntityOnTag( &flash, &gun, gun.hModel, "tag_flash");
			VectorCopy(flash.origin, flashorigin);
			VectorCopy(flash.axis[0], flashdir);
		}
		else
		{
			mdxaBone_t 		boltMatrix;

			if (!trap_G2API_HasGhoul2ModelOnIndex(&(cent->ghoul2), 1))
			{ //it's quite possible that we may have have no weapon model and be in a valid state, so return here if this is the case
				return;
			}

			// go away and get me the bolt position for this frame please
 			if (!(trap_G2API_GetBoltMatrix(cent->ghoul2, 1, 0, &boltMatrix, newAngles, cent->lerpOrigin, cg.time, cgs.gameModels, cent->modelScale)))
			{	// Couldn't find bolt point.
				return;
			}
			
			BG_GiveMeVectorFromMatrix(&boltMatrix, ORIGIN, flashorigin);
			BG_GiveMeVectorFromMatrix(&boltMatrix, POSITIVE_X, flashdir);
		}

		if ( cg.time - cent->muzzleFlashTime <= MUZZLE_FLASH_TIME + 10 )
		{	// Handle muzzle flashes
			if ( cent->currentState.eFlags & EF_ALT_FIRING )
			{
				// Check the alt firing first.
				if (weapon->altMuzzleEffect)
				{
					if (!thirdPerson)
					{
						trap_FX_PlayEntityEffectID(weapon->altMuzzleEffect, flashorigin, flash.axis, -1, -1, -1, -1  );
					}
					else
					{
						trap_FX_PlayEffectID(weapon->altMuzzleEffect, flashorigin, flashdir, -1, -1);
					}
				}
			}
			else
			{
				// Regular firing
				if (weapon->muzzleEffect)
				{
					if (!thirdPerson)
					{
						trap_FX_PlayEntityEffectID(weapon->muzzleEffect, flashorigin, flash.axis, -1, -1, -1, -1  );
					}
					else
					{
						trap_FX_PlayEffectID(weapon->muzzleEffect, flashorigin, flashdir, -1, -1);
					}
				}
			}
		}

		// add lightning bolt
		CG_LightningBolt( nonPredictedCent, flashorigin );

		if ( weapon->flashDlightColor[0] || weapon->flashDlightColor[1] || weapon->flashDlightColor[2] ) {
			trap_R_AddLightToScene( flashorigin, 300 + (rand()&31), weapon->flashDlightColor[0],
				weapon->flashDlightColor[1], weapon->flashDlightColor[2] );
		}
	}
}

/*
==============
CG_AddViewWeapon

Add the weapon, and flash for the player's view
==============
*/

void CG_AddViewWeapon( playerState_t *ps ) {
	refEntity_t	hand;
	centity_t	*cent;
	clientInfo_t	*ci;
	float		fovOffset;
	vec3_t		angles;
	weaponInfo_t	*weapon;
	//[TrueView]
	float	cgFov;

	if(!cg.renderingThirdPerson && (cg_trueguns.integer || cg.predictedPlayerState.weapon == WP_SABER
	|| cg.predictedPlayerState.weapon == WP_MELEE) && cg_truefov.value 
		&& (cg.predictedPlayerState.pm_type != PM_SPECTATOR)
		&& (cg.predictedPlayerState.pm_type != PM_INTERMISSION))
	{
		cgFov = cg_truefov.value;
	}
	else
	{
		cgFov = cg_fov.value;
	}
	//float	cgFov = cg_fov.value;
	//[TrueView]

	if (cgFov < 1)
	{
		cgFov = 1;
	}

	if (cgFov > 97)
	{
		cgFov = 97;
	}

	if ( ps->persistant[PERS_TEAM] == TEAM_SPECTATOR ) {
		return;
	}

	if ( ps->pm_type == PM_INTERMISSION ) {
		return;
	}

	// no gun if in third person view or a camera is active
	//if ( cg.renderingThirdPerson || cg.cameraMode) {
	if ( cg.renderingThirdPerson ) {
		return;
	}

	/* JKG - Don't render weapons while in a vehicle, uhuu */
	if (cg.snap && cg.snap->ps.m_iVehicleNum )
	{
		return;
	}

	// allow the gun to be completely removed
		//[TrueView]
	if ( !cg_drawGun.integer || cg.predictedPlayerState.zoomMode || cg_trueguns.integer
		|| cg.predictedPlayerState.weapon == WP_SABER || cg.predictedPlayerState.weapon == WP_MELEE) {
	//if ( !cg_drawGun.integer || cg.predictedPlayerState.zoomMode) {
	//[TrueView]
		vec3_t		origin;

		if ( cg.predictedPlayerState.eFlags & EF_FIRING ) {
			// special hack for lightning gun...
			VectorCopy( cg.refdef.vieworg, origin );
			VectorMA( origin, -8, cg.refdef.viewaxis[2], origin );
			CG_LightningBolt( &cg_entities[ps->clientNum], origin );
		}
		return;
	}

	// don't draw if testing a gun model
	if ( cg.testGun ) {
		return;
	}

	// drop gun lower at higher fov
	if ( cgFov > 90 ) {
		fovOffset = -0.2 * ( cgFov - 90 );
	} else {
		fovOffset = 0;
	}

	cent = &cg_entities[cg.predictedPlayerState.clientNum];
	weapon = CG_WeaponInfo (ps->weapon, ps->weaponVariation);

	memset (&hand, 0, sizeof(hand));

	// set up gun position
	CG_CalculateWeaponPosition( hand.origin, angles );
	{
		float xoffs = cg_gun_x.value, yoffs = cg_gun_y.value, zoffs = cg_gun_z.value;
		/*if (ps->weapon == WP_BLASTER) {
			xoffs += 1;
			zoffs -= 0.5;
		} else if (ps->weapon == WP_REPEATER) {
			// Clone rifle hack
			xoffs -= 5;
		}*/
		VectorMA( hand.origin, xoffs, cg.refdef.viewaxis[0], hand.origin );
		VectorMA( hand.origin, yoffs, cg.refdef.viewaxis[1], hand.origin );
		VectorMA( hand.origin, (zoffs+fovOffset), cg.refdef.viewaxis[2], hand.origin );
	}
	AnglesToAxis( angles, hand.axis );

	// map torso animations to weapon animations
	if ( cg_gun_frame.integer ) {
		// development tool
		hand.frame = hand.oldframe = cg_gun_frame.integer;
		hand.backlerp = 0;
	} else {
		// get clientinfo for animation map
		if (cent->currentState.eType == ET_NPC)
		{
			if (!cent->npcClient)
			{
				return;
			}

			ci = cent->npcClient;
		}
		else
		{
			ci = &cgs.clientinfo[ cent->currentState.clientNum ];
		}		
			
		hand.frame = CG_MapTorsoToWeaponFrame( ci, cent->pe.torso.frame, cent->currentState.torsoAnim );
		hand.oldframe = CG_MapTorsoToWeaponFrame( ci, cent->pe.torso.oldFrame, cent->currentState.torsoAnim );
		hand.backlerp = cent->pe.torso.backlerp;
	
		// Handle the fringe situation where oldframe is invalid
		if ( hand.frame == -1 )
		{
			hand.frame = 0;
			hand.oldframe = 0;
			hand.backlerp = 0;
		}
		else if ( hand.oldframe == -1 )
		{
			hand.oldframe = hand.frame;
			hand.backlerp = 0;
		}
	}

	hand.hModel = weapon->handsModel;
	hand.renderfx = RF_DEPTHHACK | RF_FIRST_PERSON;// | RF_MINLIGHT;

	// add everything onto the hand
	CG_AddPlayerWeapon( &hand, ps, &cg_entities[cg.predictedPlayerState.clientNum], ps->persistant[PERS_TEAM], angles, qfalse );
}

/*
==============================================================================

WEAPON SELECTION

==============================================================================
*/
#define ICON_WEAPONS	0
#define ICON_FORCE		1
#define ICON_INVENTORY	2


void CG_DrawIconBackground(void)
{
	int				height,xAdd,x2,y2,t;
//	int				prongLeftX,prongRightX;
	float			inTime = cg.invenSelectTime+WEAPON_SELECT_TIME;
	float			wpTime = cg.weaponSelectTime+WEAPON_SELECT_TIME;
	float			fpTime = cg.forceSelectTime+WEAPON_SELECT_TIME;
//	int				drawType = cgs.media.weaponIconBackground;
//	int				yOffset = 0;

#ifdef _XBOX
	//yOffset = -50;
#endif

	// don't display if dead
	if ( cg.snap->ps.stats[STAT_HEALTH] <= 0 ) 
	{
		return;
	}

	if (cg_hudFiles.integer)
	{ //simple hud
		return;
	}

	x2 = 30;
	y2 = SCREEN_HEIGHT-70;

	//prongLeftX =x2+37; 
	//prongRightX =x2+544; 

	if (inTime > wpTime)
	{
//		drawType = cgs.media.inventoryIconBackground;
		cg.iconSelectTime = cg.invenSelectTime;
	}
	else
	{
//		drawType = cgs.media.weaponIconBackground;
		cg.iconSelectTime = cg.weaponSelectTime;
	}

	if (fpTime > inTime && fpTime > wpTime)
	{
//		drawType = cgs.media.forceIconBackground;
		cg.iconSelectTime = cg.forceSelectTime;
	}

	if ((cg.iconSelectTime+WEAPON_SELECT_TIME)<cg.time)	// Time is up for the HUD to display
	{
		if (cg.iconHUDActive)		// The time is up, but we still need to move the prongs back to their original position
		{
			t =  cg.time - (cg.iconSelectTime+WEAPON_SELECT_TIME);
			cg.iconHUDPercent = t/ 130.0f;
			cg.iconHUDPercent = 1 - cg.iconHUDPercent;

			if (cg.iconHUDPercent<0)
			{
				cg.iconHUDActive = qfalse;
				cg.iconHUDPercent=0;
			}

			xAdd = (int) 8*cg.iconHUDPercent;

			height = (int) (60.0f*cg.iconHUDPercent);
			//CG_DrawPic( x2+60, y2+30+yOffset, 460, -height, drawType);	// Top half
			//CG_DrawPic( x2+60, y2+30-2+yOffset, 460, height, drawType);	// Bottom half

		}
		else
		{
			xAdd = 0;
		}

		return;
	}
	//prongLeftX =x2+37; 
	//prongRightX =x2+544; 

	if (!cg.iconHUDActive)
	{
		t = cg.time - cg.iconSelectTime;
		cg.iconHUDPercent = t/ 130.0f;

		// Calc how far into opening sequence we are
		if (cg.iconHUDPercent>1)
		{
			cg.iconHUDActive = qtrue;
			cg.iconHUDPercent=1;
		}
		else if (cg.iconHUDPercent<0)
		{
			cg.iconHUDPercent=0;
		}
	}
	else
	{
		cg.iconHUDPercent=1;
	}

	//trap_R_SetColor( colorTable[CT_WHITE] );					
	//height = (int) (60.0f*cg.iconHUDPercent);
	//CG_DrawPic( x2+60, y2+30+yOffset, 460, -height, drawType);	// Top half
	//CG_DrawPic( x2+60, y2+30-2+yOffset, 460, height, drawType);	// Bottom half

	// And now for the prongs
/*	if ((cg.inventorySelectTime+WEAPON_SELECT_TIME)>cg.time)	
	{
		cgs.media.currentBackground = ICON_INVENTORY;
		background = &cgs.media.inventoryProngsOn;
	}
	else if ((cg.weaponSelectTime+WEAPON_SELECT_TIME)>cg.time)	
	{
		cgs.media.currentBackground = ICON_WEAPONS;
	}
	else 
	{
		cgs.media.currentBackground = ICON_FORCE;
		background = &cgs.media.forceProngsOn;
	}
*/
	// Side Prongs
//	trap_R_SetColor( colorTable[CT_WHITE]);					
//	xAdd = (int) 8*cg.iconHUDPercent;
//	CG_DrawPic( prongLeftX+xAdd, y2-10, 40, 80, background);
//	CG_DrawPic( prongRightX-xAdd, y2-10, -40, 80, background);

}

qboolean CG_WeaponCheck(int weap)
{
	if ( cg.snap->ps.ammo[GetWeaponAmmoIndexSingle( weap )] < GetWeaponPrimaryCostSingle( weap ) && cg.snap->ps.ammo[GetWeaponAmmoIndexSingle( weap )] < GetWeaponSecondaryCostSingle( weap ))
	{
		return qfalse;
	}

	return qtrue;
}

/*
===============
CG_WeaponSelectable
===============
*/
static qboolean CG_WeaponSelectable( int i ) {
	/*if ( !cg.snap->ps.ammo[weaponData[i].ammoIndex] ) {
		return qfalse;
	}*/
	if (!i)
	{
		return qfalse;
	}

	// Jedi Knight Galaxies
	// Weapons without ammo are still selectable, they just dont fire!

	/*if (cg.predictedPlayerState.ammo[weaponData[i].ammoIndex] < weaponData[i].energyPerShot &&
		cg.predictedPlayerState.ammo[weaponData[i].ammoIndex] < weaponData[i].altEnergyPerShot)
	{
		return qfalse;
	}

	if (i == WP_DET_PACK && cg.predictedPlayerState.ammo[weaponData[i].ammoIndex] < 1 &&
		!cg.predictedPlayerState.hasDetPackPlanted)
	{
		return qfalse;
	}*/

	if ( ! (cg.predictedPlayerState.stats[ STAT_WEAPONS ] & ( 1 << i ) ) ) {
		return qfalse;
	}

	return qtrue;
}

/*
===================
CG_DrawWeaponSelect
===================
*/
#ifdef _XBOX
extern bool CL_ExtendSelectTime(void);
#endif
void CG_DrawWeaponSelect( void ) {
	int				i;
	int				bits;
	int				count;
	int				smallIconSize,bigIconSize;
	int				holdX,x,y,pad;
	int				sideLeftIconCnt,sideRightIconCnt;
	int				sideMax,holdCount,iconCnt;
	int				height;
	int		yOffset = 0;
	//qboolean drewConc = qfalse;
	const weaponData_t *weaponData;
	static const vec4_t textColor = { .875f, .718f, .121f, 1.0f };
	char text[MAX_STRING_CHARS] = { 0 };

	if (cg.predictedPlayerState.emplacedIndex)
	{ //can't cycle when on a weapon
		cg.weaponSelectTime = 0;
	}

	if ((cg.weaponSelectTime+WEAPON_SELECT_TIME)<cg.time)	// Time is up for the HUD to display
	{
		return;
	}

	// don't display if dead
	if ( cg.predictedPlayerState.stats[STAT_HEALTH] <= 0 ) 
	{
		return;
	}

#ifdef _XBOX
	if(CL_ExtendSelectTime()) {
		cg.weaponSelectTime = cg.time;
	}

	yOffset = -50;
#endif

	// showing weapon select clears pickup item display, but not the blend blob
	cg.itemPickupTime = 0;

	bits = cg.predictedPlayerState.stats[ STAT_WEAPONS ];

	// count the number of weapons owned
	count = 0;

	if ( !CG_WeaponSelectable(cg.weaponSelect) &&
		(cg.weaponSelect == WP_THERMAL || cg.weaponSelect == WP_TRIP_MINE) )
	{ //display this weapon that we don't actually "have" as unhighlighted until it's deselected
	  //since it's selected we must increase the count to display the proper number of valid selectable weapons
		count++;
	}

	for ( i = 1 ; i < LAST_USEABLE_WEAPON ; i++ ) 
	{
		if ( bits & ( 1 << i ) ) 
		{
			if ( CG_WeaponSelectable(i) ||
				(i != WP_THERMAL && i != WP_TRIP_MINE) )
			{
				count++;
			}
		}
	}

	if (count == 0)	// If no weapons, don't display
	{
		return;
	}
	
	weaponData = GetWeaponData (cg.weaponSelect, 0);

	sideMax = 3;	// Max number of icons on the side

	// Calculate how many icons will appear to either side of the center one
	holdCount = count - 1;	// -1 for the center icon
	if (holdCount == 0)			// No icons to either side
	{
		sideLeftIconCnt = 0;
		sideRightIconCnt = 0;
	}
	else if (count > (2*sideMax))	// Go to the max on each side
	{
		sideLeftIconCnt = sideMax;
		sideRightIconCnt = sideMax;
	}
	else							// Less than max, so do the calc
	{
		sideLeftIconCnt = holdCount/2;
		sideRightIconCnt = holdCount - sideLeftIconCnt;
	}

	
	i = cg.weaponSelect - 1;
	if (i < 1)
	{
		i = LAST_USEABLE_WEAPON;
	}

	smallIconSize = 40;
	bigIconSize = 80;
	pad = 12;

	x = 320;
	y = 410;

	// Background
//	memcpy(calcColor, colorTable[CT_WHITE], sizeof(vec4_t));
//	calcColor[3] = .35f;
//	trap_R_SetColor( calcColor);					

	// Left side ICONS
	trap_R_SetColor(colorTable[CT_WHITE]);
	// Work backwards from current icon
	holdX = x - ((bigIconSize/2) + pad + smallIconSize);
	height = smallIconSize * 1;//cg.iconHUDPercent;
	//drewConc = qfalse;

	for (iconCnt=1;iconCnt<(sideLeftIconCnt+1);i--)
	{
		if (i < 1)
		{
			//i = 13;
			//...don't ever do this.
			i = LAST_USEABLE_WEAPON;
		}

		if ( !(bits & ( 1 << i )))	// Does he have this weapon?
		{
			continue;
		}

		if ( !CG_WeaponSelectable(i) &&
			(i == WP_THERMAL || i == WP_TRIP_MINE) )
		{ //Don't show thermal and tripmine when out of them
			continue;
		}

		++iconCnt;					// Good icon

		//if (cgs.media.weaponIcons[i])
		{
			weaponInfo_t	*weaponInfo;
			weaponInfo = CG_WeaponInfo (i, 0);

			trap_R_SetColor(colorTable[CT_WHITE]);
			if (!CG_WeaponCheck(i))
			{
				CG_DrawPic( holdX, y+10+yOffset, smallIconSize, smallIconSize, /*weaponInfo->weaponIconNoAmmo*/weaponInfo->hudNAIcon );
			}
			else
			{
				CG_DrawPic( holdX, y+10+yOffset, smallIconSize, smallIconSize, /*weaponInfo->weaponIcon*/weaponInfo->hudIcon );
			}

			holdX -= (smallIconSize+pad);
		}
	}

	// Current Center Icon
	height = bigIconSize * cg.iconHUDPercent;
	//if (cgs.media.weaponIcons[cg.weaponSelect])
	{
		weaponInfo_t	*weaponInfo;	
		weaponInfo = CG_WeaponInfo (cg.weaponSelect, 0);

		trap_R_SetColor( colorTable[CT_WHITE]);
		if (!CG_WeaponCheck(cg.weaponSelect))
		{
			CG_DrawPic( x-(bigIconSize/2), (y-((bigIconSize-smallIconSize)/2))+10+yOffset, bigIconSize, bigIconSize, weaponInfo->hudNAIcon );
		}
		else
		{
			CG_DrawPic( x-(bigIconSize/2), (y-((bigIconSize-smallIconSize)/2))+10+yOffset, bigIconSize, bigIconSize, weaponInfo->hudIcon );
		}
	}

	i = cg.weaponSelect + 1;
	if (i > LAST_USEABLE_WEAPON)
	{
		i = 1;
	}

	// Right side ICONS
	// Work forwards from current icon
	holdX = x + (bigIconSize/2) + pad;
	height = smallIconSize * cg.iconHUDPercent;
	for (iconCnt=1;iconCnt<(sideRightIconCnt+1);i++)
	{
		if (i>LAST_USEABLE_WEAPON)
		{
			i = 1;
		}

		if ( !(bits & ( 1 << i )))	// Does he have this weapon?
		{
			continue;
		}

		if ( !CG_WeaponSelectable(i) &&
			(i == WP_THERMAL || i == WP_TRIP_MINE) )
		{ //Don't show thermal and tripmine when out of them
			continue;
		}

		++iconCnt;					// Good icon

		//if (/*weaponData[i].weaponIcon[0]*/cgs.media.weaponIcons[i])
		{
			weaponInfo_t	*weaponInfo = CG_WeaponInfo (i, 0);
			// No ammo for this weapon?
			trap_R_SetColor( colorTable[CT_WHITE]);
			if (!CG_WeaponCheck(i))
			{
				CG_DrawPic( holdX, y+10+yOffset, smallIconSize, smallIconSize, weaponInfo->hudNAIcon );
			}
			else
			{
				CG_DrawPic( holdX, y+10+yOffset, smallIconSize, smallIconSize, weaponInfo->hudIcon );
			}


			holdX += (smallIconSize+pad);
		}
	}

	// draw the selected name
	if ( weaponData->visuals.displayName[0] == '@' &&
	    trap_SP_GetStringTextString (weaponData->visuals.displayName + 1, text, sizeof (text)) )
	{
	    UI_DrawProportionalString(320, y+45+yOffset, text, UI_CENTER|UI_SMALLFONT, textColor);
	}
	else
	{
	    UI_DrawProportionalString(320, y+45+yOffset, weaponData->visuals.displayName, UI_CENTER|UI_SMALLFONT, textColor);
	}

	trap_R_SetColor( NULL );
}


/*
===============
CG_NextWeapon_f
===============
*/
void CG_NextWeapon_f( void ) {
	int		i;
	int		original;

	if ( !cg.snap ) {
		return;
	}
	if ( cg.snap->ps.pm_flags & PMF_FOLLOW ) {
		return;
	}

	if (cg.predictedPlayerState.pm_type == PM_SPECTATOR)
	{
		return;
	}

	if (cg.snap->ps.emplacedIndex)
	{
		return;
	}

	cg.weaponSelectTime = cg.time;
	original = cg.weaponSelect;

	for ( i = 0 ; i < WP_NUM_WEAPONS ; i++ ) {
		//*SIGH*... Hack to put concussion rifle before rocketlauncher
		/*if ( cg.weaponSelect == WP_FLECHETTE )
		{
			cg.weaponSelect = WP_CONCUSSION;
		}
		else if ( cg.weaponSelect == WP_CONCUSSION )
		{
			cg.weaponSelect = WP_ROCKET_LAUNCHER;
		}
		else if ( cg.weaponSelect == WP_DET_PACK )
		{
			cg.weaponSelect = WP_BRYAR_OLD;
		}
		else*/
		{
			cg.weaponSelect++;
		}
		if ( cg.weaponSelect == WP_NUM_WEAPONS ) {
			cg.weaponSelect = 0;
		}
	//	if ( cg.weaponSelect == WP_STUN_BATON ) {
	//		continue;		// never cycle to gauntlet
	//	}
		if ( CG_WeaponSelectable( cg.weaponSelect ) ) {
			break;
		}
	}
	if ( i == WP_NUM_WEAPONS ) {
		cg.weaponSelect = original;
	}
	else
	{
		trap_S_MuteSound(cg.snap->ps.clientNum, CHAN_WEAPON);
	}
}

/*
===============
CG_PrevWeapon_f
===============
*/
void CG_PrevWeapon_f( void ) {
	int		i;
	int		original;

	if ( !cg.snap ) {
		return;
	}
	if ( cg.snap->ps.pm_flags & PMF_FOLLOW ) {
		return;
	}

	if (cg.predictedPlayerState.pm_type == PM_SPECTATOR)
	{
		return;
	}

	if (cg.snap->ps.emplacedIndex)
	{
		return;
	}

	cg.weaponSelectTime = cg.time;
	original = cg.weaponSelect;

	for ( i = 0 ; i < WP_NUM_WEAPONS ; i++ ) {
		//*SIGH*... Hack to put concussion rifle before rocketlauncher
		/*if ( cg.weaponSelect == WP_ROCKET_LAUNCHER )
		{
			cg.weaponSelect = WP_CONCUSSION;
		}
		else if ( cg.weaponSelect == WP_CONCUSSION )
		{
			cg.weaponSelect = WP_FLECHETTE;
		}
		else if ( cg.weaponSelect == WP_BRYAR_OLD )
		{
			cg.weaponSelect = WP_DET_PACK;
		}
		else*/
		{
			cg.weaponSelect--;
		}
		if ( cg.weaponSelect == -1 ) {
			cg.weaponSelect = WP_NUM_WEAPONS-1;
		}
	//	if ( cg.weaponSelect == WP_STUN_BATON ) {
	//		continue;		// never cycle to gauntlet
	//	}
		if ( CG_WeaponSelectable( cg.weaponSelect ) ) {
			break;
		}
	}
	if ( i == WP_NUM_WEAPONS ) {
		cg.weaponSelect = original;
	}
	else
	{
		trap_S_MuteSound(cg.snap->ps.clientNum, CHAN_WEAPON);
	}
}

/*
===============
CG_Weapon_f
===============
*/
void CG_Weapon_f( void ) {
	int		num;

	if ( !cg.snap ) {
		return;
	}
	if ( cg.snap->ps.pm_flags & PMF_FOLLOW ) {
		return;
	}

	if (cg.snap->ps.emplacedIndex)
	{
		return;
	}

	num = atoi( CG_Argv( 1 ) );

	if ( num < 1 || num > LAST_USEABLE_WEAPON ) {
		return;
	}

	if (num == 1 && cg.snap->ps.weapon == WP_SABER)
	{
		if (cg.snap->ps.weaponTime < 1)
		{
			trap_SendConsoleCommand("sv_saberswitch\n");
		}
		return;
	}

	//rww - hack to make weapon numbers same as single player
	if (num > WP_STUN_BATON)
	{
		//num++;
		num += 2; //I suppose this is getting kind of crazy, what with the wp_melee in there too now.
	}
	else
	{
		if (cg.snap->ps.stats[STAT_WEAPONS] & (1 << WP_SABER))
		{
			num = WP_SABER;
		}
		else
		{
			num = WP_MELEE;
		}
	}

	if (num > LAST_USEABLE_WEAPON+1)
	{ //other weapons are off limits due to not actually being weapon weapons
		return;
	}

	if (num >= WP_THERMAL && num <= WP_DET_PACK)
	{
		int weap, i = 0;

		if (cg.snap->ps.weapon >= WP_THERMAL &&
			cg.snap->ps.weapon <= WP_DET_PACK)
		{
			// already in cycle range so start with next cycle item
			weap = cg.snap->ps.weapon + 1;
		}
		else
		{
			// not in cycle range, so start with thermal detonator
			weap = WP_THERMAL;
		}

		// prevent an endless loop
		while ( i <= 4 )
		{
			if (weap > WP_DET_PACK)
			{
				weap = WP_THERMAL;
			}

			if (CG_WeaponSelectable(weap))
			{
				num = weap;
				break;
			}

			weap++;
			i++;
		}
	}

	if (!CG_WeaponSelectable(num))
	{
		return;
	}

	cg.weaponSelectTime = cg.time;

	if ( ! ( cg.snap->ps.stats[STAT_WEAPONS] & ( 1 << num ) ) )
	{
		if (num == WP_SABER)
		{ //don't have saber, try melee on the same slot
			num = WP_MELEE;

			if ( ! ( cg.snap->ps.stats[STAT_WEAPONS] & ( 1 << num ) ) )
			{
				return;
			}
		}
		else
		{
			return;		// don't have the weapon
		}
	}

	if (cg.weaponSelect != num)
	{
		trap_S_MuteSound(cg.snap->ps.clientNum, CHAN_WEAPON);
	}

	cg.weaponSelect = num;
}


//Version of the above which doesn't add +2 to a weapon.  The above can't
//triger WP_MELEE or WP_STUN_BATON.  Derogatory comments go here.
void CG_WeaponClean_f( void ) {
	int		num;

	if ( !cg.snap ) {
		return;
	}
	if ( cg.snap->ps.pm_flags & PMF_FOLLOW ) {
		return;
	}

	if (cg.snap->ps.emplacedIndex)
	{
		return;
	}

	num = atoi( CG_Argv( 1 ) );

	if ( num < 1 || num > LAST_USEABLE_WEAPON ) {
		return;
	}

	if (num == 1 && cg.snap->ps.weapon == WP_SABER)
	{
		if (cg.snap->ps.weaponTime < 1)
		{
			trap_SendConsoleCommand("sv_saberswitch\n");
		}
		return;
	}

	if(num == WP_STUN_BATON) {
		if (cg.snap->ps.stats[STAT_WEAPONS] & (1 << WP_SABER))
		{
			num = WP_SABER;
		}
		else
		{
			num = WP_MELEE;
		}
	}

	if (num > LAST_USEABLE_WEAPON+1)
	{ //other weapons are off limits due to not actually being weapon weapons
		return;
	}

	if (num >= WP_THERMAL && num <= WP_DET_PACK)
	{
		int weap, i = 0;

		if (cg.snap->ps.weapon >= WP_THERMAL &&
			cg.snap->ps.weapon <= WP_DET_PACK)
		{
			// already in cycle range so start with next cycle item
			weap = cg.snap->ps.weapon + 1;
		}
		else
		{
			// not in cycle range, so start with thermal detonator
			weap = WP_THERMAL;
		}

		// prevent an endless loop
		while ( i <= 4 )
		{
			if (weap > WP_DET_PACK)
			{
				weap = WP_THERMAL;
			}

			if (CG_WeaponSelectable(weap))
			{
				num = weap;
				break;
			}

			weap++;
			i++;
		}
	}

	if (!CG_WeaponSelectable(num))
	{
		return;
	}

	cg.weaponSelectTime = cg.time;

	if ( ! ( cg.snap->ps.stats[STAT_WEAPONS] & ( 1 << num ) ) )
	{
		if (num == WP_SABER)
		{ //don't have saber, try melee on the same slot
			num = WP_MELEE;

			if ( ! ( cg.snap->ps.stats[STAT_WEAPONS] & ( 1 << num ) ) )
			{
				return;
			}
		}
		else
		{
			return;		// don't have the weapon
		}
	}

	if (cg.weaponSelect != num)
	{
		trap_S_MuteSound(cg.snap->ps.clientNum, CHAN_WEAPON);
	}

	cg.weaponSelect = num;
}



/*
===================
CG_OutOfAmmoChange

The current weapon has just run out of ammo
===================
*/
void CG_OutOfAmmoChange( int oldWeapon )
{
	//int		i;

	// Jedi Knight Galaxies
	return;		// Dont switch weapons automatically, they just dont fire
/*
	cg.weaponSelectTime = cg.time;

	for ( i = LAST_USEABLE_WEAPON ; i > 0 ; i-- )	//We don't want the emplaced or turret
	{
		if ( CG_WeaponSelectable( i ) )
		{
			/*
			if ( 1 == cg_autoswitch.integer && 
				( i == WP_TRIP_MINE || i == WP_DET_PACK || i == WP_THERMAL || i == WP_ROCKET_LAUNCHER) ) // safe weapon switch
			*/
/*
			//rww - Don't we want to make sure i != one of these if autoswitch is 1 (safe)?
			if (cg_autoswitch.integer != 1 || (i != WP_TRIP_MINE && i != WP_DET_PACK && i != WP_THERMAL && i != WP_ROCKET_LAUNCHER))
			{
				if (i != oldWeapon)
				{ //don't even do anything if we're just selecting the weapon we already have/had
					cg.weaponSelect = i;
					break;
				}
			}
		}
	}

	trap_S_MuteSound(cg.snap->ps.clientNum, CHAN_WEAPON);
*/
}



/*
===================================================================================================

WEAPON EVENTS

===================================================================================================
*/

void CG_GetClientWeaponMuzzleBoltPoint(int clIndex, vec3_t to)
{
	centity_t *cent;
	mdxaBone_t	boltMatrix;

	if (clIndex < 0 || clIndex >= MAX_CLIENTS)
	{
		return;
	}

	cent = &cg_entities[clIndex];

	if (!cent || !cent->ghoul2 || !trap_G2_HaveWeGhoul2Models(cent->ghoul2) ||
		!trap_G2API_HasGhoul2ModelOnIndex(&(cent->ghoul2), 1))
	{
		return;
	}

	trap_G2API_GetBoltMatrix(cent->ghoul2, 1, 0, &boltMatrix, cent->turAngles, cent->lerpOrigin, cg.time, cgs.gameModels, cent->modelScale);
	BG_GiveMeVectorFromMatrix(&boltMatrix, ORIGIN, to);
}

/*
================
CG_FireWeapon

Caused by an EV_FIRE_WEAPON event
================
*/
void CG_FireWeapon( centity_t *cent, qboolean altFire ) {
	entityState_t *ent;
	int				c;
	weaponInfo_t	*weap;

	ent = &cent->currentState;
	if ( ent->weapon == WP_NONE ) {
		return;
	}
	if ( ent->weapon >= WP_NUM_WEAPONS ) {
		CG_Error( "CG_FireWeapon: ent->weapon >= WP_NUM_WEAPONS" );
		return;
	}
	weap = CG_WeaponInfo (ent->weapon, 0);
	cent->muzzleFlashTime = cg.time;

	if ( cg.predictedPlayerState.clientNum == cent->currentState.number )
	{
		weaponData_t *thisWeaponData = GetWeaponData( cg.snap->ps.weapon, cg.snap->ps.weaponVariation );
		float		  fRecoil		 = ( altFire ) ? thisWeaponData->secondary.recoil : thisWeaponData->primary.recoil;

		if ( fRecoil )
		{
			/* This used pointer (0x97DF88) is the base address of cl.viewangles. Therefore this is PITCH and +4 is YAW */
			float fYawRecoil = flrand( 0.15 * fRecoil, 0.25 * fRecoil );
			CGCam_Shake( flrand( 0.85 * fRecoil, 0.15 * fRecoil), 100 );
			*(( float * ) 0x97DF8C ) += Q_irand( 0, 1 ) ? -fYawRecoil : fYawRecoil; // yaw
			*(( float * ) 0x97DF88 ) -= fRecoil; // pitch
		}
		
		/*
		if ( cg.predictedPlayerState.weapon
		if ((ent->weapon == WP_BRYAR_PISTOL && altFire) ||
			(ent->weapon == WP_BRYAR_OLD && altFire) ||
			(ent->weapon == WP_BOWCASTER && !altFire) ||
			(ent->weapon == WP_DEMP2 && altFire))
		{
			float val = ( cg.time - cent->currentState.constantLight ) * 0.001f;

			if (val > 3)
			{
				val = 3;
			}
			if (val < 0.2)
			{
				val = 0.2;
			}

			val *= 2;

			CGCam_Shake( val, 250 );
		}
		else if (ent->weapon == WP_ROCKET_LAUNCHER ||
			(ent->weapon == WP_REPEATER && altFire) ||
			ent->weapon == WP_FLECHETTE ||
			(ent->weapon == WP_CONCUSSION && !altFire))
		{
			if (ent->weapon == WP_CONCUSSION)
			{
				if (!cg.renderingThirdPerson )//gives an advantage to being in 3rd person, but would look silly otherwise
				{//kick the view back
					cg.kick_angles[PITCH] = flrand( -10, -15 );
					cg.kick_time = cg.time;
				}
			}
			else if (ent->weapon == WP_ROCKET_LAUNCHER)
			{
				CGCam_Shake(flrand(2, 3), 350);
			}
			else if (ent->weapon == WP_REPEATER)
			{
				CGCam_Shake(flrand(2, 3), 350);
			}
			else if (ent->weapon == WP_FLECHETTE)
			{
				if (altFire)
				{
					CGCam_Shake(flrand(2, 3), 350);
				}
				else
				{
					CGCam_Shake(1.5, 250);
				}
			}
		}
		*/
	}
	// lightning gun only does this this on initial press
	if ( ent->weapon == WP_DEMP2 ) {
		if ( cent->pe.lightningFiring ) {
			return;
		}
	}

	// play quad sound if needed
	if ( cent->currentState.powerups & ( 1 << PW_QUAD ) ) {
		//trap_S_StartSound (NULL, cent->currentState.number, CHAN_ITEM, cgs.media.quadSound );
	}


	// play a sound
	if (altFire)
	{
		// play a sound
		for ( c = 0 ; c < 4 ; c++ ) {
			if ( !weap->altFlashSound[c] ) {
				break;
			}
		}
		if ( c > 0 ) {
			c = rand() % c;
			if ( weap->altFlashSound[c] )
			{
				trap_S_StartSound( NULL, ent->number, CHAN_WEAPON, weap->altFlashSound[c] );
			}
		}
//		if ( weap->altFlashSnd )
//		{
//			trap_S_StartSound( NULL, ent->number, CHAN_WEAPON, weap->altFlashSnd );
//		}
	}
	else
	{	
		// play a sound
		for ( c = 0 ; c < 4 ; c++ ) {
			if ( !weap->flashSound[c] ) {
				break;
			}
		}
		if ( c > 0 ) {
			c = rand() % c;
			if ( weap->flashSound[c] )
			{
				trap_S_StartSound( NULL, ent->number, CHAN_WEAPON, weap->flashSound[c] );
			}
		}
	}
}

qboolean CG_VehicleWeaponImpact( centity_t *cent )
{//see if this is a missile entity that's owned by a vehicle and should do a special, overridden impact effect
	if ((cent->currentState.eFlags&EF_JETPACK_ACTIVE)//hack so we know we're a vehicle Weapon shot
		&& cent->currentState.otherEntityNum2
		&& g_vehWeaponInfo[cent->currentState.otherEntityNum2].iImpactFX)
	{//missile is from a special vehWeapon
		vec3_t normal;
		ByteToDir( cent->currentState.eventParm, normal );

		trap_FX_PlayEffectID( g_vehWeaponInfo[cent->currentState.otherEntityNum2].iImpactFX, cent->lerpOrigin, normal, -1, -1 );
		return qtrue;
	}
	return qfalse;
}

/*
=================
CG_MissileHitWall

Caused by an EV_MISSILE_MISS event, or directly by local bullet tracing
=================
*/
void CG_MissileHitWall(int weapon, int clientNum, vec3_t origin, vec3_t dir, impactSound_t soundType, qboolean altFire, int charge) 
{
	int parm;
	vec3_t up={0,0,1};

	switch( weapon )
	{
	case WP_BRYAR_PISTOL:
		if ( altFire )
		{
			parm = charge;
			FX_BryarAltHitWall( origin, dir, parm );
		}
		else
		{
			FX_BryarHitWall( origin, dir );
		}
		break;

	case WP_CONCUSSION:
		FX_ConcussionHitWall( origin, dir );
		break;

	case WP_BRYAR_OLD:
		if ( altFire )
		{
			parm = charge;
			FX_BryarAltHitWall( origin, dir, parm );
		}
		else
		{
			FX_BryarHitWall( origin, dir );
		}
		break;

	case WP_TURRET:
		FX_TurretHitWall( origin, dir );
		break;

	case WP_BLASTER:
		FX_BlasterWeaponHitWall( origin, dir );
		break;

	case WP_DISRUPTOR:
		FX_DisruptorAltMiss( origin, dir );
		break;

	case WP_BOWCASTER:
		FX_BowcasterHitWall( origin, dir );
		break;

	case WP_REPEATER:
		if ( altFire )
		{
			FX_RepeaterAltHitWall( origin, dir );
		}
		else
		{
			FX_RepeaterHitWall( origin, dir );
		}
		break;

	case WP_DEMP2:
		if (altFire)
		{
			trap_FX_PlayEffectID(cgs.effects.mAltDetonate, origin, dir, -1, -1);
		}
		else
		{
			FX_DEMP2_HitWall( origin, dir );
		}
		break;

	case WP_FLECHETTE:
		/*if (altFire)
		{
			CG_SurfaceExplosion(origin, dir, 20.0f, 12.0f, qtrue);
		}
		else
		*/
		if (!altFire)
		{
			FX_FlechetteWeaponHitWall( origin, dir );
		}
		break;

	case WP_ROCKET_LAUNCHER:
		FX_RocketHitWall( origin, dir );
		break;

	case WP_THERMAL:
		trap_FX_PlayEffectID( cgs.effects.thermalExplosionEffect, origin, dir, -1, -1 );
		trap_FX_PlayEffectID( cgs.effects.thermalShockwaveEffect, origin, up, -1, -1 );
		break;

	case WP_EMPLACED_GUN:
		FX_BlasterWeaponHitWall( origin, dir );
		//FIXME: Give it its own hit wall effect
		break;
	}
}


/*
=================
CG_MissileHitPlayer
=================
*/
void CG_MissileHitPlayer(int weapon, vec3_t origin, vec3_t dir, int entityNum, qboolean altFire) 
{
	qboolean	humanoid = qtrue;
	vec3_t up={0,0,1};

	/*
	// NOTENOTE Non-portable code from single player
	if ( cent->gent )
	{
		other = &g_entities[cent->gent->s.otherEntityNum];

		if ( other->client && other->client->playerTeam == TEAM_BOTS )
		{
			humanoid = qfalse;
		}
	}
	*/	

	// NOTENOTE No bleeding in this game
//	CG_Bleed( origin, entityNum );

	// some weapons will make an explosion with the blood, while
	// others will just make the blood
	switch ( weapon ) {
	case WP_BRYAR_PISTOL:
		if ( altFire )
		{
			FX_BryarAltHitPlayer( origin, dir, humanoid );
		}
		else
		{
			FX_BryarHitPlayer( origin, dir, humanoid );
		}
		break;

	case WP_CONCUSSION:
		FX_ConcussionHitPlayer( origin, dir, humanoid );
		break;

	case WP_BRYAR_OLD:
		if ( altFire )
		{
			FX_BryarAltHitPlayer( origin, dir, humanoid );
		}
		else
		{
			FX_BryarHitPlayer( origin, dir, humanoid );
		}
		break;

	case WP_TURRET:
		FX_TurretHitPlayer( origin, dir, humanoid );
		break;

	case WP_BLASTER:
		FX_BlasterWeaponHitPlayer( origin, dir, humanoid );
		break;

	case WP_DISRUPTOR:
		FX_DisruptorAltHit( origin, dir);
		break;

	case WP_BOWCASTER:
		FX_BowcasterHitPlayer( origin, dir, humanoid );
		break;

	case WP_REPEATER:
		if ( altFire )
		{
			FX_RepeaterAltHitPlayer( origin, dir, humanoid );
		}
		else
		{
			FX_RepeaterHitPlayer( origin, dir, humanoid );
		}
		break;

	case WP_DEMP2:
		// Do a full body effect here for some more feedback
		// NOTENOTE The chaining of the demp2 is not yet implemented.
		/*
		if ( other )
		{
			other->s.powerups |= ( 1 << PW_DISINT_1 );
			other->client->ps.powerups[PW_DISINT_1] = cg.time + 650;
		}
		*/
		if (altFire)
		{
			trap_FX_PlayEffectID(cgs.effects.mAltDetonate, origin, dir, -1, -1);
		}
		else
		{
			FX_DEMP2_HitPlayer( origin, dir, humanoid );
		}
		break;

	case WP_FLECHETTE:
		FX_FlechetteWeaponHitPlayer( origin, dir, humanoid );
		break;

	case WP_ROCKET_LAUNCHER:
		FX_RocketHitPlayer( origin, dir, humanoid );
		break;

	case WP_THERMAL:
		trap_FX_PlayEffectID( cgs.effects.thermalExplosionEffect, origin, dir, -1, -1 );
		trap_FX_PlayEffectID( cgs.effects.thermalShockwaveEffect, origin, up, -1, -1 );
		break;
	case WP_EMPLACED_GUN:
		//FIXME: Its own effect?
		FX_BlasterWeaponHitPlayer( origin, dir, humanoid );
		break;

	default:
		break;
	}
}


/*
============================================================================

BULLETS

============================================================================
*/


/*
======================
CG_CalcMuzzlePoint
======================
*/

// Modified copy of the server routine in g_weapon.c
void WP_CalculateMuzzlePoint( centity_t *cent, vec3_t forward, vec3_t right, vec3_t up, vec3_t muzzlePoint ) 
{
	void *g2Weapon = cent->ghoul2;
	
	if ( trap_G2_HaveWeGhoul2Models (g2Weapon) && trap_G2API_HasGhoul2ModelOnIndex (&g2Weapon, 1) )
	{
	    mdxaBone_t muzzleBone;
	    
	    trap_G2API_GetBoltMatrix (g2Weapon, 1, 0, &muzzleBone, cent->lerpAngles, cent->lerpOrigin, cg.time, cgs.gameModels, cent->modelScale);
	    BG_GiveMeVectorFromMatrix (&muzzleBone, ORIGIN, muzzlePoint);
	}
	else
	{
	    VectorCopy (cent->lerpOrigin, muzzlePoint);
	    muzzlePoint[2] += cg.snap->ps.viewheight;
	}
	
	
}

qboolean CG_CalcMuzzlePoint( int entityNum, vec3_t muzzle ) {
	vec3_t		forward, right;
	vec3_t		gunpoint;
	centity_t	*cent;
	int			anim;

	if ( entityNum == cg.snap->ps.clientNum )
	{ //I'm not exactly sure why we'd be rendering someone else's crosshair, but hey.
		int weapontype = cg.snap->ps.weapon;
		vec3_t weaponMuzzle;
		centity_t *pEnt = &cg_entities[cg.predictedPlayerState.clientNum];

		/* JKG - Muzzle Calculation */
		
		// This code is causing crosshair glitches, so i'll disable it for now - BobaFett

		/*if ( weapontype != WP_STUN_BATON && weapontype != WP_MELEE && weapontype != WP_SABER )
		{
			vec3_t mforward, mright, mup;
			AngleVectors( cg.predictedPlayerState.viewangles, mforward, mright, mup );
			WP_CalculateMuzzlePoint( &cg_entities[cg.snap->ps.clientNum], mforward, mright, mup, muzzle );
			return qtrue;
		}
		else*/
		{
			VectorClear( weaponMuzzle );
		}
		/* JKG - Muzzle Calculation End */

		if (cg.renderingThirdPerson)
		{
			VectorCopy( pEnt->lerpOrigin, gunpoint );
			AngleVectors( pEnt->lerpAngles, forward, right, NULL );
		}
		else
		{
			VectorCopy( cg.refdef.vieworg, gunpoint );
			AngleVectors( cg.refdef.viewangles, forward, right, NULL );
		}

		if (weapontype == WP_EMPLACED_GUN && cg.snap->ps.emplacedIndex)
		{
			centity_t *gunEnt = &cg_entities[cg.snap->ps.emplacedIndex];

			if (gunEnt)
			{
				vec3_t pitchConstraint;

				VectorCopy(gunEnt->lerpOrigin, gunpoint);
				gunpoint[2] += 46;

				if (cg.renderingThirdPerson)
				{
					VectorCopy(pEnt->lerpAngles, pitchConstraint);
				}
				else
				{
					VectorCopy(cg.refdef.viewangles, pitchConstraint);
				}

				if (pitchConstraint[PITCH] > 40)
				{
					pitchConstraint[PITCH] = 40;
				}
				AngleVectors( pitchConstraint, forward, right, NULL );
			}
		}

		VectorCopy(gunpoint, muzzle);

		VectorMA(muzzle, weaponMuzzle[0], forward, muzzle);
		VectorMA(muzzle, weaponMuzzle[1], right, muzzle);

		if (weapontype == WP_EMPLACED_GUN && cg.snap->ps.emplacedIndex)
		{
			//Do nothing
		}
		else if (cg.renderingThirdPerson)
		{
			muzzle[2] += cg.snap->ps.viewheight + weaponMuzzle[2];
		}
		else
		{
			muzzle[2] += weaponMuzzle[2];
		}

		return qtrue;
	}

	cent = &cg_entities[entityNum];
	if ( !cent->currentValid ) {
		return qfalse;
	}

	VectorCopy( cent->currentState.pos.trBase, muzzle );

	AngleVectors( cent->currentState.apos.trBase, forward, NULL, NULL );
	anim = cent->currentState.legsAnim;
	if ( anim == BOTH_CROUCH1WALK || anim == BOTH_CROUCH1IDLE ) {
		muzzle[2] += CROUCH_VIEWHEIGHT;
	} else {
		muzzle[2] += DEFAULT_VIEWHEIGHT;
	}

	VectorMA( muzzle, 14, forward, muzzle );

	return qtrue;

}



/*
Ghoul2 Insert Start
*/

// create one instance of all the weapons we are going to use so we can just copy this info into each clients gun ghoul2 object in fast way
static struct
{
    unsigned int weaponNum;
    unsigned int weaponVariation;
    
    void *ghoul2;
} g2WeaponInstances[MAX_WEAPON_INFO_SLOTS];

void CG_InitG2Weapons(void)
{
	unsigned int i = 0;
	unsigned int j;
	unsigned int id = 0;
	//gitem_t		*item;
	
	memset(g2WeaponInstances, 0, sizeof(g2WeaponInstances));
	
	for ( i = 0; i <= LAST_USEABLE_WEAPON; i++ )
	{
	    unsigned int numVariations = BG_NumberOfWeaponVariations (i);
	    for ( j = 0; j < numVariations; j++ )
	    {
	        const weaponData_t *weaponData = GetWeaponData (i, j);
	        void *ghoul2 = NULL;
	        
	        trap_G2API_InitGhoul2Model (&g2WeaponInstances[id].ghoul2, weaponData->visuals.world_model, 0, 0, 0, 0, 0);
	        
	        ghoul2 = g2WeaponInstances[id].ghoul2;
	        
	        if ( trap_G2_HaveWeGhoul2Models (ghoul2) )
	        {
	            trap_G2API_SetBoltInfo (ghoul2, 0, 0);
	            trap_G2API_AddBolt (ghoul2, 0, i == WP_SABER ? "*blade1" : "*flash");
	            
	            g2WeaponInstances[id].weaponNum = i;
	            g2WeaponInstances[id].weaponVariation = j;
	            id++;
	        }
	    }
	}
	
#if 0
	for ( item = bg_itemlist + 1 ; item->classname ; item++ ) 
	{
		if ( item->giType == IT_WEAPON )
		{
			assert(item->giTag < MAX_WEAPONS);

			// initialise model
			trap_G2API_InitGhoul2Model(&g2WeaponInstances[/*i*/item->giTag], item->world_model[0], 0, 0, 0, 0, 0);
//			trap_G2API_InitGhoul2Model(&g2WeaponInstances[i], item->world_model[0],G_ModelIndex( item->world_model[0] ) , 0, 0, 0, 0);
			if (g2WeaponInstances[/*i*/item->giTag])
			{
				// indicate we will be bolted to model 0 (ie the player) on bolt 0 (always the right hand) when we get copied
				trap_G2API_SetBoltInfo(g2WeaponInstances[/*i*/item->giTag], 0, 0);
				// now set up the gun bolt on it
				if (item->giTag == WP_SABER)
				{
					trap_G2API_AddBolt(g2WeaponInstances[/*i*/item->giTag], 0, "*blade1");
				}
				else
				{
					trap_G2API_AddBolt(g2WeaponInstances[/*i*/item->giTag], 0, "*flash");
				}
				i++;
			}
			if (i == MAX_WEAPONS)
			{
				assert(0);	
				break;
			}
			
		}
	}
#endif
}

// clean out any g2 models we instanciated for copying purposes
void CG_ShutDownG2Weapons(void)
{
	int i;
	weaponInfo_t *weapon;
	
	for ( i = 0; i < MAX_WEAPON_INFO_SLOTS; i++ )
	{
	    trap_G2API_CleanGhoul2Models (&g2WeaponInstances[i].ghoul2);
	    
	    weapon = CG_WeaponInfoUnsafe (g2WeaponInstances[i].weaponNum, g2WeaponInstances[i].weaponVariation);
	    if ( weapon != NULL )
	    {
	        trap_G2API_CleanGhoul2Models (&weapon->g2ViewModel);
	        trap_G2API_CleanGhoul2Models (&weapon->primDrawData.explosiveRender.tripmine.g2Model);
	        trap_G2API_CleanGhoul2Models (&weapon->primDrawData.explosiveRender.detpack.g2Model);
	        trap_G2API_CleanGhoul2Models (&weapon->altDrawData.explosiveRender.tripmine.g2Model);
	        trap_G2API_CleanGhoul2Models (&weapon->altDrawData.explosiveRender.detpack.g2Model);
	    }
	}
	
	/*for (i=0; i<MAX_WEAPONS; i++)
	{
		trap_G2API_CleanGhoul2Models(&g2WeaponInstances[i]);
		
		weapon = CG_WeaponInfo (i, 0);
		if ( weapon && weapon->g2ViewModel )
        {
            trap_G2API_CleanGhoul2Models (&weapon->g2ViewModel);
        }
	}*/
}

static void *CG_GetGhoul2WorldModel ( int weaponNum, int weaponVariation )
{
    unsigned int i;
    for ( i = 0; i < MAX_WEAPON_INFO_SLOTS; i++ )
	{
	    if ( g2WeaponInstances[i].weaponNum == weaponNum && g2WeaponInstances[i].weaponVariation == weaponVariation )
	    {
	        return g2WeaponInstances[i].ghoul2;
	    }
	}
	
	return NULL;
}

void *CG_G2WeaponInstance(centity_t *cent, int weapon, int variation)
{
	clientInfo_t *ci = NULL;

	if (weapon != WP_SABER)
	{
		return CG_GetGhoul2WorldModel (weapon, variation);
	}

	if (cent->currentState.eType != ET_PLAYER &&
		cent->currentState.eType != ET_NPC)
	{
		return CG_GetGhoul2WorldModel (weapon, variation);
	}

	if (cent->currentState.eType == ET_NPC)
	{
		ci = cent->npcClient;
	}
	else
	{
		ci = &cgs.clientinfo[cent->currentState.number];
	}

	if (!ci)
	{
		return CG_GetGhoul2WorldModel (weapon, variation);
	}

	//Try to return the custom saber instance if we can.
	if (ci->saber[0].model[0] &&
		ci->ghoul2Weapons[0])
	{
		return ci->ghoul2Weapons[0];
	}

	//If no custom then just use the default.
	return CG_GetGhoul2WorldModel (weapon, variation);
}

// what ghoul2 model do we want to copy ?
void CG_CopyG2WeaponInstance(centity_t *cent, int weaponNum, int weaponVariation, void *toGhoul2)
{
	//rww - the -1 is because there is no "weapon" for WP_NONE
	assert(weaponNum < MAX_WEAPONS);
	if (CG_G2WeaponInstance(cent, weaponNum/*-1*/, weaponVariation))
	{
		if (weaponNum == WP_SABER)
		{
			clientInfo_t *ci = NULL;

			if (cent->currentState.eType == ET_NPC)
			{
				ci = cent->npcClient;
			}
			else
			{
				ci = &cgs.clientinfo[cent->currentState.number];
			}

			if (!ci)
			{
				trap_G2API_CopySpecificGhoul2Model(CG_G2WeaponInstance(cent, weaponNum/*-1*/, weaponVariation), 0, toGhoul2, 1); 
			}
			else
			{ //Try both the left hand saber and the right hand saber
				int i = 0;

				while (i < MAX_SABERS)
				{
					if (ci->saber[i].model[0] &&
						ci->ghoul2Weapons[i])
					{
						trap_G2API_CopySpecificGhoul2Model(ci->ghoul2Weapons[i], 0, toGhoul2, i+1); 
					}
					else if (ci->ghoul2Weapons[i])
					{ //if the second saber has been removed, then be sure to remove it and free the instance.
						qboolean g2HasSecondSaber = trap_G2API_HasGhoul2ModelOnIndex(&(toGhoul2), 2);

						if (g2HasSecondSaber)
						{ //remove it now since we're switching away from sabers
							trap_G2API_RemoveGhoul2Model(&(toGhoul2), 2);
						}
						trap_G2API_CleanGhoul2Models(&ci->ghoul2Weapons[i]);
					}

					i++;
				}
			}
		}
		else
		{
			qboolean g2HasSecondSaber = trap_G2API_HasGhoul2ModelOnIndex(&(toGhoul2), 2);

			if (g2HasSecondSaber)
			{ //remove it now since we're switching away from sabers
				trap_G2API_RemoveGhoul2Model(&(toGhoul2), 2);
			}

			if (weaponNum == WP_EMPLACED_GUN)
			{ //a bit of a hack to remove gun model when using an emplaced weap
				if (trap_G2API_HasGhoul2ModelOnIndex(&(toGhoul2), 1))
				{
					trap_G2API_RemoveGhoul2Model(&(toGhoul2), 1);
				}
			}
			else if (weaponNum == WP_MELEE)
			{ //don't want a weapon on the model for this one
				if (trap_G2API_HasGhoul2ModelOnIndex(&(toGhoul2), 1))
				{
					trap_G2API_RemoveGhoul2Model(&(toGhoul2), 1);
				}
			}
			else
			{
				trap_G2API_CopySpecificGhoul2Model(CG_G2WeaponInstance(cent, weaponNum/*-1*/, weaponVariation), 0, toGhoul2, 1); 
			}
		}
	}
}

void CG_CheckPlayerG2Weapons(playerState_t *ps, centity_t *cent) 
{
	if (!ps)
	{
		assert(0);
		return;
	}

	if (ps->pm_flags & PMF_FOLLOW)
	{
		return;
	}

	if (cent->currentState.eType == ET_NPC)
	{
		assert(0);
		return;
	}

	// should we change the gun model on this player?
	if (cent->currentState.saberInFlight)
	{
		cent->ghoul2weapon = CG_G2WeaponInstance(cent, WP_SABER, 0);
	}

	if (cent->currentState.eFlags & EF_DEAD)
	{ //no updating weapons when dead
		cent->ghoul2weapon = NULL;
		return;
	}

	if (cent->torsoBolt)
	{ //got our limb cut off, no updating weapons until it's restored
		cent->ghoul2weapon = NULL;
		return;
	}

	if (cgs.clientinfo[ps->clientNum].team == TEAM_SPECTATOR ||
		ps->persistant[PERS_TEAM] == TEAM_SPECTATOR)
	{
		cent->ghoul2weapon = cg_entities[ps->clientNum].ghoul2weapon = NULL;
		cent->weapon = cg_entities[ps->clientNum].weapon = 0;
		return;
	}

	if (cent->ghoul2 && cent->ghoul2weapon != CG_G2WeaponInstance(cent, ps->weapon, ps->weaponVariation) &&
		ps->clientNum == cent->currentState.number) //don't want spectator mode forcing one client's weapon instance over another's
	{
		CG_CopyG2WeaponInstance(cent, ps->weapon, ps->weaponVariation, cent->ghoul2);
		cent->ghoul2weapon = CG_G2WeaponInstance(cent, ps->weapon, ps->weaponVariation);
		if (cent->weapon == WP_SABER && cent->weapon != ps->weapon && !ps->saberHolstered)
		{ //switching away from the saber
			//trap_S_StartSound(cent->lerpOrigin, cent->currentState.number, CHAN_AUTO, trap_S_RegisterSound( "sound/weapons/saber/saberoffquick.wav" ));
			if (cgs.clientinfo[ps->clientNum].saber[0].soundOff && !ps->saberHolstered)
			{
				trap_S_StartSound(cent->lerpOrigin, cent->currentState.number, CHAN_AUTO, cgs.clientinfo[ps->clientNum].saber[0].soundOff);
			}

			if (cgs.clientinfo[ps->clientNum].saber[1].soundOff &&
				cgs.clientinfo[ps->clientNum].saber[1].model[0] &&
				!ps->saberHolstered)
			{
				trap_S_StartSound(cent->lerpOrigin, cent->currentState.number, CHAN_AUTO, cgs.clientinfo[ps->clientNum].saber[1].soundOff);
			}
		}
		else if (ps->weapon == WP_SABER && cent->weapon != ps->weapon && !cent->saberWasInFlight)
		{ //switching to the saber
			//trap_S_StartSound(cent->lerpOrigin, cent->currentState.number, CHAN_AUTO, trap_S_RegisterSound( "sound/weapons/saber/saberon.wav" ));
			if (cgs.clientinfo[ps->clientNum].saber[0].soundOn)
			{
				trap_S_StartSound(cent->lerpOrigin, cent->currentState.number, CHAN_AUTO, cgs.clientinfo[ps->clientNum].saber[0].soundOn);
			}

			if (cgs.clientinfo[ps->clientNum].saber[1].soundOn)
			{
				trap_S_StartSound(cent->lerpOrigin, cent->currentState.number, CHAN_AUTO, cgs.clientinfo[ps->clientNum].saber[1].soundOn);
			}

			BG_SI_SetDesiredLength(&cgs.clientinfo[ps->clientNum].saber[0], 0, -1);
			BG_SI_SetDesiredLength(&cgs.clientinfo[ps->clientNum].saber[1], 0, -1);
		}
		cent->weapon = ps->weapon;
	}
}


/*
Ghoul2 Insert End
*/


static weaponInfo_t defaultWeapon;
weaponInfo_t *CG_WeaponInfo ( unsigned int weaponNum, unsigned int variation )
{
    static weaponInfo_t *lastInfo = NULL;
    
    if ( lastInfo != NULL && lastInfo->weaponNum == weaponNum && lastInfo->variation == variation )
    {
        return lastInfo;
    }
    else
    {
        weaponInfo_t *info = &cg_weapons[0];
        unsigned int i;
        
        #ifdef _DEBUG
        //CG_Printf ("Attempting to load weapon %d, variation %d\n", weaponNum, variation);
        #endif
        
        CG_RegisterWeapon (weaponNum, variation);
        
        for ( i = 0; i < MAX_WEAPON_INFO_SLOTS; i++, info++ )
        {
            if ( info->weaponNum == weaponNum && info->variation == variation )
            {
                lastInfo = info;
                return info;
            }
        }
    }
    
    CG_Printf (S_COLOR_RED "Failed to find weapon info for weapon %d, variation %d.\n", weaponNum, variation);
    return CG_WeaponInfo (0, 0);
}

weaponInfo_t *CG_WeaponInfoUnsafe ( unsigned int weaponNum, unsigned int variation )
{
    static weaponInfo_t *lastInfo = NULL;
    
    if ( lastInfo != NULL && lastInfo->weaponNum == weaponNum && lastInfo->variation == variation )
    {
        return lastInfo;
    }
    else
    {
        weaponInfo_t *info = &cg_weapons[0];
        unsigned int i;
        for ( i = 0; i < MAX_WEAPON_INFO_SLOTS; i++, info++ )
        {
            if ( info->weaponNum == weaponNum && info->variation == variation )
            {
                lastInfo = info;
                return lastInfo;
            }
        }
    }
    
    return NULL;
}

weaponInfo_t *CG_NextFreeWeaponInfo ( void )
{
    weaponInfo_t *info = &cg_weapons[0];
    unsigned int i;
    for ( i = 0; i < MAX_WEAPON_INFO_SLOTS; i++, info++ )
    {
        if ( !info->registered )
        {
            return info;
        }
    }
    
    return NULL;
}

//=========================================================
// Weapon event handling functions
//=========================================================
#define VIEWANGLES_YAW_ADDRESS (0x97DF8C)
#define VIEWANGLES_PITCH_ADDRESS (0x97DF88)
static void JKG_FireBlaster ( centity_t *cent, const weaponDrawData_t *weaponData, qboolean altFire )
{
    const entityState_t *s = &cent->currentState;

    // Update the muzzle flash time, so we know to draw it in the render function.
    if ( (cent->shotCount + 1) == UINT_MAX )
    {
        cent->shotCount = 0;
        cent->shotCountOverflowed = qtrue;
    }
    else
    {
        cent->shotCount++;
        cent->shotCountOverflowed = qfalse;
    }
    
    // Do recoil
    if ( s->number == cg.snap->ps.clientNum )
    {
        const weaponData_t *thisWeaponData = GetWeaponData (cg.snap->ps.weapon, cg.snap->ps.weaponVariation);
		float pitchRecoil = ( altFire ) ? thisWeaponData->secondary.recoil : thisWeaponData->primary.recoil;

		if ( pitchRecoil )
		{
			float yawRecoil = flrand (0.15 * pitchRecoil, 0.25 * pitchRecoil);
			
			if ( Q_irand (0, 1) )
			{
			    yawRecoil = -yawRecoil;
			}
			
			CGCam_Shake (flrand (0.85f * pitchRecoil, 0.15f * pitchRecoil), 100);
			
			*(float *)VIEWANGLES_YAW_ADDRESS += yawRecoil;
			*(float *)VIEWANGLES_PITCH_ADDRESS -= pitchRecoil;
		}
    }
    
    if ( weaponData->weaponFire.generic.fireSoundCount > 0 )
    {
        int index = Q_irand (0, weaponData->weaponFire.generic.fireSoundCount - 1);
        trap_S_StartSound (NULL, s->number, CHAN_AUTO, weaponData->weaponFire.generic.fireSound[index]);
    }
}

static void JKG_RenderGenericProjectile ( const centity_t *cent, const weaponDrawData_t *weaponData )
{
    const entityState_t *s = &cent->currentState;

    if ( weaponData->projectileRender.generic.projectileEffect )
    {
        vec3_t forward;
        if ( VectorNormalize2 (s->pos.trDelta, forward) == 0.0f )
        {
            forward[2] = 1.0f;
        }
        
        trap_FX_PlayEffectID (
            weaponData->projectileRender.generic.projectileEffect,
            cent->lerpOrigin,
            forward,
            -1, -1
        );
    }
    
    if ( weaponData->projectileRender.generic.lightIntensity > 0.0f )
    {
        trap_R_AddLightToScene (
            cent->lerpOrigin,
             weaponData->projectileRender.generic.lightIntensity,
             weaponData->projectileRender.generic.lightColor[0],
             weaponData->projectileRender.generic.lightColor[1],
             weaponData->projectileRender.generic.lightColor[2]
        );
    }
    
    if ( weaponData->projectileRender.generic.runSound )
    {
        vec3_t velocity;
        BG_EvaluateTrajectory (&s->pos, cg.time, velocity);
        
        trap_S_AddLoopingSound (s->number, cent->lerpOrigin, velocity, weaponData->projectileRender.generic.runSound);
    }
    
    if ( weaponData->projectileRender.generic.projectileModel )
    {
        refEntity_t ent;
        
        memset (&ent, 0, sizeof (ent));
        VectorCopy (cent->lerpOrigin, ent.origin);
        VectorCopy (cent->lerpOrigin, ent.oldorigin);
        
        CG_SetGhoul2Info (&ent, cent);
        
        ent.skinNum = cg.clientFrame & 1;
        ent.renderfx = RF_NOSHADOW;
        
        ent.hModel = weaponData->projectileRender.generic.projectileModel;
        
        if ( s->apos.trType != TR_INTERPOLATE )
        {
            if ( VectorNormalize2 (s->pos.trDelta, ent.axis[0]) == 0.0f )
            {
                ent.axis[0][2] = 1.0f;
            }
            
            if ( s->pos.trType != TR_STATIONARY )
            {
                RotateAroundDirection (ent.axis, cg.time * ((s->eFlags & EF_MISSILE_STICK) ? 0.5f : 0.25f));
            }
            else
            {
                RotateAroundDirection (ent.axis, ((s->eFlags & EF_MISSILE_STICK) ? s->pos.trTime * 0.5f : s->time * 0.25f));
            }
        }
        else
        {
            AnglesToAxis (cent->lerpAngles, ent.axis);
        }
        
        CG_AddRefEntityWithPowerups ((const refEntity_t *)&ent, s, TEAM_FREE);
    }
}

static void JKG_BounceSpecialGrenade ( const centity_t *cent, const weaponDrawData_t *weaponData )
{
    if ( weaponData->grenadeBounce.grenade.bounceSound[0] )
    {
        trap_S_StartSound (NULL, cent->currentState.number, CHAN_BODY, weaponData->grenadeBounce.grenade.bounceSound[Q_irand (0, 1)]);
    }
}

static void JKG_RenderGenericProjectileMiss ( const centity_t *cent, const weaponDrawData_t *weaponData, const vec3_t origin, const vec3_t normal )
{
    if ( weaponData->projectileMiss.generic.impactEffect )
    {
        trap_FX_PlayEffectID (weaponData->projectileMiss.generic.impactEffect, origin, normal, -1, -1);
    }
}

static void JKG_RenderGenericProjectileDeath ( const centity_t *cent, const weaponDrawData_t *weaponData, const vec3_t origin, const vec3_t normal )
{
    if ( weaponData->projectileRender.generic.deathEffect )
    {
        trap_FX_PlayEffectID (weaponData->projectileRender.generic.deathEffect, origin, normal, -1, -1);
    }
}

static void JKG_RenderGenericProjectileHitPlayer ( const weaponDrawData_t *weaponData, const vec3_t origin, const vec3_t normal )
{
    if ( weaponData->projectileHitPlayer.generic.impactEffect )
    {
        trap_FX_PlayEffectID (weaponData->projectileHitPlayer.generic.impactEffect, origin, normal, -1, -1);
    }
}

static void JKG_RenderExplosiveProjectileMiss ( const centity_t *cent, const weaponDrawData_t *weaponData, const vec3_t origin, const vec3_t normal )
{
    if ( weaponData->projectileMiss.explosive.stickSound )
    {
        trap_S_StartSound (NULL, cent->currentState.number, CHAN_WEAPON, weaponData->projectileMiss.explosive.stickSound);
    }
}

static void JKG_RenderGrenadeProjectileMiss ( const centity_t *cent, const weaponDrawData_t *weaponData, const vec3_t origin, const vec3_t normal )
{
    if ( weaponData->projectileMiss.grenade.impactEffect )
    {
        trap_FX_PlayEffectID (weaponData->projectileMiss.grenade.impactEffect, origin, normal, -1, -1);
    }
    
    if ( weaponData->projectileMiss.grenade.shockwaveEffect )
    {
        trap_FX_PlayEffectID (weaponData->projectileMiss.grenade.shockwaveEffect, origin, normal, -1, -1);
    }
}

static void JKG_RenderGrenadeProjectileHitPlayer ( const weaponDrawData_t *weaponData, const vec3_t origin, const vec3_t normal )
{
    if ( weaponData->projectileHitPlayer.grenade.impactEffect )
    {
        trap_FX_PlayEffectID (weaponData->projectileHitPlayer.grenade.impactEffect, origin, normal, -1, -1);
    }
    
    if ( weaponData->projectileMiss.grenade.shockwaveEffect )
    {
        trap_FX_PlayEffectID (weaponData->projectileMiss.grenade.shockwaveEffect, origin, normal, -1, -1);
    }
}

static void JKG_RenderGenericProjectileDeflected ( const weaponDrawData_t *weaponData, const vec3_t origin, const vec3_t normal )
{
    if ( weaponData->projectileDeflected.generic.deflectEffect )
    {
        trap_FX_PlayEffectID (weaponData->projectileDeflected.generic.deflectEffect, origin, normal, -1, -1);
    }
}

static void JKG_ChargeGenericWeapon ( const centity_t *cent, const weaponDrawData_t *weaponData )
{
    if ( weaponData->weaponCharge.chargingSound )
    {
        trap_S_StartSound (NULL, cent->currentState.number, CHAN_WEAPON, weaponData->weaponCharge.chargingSound);
    }
}

static void JKG_GetMuzzleLocation ( centity_t *cent, const vec3_t angles, vec3_t origin, vec3_t direction )
{
    mdxaBone_t boltMatrix;
    
    if ( !trap_G2API_HasGhoul2ModelOnIndex (&cent->ghoul2, 1) )
    {
        // No weapon model on this player
        return;
    }
    
    if ( !trap_G2API_GetBoltMatrix (cent->ghoul2, 1, 0, &boltMatrix, angles, cent->lerpOrigin, cg.time, cgs.gameModels, cent->modelScale) )
    {
        // Couldn't find the muzzle bolt
        return;
    }
    
    if ( origin != NULL )
    {
        BG_GiveMeVectorFromMatrix (&boltMatrix, ORIGIN, origin);
    }
    
    if ( direction != NULL )
    {
        BG_GiveMeVectorFromMatrix (&boltMatrix, POSITIVE_X, direction);
    }
}

#define FX_USE_ALPHA (0x08000000)
static __inline void JKG_RenderChargingEffect ( const vec3_t muzzlePosition, vec3_t *axis, fxHandle_t chargingEffect, qboolean isLocalPlayer, qboolean isFirstPerson, int startedChargingTime )
{
    float time = (cg.time - startedChargingTime) * 0.01f;
    
    if ( time < 0.0f )
    {
        time = 0.0f;
    }
    else if ( time > 1.0f )
    {
        time = 1.0f;
        if ( isLocalPlayer )
        {
            CGCam_Shake (0.2f, 100);
        }
    }
    else
    {
        if ( isLocalPlayer )
        {
            CGCam_Shake (time * time * 0.6f, 100);
        }
    }
    
    if ( isFirstPerson )
    {
        trap_FX_PlayEntityEffectID (chargingEffect, muzzlePosition, axis, -1, -1, -1, -1);
    }
    else
    {
        trap_FX_PlayEffectID (chargingEffect, muzzlePosition, axis[0], -1, -1);
    }
}

void JKG_RenderGenericWeaponWorld ( centity_t *cent, const weaponDrawData_t *weaponData, qboolean altFire, const vec3_t angles )
{
    const entityState_t *s = &cent->currentState;
    qboolean isLocalPlayer = (qboolean)(s->number == cg.predictedPlayerState.clientNum);
    
    qboolean hasMuzzleLocation = qfalse;
    vec3_t flashOrigin, flashDirection;
    //refEntity_t flash;
	
	// Do muzzle charge effects
	if ( !isLocalPlayer || cg.renderingThirdPerson || cg_trueguns.integer )
	{
	    if ( (s->modelindex2 == WEAPON_CHARGING || s->modelindex2 == WEAPON_CHARGING_ALT) &&
	            weaponData->weaponRender.generic.chargingEffect )
	    {
	        hasMuzzleLocation = qtrue;
            JKG_GetMuzzleLocation (cent, angles, flashOrigin, flashDirection);
            
            JKG_RenderChargingEffect (
                flashOrigin,
                &flashDirection,
                weaponData->weaponRender.generic.chargingEffect,
                isLocalPlayer,
                qfalse,
                s->constantLight
            );
	    }
	}
	
	VectorClear (cg.lastFPFlashPoint);
	
	if ( !cent->shotCountOverflowed && cent->shotCount <= cent->muzzleFlashCount )
	{
	    return;
	}
	
	cent->muzzleFlashCount = cent->shotCount;
	cent->shotCountOverflowed = qfalse;
	
	// Do muzzle flash
	if ( !isLocalPlayer || cg.renderingThirdPerson || cg_trueguns.integer )
    {
	    if ( !hasMuzzleLocation )
	    {
	        JKG_GetMuzzleLocation (cent, angles, flashOrigin, flashDirection);
	    }
	    
	    trap_FX_PlayEffectID (
	        weaponData->weaponRender.generic.muzzleEffect,
	        flashOrigin, flashDirection,
	        -1, -1
	    );
	    
	    if ( weaponData->weaponRender.generic.muzzleLightIntensity > 0.0f )
	    {
	        trap_R_AddLightToScene (
	            flashOrigin,
	            weaponData->weaponRender.generic.muzzleLightIntensity + (rand() & 31),
	            weaponData->weaponRender.generic.muzzleLightColor[0],
	            weaponData->weaponRender.generic.muzzleLightColor[1],
	            weaponData->weaponRender.generic.muzzleLightColor[2]
	        );
	    }
	}
}

static void JKG_RenderGenericTraceline ( const weaponDrawData_t *weaponData, const vec3_t start, const vec3_t end )
{
    if ( weaponData->tracelineRender.generic.tracelineShader )
    {
        static const vec3_t WHITE = { 1.0f, 1.0f, 1.0f };
        
        trap_FX_AddLine (
            start, end,
            weaponData->tracelineRender.generic.minSize,
            weaponData->tracelineRender.generic.maxSize,
            0.0f,
            1.0f, 0.0f, 0.0f,
            WHITE, WHITE, 0.0f,
            weaponData->tracelineRender.generic.lifeTime,
            weaponData->tracelineRender.generic.tracelineShader,
            FX_SIZE_LINEAR | FX_ALPHA_LINEAR
        );
    }
}

static void JKG_PrepareExplosiveModel ( refEntity_t *ent, const centity_t *cent, void *g2Model, float radius )
{
    const entityState_t *s = &cent->currentState;
    
    memset (ent, 0, sizeof (*ent));
    
    memcpy (ent->shaderRGBA, s->customRGBA, sizeof (ent->shaderRGBA));
    
    ent->oldframe = ent->frame = s->frame;
    
    ent->ghoul2 = g2Model;
    ent->radius = radius;
    
    VectorCopy (cent->lerpOrigin, ent->origin);
    VectorCopy (cent->lerpOrigin, ent->oldorigin);
    
    VectorCopy (cent->lerpAngles, ent->angles);
    
    AnglesToAxis (cent->lerpAngles, ent->axis);
}

static __inline qboolean JKG_IsTripMineArmed ( const centity_t *cent )
{
    const entityState_t *s = &cent->currentState;

    return (s->eFlags & EF_FIRING) && s->time == -1 && s->bolt2 == 1;
}

static __inline qboolean JKG_TripMineHasBeam ( const centity_t *cent )
{
    return cent->currentState.time == -1;
}

static void JKG_RenderTripmineExplosive ( const centity_t *cent, const weaponDrawData_t *weaponData, qboolean altFire )
{
    if ( weaponData->explosiveRender.tripmine.g2Model )
    {
        refEntity_t ent;
    
        JKG_PrepareExplosiveModel (
            &ent,
            cent,
            weaponData->explosiveRender.tripmine.g2Model,
            weaponData->explosiveRender.tripmine.g2Radius
        );
        
        if ( altFire )
        {
            if ( !JKG_IsTripMineArmed (cent) )
            {
                trap_R_AddRefEntityToScene (&ent);
            }
            
            ent.renderfx &= ~RF_FORCE_ENT_ALPHA;
		    ent.renderfx |= RF_RGB_TINT;
		    ent.shaderRGBA[0] = ent.shaderRGBA[1] = ent.shaderRGBA[2] = 255;
		    ent.shaderRGBA[3] = 0;
		    ent.customShader = cgs.media.cloakedShader;
		}

		trap_R_AddRefEntityToScene (&ent);
		
		if ( weaponData->explosiveRender.tripmine.lineEffect )
        {
            if ( JKG_TripMineHasBeam (cent) )
            {
                if ( !(((cg.time >> 10) & 1) &&
                    TeamFriendly (cent->currentState.owner) &&
                    cent->currentState.owner != cg.snap->ps.clientNum) )
                {
                    vec3_t beamOrigin;
				    int i;
				    int max = 1;
				    
				    VectorMA (ent.origin, 6.6f, ent.axis[0], beamOrigin);
				    
				    if ( JKG_IsTripMineArmed (cent) && cg.snap->ps.fd.forcePowersActive & (1 << FP_SEE) )
				    {
				        max += cg.snap->ps.fd.forcePowerLevel[FP_SEE] * 2;
				    }
				    
				    for ( i = 0; i < max; i++ )
				    {
					    trap_FX_PlayEffectID (
				            weaponData->explosiveRender.tripmine.lineEffect,
				            beamOrigin,
				            cent->currentState.pos.trDelta,
				            -1, -1
				        );
				    }
                }
            }
        }
    }
}

static void JKG_RenderDetpackExplosive ( const centity_t *cent, const weaponDrawData_t *weaponData, qboolean altFire )
{
    if ( weaponData->explosiveRender.detpack.g2Model )
    {
        refEntity_t ent;
    
        JKG_PrepareExplosiveModel (
            &ent,
            cent,
            weaponData->explosiveRender.detpack.g2Model,
            weaponData->explosiveRender.detpack.g2Radius
        );
        
        trap_R_AddRefEntityToScene (&ent);
    }
}

static void JKG_BlowGenericExplosive ( const centity_t *cent, const weaponDrawData_t *weaponData )
{
    if ( weaponData->explosiveBlow.generic.explodeEffect )
    {
        const entityState_t *s = &cent->currentState;
        vec3_t forward;
        
        VectorCopy (s->angles, forward);
        
        if ( VectorNormalize2 (forward, forward) == 0.0f )
        {
            forward[1] = 1.0f;
        }
        
        trap_FX_PlayEffectID (weaponData->explosiveBlow.generic.explodeEffect, s->origin, forward, -1, -1);
    }
}

static void JKG_ArmGenericExplosive ( const centity_t *cent, const weaponDrawData_t *weaponData )
{
    if ( weaponData->explosiveArm.armSound )
    {
        trap_S_StartSound (NULL, cent->currentState.number, CHAN_WEAPON, weaponData->explosiveArm.armSound);
    }
}

qboolean JKG_ShouldRenderWeaponViewModel ( const centity_t *cent, const playerState_t *ps )
{
    if ( ps->persistant[PERS_TEAM] == TEAM_SPECTATOR )
    {
        return qfalse;
    }
    
    if ( ps->pm_type == PM_INTERMISSION || ps->pm_type == PM_SPECTATOR )
    {
        return qfalse;
    }
    
    if ( cg.renderingThirdPerson )
    {
        return qfalse;
    }
    
    if ( !cg.renderingThirdPerson && cg_trueguns.integer && ps->weapon != WP_SABER && ps->weapon != WP_MELEE )
    {
        return qfalse;
    }
    
    // Don't render weapons while in a vehicle
    if ( ps->m_iVehicleNum )
    {
        return qfalse;
    }
    
    if ( cg.testGun )
    {
        return qfalse;
    }
    
    if ( ps->zoomMode )
    {
        return qfalse;
    }
    
    return qtrue;
}

static void JKG_RenderGenericWeaponView ( const weaponDrawData_t *weaponData )
{
    static const char *barrelTags[] = {
        "tag_barrel",
        "tag_barrel2",
        "tag_barrel3",
        "tag_barrel4"
    };

    const playerState_t *ps = &cg.predictedPlayerState;
    centity_t *cent = &cg_entities[ps->clientNum];
    const entityState_t *s = &cent->currentState;
    
    const weaponInfo_t *weapon = NULL;
    
    float fov = cg_fov.value;
    float fovOffset = 0.0f;
    
    vec3_t gunPosition;
    
    refEntity_t hand;
    refEntity_t gun;
    refEntity_t muzzle;
    
    int i;
    refEntity_t barrel;

    if ( !JKG_ShouldRenderWeaponViewModel (cent, ps) )
    {
        return;
    }
    
    if ( !cg_drawGun.integer )
    {
        return;
    }

    // Calculate fov
    fov = CG_ClampFov (fov);
    if ( fov > 90.0f )
    {
        fovOffset = -0.2f * (fov - 90.0f);
    }
    
    // Position hand correctly on-screen
    weapon = CG_WeaponInfo (ps->weapon, ps->weaponVariation);
    
    if ( !weapon->viewModel )
    {
        return;
    }
    
    memset (&hand, 0, sizeof (hand));
    CG_CalculateWeaponPosition (hand.origin, hand.angles);
    
    // Offset the gun if necessary
    gunPosition[0] = abs (cg_gun_x.value) > FLT_EPSILON ? cg_gun_x.value : weapon->gunPosition[0];
    gunPosition[1] = abs (cg_gun_y.value) > FLT_EPSILON ? cg_gun_y.value : weapon->gunPosition[1];
    gunPosition[2] = abs (cg_gun_z.value) > FLT_EPSILON ? cg_gun_z.value : weapon->gunPosition[2];
    
    if ( ps->ironsightsTime & IRONSIGHTS_MSB )
    {
        unsigned int time = ps->ironsightsTime & ~IRONSIGHTS_MSB;
        double phase = CubicBezierInterpolate (min (cg.time - time, IRONSIGHTS_TIME) / (double)IRONSIGHTS_TIME, 0.0, 0.0, 1.0, 1.0);
        
        if ( phase >= 1.0 )
        {
            VectorCopy (weapon->ironsightsPosition, gunPosition);
        }
        else if ( phase > 0.0 )
        {
            vec3_t s;
            VectorSubtract (weapon->ironsightsPosition, gunPosition, s);
            
            VectorMA (gunPosition, phase, s, gunPosition);
        }
    }
    else
    {
        unsigned int time = ps->ironsightsTime & ~IRONSIGHTS_MSB;
        double phase = CubicBezierInterpolate (min (cg.time - time, IRONSIGHTS_TIME) / (double)IRONSIGHTS_TIME, 0.0, 0.0, 1.0, 1.0);
        
        if ( phase > 0.0 && phase < 1.0 )
        {
            vec3_t s;
            VectorSubtract (gunPosition, weapon->ironsightsPosition, s);
            
            VectorMA (weapon->ironsightsPosition, phase, s, gunPosition);
        }
    }
    
    VectorMA (hand.origin, gunPosition[0], cg.refdef.viewaxis[0], hand.origin);
    VectorMA (hand.origin, gunPosition[1], cg.refdef.viewaxis[1], hand.origin);
    VectorMA (hand.origin, gunPosition[2] + fovOffset, cg.refdef.viewaxis[2], hand.origin);
    
    AnglesToAxis (hand.angles, hand.axis);
    
    // Set the correct animation frame
    if ( cg_gun_frame.integer )
    {
        hand.frame = hand.oldframe = cg_gun_frame.integer;
        hand.backlerp = 0.0f;
    }
    else
    {
        const clientInfo_t *ci;
        
        // Xy: The original code checked if the current player is an NPC. I'm
        // thinking this was just left over from the SP code, so I've removed it
        // but added an assertion here just in case it's still needed.
        assert (cent->currentState.eType != ET_NPC);
        
        ci = &cgs.clientinfo[s->clientNum];
        
        hand.frame = CG_MapTorsoToWeaponFrame (ci, cent->pe.torso.frame, s->torsoAnim);
        hand.oldframe = CG_MapTorsoToWeaponFrame (ci, cent->pe.torso.oldFrame, s->torsoAnim);
        hand.backlerp = cent->pe.torso.backlerp;
        
        if ( hand.frame == -1 )
        {
            hand.frame = hand.oldframe = 0;
            hand.backlerp = 0.0f;
        }
        else if ( hand.oldframe )
        {
            hand.oldframe = hand.frame;
            hand.backlerp = 0.0f;
        }
    }
    
    // Render hands with gun
    hand.hModel = weapon->handsModel;
    hand.renderfx = RF_DEPTHHACK | RF_FIRST_PERSON;
    
    memset (&gun, 0, sizeof (gun));
    
    gun.renderfx = hand.renderfx;
    gun.hModel = weapon->viewModel;
    
    CG_PositionEntityOnTag (&gun, &hand, hand.hModel, "tag_weapon");
    if ( !CG_IsMindTricked (s->trickedentindex,
        s->trickedentindex2, s->trickedentindex3,
        s->trickedentindex4, ps->clientNum) )
    {
        CG_AddWeaponWithPowerups (&gun, s->powerups);
    }
    
    /*static const char *barrelModels[] = {
        "models/weapons2/stun_baton/baton_barrel.md3",
        "models/weapons2/stun_baton/baton_barrel2.md3",
        "models/weapons2/stun_baton/baton_barrel3.md3"
    };*/
    
    // Draw barrel models if any
    for ( i = 0; i < 4; i++ )
    {
        if ( weapon->barrelModels[i] == NULL_HANDLE ) break;
        
        memset (&barrel, 0, sizeof (barrel));
        barrel.renderfx = hand.renderfx;
        barrel.hModel = weapon->barrelModels[i];
        
        AnglesToAxis (vec3_origin, barrel.axis);
        CG_PositionRotatedEntityOnTag (&barrel, &hand, hand.hModel, barrelTags[i]);
        
        CG_AddWeaponWithPowerups (&barrel, s->powerups);
    }
    
    memset (&muzzle, 0, sizeof (muzzle));
    CG_PositionRotatedEntityOnTag (&muzzle, &gun, gun.hModel, "tag_flash");
    
    VectorCopy (muzzle.origin, cg.lastFPFlashPoint);
    
    if ( (s->modelindex2 == WEAPON_CHARGING || s->modelindex2 == WEAPON_CHARGING_ALT) &&
            weaponData->weaponRender.generic.chargingEffect )
    {
        //vec3_t origin, angles;
        //JKG_GetMuzzleLocation (cent, hand.angles, origin, angles);
    
        JKG_RenderChargingEffect (
            muzzle.origin,
            muzzle.axis,
            weaponData->weaponRender.generic.chargingEffect,
            qtrue,
            qtrue,
            s->constantLight);
    }
    
    // TODO: At some point, I want to put this into a common function which the
    // world model and view model can call. For now it's just copy/pasted.
    // Do muzzle flash
    if ( !cent->shotCountOverflowed && cent->shotCount <= cent->muzzleFlashCount )
	{
	    return;
	}
	
	cent->muzzleFlashCount = cent->shotCount;
	cent->shotCountOverflowed = qfalse;
	
    trap_FX_PlayEffectID (
        weaponData->weaponRender.generic.muzzleEffect,
        muzzle.origin, muzzle.axis[0],
        -1, -1
    );
    
    if ( weaponData->weaponRender.generic.muzzleLightIntensity > 0.0f )
    {
        trap_R_AddLightToScene (
            muzzle.origin,
            weaponData->weaponRender.generic.muzzleLightIntensity + (rand() & 31),
            weaponData->weaponRender.generic.muzzleLightColor[0],
            weaponData->weaponRender.generic.muzzleLightColor[1],
            weaponData->weaponRender.generic.muzzleLightColor[2]
        );
    }
}

//=========================================================
// Event handlers
//=========================================================
static const weaponEventsHandler_t wpEventsTable[] = 
{
    // Does anyone mind me listing these vertically? I don't think we'll have that many of them here
    // so it shouldn't take up too much screen space.

    {
        "blaster",
        JKG_RenderGenericWeaponWorld,
        JKG_RenderGenericWeaponView,
        JKG_FireBlaster,
        JKG_ChargeGenericWeapon,
        JKG_RenderGenericTraceline,
        NULL,
        NULL,
        NULL,
        NULL,
        JKG_RenderGenericProjectile,
        JKG_RenderGenericProjectileMiss,
        JKG_RenderGenericProjectileDeath,
        JKG_RenderGenericProjectileHitPlayer,
        JKG_RenderGenericProjectileDeflected
    },
    
    {
        "grenade",
        JKG_RenderGenericWeaponWorld,
        JKG_RenderGenericWeaponView,
        JKG_FireBlaster,
        JKG_ChargeGenericWeapon,
        NULL,
        JKG_BounceSpecialGrenade,
        NULL,
        NULL,
        NULL,
        JKG_RenderGenericProjectile,
        JKG_RenderGrenadeProjectileMiss,
        NULL,
        JKG_RenderGrenadeProjectileHitPlayer,
        NULL
    },
    
    {
        "tripmine",
        JKG_RenderGenericWeaponWorld,
        JKG_RenderGenericWeaponView,
        NULL,
        NULL,
        NULL,
        NULL,
        JKG_RenderTripmineExplosive,
        JKG_BlowGenericExplosive,
        JKG_ArmGenericExplosive,
        NULL,
        JKG_RenderExplosiveProjectileMiss,
        NULL,
        JKG_RenderGenericProjectileHitPlayer,
        NULL
    },
    
    {
        "detpack",
        JKG_RenderGenericWeaponWorld,
        JKG_RenderGenericWeaponView,
        NULL,
        NULL,
        NULL,
        NULL,
        JKG_RenderDetpackExplosive,
        JKG_BlowGenericExplosive,
        NULL,
        NULL,
        JKG_RenderExplosiveProjectileMiss,
        NULL,
        JKG_RenderGenericProjectileHitPlayer,
        NULL
    },
    
    // End sentinel
    { NULL }
};

void JKG_SetWeaponEventsHandler ( weaponInfo_t *weaponInfo, const char *primaryEventHandlerName, const char *altEventHandlerName )
{
    const weaponEventsHandler_t *wpEventHandler = wpEventsTable;
    int found = 0;
    
    weaponInfo->primaryEventsHandler = NULL;
    weaponInfo->altEventsHandler = NULL;
    
    for ( ; found < 2 && wpEventHandler->handlerName != NULL; wpEventHandler++ )
    {
        if ( !weaponInfo->primaryEventsHandler &&
            Q_stricmp (primaryEventHandlerName, wpEventHandler->handlerName) == 0 )
        {
            weaponInfo->primaryEventsHandler = wpEventHandler;
            found++;
        }
        
        if ( !weaponInfo->altEventsHandler &&
            Q_stricmp (altEventHandlerName, wpEventHandler->handlerName) == 0 )
        {
            weaponInfo->altEventsHandler = wpEventHandler;
            found++;
        }
    }
    
    if ( found < 2 )
    {
        if ( weaponInfo->primaryEventsHandler == NULL )
        {
            CG_Printf ("Weapon %d: invalid primary event handler \"%s\".\n", weaponInfo->weaponNum, primaryEventHandlerName);
        }
        
        if ( weaponInfo->altEventsHandler == NULL )
        {
            CG_Printf ("Weapon %d: invalid alt event handler \"%s\".\n", weaponInfo->weaponNum, altEventHandlerName);
        }
    }
}

void JKG_RenderWeaponWorldModel ( centity_t *cent, const vec3_t angles )
{
    const weaponInfo_t *weapon;
    const entityState_t *s = &cent->currentState;
    qboolean isLocalPlayer = (qboolean)(s->number == cg.predictedPlayerState.clientNum);
    playerState_t *ps = isLocalPlayer ? &cg.predictedPlayerState : NULL;
    
    if ( s->weapon == WP_EMPLACED_GUN )
	{
		return;
	}

	if ( isLocalPlayer && ps->pm_type == PM_SPECTATOR )
	{
	    // Don't draw our own weapon if we're spectating.
		return;
	}
	
	JKG_WeaponIndicators_Update (cent, NULL);
	
	weapon = CG_WeaponInfo (s->weapon, s->weaponVariation);
	
	if ( s->eFlags & EF_ALT_FIRING )
	{
	    if ( weapon->altEventsHandler && weapon->altEventsHandler->WeaponRenderWorld )
	    {
	        weapon->altEventsHandler->WeaponRenderWorld (cent, &weapon->altDrawData, qtrue, angles);
	    }
	}
	else
	{
	    if ( weapon->primaryEventsHandler && weapon->primaryEventsHandler->WeaponRenderWorld )
	    {
	        weapon->primaryEventsHandler->WeaponRenderWorld (cent, &weapon->primDrawData, qfalse, angles);
	    }
	}
}

void JKG_RenderWeaponViewModel ( void )
{
    const weaponInfo_t *weapon;
    playerState_t *ps = &cg.predictedPlayerState;
    const centity_t *cent = &cg_entities[ps->clientNum];
    const entityState_t *s = &cent->currentState;
    
    if ( s->weapon == WP_EMPLACED_GUN )
	{
		return;
	}
	
	if ( !JKG_ShouldRenderWeaponViewModel (cent, ps) )
	{
	    return;
	}
	
	JKG_WeaponIndicators_Update (cent, ps);
	
	weapon = CG_WeaponInfo (s->weapon, s->weaponVariation);
	
	if ( !(s->eFlags & EF_ALT_FIRING) )
	{
	    if ( weapon->primaryEventsHandler && weapon->primaryEventsHandler->WeaponRenderView )
	    {
	        weapon->primaryEventsHandler->WeaponRenderView (&weapon->primDrawData);
	    }
	}
	else
	{
	    if ( weapon->altEventsHandler && weapon->altEventsHandler->WeaponRenderView )
	    {
	        weapon->altEventsHandler->WeaponRenderView (&weapon->altDrawData);
	    }
	}
}

void JKG_RenderProjectileHitPlayer ( const centity_t *cent, const vec3_t origin, const vec3_t direction, qboolean altFire )
{
    const entityState_t *s = &cent->currentState;
    const weaponInfo_t *weapon = CG_WeaponInfo (s->weapon, s->weaponVariation);
    
    if ( !altFire )
    {
        if ( weapon->primaryEventsHandler && weapon->primaryEventsHandler->ProjectileHitPlayer )
        {
            weapon->primaryEventsHandler->ProjectileHitPlayer (&weapon->primDrawData, origin, direction);
        }
    }
    else
    {
        if ( weapon->altEventsHandler && weapon->altEventsHandler->ProjectileHitPlayer )
        {
            weapon->altEventsHandler->ProjectileHitPlayer (&weapon->altDrawData, origin, direction);
        }
    }
}

void JKG_RenderProjectileMiss ( const centity_t *cent, const vec3_t origin, const vec3_t direction, qboolean altFire )
{
    const entityState_t *s = &cent->currentState;
    const weaponInfo_t *weapon = CG_WeaponInfo (s->weapon, s->weaponVariation);
    
    if ( !altFire )
    {
        if ( weapon->primaryEventsHandler && weapon->primaryEventsHandler->ProjectileMiss )
        {
            weapon->primaryEventsHandler->ProjectileMiss (cent, &weapon->primDrawData, origin, direction);
        }
    }
    else
    {
        if ( weapon->altEventsHandler && weapon->altEventsHandler->ProjectileMiss )
        {
            weapon->altEventsHandler->ProjectileMiss (cent, &weapon->altDrawData, origin, direction);
        }
    }
}

void JKG_RenderProjectileDeath ( const centity_t *cent, const vec3_t origin, const vec3_t direction, qboolean altFire )
{
    const entityState_t *s = &cent->currentState;
    const weaponInfo_t *weapon = CG_WeaponInfo (s->weapon, s->weaponVariation);
    
    if ( !altFire )
    {
        if ( weapon->primaryEventsHandler && weapon->primaryEventsHandler->ProjectileDeath )
        {
            weapon->primaryEventsHandler->ProjectileDeath (cent, &weapon->primDrawData, origin, direction);
        }
    }
    else
    {
        if ( weapon->altEventsHandler && weapon->altEventsHandler->ProjectileDeath )
        {
            weapon->altEventsHandler->ProjectileDeath (cent, &weapon->altDrawData, origin, direction);
        }
    }
}

void JKG_RenderProjectile ( const centity_t *cent, qboolean altFire )
{
    const entityState_t *s = &cent->currentState;
    const weaponInfo_t *weapon = CG_WeaponInfo (s->weapon, s->weaponVariation);
    
    if ( !altFire )
    {
        if ( weapon->primaryEventsHandler && weapon->primaryEventsHandler->ProjectileRender )
        {
            weapon->primaryEventsHandler->ProjectileRender (cent, &weapon->primDrawData);
        }
    }
    else
    {
        if ( weapon->altEventsHandler && weapon->altEventsHandler->ProjectileRender )
        {
            weapon->altEventsHandler->ProjectileRender (cent, &weapon->altDrawData);
        }
    }
}

void JKG_RenderTraceline ( const centity_t *cent, const vec3_t start, const vec3_t end, qboolean altFire )
{
    const entityState_t *s = &cent->currentState;
    const weaponInfo_t *weapon = CG_WeaponInfo (s->weapon, s->weaponVariation);
    
    if ( !altFire )
    {
        if ( weapon->primaryEventsHandler && weapon->primaryEventsHandler->TracelineRender )
        {
            weapon->primaryEventsHandler->TracelineRender (&weapon->primDrawData, start, end);
        }
    }
    else
    {
        if ( weapon->altEventsHandler && weapon->altEventsHandler->TracelineRender )
        {
            weapon->altEventsHandler->TracelineRender (&weapon->altDrawData, start, end);
        }
    }
}

void JKG_BounceGrenade ( const centity_t *cent, qboolean altFire )
{
    const entityState_t *s = &cent->currentState;
    const weaponInfo_t *weapon = CG_WeaponInfo (s->weapon, s->weaponVariation);
    
    if ( !altFire )
    {
        if ( weapon->primaryEventsHandler && weapon->primaryEventsHandler->GrenadeBounce )
        {
            weapon->primaryEventsHandler->GrenadeBounce (cent, &weapon->primDrawData);
        }
    }
    else
    {
        if ( weapon->altEventsHandler && weapon->altEventsHandler->GrenadeBounce )
        {
            weapon->altEventsHandler->GrenadeBounce (cent, &weapon->altDrawData);
        }
    }
}

void JKG_RenderExplosive ( const centity_t *cent, qboolean altFire )
{
    const entityState_t *s = &cent->currentState;
    const weaponInfo_t *weapon = CG_WeaponInfo (s->weapon, s->weaponVariation);
    
    if ( !altFire )
    {
        if ( weapon->primaryEventsHandler && weapon->primaryEventsHandler->ExplosiveRender )
        {
            weapon->primaryEventsHandler->ExplosiveRender (cent, &weapon->primDrawData, qfalse);
        }
    }
    else
    {
        if ( weapon->altEventsHandler && weapon->altEventsHandler->ExplosiveRender )
        {
            weapon->altEventsHandler->ExplosiveRender (cent, &weapon->altDrawData, qtrue);
        }
    }
}

void JKG_ArmExplosive ( const centity_t *cent, qboolean altFire )
{
    const entityState_t *s = &cent->currentState;
    const weaponInfo_t *weapon = CG_WeaponInfo (s->weapon, s->weaponVariation);
    
    if ( !altFire )
    {
        if ( weapon->primaryEventsHandler && weapon->primaryEventsHandler->ExplosiveArm )
        {
            weapon->primaryEventsHandler->ExplosiveArm (cent, &weapon->primDrawData);
        }
    }
    else
    {
        if ( weapon->altEventsHandler && weapon->altEventsHandler->ExplosiveArm )
        {
            weapon->altEventsHandler->ExplosiveArm (cent, &weapon->altDrawData);
        }
    }
}

void JKG_ToggleScope ( const centity_t *cent )
{
    const entityState_t *s = &cent->currentState;
    const weaponInfo_t *weapon = CG_WeaponInfo (s->weapon, s->weaponVariation);
    
    if ( cg.predictedPlayerState.zoomMode )
    {
        if ( weapon->scopeStartSound )
        {
            trap_S_StartLocalSound (weapon->scopeStartSound, CHAN_AUTO);
        }
    }
    else
    {
        if ( weapon->scopeStopSound )
        {
            trap_S_StartLocalSound (weapon->scopeStopSound, CHAN_AUTO);
        }
    }
}

void JKG_RenderScope ( const centity_t *cent )
{
    const entityState_t *s = &cent->currentState;
    const weaponInfo_t *weapon = CG_WeaponInfo (s->weapon, s->weaponVariation);
    
    trap_R_SetColor (colorTable[CT_WHITE]);
    if ( weapon->scopeShader )
    {
        CG_DrawPic (0.0f, 0.0f, 640.0f, 480.0f, weapon->scopeShader);
    }
}

void JKG_ZoomScope ( const centity_t *cent )
{
    const entityState_t *s = &cent->currentState;
    const weaponInfo_t *weapon = CG_WeaponInfo (s->weapon, s->weaponVariation);
    
    static int zoomSoundTime = 0;

    if ( weapon->scopeLoopSound && weapon->scopeSoundLoopTime > 0 )
    {
        if ( zoomSoundTime < cg.time || zoomSoundTime > (cg.time + 10000) )
	    {
		    trap_S_StartSound (cg.refdef.vieworg, ENTITYNUM_WORLD, CHAN_LOCAL, weapon->scopeLoopSound);
		    zoomSoundTime = cg.time + weapon->scopeSoundLoopTime;
	    }
    }
}

void JKG_ChargeWeapon ( const centity_t *cent, qboolean altFire )
{
    const entityState_t *s = &cent->currentState;
    int weaponNum, variation;
    const weaponInfo_t *weapon;
    
    weaponNum = s->eventParm >> 8;
    variation = s->eventParm & 0xFF;
    
    weapon = CG_WeaponInfo (weaponNum, variation);
    
    if ( !altFire )
    {
        if ( weapon->primaryEventsHandler && weapon->primaryEventsHandler->WeaponCharge )
        {
            weapon->primaryEventsHandler->WeaponCharge (cent, &weapon->primDrawData);
        }
    }
    else
    {
        if ( weapon->altEventsHandler && weapon->altEventsHandler->WeaponCharge )
        {
            weapon->altEventsHandler->WeaponCharge (cent, &weapon->altDrawData);
        }
    }
}

void JKG_BlowExplosive ( const centity_t *cent, qboolean altFire )
{
    const entityState_t *s = &cent->currentState;
    const weaponInfo_t *weapon = CG_WeaponInfo (s->weapon, s->weaponVariation);
    
    if ( !altFire )
    {
        if ( weapon->primaryEventsHandler && weapon->primaryEventsHandler->ExplosiveBlow )
        {
            weapon->primaryEventsHandler->ExplosiveBlow (cent, &weapon->primDrawData);
        }
    }
    else
    {
        if ( weapon->altEventsHandler && weapon->altEventsHandler->ExplosiveBlow )
        {
            weapon->altEventsHandler->ExplosiveBlow (cent, &weapon->altDrawData);
        }
    }
}

void JKG_FireWeapon ( centity_t *cent, qboolean altFire )
{
    const weaponInfo_t *weapon;
    const entityState_t *s = &cent->currentState;
    
    if ( s->weapon == WP_NONE )
    {
        return;
    }
    
    if ( s->weapon >= WP_NUM_WEAPONS )
    {
        CG_Error ("JKG_FireWeapon: entityState_t::weapon >= WP_NUM_WEAPONS");
        return;
    }
    
    weapon = CG_WeaponInfo (s->weapon, s->weaponVariation);
    if ( !altFire )
    {
        if ( weapon->primaryEventsHandler && weapon->primaryEventsHandler->WeaponFire )
        {
            weapon->primaryEventsHandler->WeaponFire (cent, &weapon->primDrawData, qfalse);
        }
    }
    else
    {
        if ( weapon->altEventsHandler && weapon->altEventsHandler->WeaponFire )
        {
            weapon->altEventsHandler->WeaponFire (cent, &weapon->altDrawData, qtrue);
        }
    }
}
