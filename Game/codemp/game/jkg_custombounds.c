//////////////////////////////////////////
// Jedi Knight Galaxies Custom bounding boxes code
//
// Used to properly handle nonstandard bounding boxes
//
// Server-side
//
// By BobaFett

#include "g_local.h"

typedef struct {
	int inuse;
	int refcount;
	vec3_t mins;
	vec3_t maxs;
} JkgCBB_t;

static int CBBSlots;
static JkgCBB_t CBBs[1024]; // CBBs = Custom Bounding Boxes


static void JKG_CBB_NetworkUpdate(int newslot, vec3_t mins, vec3_t maxs) {
	int i;
	gentity_t *ent;
	char msg[1024]= {0};
	Com_sprintf(msg, 1024, "cbb %i %.1f %.1f %.1f %.1f %.1f %.1f", newslot, mins[0], mins[1], mins[2], maxs[0], maxs[1], maxs[2]);
	for (i=0; i<level.maxclients; i++) {
		ent = &g_entities[i];
		if (!ent->client)
			continue;
		if (ent->client->pers.connected != CON_CONNECTED)
			continue;
		// Valid client
		trap_SendServerCommand(i, msg);
	}
}

void JKG_CBB_SendAll(int client) {
	// Send all custom bounding boxes (for new clients)
	// Might result in multiple messages depending on the size
	int msgsz;		// Message size (cut off if > 900)
	char msg[1024]= {0}; // Message to send
	char sb[1024] = {0}; // New chunks to add
	int i;
	// Init
	Q_strncpyz(msg, "cbb ", 1024);
	msgsz = 4;
	for (i=0; i<CBBSlots; i++) {
		if (!CBBs[i].inuse)
			continue;
		// Valid bounding box
		Com_sprintf(sb, 1024, "%i %.1f %.1f %.1f %.1f %.1f %.1f ", i, CBBs[i].mins[0],CBBs[i].mins[1],CBBs[i].mins[2],CBBs[i].maxs[0],CBBs[i].maxs[1],CBBs[i].maxs[2]);
		if (msgsz + strlen(sb) > 900) {
			// Gettin too long, send it now and make a new message
			trap_SendServerCommand(client, msg);
			Q_strncpyz(msg, "cbb ", 1024);
			msgsz = 4;
		}
		Q_strcat(msg, 1024, sb);
		msgsz += strlen(sb);
	}
	if (msgsz != 4) {
		// send it
		trap_SendServerCommand(client, msg);
	}
}

static int JKG_CBB_Add(vec3_t mins, vec3_t maxs) {
	int i;
	for (i=0; i<CBBSlots; i++) {
		if (!CBBs[i].inuse)
			continue;
		if (VectorCompare(CBBs[i].mins, mins) && VectorCompare(CBBs[i].maxs, maxs)) {
			// Same bounds
			CBBs[i].refcount++;
			return i;
		}
	}
	// Not found, search for an empty slot
	for (i=0; i<CBBSlots; i++) {
		if (!CBBs[i].inuse) {
			// Got one!
			CBBs[i].inuse = 1;
			VectorCopy(mins, CBBs[i].mins);
			VectorCopy(maxs, CBBs[i].maxs);
			CBBs[i].refcount = 1;
			JKG_CBB_NetworkUpdate(i, mins, maxs);
			return i;
		}
	}
	
	// Add new entry
	if (CBBSlots == 1024) {
		// Ran outta slots.. should never happen
		return 0;
	}
	CBBs[CBBSlots].inuse = 1;
	VectorCopy(mins, CBBs[CBBSlots].mins);
	VectorCopy(maxs, CBBs[CBBSlots].maxs);
	CBBs[CBBSlots].refcount = 1;
	JKG_CBB_NetworkUpdate(CBBSlots, mins, maxs);
	return (CBBSlots++);
}

static void JKG_CBB_Remove(int index) {
	// Error checking time
	if (index < 0 || index >= CBBSlots)
		return; // Wtf? Bad index...
	if (!CBBs[index].inuse)
		return; // Again.. wtf
	
	// Lower refcount and free if 0
	CBBs[index].refcount--;
	if (CBBs[index].refcount <= 0) {
		CBBs[index].inuse = 0; // free the slot
	}
}

void JKG_CBB_SetBB(gentity_t *ent, vec3_t mins, vec3_t maxs) {
	// If this ent already uses a custom bounding box, remove it
	if (ent->s.eType != ET_GENERAL && ent->s.eType != ET_MOVER && ent->s.eType != ET_SPECIAL)
		return; // Invalid ent type
	if (ent->s.eFlags & EF_CUSTOMBB) {
		JKG_CBB_Remove(ent->s.trickedentindex4);
		ent->s.eFlags &= ~EF_CUSTOMBB;
		ent->s.trickedentindex = 0;
	}
	
	VectorCopy (mins, ent->r.mins);
	VectorCopy (maxs, ent->r.maxs);

	// First, check if this bounding box is compatible with the standard algorithm
	// Check for symmetry
	if (mins[0] == mins[1] && maxs[0] == maxs[1]) {
		// Check for bounds on X and Y
		if ((mins[0] <= 0 && mins[0] >= -255) && (maxs[0] >= 0 && maxs[0] <= 255)) {
			// Check for bounds on Z
			if ((mins[2] <= 0 && mins[2] >= -255) && (maxs[2] >= -32 && maxs[2] <= 223)) {
				// Compatible, dont set up a custom bounding box
				return;
			}
		}
	}
	// Not compatible if we get here
	// So register the bounding box
	ent->s.eFlags |= EF_CUSTOMBB;
	ent->s.trickedentindex4 = JKG_CBB_Add(mins, maxs);
}

void JKG_CBB_RemoveBB(gentity_t *ent) {
	// Unregisters the bounding box
	// its important to do this so slots arent needlessly occupied
	if (ent->s.eType != ET_GENERAL && ent->s.eType != ET_MOVER)
		return; // Invalid ent type

	// NOTE: This wont clear the actual bounding box, it only removes the reference
	if (ent->s.eFlags & EF_CUSTOMBB) {
		JKG_CBB_Remove(ent->s.trickedentindex4);
		ent->s.eFlags &= ~EF_CUSTOMBB;
		ent->s.trickedentindex = 0;
	}
}