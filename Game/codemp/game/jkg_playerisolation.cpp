//////////////////////////////////////////
//
//	Jedi Knight Galaxies Player Isolation system
//	This system allows JKG to make players
//  non-existant to eachother.
//
//  They cant see eachother, they cant collide.. nothing
//  The integral parts are controlled by an engine hook and trap call replacement
//  But the control over the system is in here

#include "g_local.h"

char PlayerHide[MAX_CLIENTS][MAX_CLIENTS];	// PlayerHide[client][target]. 1 = hidden, 0 = normal

typedef struct {
	int entNum;
	int contents;
} contbackup_t;

contbackup_t contbackup[MAX_GENTITIES];

void JKG_PlayerIsolationInit() {
	memset(&PlayerHide, 0, sizeof(PlayerHide));
}

void JKG_PlayerIsolationClear(int client) {
	int i;
	for (i=0; i<MAX_CLIENTS; i++) {
		PlayerHide[i][client] = 0;
		PlayerHide[client][i] = 0;
	}

}

void JKG_PlayerIsolate(int client1, int client2) {
	if (client1 == client2) return;
	PlayerHide[client1][client2] = 1;
	PlayerHide[client2][client1] = 1;
}

void JKG_PlayerReveal(int client1, int client2) {
	PlayerHide[client1][client2] = 0;
	PlayerHide[client2][client1] = 0;
}

int JKG_IsIsolated(int client1, int client2) {
	if (PlayerHide[client1][client2] || PlayerHide[client2][client1]) {
		return 1;
	} else {
		return 0;
	}
}

void JKG_PMTrace( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask ) {
	// Special trace for the PM system (movement)
	// This will pretend player collisions dont happen by ignoring those
	// The player we're tracing from should be the one provided as passEntityNum
	// If that is not the a valid client we just ignore it and do a regular trace
	int i, j;
	if (passEntityNum < 0 || passEntityNum >= MAX_CLIENTS) {
		// Not a valid client
		trap_Trace(results, start, mins, maxs, end, passEntityNum, contentmask);
		return;
	}
	// This of for a player, so clear out the contents of all player's he's hidden to
	for (i=0, j=0; i<g_maxclients.integer; i++) {
		if (g_entities[i].inuse && g_entities[i].client && JKG_IsIsolated(passEntityNum, i)) {
			contbackup[j].entNum = i;
			contbackup[j].contents = g_entities[i].r.contents;
			j++;
			g_entities[i].r.contents = 0;
		}
	}

	trap_Trace(results, start, mins, maxs, end, passEntityNum, contentmask);
	
	for (i=0; i<j; i++) {
		g_entities[contbackup[i].entNum].r.contents = contbackup[i].contents;
	}
}