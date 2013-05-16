#ifndef JKG_BSP_H
#define JKG_BSP_H

#include "q_shared.h"
#include "qcommon/qfiles.h"

typedef enum surfaceType_e
{
    SF_BAD = 0,
    SF_SKIP,
    SF_FACE,
    SF_GRID,
    SF_TRIANGLES,
    SF_POLY,
    SF_UNKNOWN, // I haven't worked out what this one is yet.
    SF_MD3,
    SF_GLM,
    SF_FLARE,
    SF_ENTITY,
    SF_DISPLAY_LIST
} surfaceType_t;

typedef struct srfSurfaceFace_s
{
    surfaceType_t surfaceType;
    
    cplane_t plane;
    int dlightbits;
    
    int numPoints;
    int numIndexes;
    int ofsIndexes;
    float points[1][8]; // variable sized apparently, and somehow a list of indices afterwards?!
} srfSurfaceFace_t;

typedef struct srfGridMesh_s
{
    surfaceType_t surfaceType;
    
    int dlightbits;
    
    vec3_t meshBounds[2];
    vec3_t localOrigin;
    float meshRadius;
    
    vec3_t lodOrigin;
    float lodRadius;
    float lodFixed;
    int lodStitched;
    
    int width;
    int height;
    float widthLodError;
    float heightLodError;
    
    drawVert_t *verts;
} srfGridMesh_t;

typedef struct srfTriangles_s
{
    surfaceType_t surfaceType;
    
    int dlightbits;
    
    vec3_t bounds[2];
    vec3_t localOrigin;
    float radius;
    
    int numIndexes;
    int *indexes;
    
    int numVerts;
    drawVert_t *verts;
} srfTriangles_t;

typedef struct msurface_s
{
    int viewcount;
    void *shader;
    int fogIndex;
    surfaceType_t *surfaceType;
} msurface_t;

typedef struct world_s
{
    char name[MAX_QPATH];
    char baseName[MAX_QPATH];
    int dataSize;
    
    int numShaders;
    void *shaders;
    
    void *bmodels;
    
    int numPlanes;
    cplane_t *planes;
    
    int numNodes;
    int numDecisionNodes;
    void *nodes;
    
    int numSurfaces;
    msurface_t *surfaces;
    
    int numMarkSurfaces;
    void *markSurfaces;
    
    int numFogs;
    void *fogs;
    int currentFog;
    
    vec3_t lightGridOrigin;
    vec3_t lightGridSize;
    vec3_t lightGridInverseSize;
    vec3_t lightGridBounds;
    vec3_t lightGridData;
    
    int numClusters;
    int clusterBytes;
    void *vis;
    void *novis;
    const char *entityString;
    const char *entityParsePoint;
} world_t;

void JKG_BSP_LoadIntoVBOs ( const world_t *worldData, qboolean isSubBSP );

#endif
