//
// UQ1: Weapon Holster System Client Implementation
// 
// Notes:
// * This should be using tags, not bones - I don't like this... The pig guy in the cantina did not like this either...
// * Animations can cause clipping...
// * No weapon class system yet (size based positions) - see above..
// * No .wpn file offsets added yet - see above..
// * Enemy weapons??? How to transfer this data?

// cg_players.c -- handle the media and animation for player entities
#include "cg_local.h"
#include "ghoul2/g2.h"
#include "bg_saga.h"
#include "bg_public.h"
#include "jkg_cg_damagetypes.h"

#ifdef __WEAPON_HOLSTER__
extern vmCvar_t	d_poff;
extern vmCvar_t	d_roff;
extern vmCvar_t	d_yoff;

qboolean CG_SnapRefEntToBone(centity_t *cent, refEntity_t *refEnt, const char *bone_name)
{
	centity_t *cl = cent;
	mdxaBone_t matrix;
	vec3_t boltOrg, holsterAngles, boltAng;
	//vec3_t out_axis[3];
	int getBolt = -1;

	if (!cl->ghoul2)
	{
		assert(0);
		return qfalse;
	}

	if (cent->currentState.clientNum == cg.predictedPlayerState.clientNum &&
		!cg.renderingThirdPerson)
	{ //If in first person and you have it then render the thing spinning around on your hud.
		return qfalse;
	}

	getBolt = trap_G2API_AddBolt(cl->ghoul2, 0, bone_name);

	VectorCopy(cl->lerpAngles, holsterAngles);
	holsterAngles[PITCH] = 0;
	holsterAngles[ROLL] = 0;

	trap_G2API_GetBoltMatrix(cl->ghoul2, 0, getBolt, &matrix, holsterAngles /*cl->lerpAngles*/, cl->lerpOrigin, cg.time, cgs.gameModels, cl->modelScale);

	BG_GiveMeVectorFromMatrix(&matrix, ORIGIN, boltOrg);
	BG_GiveMeVectorFromMatrix(&matrix, NEGATIVE_Y, boltAng);
	vectoangles(boltAng, boltAng);
	//boltAng[PITCH] = boltAng[ROLL] = 0;

	//BG_GiveMeVectorFromMatrix(&matrix, ORIGIN, boltOrg);
	//BG_GiveMeVectorFromMatrix( &matrix, NEGATIVE_Y, refEnt->axis[0] );//out_axis[0] );
	//BG_GiveMeVectorFromMatrix( &matrix, POSITIVE_X, refEnt->axis[1] );//out_axis[1] );
	//BG_GiveMeVectorFromMatrix( &matrix, POSITIVE_Z, refEnt->axis[2] );//out_axis[2] );

	VectorCopy(boltOrg, refEnt->origin);
	
	//VectorCopy(cl->lerpAngles, refEnt->angles);
	VectorCopy(boltAng, refEnt->angles);
	
	return qtrue;
}

