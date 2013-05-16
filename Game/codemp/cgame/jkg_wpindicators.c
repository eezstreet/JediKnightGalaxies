/////////////////////////////////
//
// JKG Weapon Indicators
// 
// By BobaFett
//
/////////////////////////////////

#include "cg_local.h"
#include "jkg_eshader.h"

static long generateHashValue( const char *fname, const int size ) {
	int		i;
	long	hash;
	char	letter;

	hash = 0;
	i = 0;
	while (fname[i] != '\0') {
		letter = tolower(fname[i]);
		if (letter =='.') break;				// don't include extension
		if (letter =='\\') letter = '/';		// damn path names
		if (letter == PATH_SEP) letter = '/';		// damn path names
		hash+=(long)(letter)*(i+119);
		i++;
	}
	hash = (hash ^ (hash >> 10) ^ (hash >> 20));
	hash &= (size-1);
	return hash;
}

qhandle_t CG_FindShader(const char *name) {
	// Searches the shader table for the specified shader, returns -1 if not found
	long hash;
	shader_t *sh;

	// Info:
	// Hashtable root: 1072B90 (1024 indexes, pointers in each)

	hash = generateHashValue(name, 1024);

	for (sh = *(void **)(0x1072B90 + 4*hash); sh; sh = sh->next) {
		if (!Q_stricmp(sh->name, name)) {
			// We found it
			return sh->index;
		}
	}
	return 0;
}

static void CG_OverrideShaderFrame(qhandle_t shader, int desiredFrame) {
	// This function here allows us to manually select the frame to render in the current render
	// It'll override the shader's time offset to force a specific frame to be used
	shader_t *sh;
	float desiredShadertime;
	float animFrequency;
	int i;
	float floattime = (float)cg.time * 0.001f;

	if (shader <= 0) {
		return;
	}
	sh = *(void **)(0xFE742c + (4*shader)); // shader_t * tr.shaders[shader];

	animFrequency = 1; // Default
	for (i=0; i < (sh->numUnfoggedPasses ? sh->numUnfoggedPasses : 1) ; i++) {
		if (sh->stages[i].bundle[0].numImageAnimations > 1) {
			animFrequency = sh->stages[i].bundle[0].imageAnimationSpeed;
		}
	}
	
	
	// Put it halfway through the frame to avoid any chance of flickering due to rounding errors
	desiredShadertime = (1 / animFrequency) * (desiredFrame + 0.5);
	sh->timeOffset = floattime - desiredShadertime;
}

