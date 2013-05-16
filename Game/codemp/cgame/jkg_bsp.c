#include <GL/glew.h>
#include "cg_local.h"
#include "jkg_bsp.h"

typedef struct gpuVertex_s
{
    vec3_t position;
    vec2_t texcoord;
} gpuVertex_t;

void __cdecl JKG_BSP_LoadIntoVBOs ( const world_t *worldData, qboolean isSubBSP )
{
    CG_Printf ("YOUR MAP HAS LOADED ETC called %s\n", worldData->name);
    CG_Printf ("It has %d surfaces.\n", worldData->numSurfaces);
    CG_Printf ("Is sub BSP? %d\n", isSubBSP);
    
    if ( isSubBSP )
    {
        return;
    }
    else
    {
        int i;
        int numVerts = 0;
        for ( i = 0; i < worldData->numSurfaces; i++ )
        {
            surfaceType_t surfaceType = *worldData->surfaces[i].surfaceType;
            switch ( surfaceType )
            {
                case SF_FACE:
                {
                    srfSurfaceFace_t *s = (srfSurfaceFace_t *)worldData->surfaces[i].surfaceType;
                    numVerts += s->numPoints;
                }
                break;
                
                case SF_TRIANGLES:
                {
                    srfTriangles_t *s = (srfTriangles_t *)worldData->surfaces[i].surfaceType;
                    numVerts += s->numVerts;
                }
                break;
                
                case SF_GRID:
                {
                    srfGridMesh_t *s = (srfGridMesh_t *)worldData->surfaces[i].surfaceType;
                    numVerts += s->width * s->height;
                }
                break;
                
                default:
                break;
            }
        }
    }
}
