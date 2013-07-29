// ==========================================================================================================================
//
//                                                  UniqueOne's
//         The new super-duper ultra mega total chaos bot AI evasion and acrobatics. Ported from the NPC code...
//
// ==========================================================================================================================
//
//                                     Let's make bots fight like champions!
//
//
//          With a bot's faster thinktime and smooter animations, this actually makes them really, really cool!
//
//                                  Why the hell has nobody done this before???
//
// ==========================================================================================================================

//includes
#include "g_local.h"
#include "ai_main.h"
#include "b_local.h"
#include "g_nav.h"
#include "anims.h"
#include "w_saber.h"

//externs
extern bot_state_t	*botstates[MAX_CLIENTS];
extern gentity_t	*NPC;

extern int bg_parryDebounce[NUM_FORCE_POWER_LEVELS];

extern qboolean BG_SabersOff( playerState_t *ps );

extern void CG_DrawAlert( vec3_t origin, float rating );
extern void G_AddVoiceEvent( gentity_t *self, int event, int speakDebounceTime );
extern void ForceJump( gentity_t *self, usercmd_t *ucmd );
extern vmCvar_t	g_saberRealisticCombat;

extern void NPC_ClearLookTarget( gentity_t *self );
extern void NPC_SetLookTarget( gentity_t *self, int entNum, int clearTime );
extern void NPC_TempLookTarget( gentity_t *self, int lookEntNum, int minLookTime, int maxLookTime );
extern qboolean G_ExpandPointToBBox( vec3_t point, const vec3_t mins, const vec3_t maxs, int ignore, int clipmask );
extern qboolean NPC_CheckEnemyStealth( void );
extern qboolean DOM_MoveDirClear( int forwardmove, int rightmove, qboolean reset );
extern void G_SoundOnEnt( gentity_t *ent, soundChannel_t channel, const char *soundPath );

extern gitem_t	*BG_FindItemForAmmo( ammoType_t ammo );

extern void ForceThrow( gentity_t *self, qboolean pull );
extern void ForceLightning( gentity_t *self );
extern void ForceHeal( gentity_t *self );
extern void ForceRage( gentity_t *self );
extern void ForceProtect( gentity_t *self );
extern void ForceAbsorb( gentity_t *self );
extern int WP_MissileBlockForBlock( int saberBlock );
extern qboolean G_GetHitLocFromSurfName( gentity_t *ent, const char *surfName, int *hitLoc, vec3_t point, vec3_t dir, vec3_t bladeDir, int mod );
extern qboolean WP_ForcePowerUsable( gentity_t *self, forcePowers_t forcePower, int overrideAmt );
extern qboolean WP_ForcePowerAvailable( gentity_t *self, forcePowers_t forcePower, int overrideAmt );
extern void WP_ForcePowerStop( gentity_t *self, forcePowers_t forcePower );
extern void WP_DeactivateSaber( gentity_t *self, qboolean clearLength ); //clearLength = qfalse
extern void WP_ActivateSaber( gentity_t *self );
//extern void WP_SaberBlock(gentity_t *saber, vec3_t hitloc);

extern qboolean PM_SaberInStart( int move );
extern qboolean BG_SaberInSpecialAttack( int anim );
extern qboolean BG_SaberInAttack( int move );
extern qboolean PM_SaberInBounce( int move );
extern qboolean PM_SaberInParry( int move );
extern qboolean PM_SaberInKnockaway( int move );
extern qboolean PM_SaberInBrokenParry( int move );
extern qboolean PM_SaberInDeflect( int move );
extern qboolean BG_SpinningSaberAnim( int anim );
extern qboolean BG_FlippingAnim( int anim );
extern qboolean PM_RollingAnim( int anim );
extern qboolean PM_InKnockDown( playerState_t *ps );
extern qboolean BG_InRoll( playerState_t *ps, int anim );
extern qboolean BG_CrouchAnim( int anim );

extern qboolean NPC_SomeoneLookingAtMe(gentity_t *ent);

extern int WP_GetVelocityForForceJump( gentity_t *self, vec3_t jumpVel, usercmd_t *ucmd );

extern void G_TestLine(vec3_t start, vec3_t end, int color, int time);

qboolean NPC_Humanoid_WaitingAmbush( gentity_t *self );

qboolean DOM_MoveDirClear( int forwardmove, int rightmove, qboolean reset )
{
	vec3_t	forward, right, testPos, angles, mins;
	trace_t	trace;
	float	fwdDist, rtDist;
	float	bottom_max = -STEPSIZE*4 - 1;

	if ( !forwardmove && !rightmove )
	{//not even moving
		//Com_Printf( "%d skipping walk-cliff check (not moving)\n", level.time );
		return qtrue;
	}

	if ( ucmd.upmove > 0 || NPC->client->ps.fd.forceJumpCharge )
	{//Going to jump
		//Com_Printf( "%d skipping walk-cliff check (going to jump)\n", level.time );
		return qtrue;
	}

	if ( NPC->client->ps.groundEntityNum == ENTITYNUM_NONE )
	{//in the air
		//Com_Printf( "%d skipping walk-cliff check (in air)\n", level.time );
		return qtrue;
	}
	/*
	if ( fabs( AngleDelta( NPC->r.currentAngles[YAW], NPCInfo->desiredYaw ) ) < 5.0 )//!ucmd.angles[YAW] )
	{//Not turning much, don't do this
		//NOTE: Should this not happen only if you're not turning AT ALL?
		//	You could be turning slowly but moving fast, so that would
		//	still let you walk right off a cliff...
		//NOTE: Or maybe it is a good idea to ALWAYS do this, regardless
		//	of whether ot not we're turning?  But why would we be walking
		//  straight into a wall or off	a cliff unless we really wanted to?
		return;
	}
	*/

	//FIXME: to really do this right, we'd have to actually do a pmove to predict where we're
	//going to be... maybe this should be a flag and pmove handles it and sets a flag so AI knows
	//NEXT frame?  Or just incorporate current velocity, runspeed and possibly friction?
	VectorCopy( NPC->r.mins, mins );
	mins[2] += STEPSIZE;
	angles[PITCH] = angles[ROLL] = 0;
	angles[YAW] = NPC->client->ps.viewangles[YAW];//Add ucmd.angles[YAW]?
	AngleVectors( angles, forward, right, NULL );
	fwdDist = ((float)forwardmove)/2.0f;
	rtDist = ((float)rightmove)/2.0f;
	VectorMA( NPC->r.currentOrigin, fwdDist, forward, testPos );
	VectorMA( testPos, rtDist, right, testPos );
	trap_Trace( &trace, NPC->r.currentOrigin, mins, NPC->r.maxs, testPos, NPC->s.number, NPC->clipmask|CONTENTS_BOTCLIP );
	if ( trace.allsolid || trace.startsolid )
	{//hmm, trace started inside this brush... how do we decide if we should continue?
		//FIXME: what do we do if we start INSIDE a CONTENTS_BOTCLIP? Try the trace again without that in the clipmask?
		if ( reset )
		{
			trace.fraction = 1.0f;
		}
		VectorCopy( testPos, trace.endpos );
		//return qtrue;
	}
	if ( trace.fraction < 0.6 )
	{//Going to bump into something very close, don't move, just turn
		if ( NPC->enemy && trace.entityNum == NPC->enemy->s.number )
		{//okay to bump into enemy or goal
			//Com_Printf( "%d bump into enemy/goal okay\n", level.time );
			return qtrue;
		}
		else if ( reset )
		{//actually want to screw with the ucmd
			//Com_Printf( "%d avoiding walk into wall (entnum %d)\n", level.time, trace.entityNum );
			ucmd.forwardmove = 0;
			ucmd.rightmove = 0;
			VectorClear( NPC->client->ps.moveDir );
		}
		return qfalse;
	}

	/*
	if (  )
	{
		if ( NPCInfo->goalEntity->r.currentOrigin[2] < NPC->r.currentOrigin[2] )
		{//goal is below me, okay to step off at least that far plus stepheight
			bottom_max += NPCInfo->goalEntity->r.currentOrigin[2] - NPC->r.currentOrigin[2];
		}
	}
	*/

	VectorCopy( trace.endpos, testPos );
	testPos[2] += bottom_max;

	trap_Trace( &trace, trace.endpos, mins, NPC->r.maxs, testPos, NPC->s.number, NPC->clipmask );

	//FIXME:Should we try to see if we can still get to our goal using the waypoint network from this trace.endpos?
	//OR: just put NPC clip brushes on these edges (still fall through when die)

	if ( trace.allsolid || trace.startsolid )
	{//Not going off a cliff
		//Com_Printf( "%d walk off cliff okay (droptrace in solid)\n", level.time );
		return qtrue;
	}

	if ( trace.fraction < 1.0 )
	{//Not going off a cliff
		//FIXME: what if plane.normal is sloped?  We'll slide off, not land... plus this doesn't account for slide-movement... 
		//Com_Printf( "%d walk off cliff okay will hit entnum %d at dropdist of %4.2f\n", level.time, trace.entityNum, (trace.fraction*bottom_max) );
		return qtrue;
	}

	//going to fall at least bottom_max, don't move, just turn... is this bad, though?  What if we want them to drop off?
	if ( reset )
	{//actually want to screw with the ucmd
		//Com_Printf( "%d avoiding walk off cliff\n", level.time );
		ucmd.forwardmove *= -1.0;//= 0;
		ucmd.rightmove *= -1.0;//= 0;
		VectorScale( NPC->client->ps.moveDir, -1, NPC->client->ps.moveDir );
	}
	return qfalse;
}