/*static struct {
	// E-11
	qhandle_t	e11_number3;
	qhandle_t	e11_number2;
	qhandle_t	e11_number1;
	qhandle_t	e11_firemode;

	// DC-15a repeater
	qhandle_t	rep_number3;
	qhandle_t	rep_number2;
	qhandle_t	rep_number1;
	qhandle_t	rep_firemode;

	// Bowcaster
	qhandle_t	bow_number3;
	qhandle_t	bow_number2;
	qhandle_t	bow_number1;
	qhandle_t	bow_firemode;

	// Disruptor
	qhandle_t	rup_number3;
	qhandle_t	rup_number2;
	qhandle_t	rup_number1;
	qhandle_t	rup_firemode;

	
	// Golan Arms Flechette
	qhandle_t	gaf_number3;
	qhandle_t	gaf_number2;
	qhandle_t	gaf_number1;
	qhandle_t	gaf_firemode;

	// DEMP2
	qhandle_t	dp2_number3;
	qhandle_t	dp2_number2;
	qhandle_t	dp2_number1;
	qhandle_t	dp2_firemode;

	// Pistol
	qhandle_t	pst_number3;
	qhandle_t	pst_number2;
	qhandle_t	pst_firemode;

	// Bryar
	qhandle_t	bry_number3;
	qhandle_t	bry_number2;
	qhandle_t	bry_number1;
	qhandle_t	bry_firemode;

	// Concussion
	qhandle_t	con_bar;
	qhandle_t	con_led1;
	qhandle_t	con_led2;
	qhandle_t	con_led3;

	// Rocket launcher
	qhandle_t	rocket_indic;
	
	// Thermal detonator
	qhandle_t   det_red;
	qhandle_t   det_led1;
	qhandle_t   det_led2;
	qhandle_t   det_led3;

} IndShaders;		// Indicator Shaders

void JKG_WeaponIndicators_Init() {
	memset(&IndShaders, -1, sizeof(IndShaders));
}

static void CacheShaders(int wp) {
	switch(wp) {
	    case WP_BRYAR_PISTOL:
			if (IndShaders.pst_number3 < 0) IndShaders.pst_number3 = CG_FindShader("models/weapons2/elg-3a/numbers3");
			if (IndShaders.pst_number2 < 0) IndShaders.pst_number2 = CG_FindShader("models/weapons2/elg-3a/numbers2");
			if (IndShaders.pst_firemode < 0) IndShaders.pst_firemode = CG_FindShader("models/weapons2/elg-3a/firemode");
			break;
			
		case WP_BLASTER:
			if (IndShaders.e11_number3 < 0) IndShaders.e11_number3 = CG_FindShader("models/weapons2/e-11/numbers3");
			if (IndShaders.e11_number2 < 0) IndShaders.e11_number2 = CG_FindShader("models/weapons2/e-11/numbers2");
			if (IndShaders.e11_number1 < 0) IndShaders.e11_number1 = CG_FindShader("models/weapons2/e-11/numbers1");
			if (IndShaders.e11_firemode < 0) IndShaders.e11_firemode = CG_FindShader("models/weapons2/e-11/firemode");
			break;
			
		case WP_DISRUPTOR:
			if (IndShaders.rup_number3 < 0) IndShaders.rup_number3 = CG_FindShader("models/weapons2/na_acp_rifle/numbers3");
			if (IndShaders.rup_number2 < 0) IndShaders.rup_number2 = CG_FindShader("models/weapons2/na_acp_rifle/numbers2");
			if (IndShaders.rup_number1 < 0) IndShaders.rup_number1 = CG_FindShader("models/weapons2/na_acp_rifle/numbers1");
			if (IndShaders.rup_firemode < 0) IndShaders.rup_firemode = CG_FindShader("models/weapons2/na_acp_rifle/firemode");
			break;
			
		case WP_BOWCASTER:
			if (IndShaders.bow_number3 < 0) IndShaders.bow_number3 = CG_FindShader("models/weapons2/bowcaster/numbers3");
			if (IndShaders.bow_number2 < 0) IndShaders.bow_number2 = CG_FindShader("models/weapons2/bowcaster/numbers2");
			if (IndShaders.bow_number1 < 0) IndShaders.bow_number1 = CG_FindShader("models/weapons2/bowcaster/numbers1");
			if (IndShaders.bow_firemode < 0) IndShaders.bow_firemode = CG_FindShader("models/weapons2/bowcaster/firemode");
			break;
			
		case WP_REPEATER:
			if (IndShaders.rep_number3 < 0) IndShaders.rep_number3 = CG_FindShader("models/weapons2/dc-15a/numbers3");
			if (IndShaders.rep_number2 < 0) IndShaders.rep_number2 = CG_FindShader("models/weapons2/dc-15a/numbers2");
			if (IndShaders.rep_number1 < 0) IndShaders.rep_number1 = CG_FindShader("models/weapons2/dc-15a/numbers1");
			if (IndShaders.rep_firemode < 0) IndShaders.rep_firemode = CG_FindShader("models/weapons2/dc-15a/firemode");
			break;
		
		case WP_DEMP2:
			if (IndShaders.dp2_number3 < 0) IndShaders.dp2_number3 = CG_FindShader("models/weapons2/l8/numbers3");
			if (IndShaders.dp2_number2 < 0) IndShaders.dp2_number2 = CG_FindShader("models/weapons2/l8/numbers2");
			if (IndShaders.dp2_number1 < 0) IndShaders.dp2_number1 = CG_FindShader("models/weapons2/l8/numbers1");
			if (IndShaders.dp2_firemode < 0) IndShaders.dp2_firemode = CG_FindShader("models/weapons2/l8/firemode");
			break;
		
		case WP_FLECHETTE:
			if (IndShaders.gaf_number3 < 0) IndShaders.gaf_number3 = CG_FindShader("models/weapons2/flechette_pistol/numbers3");
			if (IndShaders.gaf_number2 < 0) IndShaders.gaf_number2 = CG_FindShader("models/weapons2/flechette_pistol/numbers2");
			if (IndShaders.gaf_number1 < 0) IndShaders.gaf_number1 = CG_FindShader("models/weapons2/flechette_pistol/numbers1");
			if (IndShaders.gaf_firemode < 0) IndShaders.gaf_firemode = CG_FindShader("models/weapons2/flechette_pistol/firemode");
			break;
			
		case WP_CONCUSSION:
			if (IndShaders.con_bar < 0) IndShaders.con_bar = CG_FindShader("models/weapons2/concussion/firemode");
			//if (IndShaders.con_led1 < 0) IndShaders.con_led1 = CG_FindShader("models/weapons2/concussion/led1");
			if (IndShaders.con_led2 < 0) IndShaders.con_led2 = CG_FindShader("models/weapons2/concussion/numbers2");
			if (IndShaders.con_led3 < 0) IndShaders.con_led3 = CG_FindShader("models/weapons2/concussion/numbers3");
			break;
			
		case WP_ROCKET_LAUNCHER:
			if (IndShaders.rocket_indic < 0) IndShaders.rocket_indic = CG_FindShader("models/weapons2/dc-15a_antiarmor/antiarmor_glow");
			break;
			
		case WP_BRYAR_OLD:
			if (IndShaders.bry_number3 < 0) IndShaders.bry_number3 = CG_FindShader("models/weapons2/briar_pistol/numbers3");
			if (IndShaders.bry_number2 < 0) IndShaders.bry_number2 = CG_FindShader("models/weapons2/briar_pistol/numbers2");
			if (IndShaders.bry_number1 < 0) IndShaders.bry_number1 = CG_FindShader("models/weapons2/briar_pistol/numbers1");
			if (IndShaders.bry_firemode < 0) IndShaders.bry_firemode = CG_FindShader("models/weapons2/briar_pistol/firemode");
			break;
			
		case WP_THERMAL:
		    if (IndShaders.det_red < 0 ) IndShaders.det_red = CG_FindShader ("models/weapons2/thermal/redlight");
		    if (IndShaders.det_led1 < 0 ) IndShaders.det_led1 = CG_FindShader ("models/weapons2/thermal/led1");
		    if (IndShaders.det_led2 < 0 ) IndShaders.det_led2 = CG_FindShader ("models/weapons2/thermal/led2");
		    if (IndShaders.det_led3 < 0 ) IndShaders.det_led3 = CG_FindShader ("models/weapons2/thermal/led3");
		    break;
			
		default:
			break;
	}
}*/

