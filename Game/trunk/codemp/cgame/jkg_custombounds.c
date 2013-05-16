//////////////////////////////////////////
// Jedi Knight Galaxies Custom bounding boxes code
//
// Used to properly handle nonstandard bounding boxes
//
// Client-side
//
// By BobaFett

#include "cg_local.h"

typedef struct {
	vec3_t mins;
	vec3_t maxs;
} JkgCBBc_t; // Client-side version of this struct

static JkgCBBc_t CBBs[1024]; // CBBs = Custom Bounding Boxes

// Parsing code

typedef struct {
	int arg;
	int argc;
	char buff[1024];
} parsebuff_t;

static void CBB_InitParseBuff(parsebuff_t *pb) {
	memset(pb,0,sizeof(parsebuff_t));
	pb->arg = 1;
	pb->argc = trap_Argc();
}

static const char *CBB_NextToken(parsebuff_t *pb) {
	if (pb->arg > pb->argc) return NULL;
	trap_Argv(pb->arg++,pb->buff, sizeof(pb->buff));
	return pb->buff;
}

static qboolean CBB_TokensAvailable(parsebuff_t *pb) {
	if (pb->arg >= pb->argc) return qfalse;
	return qtrue;
}

static int CBB_ParseVector(parsebuff_t *pb, vec3_t *vec) {
	const char *token;
	int i;
	for (i=0; i<3; i++) {
		token = CBB_NextToken(pb);
		if (!token) {
			CG_Printf("WARNING: ^3Error processing custom bounding box info: Could not parse vector");
			return 1;
		}
		(*vec)[i] = atof(token);
	}
	return 0;
}

static int CBB_ParseInt(parsebuff_t *pb, int *num) {
	const char *token;
	token = CBB_NextToken(pb);
	if (!token) {
		CG_Printf("WARNING: ^3Error processing custom bounding box info: Could not parse int");
		return 1;
	}
	*num = atoi(token);

	return 0;
}

void Cmd_CBB_f(void) {
	// Incoming CBB update message, parse it
	parsebuff_t pb;
	int index;
	vec3_t mins;
	vec3_t maxs;

	CBB_InitParseBuff(&pb);
	
	if (pb.argc < 2) {
		// Blank, ignore
		return;
	}
	while (1) {
		if (!CBB_TokensAvailable(&pb))
			return;
		if (CBB_ParseInt(&pb, &index)) return;
		if (CBB_ParseVector(&pb, &mins)) return;
		if (CBB_ParseVector(&pb, &maxs)) return;
		VectorCopy(mins, CBBs[index].mins);
		VectorCopy(maxs, CBBs[index].maxs);
	}
}

void CBB_GetBoundingBox(int index, vec3_t *mins, vec3_t *maxs) {
	if (index < 0 || index > 1023) {
		VectorSet(*mins,0,0,0);
		VectorSet(*maxs,0,0,0);
		return;
	}
	VectorCopy(CBBs[index].mins,*mins);
	VectorCopy(CBBs[index].maxs,*maxs);
}