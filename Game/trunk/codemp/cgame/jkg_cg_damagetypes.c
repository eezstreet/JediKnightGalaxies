#include "cg_local.h"
#include "bg_public.h"
#include "jkg_cg_damagetypes.h"

void JKG_PlayerDebuffVisuals ( centity_t *cent, refEntity_t *refEntity )
{
    const entityState_t *es = &cent->currentState;
    const qboolean isLocalPlayer = (es->number == cg.predictedPlayerState.clientNum);
    
    if ( es->damageTypeFlags & (1 << DT_FIRE) )
    {
        if ( (cent->debuffVisuals.lastFireEFXTime + 100) <= cg.time )
        {
            int lowerLumbarBolt = trap_G2API_AddBolt (cent->ghoul2, 0, "lower_lumbar");
            trap_FX_PlayBoltedEffectID (cgs.media.playerFireEffect, cent->lerpOrigin, cent->ghoul2, lowerLumbarBolt, es->number, 0, 0, qtrue);
            cent->debuffVisuals.lastFireEFXTime = cg.time;
        }
    }
    
    if ( es->damageTypeFlags & (1 << DT_FREEZE) )
    {
        if ( cg.renderingThirdPerson || !isLocalPlayer )
        {
            refEntity->shaderRGBA[0] = 63;
            refEntity->shaderRGBA[1] = 63;
            refEntity->shaderRGBA[2] = 127;
            refEntity->shaderRGBA[3] = 254;
            refEntity->renderfx = 0;
            refEntity->customShader = cgs.media.iceOverlay;
            
            trap_R_AddRefEntityToScene (refEntity);
        }
    }
    
    if ( es->damageTypeFlags & (1 << DT_CARBONITE) )
    {
        if ( cg.renderingThirdPerson || !isLocalPlayer )
        {
            refEntity->shaderRGBA[0] = 50;
            refEntity->shaderRGBA[1] = 50;
            refEntity->shaderRGBA[2] = 50;
            refEntity->shaderRGBA[3] = 254;
            refEntity->renderfx = 0;
            refEntity->customShader = cgs.media.carboniteOverlay;
            
            trap_R_AddRefEntityToScene (refEntity);
        }
    }
    
    if ( es->damageTypeFlags & (1 << DT_STUN) )
    {
        if ( cent->debuffVisuals.stunStartTime == 0 ||
            (cent->debuffVisuals.stunStartTime + 300) > cg.time )
        {
            if ( cg.renderingThirdPerson || !isLocalPlayer )
            {
                refEntity->shaderRGBA[0] = 0;
                refEntity->shaderRGBA[1] = 0;
                refEntity->shaderRGBA[2] = 127;
                refEntity->shaderRGBA[3] = 254;
                refEntity->renderfx = 0;
                refEntity->customShader = cgs.media.stunOverlay;
                
                trap_R_AddRefEntityToScene (refEntity);
            }
            
            if ( cent->debuffVisuals.stunStartTime == 0 )
            {
                cent->debuffVisuals.stunStartTime = cg.time;
            }
        }
    }
    else
    {
        cent->debuffVisuals.stunStartTime = 0;
    }
}