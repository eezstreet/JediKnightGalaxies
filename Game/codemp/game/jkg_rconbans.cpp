/////////////////////////////////////////
//
//	Rcon ACL/Bans
//
//  This is responsible for handling rcon 
//  ACL and hammer bans
//

#include "g_local.h"

typedef enum {
	RCfree,		// Slot is free
	RCB_BANNED,		// Permabanned
	RCB_GRANTED,	// Access granted
	RCB_DIRECT,		// Direct access, does not trigger the timeout
	RCB_HAMMERBAN,	// Hammer banned
	PCB_MARKED,		// Used an invalid pass before, but isn't banned yet (this is only used if the IP isnt in the list already)
} RconBanType_e;

typedef struct {
	int banType;
	unsigned int IP;
	unsigned int Mask;
	int Range;				// # of 0's in the IP
	int Fails;				// # of times a wrong pass was used
} RconBan_t;

#define MAX_RCBANS 1024
#define MAX_RCONTRIES 5

static RconBan_t RCBans[MAX_RCBANS];
static int RCBanCount;

static int RCB_IPMatch(unsigned int IP, RconBan_t *Ban) {
	if (Ban->banType == RCfree) {
		return 0;
	}
	if ((IP & Ban->Mask) == Ban->IP) {
		return 1;
	}
	return 0;
}

static unsigned int RCB_SetupMask(unsigned int *IP) {
	unsigned int mask;
	if (*IP & 0xFF000000) mask |= 0xFF000000;
	if (*IP & 0x00FF0000) mask |= 0x00FF0000;
	if (*IP & 0x0000FF00) mask |= 0x0000FF00;
	if (*IP & 0x000000FF) mask |= 0x000000FF;
	return mask;
}

static int RCB_GetFreeSlot() {
	int i;
	for (i=0; i < RCBanCount; i++) {
		if (RCBans[i].banType == RCfree) {
			return i;
		}
	}
	if (RCBanCount == MAX_RCBANS-1) {
		G_Printf("Warning: RCB_GetFreeSlot: Out of Rcon Ban slots\n");
		return -1;
	}
	RCBanCount++;
	return RCBanCount;
}

// 0 = No, 1 = Yes, 2 = No, and reverse timelock too
int RCB_IsBanned(unsigned int IP) {
	int i, r = 5, b = 0;	// Iterator, Min Range, Bantype
	for (i=0; i< RCBanCount; i++) {
		if (RCB_IPMatch(IP, &RCBans[i])) {
			if (RCBans[i].Range < r) {
				r = RCBans[i].Range;
				b = RCBans[i].banType;
			}
		}
	}
	if (b == RCfree || b == RCB_GRANTED) {
		return 0;
	} else if (b == RCB_BANNED || b == RCB_HAMMERBAN) {
		return 1;
	} else if (b == RCB_DIRECT) {
		return 2;
	}
	return 0;
}

void RCB_AddBan(unsigned int IP) {
	

}

void RCB_AddGrant(unsigned int IP) {
	

}

void RCB_AddDirect(unsigned int IP) {
	

}

void RCB_RemoveIP(unsigned int IP) {


}

void RCB_Load() {

}

void RCB_Save() {


}

void RCB_ClearHammerBans() {


}

int RCB_BadAuth(unsigned int IP) {
	int i;
	for (i=0; i< RCBanCount; i++) {
		if ((RCBans[i].banType != RCB_GRANTED && RCBans[i].banType != RCB_DIRECT ) && IP==RCBans[i].IP) {
			RCBans[i].Fails++;
			if (RCBans[i].Fails >= MAX_RCONTRIES) {
				RCBans[i].banType = RCB_HAMMERBAN;
				return 0;
			}
			return MAX_RCONTRIES - RCBans[i].Fails;	
		}
	}
	i = RCB_GetFreeSlot();
	RCBans[i].banType = PCB_MARKED;
	RCBans[i].IP = IP;
	RCBans[i].Mask = 0xFFFFFFFF;
	RCBans[i].Fails = 1;
	return MAX_RCONTRIES - 1;
}

void RCB_GoodAuth(unsigned int IP) {


}

unsigned int RCB_ParseIP(const char* IP) {
		byte			m[4];// = {'\0','\0','\0','\0'};
	int				i = 0;
	char			*p;

	while (i < 4)
	{
		m[i] = 0;
		i++;
	}

	i = 0;
	p = ( char * ) IP;
	while (*p && i < 4) {
		while (*p >= '0' && *p <= '9') {
			m[i] = m[i]*10 + (*p - '0');
			p++;
		}
		if (!*p || *p == ':')
			break;
		i++, p++;
	}
	return *(unsigned int *)&m[0];
}