static void DissectNumber(int number, int *hundreds, int *tens, int *ones) {
	int h, t, o;
	if (number < 0) {
		number *= -1;	// We cant handle negative numbers here
	}
	h = floor(number/100);
	number -= (h*100);
	if (h>9) {
		h=9;
	}
	t = floor(number/10);
	o = number - (t*10);

	*hundreds = h;
	*tens = t;
	*ones = o;
}

static __inline qboolean PlayerIsReloading()
{
    return (qboolean)(cg.reloadTimeStart && cg.time <= (cg.reloadTimeStart + cg.reloadTimeDuration));
}

void JKG_WeaponIndicators_Update(centity_t *cent, playerState_t *ps) {
	int h,t,o;
	int ammo;
	weaponInfo_t *weaponInfo = CG_WeaponInfo (cent->currentState.weapon, cent->currentState.weaponVariation);
	weaponData_t *weaponData = GetWeaponData (cent->currentState.weapon, cent->currentState.weaponVariation);

	if (!ps) {
		// Check if it's me anyway
		if (cent->currentState.number == cg.predictedPlayerState.clientNum) {
			ps = &cg.predictedPlayerState;
		}
	}

	if (ps) {
		if ( GetWeaponAmmoClip( cent->currentState.weapon, cent->currentState.weaponVariation ))
		{
			ammo = ps->stats[STAT_AMMO];
		}
		else
		{
			ammo = ps->ammo;
		}

	} else {
		ammo = 0;
	}

	if ( ammo )
	{
		if ( weaponData->firemodes[ps->firingMode].cost > 0 )
		{
			ammo /= weaponData->firemodes[ps->firingMode].cost;
		}
	}

	if (ps) {	// This is my weapon
        switch ( weaponInfo->indicatorType )
        {
            case IND_NORMAL:
				if ( weaponData->visuals.visualFireModes[ps->firingMode].overrideIndicatorFrame != -1 )
				{
					CG_OverrideShaderFrame (weaponInfo->fireModeIndicator, weaponData->visuals.visualFireModes[ps->firingMode].overrideIndicatorFrame);
				}
				else
				{
					CG_OverrideShaderFrame (weaponInfo->fireModeIndicator, ps->firingMode);
				}
                if ( PlayerIsReloading() )
                {
                    CG_OverrideShaderFrame (weaponInfo->groupedIndicators[0], 11);
                    CG_OverrideShaderFrame (weaponInfo->groupedIndicators[1], 11);
                    CG_OverrideShaderFrame (weaponInfo->groupedIndicators[2], 11);
                }
                else
                {
                    DissectNumber (ammo, &h, &t, &o);
                    CG_OverrideShaderFrame (weaponInfo->groupedIndicators[0], o);
                    CG_OverrideShaderFrame (weaponInfo->groupedIndicators[1], (!t && !h) ? 10 : t);
                    CG_OverrideShaderFrame (weaponInfo->groupedIndicators[2], !h ? 10 : h);
                }
                break;
                
            case IND_GRENADE:
                if ( cg.jkg_grenadeCookTimer >= cg.time )
			    {
			        int i = 0, j = 0;
			        int led1 = 0;
			        int led2 = 0;
			        int led3 = 0;
			        int dt = cg.jkg_grenadeCookTimer - cg.time - 1000;
			        
			        CG_OverrideShaderFrame (weaponInfo->fireModeIndicator, 1);
			        
			        if ( dt <= 0 )
			        {
			            led1 = led2 = led3 = 1;
			        }
			        else
			        {
			            int detonationTime = GetWeaponData (cg.snap->ps.weapon, cg.snap->ps.weaponVariation)->weaponReloadTime;
			            int interval;
			        
			            while ( i < dt )
			            {
			                interval = 600 * (1.0f - ((float)(detonationTime - i) / (float)detonationTime));
			                if ( interval < 50 )
			                    interval = 50;
    			        
			                i += interval;
			                j = (j + 1) % 4;
			            }
    			        
			            switch ( j )
			            {
			                case 0:
			                    led1 = 1;
			                    break;
    			                
			                case 1:
			                case 3:
			                    led2 = 1;
			                    break;
    			                
			                case 2:
			                    led3 = 1;
			                    break;
    			                
			                default:
			                    break;
			            }
			        }

		            CG_OverrideShaderFrame (weaponInfo->groupedIndicators[0], led1);
		            CG_OverrideShaderFrame (weaponInfo->groupedIndicators[1], led2);
		            CG_OverrideShaderFrame (weaponInfo->groupedIndicators[2], led3);
			    }
			    else
			    {
			        CG_OverrideShaderFrame (weaponInfo->fireModeIndicator, 0);
			        CG_OverrideShaderFrame (weaponInfo->groupedIndicators[0], 0);
			        CG_OverrideShaderFrame (weaponInfo->groupedIndicators[1], 0);
			        CG_OverrideShaderFrame (weaponInfo->groupedIndicators[2], 0);
			    }
                break;
                
            default:
            case IND_NONE:
                CG_OverrideShaderFrame (weaponInfo->fireModeIndicator, 0);
                break;
        }
	} else {
		// This isnt me, check if the player uses a different weapon than i do, if so, clear the display
		if (cent->currentState.weapon == cg.predictedPlayerState.weapon) {
			return;
		}

        CG_OverrideShaderFrame (weaponInfo->fireModeIndicator, 0);
        CG_OverrideShaderFrame (weaponInfo->groupedIndicators[0], 10);
        CG_OverrideShaderFrame (weaponInfo->groupedIndicators[1], 10);
        CG_OverrideShaderFrame (weaponInfo->groupedIndicators[2], 10);
	}
}