/*
qboolean NPC_Humanoid_DodgeEvasion( gentity_t *self, gentity_t *shooter, trace_t *tr, int hitLoc )

Jedi will play a dodge anim, blur, and make the force speed noise.

Right now used to dodge instant-hit weapons.

FIXME: possibly call this for saber melee evasion and/or missile evasion?
FIXME: possibly let player do this too?
*/
//rwwFIXMEFIXME: Going to use qboolean NPC_Humanoid_DodgeEvasion( gentity_t *self, gentity_t *shooter, trace_t *tr, int hitLoc ) from
//w_saber.c.. maybe use seperate one for NPCs or add cases to that one?

evasionType_t DOM_Jedi_CheckFlipEvasions( gentity_t *self, float rightdot, float zdiff )
{
	if ( self->client 
		&& (self->client->ps.fd.forceRageRecoveryTime > level.time	|| (self->client->ps.fd.forcePowersActive&(1<<FP_RAGE))) )
	{//no fancy dodges when raging
		return EVASION_NONE;
	}
	//Check for:
	//ARIALS/CARTWHEELS
	//WALL-RUNS
	//WALL-FLIPS
	//FIXME: if facing a wall, do forward wall-walk-backflip
	if ( self->client->ps.legsAnim == BOTH_WALL_RUN_LEFT || self->client->ps.legsAnim == BOTH_WALL_RUN_RIGHT )
	{//already running on a wall
		vec3_t right, fwdAngles;
		int		anim = -1;
		float animLength;

		VectorSet(fwdAngles, 0, self->client->ps.viewangles[YAW], 0);

		AngleVectors( fwdAngles, NULL, right, NULL );

		animLength = BG_AnimLength( self->localAnimIndex, (animNumber_t)self->client->ps.legsAnim );
		if ( self->client->ps.legsAnim == BOTH_WALL_RUN_LEFT && rightdot < 0 )
		{//I'm running on a wall to my left and the attack is on the left
			if ( animLength - self->client->ps.legsTimer > 400
				&& self->client->ps.legsTimer > 400 )
			{//not at the beginning or end of the anim
				anim = BOTH_WALL_RUN_LEFT_FLIP;
			}
		}
		else if ( self->client->ps.legsAnim == BOTH_WALL_RUN_RIGHT && rightdot > 0 )
		{//I'm running on a wall to my right and the attack is on the right
			if ( animLength - self->client->ps.legsTimer > 400
				&& self->client->ps.legsTimer > 400 )
			{//not at the beginning or end of the anim
				anim = BOTH_WALL_RUN_RIGHT_FLIP;
			}
		}
		if ( anim != -1 )
		{//flip off the wall!
			int parts;
			//FIXME: check the direction we will flip towards for do-not-enter/walls/drops?
			//NOTE: we presume there is still a wall there!
			if ( anim == BOTH_WALL_RUN_LEFT_FLIP )
			{
				self->client->ps.velocity[0] *= 0.5f;
				self->client->ps.velocity[1] *= 0.5f;
				VectorMA( self->client->ps.velocity, 150, right, self->client->ps.velocity );
			}
			else if ( anim == BOTH_WALL_RUN_RIGHT_FLIP )
			{
				self->client->ps.velocity[0] *= 0.5f;
				self->client->ps.velocity[1] *= 0.5f;
				VectorMA( self->client->ps.velocity, -150, right, self->client->ps.velocity );
			}
			parts = SETANIM_LEGS;
			if ( !self->client->ps.weaponTime )
			{
				parts = SETANIM_BOTH;
			}
			NPC_SetAnim( self, parts, anim, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD );
			//self->client->ps.pm_flags |= (PMF_JUMPING|PMF_SLOW_MO_FALL);
			//rwwFIXMEFIXME: Add these pm flags?
			G_AddEvent( self, EV_JUMP, 0 );
			return EVASION_OTHER;
		}
	}
	else if ( Q_irand( 0, 1 ) 
		&& !BG_InRoll( &self->client->ps, self->client->ps.legsAnim )
		&& !PM_InKnockDown( &self->client->ps )
		&& !BG_SaberInSpecialAttack( self->client->ps.torsoAnim ) )
	{
		vec3_t fwd, right, traceto, mins, maxs, fwdAngles;
		trace_t	trace;
		int parts, anim;
		float	speed, checkDist;
		qboolean allowCartWheels = qtrue;
		qboolean allowWallFlips = qtrue;

		if ( self->client->ps.weapon == WP_SABER )
		{
			if ( self->client->saber[0].model
				&& self->client->saber[0].model[0]
			&& (self->client->saber[0].saberFlags&SFL_NO_CARTWHEELS) )
			{
				allowCartWheels = qfalse;
			}
			else if ( self->client->saber[1].model
				&& self->client->saber[1].model[0]
			&& (self->client->saber[1].saberFlags&SFL_NO_CARTWHEELS) )
			{
				allowCartWheels = qfalse;
			}
			if ( self->client->saber[0].model
				&& self->client->saber[0].model[0]
			&& (self->client->saber[0].saberFlags&SFL_NO_WALL_FLIPS) )
			{
				allowWallFlips = qfalse;
			}
			else if ( self->client->saber[1].model
				&& self->client->saber[1].model[0]
			&& (self->client->saber[1].saberFlags&SFL_NO_WALL_FLIPS) )
			{
				allowWallFlips = qfalse;
			}
		}

		VectorSet(mins, self->r.mins[0],self->r.mins[1],0);
		VectorSet(maxs, self->r.maxs[0],self->r.maxs[1],24);
		VectorSet(fwdAngles, 0, self->client->ps.viewangles[YAW], 0);

		AngleVectors( fwdAngles, fwd, right, NULL );

		parts = SETANIM_BOTH;

		if ( BG_SaberInAttack( self->client->ps.saberMove )
			|| PM_SaberInStart( self->client->ps.saberMove ) )
		{
			parts = SETANIM_LEGS;
		}
		if ( rightdot >= 0 )
		{
			if ( Q_irand( 0, 1 ) )
			{
				anim = BOTH_ARIAL_LEFT;
			}
			else
			{
				anim = BOTH_CARTWHEEL_LEFT;
			}
			checkDist = -128;
			speed = -200;
		}
		else
		{
			if ( Q_irand( 0, 1 ) )
			{
				anim = BOTH_ARIAL_RIGHT;
			}
			else
			{
				anim = BOTH_CARTWHEEL_RIGHT;
			}
			checkDist = 128;
			speed = 200;
		}
		//trace in the dir that we want to go
		VectorMA( self->r.currentOrigin, checkDist, right, traceto );
		trap_Trace( &trace, self->r.currentOrigin, mins, maxs, traceto, self->s.number, CONTENTS_SOLID|CONTENTS_MONSTERCLIP|CONTENTS_BOTCLIP );
		if ( trace.fraction >= 1.0f && allowCartWheels )
		{//it's clear, let's do it
			//FIXME: check for drops?
			vec3_t fwdAngles, jumpRt;

			NPC_SetAnim( self, parts, anim, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD );
			self->client->ps.weaponTime = self->client->ps.legsTimer;//don't attack again until this anim is done
			VectorCopy( self->client->ps.viewangles, fwdAngles );
			fwdAngles[PITCH] = fwdAngles[ROLL] = 0;
			//do the flip
			AngleVectors( fwdAngles, NULL, jumpRt, NULL );
			VectorScale( jumpRt, speed, self->client->ps.velocity );
			self->client->ps.fd.forceJumpCharge = 0;//so we don't play the force flip anim
			self->client->ps.velocity[2] = 200;
			self->client->ps.fd.forceJumpZStart = self->r.currentOrigin[2];//so we don't take damage if we land at same height
			//self->client->ps.pm_flags |= PMF_JUMPING;
			G_SoundOnEnt( self, CHAN_BODY, "sound/weapons/force/jump.wav" );
			//ucmd.upmove = 0;
			return EVASION_CARTWHEEL;
		}
		else if ( !(trace.contents&CONTENTS_BOTCLIP) )
		{//hit a wall, not a do-not-enter brush
			//FIXME: before we check any of these jump-type evasions, we should check for headroom, right?
			//Okay, see if we can flip *off* the wall and go the other way
			vec3_t	idealNormal;
			gentity_t *traceEnt;

			VectorSubtract( self->r.currentOrigin, traceto, idealNormal );
			VectorNormalize( idealNormal );
			traceEnt = &g_entities[trace.entityNum];
			if ( (trace.entityNum<ENTITYNUM_WORLD&&traceEnt&&traceEnt->s.solid!=SOLID_BMODEL) || DotProduct( trace.plane.normal, idealNormal ) > 0.7f )
			{//it's a ent of some sort or it's a wall roughly facing us
				float bestCheckDist = 0;
				//hmm, see if we're moving forward
				if ( DotProduct( self->client->ps.velocity, fwd ) < 200 )
				{//not running forward very fast
					//check to see if it's okay to move the other way
					if ( (trace.fraction*checkDist) <= 32 )
					{//wall on that side is close enough to wall-flip off of or wall-run on
						bestCheckDist = checkDist;
						checkDist *= -1.0f;
						VectorMA( self->r.currentOrigin, checkDist, right, traceto );
						//trace in the dir that we want to go
						trap_Trace( &trace, self->r.currentOrigin, mins, maxs, traceto, self->s.number, CONTENTS_SOLID|CONTENTS_MONSTERCLIP|CONTENTS_BOTCLIP );
						if ( trace.fraction >= 1.0f )
						{//it's clear, let's do it
							if ( allowWallFlips )
							{//okay to do wall-flips with this saber
								int parts;

								//FIXME: check for drops?
								//turn the cartwheel into a wallflip in the other dir
								if ( rightdot > 0 )
								{
									anim = BOTH_WALL_FLIP_LEFT;
									self->client->ps.velocity[0] = self->client->ps.velocity[1] = 0;
									VectorMA( self->client->ps.velocity, 150, right, self->client->ps.velocity );
								}
								else
								{
									anim = BOTH_WALL_FLIP_RIGHT;
									self->client->ps.velocity[0] = self->client->ps.velocity[1] = 0;
									VectorMA( self->client->ps.velocity, -150, right, self->client->ps.velocity );
								}
								self->client->ps.velocity[2] = forceJumpStrength[FORCE_LEVEL_2]/2.25f;
								//animate me
								parts = SETANIM_LEGS;
								if ( !self->client->ps.weaponTime )
								{
									parts = SETANIM_BOTH;
								}
								NPC_SetAnim( self, parts, anim, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD );
								self->client->ps.fd.forceJumpZStart = self->r.currentOrigin[2];//so we don't take damage if we land at same height
								//self->client->ps.pm_flags |= (PMF_JUMPING|PMF_SLOW_MO_FALL);
								G_SoundOnEnt( self, CHAN_BODY, "sound/weapons/force/jump.wav" );
								return EVASION_OTHER;
							}
						}
						else 
						{//boxed in on both sides
							if ( DotProduct( self->client->ps.velocity, fwd ) < 0 )
							{//moving backwards
								return EVASION_NONE;
							}
							if ( (trace.fraction*checkDist) <= 32 && (trace.fraction*checkDist) < bestCheckDist )
							{
								bestCheckDist = checkDist;
							}
						}
					}
					else
					{//too far from that wall to flip or run off it, check other side
						checkDist *= -1.0f;
						VectorMA( self->r.currentOrigin, checkDist, right, traceto );
						//trace in the dir that we want to go
						trap_Trace( &trace, self->r.currentOrigin, mins, maxs, traceto, self->s.number, CONTENTS_SOLID|CONTENTS_MONSTERCLIP|CONTENTS_BOTCLIP );
						if ( (trace.fraction*checkDist) <= 32 )
						{//wall on this side is close enough
							bestCheckDist = checkDist;
						}
						else
						{//neither side has a wall within 32
							return EVASION_NONE;
						}
					}
				}
				//Try wall run?
				if ( bestCheckDist )
				{//one of the walls was close enough to wall-run on
					qboolean allowWallRuns = qtrue;
					if ( self->client->ps.weapon == WP_SABER )
					{
						if ( self->client->saber[0].model
							&& self->client->saber[0].model[0] 
						&& (self->client->saber[0].saberFlags&SFL_NO_WALL_RUNS) )
						{
							allowWallRuns = qfalse;
						}
						else if ( self->client->saber[1].model
							&& self->client->saber[1].model[0]
						&& (self->client->saber[1].saberFlags&SFL_NO_WALL_RUNS) )
						{
							allowWallRuns = qfalse;
						}
					}
					if ( allowWallRuns )
					{//okay to do wallruns with this saber
						int parts;

						//FIXME: check for long enough wall and a drop at the end?
						if ( bestCheckDist > 0 )
						{//it was to the right
							anim = BOTH_WALL_RUN_RIGHT;
						}
						else
						{//it was to the left
							anim = BOTH_WALL_RUN_LEFT;
						}
						self->client->ps.velocity[2] = forceJumpStrength[FORCE_LEVEL_2]/2.25f;
						//animate me
						parts = SETANIM_LEGS;
						if ( !self->client->ps.weaponTime )
						{
							parts = SETANIM_BOTH;
						}
						NPC_SetAnim( self, parts, anim, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD );
						self->client->ps.fd.forceJumpZStart = self->r.currentOrigin[2];//so we don't take damage if we land at same height
						//self->client->ps.pm_flags |= (PMF_JUMPING|PMF_SLOW_MO_FALL);
						G_SoundOnEnt( self, CHAN_BODY, "sound/weapons/force/jump.wav" );
						return EVASION_OTHER;
					}
				}
				//else check for wall in front, do backflip off wall
			}
		}
	}
	return EVASION_NONE;
}

