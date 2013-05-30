///////////////////////////////////////////////////////////////////
//
//  SplinePaths
//
//  This is in no way connected to the cinematics module, this just
//  allows models to be controlled along a spline path. These models
//  not have a bbox (ergo, they're perfect for Coruscant ships, etc)
//
//  By eezstreet
//
///////////////////////////////////////////////////////////////////

#include "cg_local.h"
#include <math.h>
#include "encoding/bitstream.h"
#include "encoding/base128.h"

typedef struct {
	int arg;
	int argc;
	char buff[1024];
} parsebuff_t;

extern void Cin_InitParseBuff(parsebuff_t *pb);
extern const char *Cin_NextToken(parsebuff_t *pb);
qboolean Cin_TokensAvailable(parsebuff_t *pb);
extern int Cin_ParseVector(parsebuff_t *pb, vec3_t *vec);
extern int Cin_ParseVector2(parsebuff_t *pb, vec2_t *vec);
extern int Cin_ParseInt(parsebuff_t *pb, int *num);
extern int Cin_ParseFloat(parsebuff_t *pb, float *num);
extern int Cin_GetBSplinePoint(int index, int count);
extern float BSpF[21][5];
extern float CMRF[21][5];
extern float Cin_BSplinePhaseLookup(float x, float p1, float p2, float p3, float p4);
extern float Cin_CatMullPhaseLookup(float x, float p1, float p2, float p3, float p4);


void JKG_ProcessSplinePath_f() {

}