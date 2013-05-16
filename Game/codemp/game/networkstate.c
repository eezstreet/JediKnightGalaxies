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
	{ NSF(ironsightsDebounceStart), 32 },
	{ NSF(isSprinting), 1 },
	{ NSF(isInSights), 1 },
};

netField_t extraStateFields[] =
{
	{ ESF(number), 32 },
	{ ESF(testInt), 32 },				
	{ ESF(testFloat), 0 }
};

#pragma pack()

int numNetworkStateFields = sizeof( networkStateFields ) / sizeof( networkStateFields[0] );
int numExtraStateFields = sizeof( extraStateFields ) / sizeof( extraStateFields[0] );