int DOM_Jedi_ReCalcParryTime( gentity_t *self, evasionType_t evasionType )
{
	if ( !self->client )
	{
		return 0;
	}
	if ( !self->s.number )
	{//player 
		return bg_parryDebounce[self->client->ps.fd.forcePowerLevel[FP_SABER_DEFENSE]];
	}
	else
	{
		if ( !g_saberRealisticCombat.integer && ( g_spskill.integer == 2 ) )
		{
			return Q_irand( 0, 150 );
		}
		else
		{
			int	baseTime;
			if ( evasionType == EVASION_DODGE )
			{
				baseTime = self->client->ps.torsoTimer;
			}
			else if ( evasionType == EVASION_CARTWHEEL )
			{
				baseTime = self->client->ps.torsoTimer;
			}
			else if ( self->client->ps.saberInFlight )
			{
				baseTime = Q_irand( 1, 3 ) * 50;
			}
			else
			{
				if ( g_saberRealisticCombat.integer )
				{
					baseTime = 500;

					switch ( g_spskill.integer )
					{
					case 0:
						baseTime = 500;
						break;
					case 1:
						baseTime = 300;
						break;
					case 2:
					default:
						baseTime = 100;
						break;
					}
				}
				else
				{
					baseTime = 150;//500;

					switch ( g_spskill.integer )
					{
					case 0:
						baseTime = 200;//500;
						break;
					case 1:
						baseTime = 100;//300;
						break;
					case 2:
					default:
						baseTime = 50;//100;
						break;
					}
				}

				baseTime = ceil(baseTime/2.0f);

				if ( evasionType == EVASION_DUCK || evasionType == EVASION_DUCK_PARRY )
				{
					baseTime += 100;
				}
				else if ( evasionType == EVASION_JUMP || evasionType == EVASION_JUMP_PARRY )
				{
					baseTime += 50;
				}
				else if ( evasionType == EVASION_OTHER )
				{
					baseTime += 100;
				}
				else if ( evasionType == EVASION_FJUMP )
				{
					baseTime += 100;
				}
			}

			return baseTime;
		}
	}
	return 0;
}

qboolean DOM_Jedi_QuickReactions( gentity_t *self )
{
	return qtrue;
}

qboolean DOM_Jedi_SaberBusy( gentity_t *self )
{
	if ( self->client->ps.torsoTimer > 300 
		&& ( (BG_SaberInAttack( self->client->ps.saberMove )&&self->client->ps.fd.saberAnimLevel==FORCE_LEVEL_3) 
		|| BG_SpinningSaberAnim( self->client->ps.torsoAnim ) 
		|| BG_SaberInSpecialAttack( self->client->ps.torsoAnim ) 
		//|| PM_SaberInBounce( self->client->ps.saberMove ) 
		|| PM_SaberInBrokenParry( self->client->ps.saberMove ) 
		//|| PM_SaberInDeflect( self->client->ps.saberMove ) 
		|| BG_FlippingAnim( self->client->ps.torsoAnim ) 
		|| PM_RollingAnim( self->client->ps.torsoAnim ) ) )
	{//my saber is not in a parrying position
		return qtrue;
	}
	return qfalse;
}

