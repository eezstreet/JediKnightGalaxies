///////////////////////////////////
//
// Jedi Knight Galaxies Utility functions
//
//

#include "g_local.h"

typedef struct
{
	char    id[4];
	int     ver;
	char    fileName[68];
	int     numBoneFrames;
	int     numTags;
	int     numMeshes;
	int     numMaxSkins;
	int     headerLength;
	int     tagStart;
	int     tagEnd;
	int     fileSize;
} md3header_t;

typedef struct
{
	vec3_t  mins;
	vec3_t  maxs;
	vec3_t  pos;
	vec_t   scale;
	char    creator[16];
} md3boneFrame_t;

void JKG_RotateBBox(vec3_t mins,vec3_t maxs, vec3_t angles){
	vec3_t sides[6];
	int i,j;
	vec3_t corners[8];
	if (VectorLength(angles) == 0) {
		return;
	}

	AngleVectors(angles, sides[0],sides[1],sides[2]);
	for (i = 0; i < 3;i++) {
		VectorCopy(sides[i],sides[i+3]);
		VectorScale(sides[i],maxs[i],sides[i]);
		VectorScale(sides[i+3],mins[i],sides[i+3]);
	}

	for (i = 0; i < 8;i++) {
		VectorAdd(sides[i % 6], sides[(i+(i>5?2:1)) % 6], corners[i]);
		VectorAdd(corners[i], sides[(i + (i>5?4:2)) % 6], corners[i]);
	}


	VectorCopy(corners[0],mins);
	VectorCopy(corners[0],maxs);
	for (i = 0;i < 8;i++) {
		for (j = 0;j < 3;j++) {
			if (maxs[j] < corners[i][j]) {
				maxs[j] = corners[i][j];
			}
			if (mins[j] > corners[i][j]) {
				mins[j] = corners[i][j];
			}
		}
	}
}

void JKG_GetAutoBoxForModel(const char *model, vec3_t angles, float scale, vec3_t mins, vec3_t maxs) {
	fileHandle_t f;
	md3header_t header;
	md3boneFrame_t boneframe;
	vec3_t imins, imaxs;
	trap_FS_FOpenFile(model, &f, FS_READ);
	trap_FS_Read((void *)&header, sizeof(md3header_t), f);
	trap_FS_Read((void *)&boneframe, sizeof(md3boneFrame_t), f);
	trap_FS_FCloseFile(f);
	imins[0] = boneframe.mins[0] * scale;
	imins[1] = boneframe.mins[1] * scale;
	imins[2] = boneframe.mins[2] * scale;
	imaxs[0] = boneframe.maxs[0] * scale; 
	imaxs[1] = boneframe.maxs[1] * scale;
	imaxs[2] = boneframe.maxs[2] * scale;
	JKG_RotateBBox(imins, imaxs, angles);
	VectorCopy(imins, mins);
	VectorCopy(imaxs, maxs);
}