void JKG_DrawWeaponHolsters( centity_t *cent, refEntity_t legs, float shadowPlane )
{
	if (cent->currentState.weapon > 0  // UQ1: Issues with weapon 0???
		&& !(cent->currentState.number == cg.predictedPlayerState.clientNum && !cg.renderingThirdPerson)
		&& cent->currentState.number == cg.clientNum // UQ1: Enemy ones??? Best data transfer method???
		&& (cent && cent->ghoul2 && cent->currentState.eType != ET_NPC))
	{
		int				HOLSTER_POINT = 0;
		int				NUM_HOLSTERS = 5; // UQ1: Extra 2 look stupid...
		int				i = 0;
		cgItemData_t	*LIGHT_ITEMS_LIST[5];
		cgItemData_t	*HEAVY_ITEMS_LIST[5];
		int				NUM_LIGHT_ITEMS = 0;
		int				NUM_HEAVY_ITEMS = 0;

#ifdef _DEBUG 
		//CG_Printf("Adding holster items...\n"); 
#endif //_DEBUG

		for (i = 0; i < 3; i++) LIGHT_ITEMS_LIST[i] = NULL;
		for (i = 0; i < 2; i++) HEAVY_ITEMS_LIST[i] = NULL;

		// Create Light Items List...
		for (i = 0; i < MAX_ACI_SLOTS; i++)
		{
			unsigned int weap, var;

			// FIXME: How to get enemy player ACI list????
			//        Events? Even the data for just 4 holsters would be crazy...
			if (cg.playerACI[i] < 0) continue;

			if (cg.playerInventory[cg.playerACI[i]].id->weapon == cg.predictedPlayerState.weapon) continue;
			if (cg.playerInventory[cg.playerACI[i]].equipped) continue;
			if (!cg.playerInventory[cg.playerACI[i]].id) continue;

			weap = cg.playerInventory[cg.playerACI[i]].id->weapon;
			var = cg.playerInventory[cg.playerACI[i]].id->variation;

			// How did this get through equipped above???
			if (weap == cent->currentState.weapon) continue;

			// Quick and dirty choice of where to put guns based on weight...
			if (cg.playerInventory[cg.playerACI[i]].id->weight > 1) continue; // Too heavy for waist...

			// FIXME: Sort by new WEAPON_TYPE value...
			LIGHT_ITEMS_LIST[NUM_LIGHT_ITEMS] = cg.playerInventory[cg.playerACI[i]].id;
			NUM_LIGHT_ITEMS++;

			if (NUM_LIGHT_ITEMS >= 3) break; // Full...
		}

		// Create Heavy Items List...
		for (i = 0; i < MAX_ACI_SLOTS; i++)
		{
			unsigned int weap, var;

			// FIXME: How to get enemy player ACI list????
			//        Events? Even the data for just 4 holsters would be crazy...
			if (cg.playerACI[i] < 0) continue;

			if (cg.playerInventory[cg.playerACI[i]].equipped) continue;
			if (!cg.playerInventory[cg.playerACI[i]].id) continue;

			weap = cg.playerInventory[cg.playerACI[i]].id->weapon;
			var = cg.playerInventory[cg.playerACI[i]].id->variation;

			// How did this get through equipped above???
			if (weap == cent->currentState.weapon) continue;

			// Quick and dirty choice of where to put guns based on weight...
			if (cg.playerInventory[cg.playerACI[i]].id->weight <= 1) continue; // Too light for back...

			// FIXME: Sort by new WEAPON_TYPE value...
			HEAVY_ITEMS_LIST[NUM_HEAVY_ITEMS] = cg.playerInventory[cg.playerACI[i]].id;
			NUM_HEAVY_ITEMS++;

			if (NUM_HEAVY_ITEMS >= 2) break; // Full...
		}

		for(HOLSTER_POINT = 0; HOLSTER_POINT < NUM_HOLSTERS; HOLSTER_POINT++)
		{
			void			*weaponGhoul2 = NULL;
			refEntity_t		holsterRefEnt;
			int				WEAPON_NUM = 0;
			weaponInfo_t	*weapon = NULL;

			if (HOLSTER_POINT == 0 && NUM_LIGHT_ITEMS < 1) continue; // No more light items to draw...
			if (HOLSTER_POINT == 1 && NUM_LIGHT_ITEMS < 2) continue; // No more light items to draw...
			//if (HOLSTER_POINT == 2 && NUM_LIGHT_ITEMS < 3) continue; // No more light items to draw...
			if (HOLSTER_POINT == 2) continue; // Disabled...
			if (HOLSTER_POINT == 3 && NUM_HEAVY_ITEMS < 1) continue; // No more heavy items to draw...
			if (HOLSTER_POINT == 4 && NUM_HEAVY_ITEMS < 2) continue; // No more heavy items to draw...

			
			if (HOLSTER_POINT < 3)
			{
				if (LIGHT_ITEMS_LIST[HOLSTER_POINT])
				{// Light Items (waist)...
					WEAPON_NUM = LIGHT_ITEMS_LIST[HOLSTER_POINT]->weapon;

					if (WEAPON_NUM <= 0) continue;

					weapon = CG_WeaponInfo (WEAPON_NUM, LIGHT_ITEMS_LIST[HOLSTER_POINT]->variation);
				}
				else
				{
					continue;
				}
			}
			else
			{
				if (HEAVY_ITEMS_LIST[HOLSTER_POINT-3])
				{// Heavy Items (back)...
					WEAPON_NUM = HEAVY_ITEMS_LIST[HOLSTER_POINT-3]->weapon;

					if (WEAPON_NUM <= 0) continue;

					weapon = CG_WeaponInfo (WEAPON_NUM, HEAVY_ITEMS_LIST[HOLSTER_POINT-3]->variation);
				}
				else
				{
					continue;
				}
			}

			if (!weapon) continue;

			memset( &holsterRefEnt, 0, sizeof( holsterRefEnt ) );

			weaponGhoul2 = weapon->g2WorldModel;

			if (trap_G2_HaveWeGhoul2Models(weaponGhoul2))
			{
				char bone_name[MAX_QPATH];
				vec3_t	holsterAngles, end, fwd, rt, originalOrigin;

				// UQ1: Which bone are we using?
				if (HOLSTER_POINT == 0) Q_strncpyz( bone_name , "pelvis", sizeof( bone_name ) );
				if (HOLSTER_POINT == 1) Q_strncpyz( bone_name , "pelvis", sizeof( bone_name ) );
				if (HOLSTER_POINT == 2) Q_strncpyz( bone_name , "thoracic", sizeof( bone_name ) );
				if (HOLSTER_POINT == 3) Q_strncpyz( bone_name , "thoracic", sizeof( bone_name ) );
				if (HOLSTER_POINT == 4) Q_strncpyz( bone_name , "thoracic", sizeof( bone_name ) );
				//if (HOLSTER_POINT == 4) Q_strncpyz( bone_name , "ltibia", sizeof( bone_name ) ); // UQ1: These look really dumb....
				//if (HOLSTER_POINT == 5) Q_strncpyz( bone_name , "rtibia", sizeof( bone_name ) ); // UQ1: These look really dumb....

				// UQ1: Set exact org/angles for this bone...
				if (!CG_SnapRefEntToBone(cent, &holsterRefEnt, bone_name))
				{
					#ifdef _DEBUG 
					CG_Printf("Holster point %i NOT drawn (snap to bone issue)...\n", HOLSTER_POINT);
					assert(0);
					#endif //
						
					continue;
				}

				VectorCopy( legs.modelScale, holsterRefEnt.modelScale);
				holsterRefEnt.radius = legs.radius;

				VectorCopy(holsterRefEnt.origin, originalOrigin);
				VectorCopy(cent->lerpAngles, holsterAngles);
				holsterAngles[PITCH] = 0;
				holsterAngles[ROLL] = 0;

				AngleVectors( holsterAngles, fwd, rt, NULL );

				// UQ1: Now modify the org/angles...
				switch (HOLSTER_POINT)
				{
					case 0:
						// "pelvis" left
						VectorMA( originalOrigin, -2, fwd, end );
						VectorMA( end, -6, rt, end );
						VectorCopy(end, holsterRefEnt.origin);

						holsterRefEnt.origin[2] += 8; // because the bones suck for placement...

						//AxisToAngles(holsterRefEnt.axis, holsterAngles);

						holsterAngles[PITCH] += 90;
						//holsterAngles[YAW] -= 90;
						holsterAngles[ROLL] += 90;

						holsterAngles[PITCH] += 30;
						holsterAngles[YAW] -= 50;
						holsterAngles[ROLL] -= 50;

						break;
					case 1:
						// "pelvis" right
						VectorMA( originalOrigin, -2, fwd, end );
						VectorMA( end, 6, rt, end );
						VectorCopy(end, holsterRefEnt.origin);

						holsterRefEnt.origin[2] += 8; // because the bones suck for placement...

						//AxisToAngles(holsterRefEnt.axis, holsterAngles);

						holsterAngles[PITCH] += 90;
						holsterAngles[YAW] += 180;
						holsterAngles[ROLL] += 90;

						holsterAngles[PITCH] -= 30;
						holsterAngles[YAW] += 50;
						holsterAngles[ROLL] += 50;

						break;
					case 2:
						// "thoracic" left - used for center pistol - where on the back is this meant to fit???
						VectorMA( originalOrigin, -4, fwd, end );
						VectorMA( end, -2, rt, end );
						VectorCopy(end, holsterRefEnt.origin);

						holsterRefEnt.origin[2] += 8; // because the bones suck for placement...

						//AxisToAngles(holsterRefEnt.axis, holsterAngles);

						holsterAngles[PITCH] += 90;
						//holsterAngles[YAW] -= 90;
						holsterAngles[ROLL] += 90;

						holsterAngles[PITCH] += 10;//d_poff.value;
						holsterAngles[YAW] += 0;//d_yoff.value;
						holsterAngles[ROLL] += 0;//d_roff.value;

						break;
					case 3:
						// "thoracic" left
						VectorMA( originalOrigin, -4, fwd, end );
						VectorMA( end, -6, rt, end );
						VectorCopy(end, holsterRefEnt.origin);

						holsterRefEnt.origin[2] += 6; // because the bones suck for placement...

						//AxisToAngles(holsterRefEnt.axis, holsterAngles);

						holsterAngles[PITCH] += 90;
						//holsterAngles[YAW] -= 90;
						holsterAngles[ROLL] += 90;

						holsterAngles[PITCH] += 10 + 25;
						holsterAngles[YAW] += 50 + 15;
						holsterAngles[ROLL] += 50;

						break;
					case 4:
						// "thoracic" right
						VectorMA( originalOrigin, -4, fwd, end );
						VectorMA( end, 6, rt, end );
						VectorCopy(end, holsterRefEnt.origin);

						holsterRefEnt.origin[2] += 6; // because the bones suck for placement...

						//AxisToAngles(holsterRefEnt.axis, holsterAngles);

						holsterAngles[PITCH] += 90;
						holsterAngles[YAW] += 180;
						holsterAngles[ROLL] += 90;

						holsterAngles[PITCH] -= 10 + 25;
						holsterAngles[YAW] -= 50 + 15;
						holsterAngles[ROLL] -= 50;

						break;
					/*case 5: // UQ1: These look really dumb....
						// "ltibia"
						VectorMA( originalOrigin, -4, fwd, end );
						VectorMA( end, -4, rt, end );
						VectorCopy(end, holsterRefEnt.origin);

						holsterRefEnt.origin[2] += 8; // because the bones suck for placement...

						holsterAngles[PITCH] += 90;
						//holsterAngles[YAW] -= 90;
						holsterAngles[ROLL] += 90;

						holsterAngles[PITCH] -= d_poff.value;
						holsterAngles[YAW] -= d_yoff.value;
						holsterAngles[ROLL] -= d_roff.value;
						break;
					case 6:  // UQ1: These look really dumb....
						// "rtibia"
						VectorMA( originalOrigin, -4, fwd, end );
						VectorMA( end, 4, rt, end );
						VectorCopy(end, holsterRefEnt.origin);

						holsterRefEnt.origin[2] += 8; // because the bones suck for placement...

						holsterAngles[PITCH] += 90;
						holsterAngles[YAW] += 180;
						holsterAngles[ROLL] += 90;

						holsterAngles[PITCH] += d_poff.value;
						holsterAngles[YAW] += d_yoff.value;
						holsterAngles[ROLL] += d_roff.value;
						break;*/
					default:
						continue; // should never see this...
						break;
				}

				holsterRefEnt.reType = RT_MODEL;
				holsterRefEnt.hModel = 0;
				holsterRefEnt.ghoul2 = weaponGhoul2;
				AnglesToAxis(holsterAngles, holsterRefEnt.axis);

				holsterRefEnt.shadowPlane = shadowPlane;

				CG_AddWeaponWithPowerups( &holsterRefEnt, cent->currentState.powerups );
				#ifdef _DEBUG 
				//CG_Printf("Holster point %i drawn at %f %f %f (P.ORG %f %f %f)...\n", HOLSTER_POINT, holsterRefEnt.origin[0], holsterRefEnt.origin[1], holsterRefEnt.origin[2], cent->lerpOrigin[0], cent->lerpOrigin[1], cent->lerpOrigin[2]);
				#endif //_DEBUG
			}
			#ifdef _DEBUG 
			else
			{
				//CG_Printf("Holster point %i NOT drawn...\n", HOLSTER_POINT);
				//assert(0);
			}
			#endif //_DEBUG
		}
	}
}
#endif //__WEAPON_HOLSTER__