/*
-------------------------
Jedi_SaberBlock

Pick proper block anim

FIXME: Based on difficulty level/enemy saber combat skill, make this decision-making more/less effective

NOTE: always blocking projectiles in this func!

-------------------------
*/
extern qboolean G_FindClosestPointOnLineSegment( const vec3_t start, const vec3_t end, const vec3_t from, vec3_t result );
evasionType_t DOM_Jedi_SaberBlockGo( gentity_t *self, usercmd_t *cmd, vec3_t pHitloc, vec3_t phitDir, gentity_t *incoming, float dist ) //dist = 0.0f
{
	vec3_t hitloc, hitdir, diff, fwdangles={0,0,0}, right;
	float rightdot;
	float zdiff;
	int	  duckChance = 0;
	int	  dodgeAnim = -1;
	qboolean	saberBusy = qfalse, evaded = qfalse, doDodge = qfalse;
	evasionType_t	evasionType = EVASION_NONE;

	//FIXME: if we don't have our saber in hand, pick the force throw option or a jump or strafe!
	//FIXME: reborn don't block enough anymore
	if ( !incoming )
	{
		VectorCopy( pHitloc, hitloc );
		VectorCopy( phitDir, hitdir );
		//FIXME: maybe base this on rank some?  And/or g_spskill?
		if ( self->client->ps.saberInFlight )
		{//DOH!  do non-saber evasion!
			saberBusy = qtrue;
		}
		else if ( DOM_Jedi_QuickReactions( self ) )
		{//jedi trainer and tavion are must faster at parrying and can do it whenever they like
			//Also, on medium, all level 3 people can parry any time and on hard, all level 2 or 3 people can parry any time
		}
		else
		{
			saberBusy = DOM_Jedi_SaberBusy( self );
		}
	}
	else
	{	
		if ( incoming->s.weapon == WP_SABER )
		{//flying lightsaber, face it!
			//FIXME: for this to actually work, we'd need to call update angles too?
			//Jedi_FaceEntity( self, incoming, qtrue );
		}
		VectorCopy( incoming->r.currentOrigin, hitloc );
		VectorNormalize2( incoming->s.pos.trDelta, hitdir );
	}

	VectorSubtract( hitloc, self->client->renderInfo.eyePoint, diff );
	diff[2] = 0;
	//VectorNormalize( diff );
	fwdangles[1] = self->client->ps.viewangles[1];
	// Ultimately we might care if the shot was ahead or behind, but for now, just quadrant is fine.
	AngleVectors( fwdangles, NULL, right, NULL );

	rightdot = DotProduct(right, diff);// + flrand(-0.10f,0.10f);
	//totalHeight = self->client->renderInfo.eyePoint[2] - self->r.absmin[2];
	zdiff = hitloc[2] - self->client->renderInfo.eyePoint[2];// + Q_irand(-6,6);

	//see if we can dodge if need-be
	if ( (dist>16&&(Q_irand( 0, 2 )||saberBusy)) 
		|| self->client->ps.saberInFlight 
		|| BG_SabersOff( &self->client->ps ))
	{//either it will miss by a bit (and 25% chance) OR our saber is not in-hand OR saber is off
		//acrobat or fencer or above
		if ( self->client->ps.groundEntityNum != ENTITYNUM_NONE &&//on the ground
			!(self->client->ps.pm_flags&PMF_DUCKED)&&cmd->upmove>=0&&TIMER_Done( self, "duck" )//not ducking
			&& !BG_InRoll( &self->client->ps, self->client->ps.legsAnim )//not rolling
			&& !PM_InKnockDown( &self->client->ps )//not knocked down
			&& ( self->client->ps.saberInFlight ||
			(!BG_SaberInAttack( self->client->ps.saberMove )//not attacking
			&& !PM_SaberInStart( self->client->ps.saberMove )//not starting an attack
			&& !BG_SpinningSaberAnim( self->client->ps.torsoAnim )//not in a saber spin
			&& !BG_SaberInSpecialAttack( self->client->ps.torsoAnim ))//not in a special attack
			)
			)
		{//need to check all these because it overrides both torso and legs with the dodge
			doDodge = qtrue;
		}
	}
	// Figure out what quadrant the block was in.
	if ( d_JediAI.integer )
	{
		Com_Printf( "(%d) evading attack from height %4.2f, zdiff: %4.2f, rightdot: %4.2f\n", level.time, hitloc[2]-self->r.absmin[2],zdiff,rightdot);
	}

	//UL = > -1//-6
	//UR = > -6//-9
	//TOP = > +6//+4
	//FIXME: take FP_SABER_DEFENSE into account here somehow?
	if ( zdiff >= -5 )//was 0
	{
		if ( incoming || !saberBusy )
		{
			if ( rightdot > 12 
				|| (rightdot > 3 && zdiff < 5) 
				|| (!incoming&&fabs(hitdir[2])<0.25f) )//was normalized, 0.3
			{//coming from right
				if ( doDodge )
				{
					if ( Q_irand( 0, 1 ) )
					{
						dodgeAnim = BOTH_DODGE_FL;
					}
					else
					{
						dodgeAnim = BOTH_DODGE_BL;
					}
				}
				else
				{
					self->client->ps.saberBlocked = BLOCKED_UPPER_RIGHT;
					evasionType = EVASION_PARRY;
					if ( self->client->ps.groundEntityNum != ENTITYNUM_NONE )
					{
						if ( zdiff > 5 )
						{
							TIMER_Start( self, "duck", Q_irand( 500, 1500 ) );
							evasionType = EVASION_DUCK_PARRY;
							evaded = qtrue;
							if ( d_JediAI.integer )
							{
								Com_Printf( "duck " );
							}
						}
						else
						{
							duckChance = 6;
						}
					}
				}
				if ( d_JediAI.integer )
				{
					Com_Printf( "UR block\n" );
				}
			}
			else if ( rightdot < -12 
				|| (rightdot < -3 && zdiff < 5) 
				|| (!incoming&&fabs(hitdir[2])<0.25f) )//was normalized, -0.3
			{//coming from left
				if ( doDodge )
				{
					if ( Q_irand( 0, 1 ) )
					{
						dodgeAnim = BOTH_DODGE_FR;
					}
					else
					{
						dodgeAnim = BOTH_DODGE_BR;
					}
				}
				else
				{
					self->client->ps.saberBlocked = BLOCKED_UPPER_LEFT;
					evasionType = EVASION_PARRY;
					if ( self->client->ps.groundEntityNum != ENTITYNUM_NONE )
					{
						if ( zdiff > 5 )
						{
							TIMER_Start( self, "duck", Q_irand( 500, 1500 ) );
							evasionType = EVASION_DUCK_PARRY;
							evaded = qtrue;
							if ( d_JediAI.integer )
							{
								Com_Printf( "duck " );
							}
						}
						else
						{
							duckChance = 6;
						}
					}
				}
				if ( d_JediAI.integer )
				{
					Com_Printf( "UL block\n" );
				}
			}
			else
			{
				self->client->ps.saberBlocked = BLOCKED_TOP;
				evasionType = EVASION_PARRY;
				if ( self->client->ps.groundEntityNum != ENTITYNUM_NONE )
				{
					duckChance = 4;
				}
				if ( d_JediAI.integer )
				{
					Com_Printf( "TOP block\n" );
				}
			}
			evaded = qtrue;
		}
		else
		{
			if ( self->client->ps.groundEntityNum != ENTITYNUM_NONE )
			{
				//duckChance = 2;
				TIMER_Start( self, "duck", Q_irand( 500, 1500 ) );
				evasionType = EVASION_DUCK;
				evaded = qtrue;
				if ( d_JediAI.integer )
				{
					Com_Printf( "duck " );
				}
			}
		}
	}
	//LL = -22//= -18 to -39
	//LR = -23//= -20 to -41
	else if ( zdiff > -22 )//was-15 )
	{
		if ( 1 )//zdiff < -10 )
		{//hmm, pretty low, but not low enough to use the low block, so we need to duck
			if ( self->client->ps.groundEntityNum != ENTITYNUM_NONE )
			{
				//duckChance = 2;
				TIMER_Start( self, "duck", Q_irand( 500, 1500 ) );
				evasionType = EVASION_DUCK;
				evaded = qtrue;
				if ( d_JediAI.integer )
				{
					Com_Printf( "duck " );
				}
			}
			else
			{//in air!  Ducking does no good
			}
		}
		if ( incoming || !saberBusy )
		{
			if ( rightdot > 8 || (rightdot > 3 && zdiff < -11) )//was normalized, 0.2
			{
				if ( doDodge )
				{
					dodgeAnim = BOTH_DODGE_L;
				}
				else
				{
					self->client->ps.saberBlocked = BLOCKED_UPPER_RIGHT;
					if ( evasionType == EVASION_DUCK )
					{
						evasionType = EVASION_DUCK_PARRY;
					}
					else
					{
						evasionType = EVASION_PARRY;
					}
				}
				if ( d_JediAI.integer )
				{
					Com_Printf( "mid-UR block\n" );
				}
			}
			else if ( rightdot < -8 || (rightdot < -3 && zdiff < -11) )//was normalized, -0.2
			{
				if ( doDodge )
				{
					dodgeAnim = BOTH_DODGE_R;
				}
				else
				{
					self->client->ps.saberBlocked = BLOCKED_UPPER_LEFT;
					if ( evasionType == EVASION_DUCK )
					{
						evasionType = EVASION_DUCK_PARRY;
					}
					else
					{
						evasionType = EVASION_PARRY;
					}
				}
				if ( d_JediAI.integer )
				{
					Com_Printf( "mid-UL block\n" );
				}
			}
			else
			{
				self->client->ps.saberBlocked = BLOCKED_TOP;
				if ( evasionType == EVASION_DUCK )
				{
					evasionType = EVASION_DUCK_PARRY;
				}
				else
				{
					evasionType = EVASION_PARRY;
				}
				if ( d_JediAI.integer )
				{
					Com_Printf( "mid-TOP block\n" );
				}
			}
			evaded = qtrue;
		}
	}
	else if ( saberBusy || (zdiff < -36 && ( zdiff < -44 || !Q_irand( 0, 2 ) ) ) )//was -30 and -40//2nd one was -46
	{//jump!
		if ( self->client->ps.groundEntityNum == ENTITYNUM_NONE )
		{//already in air, duck to pull up legs
			TIMER_Start( self, "duck", Q_irand( 500, 1500 ) );
			evasionType = EVASION_DUCK;
			evaded = qtrue;
			if ( d_JediAI.integer )
			{
				Com_Printf( "legs up\n" );
			}
			if ( incoming || !saberBusy )
			{
				//since the jump may be cleared if not safe, set a lower block too
				if ( rightdot >= 0 )
				{
					self->client->ps.saberBlocked = BLOCKED_LOWER_RIGHT;
					evasionType = EVASION_DUCK_PARRY;
					if ( d_JediAI.integer )
					{
						Com_Printf( "LR block\n" );
					}
				}
				else
				{
					self->client->ps.saberBlocked = BLOCKED_LOWER_LEFT;
					evasionType = EVASION_DUCK_PARRY;
					if ( d_JediAI.integer )
					{
						Com_Printf( "LL block\n" );
					}
				}
				evaded = qtrue;
			}
		}
		else 
		{//gotta jump!
			if ( (!Q_irand( 0, 10 ) || (!Q_irand( 0, 2 ) && (cmd->forwardmove || cmd->rightmove))) )
			{//superjump
				//FIXME: check the jump, if can't, then block
				if ( self->client->ps.fd.forceRageRecoveryTime < level.time
					&& !(self->client->ps.fd.forcePowersActive&(1<<FP_RAGE)) 
					&& !PM_InKnockDown( &self->client->ps ) )
				{
					self->client->ps.fd.forceJumpCharge = 320;//FIXME: calc this intelligently
					evasionType = EVASION_FJUMP;
					evaded = qtrue;
					if ( d_JediAI.integer )
					{
						Com_Printf( "force jump + " );
					}
				}
			}
			else
			{//normal jump
				//FIXME: check the jump, if can't, then block
				if ( self->client->ps.fd.forceRageRecoveryTime < level.time
					&& !(self->client->ps.fd.forcePowersActive&(1<<FP_RAGE)) )
				{
					//cmd->upmove = 127;
					self->client->ps.velocity[2] = /*JUMP_VELOCITY*/ bgConstants.baseJumpVelocity;
				}
				evasionType = EVASION_JUMP;
				evaded = qtrue;
				if ( d_JediAI.integer )
				{
					Com_Printf( "jump + " );
				}
			}

			if ( !incoming 
				&& self->client->ps.groundEntityNum < ENTITYNUM_NONE 
				&& !Q_irand( 0, 2 ) )
			{
				if ( !BG_SaberInAttack( self->client->ps.saberMove )
					&& !PM_SaberInStart( self->client->ps.saberMove ) 
					&& !BG_InRoll( &self->client->ps, self->client->ps.legsAnim )
					&& !PM_InKnockDown( &self->client->ps )
					&& !BG_SaberInSpecialAttack( self->client->ps.torsoAnim ) )
				{//do the butterfly!
					int butterflyAnim;
					if ( Q_irand( 0, 1 ) )
					{
						butterflyAnim = BOTH_BUTTERFLY_LEFT;
					}
					else
					{
						butterflyAnim = BOTH_BUTTERFLY_RIGHT;
					}
					evasionType = EVASION_CARTWHEEL;
					NPC_SetAnim( self, SETANIM_BOTH, butterflyAnim, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD );
					self->client->ps.velocity[2] = 225;
					self->client->ps.fd.forceJumpZStart = self->r.currentOrigin[2];//so we don't take damage if we land at same height
					//	self->client->ps.pm_flags |= PMF_JUMPING|PMF_SLOW_MO_FALL;
					//	self->client->ps.SaberActivateTrail( 300 );//FIXME: reset this when done!
					//Ah well. No hacking from the server for now.
					G_Sound( self, CHAN_BODY, G_SoundIndex("sound/weapons/force/jump.wav") );
					cmd->upmove = 0;
					saberBusy = qtrue;
					evaded = qtrue;
				}
			}
		}
		if ( ((evasionType = DOM_Jedi_CheckFlipEvasions( self, rightdot, zdiff ))!=EVASION_NONE) )
		{
			saberBusy = qtrue;
			evaded = qtrue;
		}
		else if ( incoming || !saberBusy )
		{
			//since the jump may be cleared if not safe, set a lower block too
			if ( rightdot >= 0 )
			{
				self->client->ps.saberBlocked = BLOCKED_LOWER_RIGHT;
				if ( evasionType == EVASION_JUMP )
				{
					evasionType = EVASION_JUMP_PARRY;
				}
				else if ( evasionType == EVASION_NONE )
				{
					evasionType = EVASION_PARRY;
				}
				if ( d_JediAI.integer )
				{
					Com_Printf( "LR block\n" );
				}
			}
			else
			{
				self->client->ps.saberBlocked = BLOCKED_LOWER_LEFT;
				if ( evasionType == EVASION_JUMP )
				{
					evasionType = EVASION_JUMP_PARRY;
				}
				else if ( evasionType == EVASION_NONE )
				{
					evasionType = EVASION_PARRY;
				}
				if ( d_JediAI.integer )
				{
					Com_Printf( "LL block\n" );
				}
			}
			evaded = qtrue;
		}
	}
	else
	{
		if ( incoming || !saberBusy )
		{
			if ( rightdot >= 0 )
			{
				self->client->ps.saberBlocked = BLOCKED_LOWER_RIGHT;
				evasionType = EVASION_PARRY;
				if ( d_JediAI.integer )
				{
					Com_Printf( "LR block\n" );
				}
			}
			else
			{
				self->client->ps.saberBlocked = BLOCKED_LOWER_LEFT;
				evasionType = EVASION_PARRY;
				if ( d_JediAI.integer )
				{
					Com_Printf( "LL block\n" );
				}
			}
			if ( incoming && incoming->s.weapon == WP_SABER )
			{//thrown saber!
				if ( (!Q_irand( 0, 10 ) || (!Q_irand( 0, 2 ) && (cmd->forwardmove || cmd->rightmove))) )
				{//superjump
					//FIXME: check the jump, if can't, then block
					if ( self->client->ps.fd.forceRageRecoveryTime < level.time
						&& !(self->client->ps.fd.forcePowersActive&(1<<FP_RAGE)) 
						&& !PM_InKnockDown( &self->client->ps ) )
					{
						self->client->ps.fd.forceJumpCharge = 320;//FIXME: calc this intelligently
						evasionType = EVASION_FJUMP;
						if ( d_JediAI.integer )
						{
							Com_Printf( "force jump + " );
						}
					}
				}
				else
				{//normal jump
					//FIXME: check the jump, if can't, then block
					if ( self->client->ps.fd.forceRageRecoveryTime < level.time
						&& !(self->client->ps.fd.forcePowersActive&(1<<FP_RAGE)))
					{
						if ( self == NPC )
						{
							cmd->upmove = 127;
						}
						else
						{
							self->client->ps.velocity[2] = /*JUMP_VELOCITY*/ bgConstants.baseJumpVelocity;
						}
						evasionType = EVASION_JUMP_PARRY;
						if ( d_JediAI.integer )
						{
							Com_Printf( "jump + " );
						}
					}
				}
			}
			evaded = qtrue;
		}
	}

	if ( evasionType == EVASION_NONE )
	{
		return EVASION_NONE;
	}
	//stop taunting
	TIMER_Set( self, "taunting", 0 );
	//stop gripping
	TIMER_Set( self, "gripping", -level.time );
	WP_ForcePowerStop( self, FP_GRIP );
	//stop draining
	TIMER_Set( self, "draining", -level.time );
	WP_ForcePowerStop( self, FP_DRAIN );

	if ( dodgeAnim != -1 )
	{//dodged
		evasionType = EVASION_DODGE;
		NPC_SetAnim( self, SETANIM_BOTH, dodgeAnim, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD );
		self->client->ps.weaponTime = self->client->ps.torsoTimer;
		//force them to stop moving in this case
		self->client->ps.pm_time = self->client->ps.torsoTimer;
		//FIXME: maybe make a sound?  Like a grunt?  EV_JUMP?
		self->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
		//dodged, not block
	}
	else
	{
		if ( duckChance )
		{
			if ( !Q_irand( 0, duckChance ) )
			{
				TIMER_Start( self, "duck", Q_irand( 500, 1500 ) );
				if ( evasionType == EVASION_PARRY )
				{
					evasionType = EVASION_DUCK_PARRY;
				}
				else
				{
					evasionType = EVASION_DUCK;
				}
				/*
				if ( d_JediAI.integer )
				{
				Com_Printf( "duck " );
				}
				*/
			}
		}

		if ( incoming )
		{
			self->client->ps.saberBlocked = WP_MissileBlockForBlock( self->client->ps.saberBlocked );
		}

	}
	//if ( self->client->ps.saberBlocked != BLOCKED_NONE )
	{
		int parryReCalcTime = DOM_Jedi_ReCalcParryTime( self, evasionType );
		if ( self->client->ps.fd.forcePowerDebounce[FP_SABER_DEFENSE] < level.time + parryReCalcTime )
		{
			self->client->ps.fd.forcePowerDebounce[FP_SABER_DEFENSE] = level.time + parryReCalcTime;
		}
	}
	return evasionType;
}

