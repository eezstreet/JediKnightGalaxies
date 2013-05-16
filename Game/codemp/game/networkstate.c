#include "q_shared.h"

#pragma pack(4)

// If you alter this, be sure to alter it in q_shared as well!
netField_t	networkStateFields[] = 
{
	// use 32 for ints, 0 for floats, 16 for shorts, 8 for bytes.
	{ NSF(start), 32 },	
	{ NSF(ironsightsTime), 32 },	
	{ NSF(sprintTime), 32 },
	{ NSF(sprintDebounceTime), 32 },

	{ NSF(forcePower), 16 },
	{ NSF(saberSwingSpeed), 0 },
	{ NSF(saberMoveSwingSpeed), 0 },

	{ NSF(saberPommel[0]), 32 },
	{ NSF(saberPommel[1]), 32 },
	{ NSF(saberShaft[0]), 32 },
	{ NSF(saberShaft[1]), 32 },
	{ NSF(saberEmitter[0]), 32 },
	{ NSF(saberEmitter[1]), 32 },
	{ NSF(saberCrystal[0]), 32 },
	{ NSF(saberCrystal[1]), 32 },

	{ NSF(isSprinting), 1 },
	{ NSF(isInSights), 1 },
	// This code always exists on at least one gametype --eez
#ifdef __JKG_NINELIVES__
	{ NSF(iLivesLeft), 8 },
#elif defined __JKG_TICKETING__
	{ NSF(iLivesLeft), 8 },
#elif defined __JKG_ROUNDBASED__
	{ NSF(iLivesLeft), 8 },
#endif
	{ NSF(blockPoints), 16 },
};

netField_t extraStateFields[] =
{
	{ ESF(number), 32 },
	{ ESF(testInt), 32 },				
	{ ESF(testFloat), 0 },

	{ ESF(saberPommel[0]), 32 },
	{ ESF(saberPommel[1]), 32 },
	{ ESF(saberShaft[0]), 32 },
	{ ESF(saberShaft[1]), 32 },
	{ ESF(saberEmitter[0]), 32 },
	{ ESF(saberEmitter[1]), 32 },
	{ ESF(saberCrystal[0]), 32 },
	{ ESF(saberCrystal[1]), 32 },

	{ ESF(forcePower), 16 },
	{ ESF(saberSwingSpeed), 0 },
	{ ESF(saberMoveSwingSpeed), 0 },
};

#pragma pack()

int numNetworkStateFields = sizeof( networkStateFields ) / sizeof( networkStateFields[0] );
int numExtraStateFields = sizeof( extraStateFields ) / sizeof( extraStateFields[0] );