extern float ShortestLineSegBewteen2LineSegs( vec3_t start1, vec3_t end1, vec3_t start2, vec3_t end2, vec3_t close_pnt1, vec3_t close_pnt2 );
extern int WPDEBUG_SaberColor( saber_colors_t saberColor );
static qboolean DOM_Jedi_SaberBlock( int saberNum, int bladeNum ) //saberNum = 0, bladeNum = 0
{
	vec3_t hitloc, saberTipOld, saberTip, top, bottom, axisPoint, saberPoint, dir;//saberBase, 
	vec3_t pointDir, baseDir, tipDir, saberHitPoint, saberMins, saberMaxs;
	float	pointDist, baseDirPerc, dist;
	float	bladeLen = 0;
	trace_t	tr;
	evasionType_t	evasionType;

	if (NPC->client->ps.weapon != WP_SABER) return qfalse; // UQ1: If they don't have a saber then they can't saber block... :)

	//FIXME: reborn don't block enough anymore
	/*
	//maybe do this on easy only... or only on grunt-level reborn
	if ( NPC->client->ps.weaponTime )
	{//i'm attacking right now
	return qfalse;
	}
	*/

	if ( !TIMER_Done( NPC, "parryReCalcTime" ) )
	{//can't do our own re-think of which parry to use yet
		return qfalse;
	}

	if ( NPC->client->ps.fd.forcePowerDebounce[FP_SABER_DEFENSE] > level.time )
	{//can't move the saber to another position yet
		return qfalse;
	}

	if ( NPC->enemy->health <= 0 || !NPC->enemy->client )
	{//don't keep blocking him once he's dead (or if not a client)
		return qfalse;
	}
	/*
	//VectorMA( NPC->enemy->client->renderInfo.muzzlePoint, NPC->enemy->client->ps.saberLength, NPC->enemy->client->renderInfo.muzzleDir, saberTip );
	//VectorMA( NPC->enemy->client->renderInfo.muzzlePointNext, NPC->enemy->client->ps.saberLength, NPC->enemy->client->renderInfo.muzzleDirNext, saberTipNext );
	VectorMA( NPC->enemy->client->renderInfo.muzzlePointOld, NPC->enemy->client->ps.saberLength, NPC->enemy->client->renderInfo.muzzleDirOld, saberTipOld );
	VectorMA( NPC->enemy->client->renderInfo.muzzlePoint, NPC->enemy->client->ps.saberLength, NPC->enemy->client->renderInfo.muzzleDir, saberTip );

	VectorSubtract( NPC->enemy->client->renderInfo.muzzlePoint, NPC->enemy->client->renderInfo.muzzlePointOld, dir );//get the dir
	VectorAdd( dir, NPC->enemy->client->renderInfo.muzzlePoint, saberBase );//extrapolate

	VectorSubtract( saberTip, saberTipOld, dir );//get the dir
	VectorAdd( dir, saberTip, saberTipOld );//extrapolate

	VectorCopy( NPC->r.currentOrigin, top );
	top[2] = NPC->r.absmax[2];
	VectorCopy( NPC->r.currentOrigin, bottom );
	bottom[2] = NPC->r.absmin[2];

	float dist = ShortestLineSegBewteen2LineSegs( saberBase, saberTipOld, bottom, top, saberPoint, axisPoint );
	if ( 0 )//dist > NPC->r.maxs[0]*4 )//was *3
	{//FIXME: sometimes he reacts when you're too far away to actually hit him
	if ( d_JediAI.integer )
	{
	Com_Printf( "enemy saber dist: %4.2f\n", dist );
	}
	TIMER_Set( NPC, "parryTime", -1 );
	return qfalse;
	}

	//get the actual point of impact
	trace_t	tr;
	trap_Trace( &tr, saberPoint, vec3_origin, vec3_origin, axisPoint, NPC->enemy->s.number, MASK_SHOT, G2_RETURNONHIT, 10 );
	if ( tr.allsolid || tr.startsolid )
	{//estimate
	VectorSubtract( saberPoint, axisPoint, dir );
	VectorNormalize( dir );
	VectorMA( axisPoint, NPC->r.maxs[0]*1.22, dir, hitloc );
	}
	else
	{
	VectorCopy( tr.endpos, hitloc );
	}
	*/
	VectorSet(saberMins,-4,-4,-4);
	VectorSet(saberMaxs,4,4,4);

	VectorMA( NPC->enemy->client->saber[saberNum].blade[bladeNum].muzzlePointOld, NPC->enemy->client->saber[saberNum].blade[bladeNum].length, NPC->enemy->client->saber[saberNum].blade[bladeNum].muzzleDirOld, saberTipOld );
	VectorMA( NPC->enemy->client->saber[saberNum].blade[bladeNum].muzzlePoint, NPC->enemy->client->saber[saberNum].blade[bladeNum].length, NPC->enemy->client->saber[saberNum].blade[bladeNum].muzzleDir, saberTip );
	//	VectorCopy(NPC->enemy->client->lastSaberBase_Always, muzzlePoint);
	//	VectorMA(muzzlePoint, GAME_SABER_LENGTH, NPC->enemy->client->lastSaberDir_Always, saberTip);
	//	VectorCopy(saberTip, saberTipOld);

	VectorCopy( NPC->r.currentOrigin, top );
	top[2] = NPC->r.absmax[2];
	VectorCopy( NPC->r.currentOrigin, bottom );
	bottom[2] = NPC->r.absmin[2];

	dist = ShortestLineSegBewteen2LineSegs( NPC->enemy->client->renderInfo.muzzlePoint, saberTip, bottom, top, saberPoint, axisPoint );
	if ( dist > NPC->r.maxs[0]*5 )//was *3
	{//FIXME: sometimes he reacts when you're too far away to actually hit him
		if ( d_JediAI.integer )
		{
			Com_Printf( S_COLOR_RED"enemy saber dist: %4.2f\n", dist );
		}
		/*
		if ( dist < 300 //close
		&& !Jedi_QuickReactions( NPC )//quick reaction people can interrupt themselves
		&& (PM_SaberInStart( NPC->enemy->client->ps.saberMove ) || BG_SaberInAttack( NPC->enemy->client->ps.saberMove )) )//enemy is swinging at me
		{//he's swinging at me and close enough to be a threat, don't start an attack right now
		TIMER_Set( NPC, "parryTime", 100 );
		}
		else
		*/
		{
			TIMER_Set( NPC, "parryTime", -1 );
		}
		return qfalse;
	}
	if ( d_JediAI.integer )
	{
		Com_Printf( S_COLOR_GREEN"enemy saber dist: %4.2f\n", dist );
	}

	VectorSubtract( saberPoint, NPC->enemy->client->renderInfo.muzzlePoint, pointDir );
	pointDist = VectorLength( pointDir );

	bladeLen = NPC->enemy->client->saber[saberNum].blade[bladeNum].length;

	if ( bladeLen <= 0 )
	{
		baseDirPerc = 0.5f;
	}
	else
	{
		baseDirPerc = pointDist/bladeLen;
	}
	VectorSubtract( NPC->enemy->client->renderInfo.muzzlePoint, NPC->enemy->client->renderInfo.muzzlePointOld, baseDir );
	VectorSubtract( saberTip, saberTipOld, tipDir );
	VectorScale( baseDir, baseDirPerc, baseDir );
	VectorMA( baseDir, 1.0f-baseDirPerc, tipDir, dir );
	VectorMA( saberPoint, 200, dir, hitloc );

	//get the actual point of impact
	trap_Trace( &tr, saberPoint, saberMins, saberMaxs, hitloc, NPC->enemy->s.number, CONTENTS_BODY );//, G2_RETURNONHIT, 10 );
	if ( tr.allsolid || tr.startsolid || tr.fraction >= 1.0f )
	{//estimate
		vec3_t	dir2Me;
		VectorSubtract( axisPoint, saberPoint, dir2Me );
		dist = VectorNormalize( dir2Me );
		if ( DotProduct( dir, dir2Me ) < 0.2f )
		{//saber is not swinging in my direction
			/*
			if ( dist < 300 //close
			&& !Jedi_QuickReactions( NPC )//quick reaction people can interrupt themselves
			&& (PM_SaberInStart( NPC->enemy->client->ps.saberMove ) || BG_SaberInAttack( NPC->enemy->client->ps.saberMove )) )//enemy is swinging at me
			{//he's swinging at me and close enough to be a threat, don't start an attack right now
			TIMER_Set( NPC, "parryTime", 100 );
			}
			else
			*/
			{
				TIMER_Set( NPC, "parryTime", -1 );
			}
			return qfalse;
		}
		ShortestLineSegBewteen2LineSegs( saberPoint, hitloc, bottom, top, saberHitPoint, hitloc );
		/*
		VectorSubtract( saberPoint, axisPoint, dir );
		VectorNormalize( dir );
		VectorMA( axisPoint, NPC->r.maxs[0]*1.22, dir, hitloc );
		*/
	}
	else
	{
		VectorCopy( tr.endpos, hitloc );
	}

	if ( d_JediAI.integer )
	{
		//G_DebugLine( saberPoint, hitloc, FRAMETIME, WPDEBUG_SaberColor( NPC->enemy->client->ps.saber[saberNum].blade[bladeNum].color ), qtrue );
		G_TestLine(saberPoint, hitloc, 0x0000ff, FRAMETIME);
	}

	//FIXME: if saber is off and/or we have force speed and want to be really cocky, 
	//		and the swing misses by some amount, we can use the dodges here... :)
	if ( (evasionType=DOM_Jedi_SaberBlockGo( NPC, &ucmd, hitloc, dir, NULL, dist )) != EVASION_DODGE )
	{//we did block (not dodge)
		int parryReCalcTime;

		if ( !NPC->client->ps.saberInFlight )
		{//make sure saber is on
			WP_ActivateSaber(NPC);
		}

		//debounce our parry recalc time
		parryReCalcTime = DOM_Jedi_ReCalcParryTime( NPC, evasionType );
		TIMER_Set( NPC, "parryReCalcTime", Q_irand( 0, parryReCalcTime ) );
		if ( d_JediAI.integer )
		{
			Com_Printf( "Keep parry choice until: %d\n", level.time + parryReCalcTime );
		}

		//determine how long to hold this anim
		if ( TIMER_Done( NPC, "parryTime" ) )
		{
			TIMER_Set( NPC, "parryTime", Q_irand( parryReCalcTime/2, parryReCalcTime*1.5 ) );
		}
	}
	else
	{
		int dodgeTime = NPC->client->ps.torsoTimer;
		dodgeTime -= 200;
		TIMER_Set( NPC, "parryReCalcTime", dodgeTime );
		TIMER_Set( NPC, "parryTime", dodgeTime );
	}
	return qtrue;
}

static qboolean DOM_Jedi_Strafe( int strafeTimeMin, int strafeTimeMax, int nextStrafeTimeMin, int nextStrafeTimeMax, qboolean walking )
{
	if ( NPC->client->ps.saberEventFlags&SEF_LOCK_WON && NPC->enemy && NPC->enemy->painDebounceTime > level.time )
	{//don't strafe if pressing the advantage of winning a saberLock
		return qfalse;
	}
	if ( TIMER_Done( NPC, "strafeLeft" ) && TIMER_Done( NPC, "strafeRight" ) )
	{
		qboolean strafed = qfalse;
		//TODO: make left/right choice a tactical decision rather than random:
		//		try to keep own back away from walls and ledges, 
		//		try to keep enemy's back to a ledge or wall
		//		Maybe try to strafe toward designer-placed "safe spots" or "goals"?
		int	strafeTime = Q_irand( strafeTimeMin, strafeTimeMax );

		if ( Q_irand( 0, 1 ) )
		{
			if ( DOM_MoveDirClear( ucmd.forwardmove, -127, qfalse ) )
			{
				TIMER_Set( NPC, "strafeLeft", strafeTime );
				strafed = qtrue;
			}
			else if ( DOM_MoveDirClear( ucmd.forwardmove, 127, qfalse ) )
			{
				TIMER_Set( NPC, "strafeRight", strafeTime );
				strafed = qtrue;
			}
		}
		else
		{
			if ( DOM_MoveDirClear( ucmd.forwardmove, 127, qfalse  ) )
			{
				TIMER_Set( NPC, "strafeRight", strafeTime );
				strafed = qtrue;
			}
			else if ( DOM_MoveDirClear( ucmd.forwardmove, -127, qfalse  ) )
			{
				TIMER_Set( NPC, "strafeLeft", strafeTime );
				strafed = qtrue;
			}
		}

		if ( strafed )
		{
			TIMER_Set( NPC, "noStrafe", strafeTime + Q_irand( nextStrafeTimeMin, nextStrafeTimeMax ) );
			if ( walking )
			{//should be a slow strafe
				TIMER_Set( NPC, "walking", strafeTime );
			}
			return qtrue;
		}
	}
	return qfalse;
}

/*
-------------------------
Jedi_EvasionSaber

defend if other is using saber and attacking me!
-------------------------
*/
void DOM_Jedi_EvasionSaber( vec3_t enemy_movedir, float enemy_dist, vec3_t enemy_dir )
{
	vec3_t	dirEnemy2Me;
	int		evasionChance = 30;//only step aside 30% if he's moving at me but not attacking
	qboolean	enemy_attacking = qfalse;
	qboolean	throwing_saber = qfalse;
	qboolean	shooting_lightning = qfalse;

	if ( !NPC->enemy || !NPC->enemy->client )
	{
		return;
	}
	else if ( NPC->enemy->client 
		&& NPC->enemy->s.weapon == WP_SABER 
		&& NPC->enemy->client->ps.saberLockTime > level.time )
	{//don't try to block/evade an enemy who is in a saberLock
		return;
	}
	else if ( NPC->client->ps.saberEventFlags&SEF_LOCK_WON && NPC->enemy->painDebounceTime > level.time )
	{//pressing the advantage of winning a saber lock
		return;
	}

	if ( NPC->enemy->client->ps.saberInFlight && !TIMER_Done( NPC, "taunting" ) )
	{//if he's throwing his saber, stop taunting
		TIMER_Set( NPC, "taunting", -level.time );
		if ( !NPC->client->ps.saberInFlight )
		{
			WP_ActivateSaber(NPC);
		}
	}

	if ( TIMER_Done( NPC, "parryTime" ) )
	{
		if ( NPC->client->ps.saberBlocked != BLOCKED_ATK_BOUNCE &&
			NPC->client->ps.saberBlocked != BLOCKED_PARRY_BROKEN )
		{//wasn't blocked myself
			NPC->client->ps.saberBlocked = BLOCKED_NONE;
		}
	}

	if ( NPC->enemy->client->ps.weaponTime && NPC->enemy->client->ps.weaponstate == WEAPON_FIRING )
	{
		if ( !NPC->client->ps.saberInFlight && DOM_Jedi_SaberBlock(0, 0) )
		{
			return;
		}
	}

	VectorSubtract( NPC->r.currentOrigin, NPC->enemy->r.currentOrigin, dirEnemy2Me );
	VectorNormalize( dirEnemy2Me );

	if ( NPC->enemy->client->ps.weaponTime && NPC->enemy->client->ps.weaponstate == WEAPON_FIRING )
	{//enemy is attacking
		enemy_attacking = qtrue;
		evasionChance = 90;
	}

	if ( (NPC->enemy->client->ps.fd.forcePowersActive&(1<<FP_LIGHTNING) ) )
	{//enemy is shooting lightning
		enemy_attacking = qtrue;
		shooting_lightning = qtrue;
		evasionChance = 50;
	}

	if ( NPC->enemy->client->ps.saberInFlight && NPC->enemy->client->ps.saberEntityNum != ENTITYNUM_NONE && NPC->enemy->client->ps.saberEntityState != SES_RETURNING )
	{//enemy is shooting lightning
		enemy_attacking = qtrue;
		throwing_saber = qtrue;
	}

	//FIXME: this needs to take skill and rank(reborn type) into account much more
	if ( Q_irand( 0, 100 ) < evasionChance )
	{//check to see if he's coming at me
		float facingAmt;
		if ( VectorCompare( enemy_movedir, vec3_origin ) || shooting_lightning || throwing_saber )
		{//he's not moving (or he's using a ranged attack), see if he's facing me
			vec3_t	enemy_fwd;
			AngleVectors( NPC->enemy->client->ps.viewangles, enemy_fwd, NULL, NULL );
			facingAmt = DotProduct( enemy_fwd, dirEnemy2Me );
		}
		else
		{//he's moving
			facingAmt = DotProduct( enemy_movedir, dirEnemy2Me );
		}

		if ( flrand( 0.25, 1 ) < facingAmt )
		{//coming at/facing me!  
			int whichDefense = 0;
			if ( NPC->client->ps.weaponTime || NPC->client->ps.saberInFlight )
			{//I'm attacking or recovering from a parry, can only try to strafe/jump right now
				whichDefense = 100;
			}
			else
			{
				if ( shooting_lightning )
				{//check for lightning attack
					//only valid defense is strafe and/or jump
					whichDefense = 100;
				}
				else if ( throwing_saber )
				{//he's thrown his saber!  See if it's coming at me
					float	saberDist;
					vec3_t	saberDir2Me;
					vec3_t	saberMoveDir;
					gentity_t *saber = &g_entities[NPC->enemy->client->ps.saberEntityNum];
					VectorSubtract( NPC->r.currentOrigin, saber->r.currentOrigin, saberDir2Me );
					saberDist = VectorNormalize( saberDir2Me );
					VectorCopy( saber->s.pos.trDelta, saberMoveDir );
					VectorNormalize( saberMoveDir );
					if ( DotProduct( saberMoveDir, saberDir2Me ) > 0.5 )
					{//it's heading towards me
						if ( saberDist < 100 )
						{//it's close
							whichDefense = Q_irand( 3, 6 );
						}
						else if ( saberDist < 200 )
						{//got some time, yet, try pushing
							whichDefense = Q_irand( 0, 8 );
						}
					}
				}
				if ( whichDefense )
				{//already chose one
				}
				else if ( enemy_dist > 80 || !enemy_attacking )
				{//he's pretty far, or not swinging, just strafe
					if ( VectorCompare( enemy_movedir, vec3_origin ) )
					{//if he's not moving, not swinging and far enough away, no evasion necc.
						return;
					}
					whichDefense = 100;
				}
				else
				{//he's getting close and swinging at me
					vec3_t	fwd;
					//see if I'm facing him
					AngleVectors( NPC->client->ps.viewangles, fwd, NULL, NULL );
					if ( DotProduct( enemy_dir, fwd ) < 0.5 )
					{//I'm not really facing him, best option is to strafe
						whichDefense = Q_irand( 5, 16 );
					}
					else if ( enemy_dist < 56 )
					{//he's very close, maybe we should be more inclined to block or throw
						whichDefense = Q_irand( 2, 12 );
					}
					else
					{
						whichDefense = Q_irand( 2, 16 );
					}
				}
			}

			if ( whichDefense >= 4 && whichDefense <= 12 )
			{//would try to block
				if ( NPC->client->ps.saberInFlight )
				{//can't, saber in not in hand, so fall back to strafe/jump
					whichDefense = 100;
				}
			}

			switch( whichDefense )
			{
			case 0:
			case 1:
			case 2:
			case 3:
				//use jedi force push?
				//FIXME: try to do this if health low or enemy back to a cliff?
				if ( TIMER_Done( NPC, "parryTime" ) )
				{//FIXME: check forcePushRadius[NPC->client->ps.fd.forcePowerLevel[FP_PUSH]]
					ForceThrow( NPC, qfalse );
				}
				break;
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
			case 11:
			case 12:
				//try to parry the blow
				//Com_Printf( "blocking\n" );
				DOM_Jedi_SaberBlock(0, 0);
				break;
			default:
				//Evade!
				//start a strafe left/right if not already
				if ( !Q_irand( 0, 5 ) || !DOM_Jedi_Strafe( 300, 1000, 0, 1000, qfalse ) )
				{//certain chance they will pick an alternative evasion
					//if couldn't strafe, try a different kind of evasion...
					if ( shooting_lightning || throwing_saber || enemy_dist < 80 )
					{
						//FIXME: force-jump+forward - jump over the guy!
						if ( shooting_lightning || (!Q_irand( 0, 2 ) && TIMER_Done( NPC, "parryTime" ) ) )
						{
							if ( !shooting_lightning && Q_irand( 0, 2 ) )
							{//FIXME: check forcePushRadius[NPC->client->ps.fd.forcePowerLevel[FP_PUSH]]
								ForceThrow( NPC, qfalse );
							}
							else if ( NPC->client->ps.fd.forceRageRecoveryTime < level.time
								&& !(NPC->client->ps.fd.forcePowersActive&(1<<FP_RAGE)) 
								&& !PM_InKnockDown( &NPC->client->ps ) )
							{//FIXME: make this a function call?
								//FIXME: check for clearance, safety of landing spot?
								NPC->client->ps.fd.forceJumpCharge = 480;
								//Don't jump again for another 2 to 5 seconds
								TIMER_Set( NPC, "jumpChaseDebounce", Q_irand( 2000, 5000 ) );
								if ( Q_irand( 0, 2 ) )
								{
									ucmd.forwardmove = 127;
									VectorClear( NPC->client->ps.moveDir );
								}
								else
								{
									ucmd.forwardmove = -127;
									VectorClear( NPC->client->ps.moveDir );
								}
								//FIXME: if this jump is cleared, we can't block... so pick a random lower block?
								if ( Q_irand( 0, 1 ) )//FIXME: make intelligent
								{
									NPC->client->ps.saberBlocked = BLOCKED_LOWER_RIGHT;
								}
								else
								{
									NPC->client->ps.saberBlocked = BLOCKED_LOWER_LEFT;
								}
							}
						}
						else if ( enemy_attacking )
						{
							DOM_Jedi_SaberBlock(0, 0);
						}
					}
				}
				else
				{//strafed
					if ( d_JediAI.integer )
					{
						Com_Printf( "def strafe\n" );
					}
					if ( NPC->client->ps.fd.forceRageRecoveryTime < level.time
						&& !(NPC->client->ps.fd.forcePowersActive&(1<<FP_RAGE)) 
						&& !PM_InKnockDown( &NPC->client->ps )
						&& !Q_irand( 0, 5 ) )
					{//FIXME: make this a function call?
						//FIXME: check for clearance, safety of landing spot?
						NPC->client->ps.fd.forceJumpCharge = 320;
						//Don't jump again for another 2 to 5 seconds
						TIMER_Set( NPC, "jumpChaseDebounce", Q_irand( 2000, 5000 ) );
					}
				}
				break;
			}

			//turn off slow walking no matter what
			TIMER_Set( NPC, "walking", -level.time );
			TIMER_Set( NPC, "taunting", -level.time );
		}
	}
}

void DOM_Jedi_SetEnemyInfo( vec3_t enemy_dest, vec3_t enemy_dir, float *enemy_dist, vec3_t enemy_movedir, float *enemy_movespeed, int prediction )
{
	if ( !NPC || !NPC->enemy )
	{//no valid enemy
		return;
	}
	if ( !NPC->enemy->client )
	{
		VectorClear( enemy_movedir );
		*enemy_movespeed = 0;
		VectorCopy( NPC->enemy->r.currentOrigin, enemy_dest );
		enemy_dest[2] += NPC->enemy->r.mins[2] + 24;//get it's origin to a height I can work with
		VectorSubtract( enemy_dest, NPC->r.currentOrigin, enemy_dir );
		//FIXME: enemy_dist calc needs to include all blade lengths, and include distance from hand to start of blade....
		*enemy_dist = VectorNormalize( enemy_dir );// - (NPC->client->ps.saberLengthMax + NPC->r.maxs[0]*1.5 + 16);
	}
	else
	{//see where enemy is headed
		VectorCopy( NPC->enemy->client->ps.velocity, enemy_movedir );
		*enemy_movespeed = VectorNormalize( enemy_movedir );
		//figure out where he'll be, say, 3 frames from now
		VectorMA( NPC->enemy->r.currentOrigin, *enemy_movespeed * 0.001 * prediction, enemy_movedir, enemy_dest );
		//figure out what dir the enemy's estimated position is from me and how far from the tip of my saber he is
		VectorSubtract( enemy_dest, NPC->r.currentOrigin, enemy_dir );//NPC->client->renderInfo.muzzlePoint
		//FIXME: enemy_dist calc needs to include all blade lengths, and include distance from hand to start of blade....
		*enemy_dist = VectorNormalize( enemy_dir ) - (NPC->client->saber[0].blade[0].lengthMax + NPC->r.maxs[0]*1.5 + 16); //just use the blade 0 len I guess
		//FIXME: keep a group of enemies around me and use that info to make decisions...
		//		For instance, if there are multiple enemies, evade more, push them away
		//		and use medium attacks.  If enemies are using blasters, switch to fast.
		//		If one jedi enemy, use strong attacks.  Use grip when fighting one or
		//		two enemies, use lightning spread when fighting multiple enemies, etc.
		//		Also, when kill one, check rest of group instead of walking up to victim.
	}
}
