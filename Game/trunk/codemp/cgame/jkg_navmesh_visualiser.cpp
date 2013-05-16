#ifdef __DISABLED__

#include <DebugDraw.h>
#include <Recast.h>
#include <RecastDebugDraw.h>

extern "C"
{
    #include "cg_local.h"
}

// JKG: Nav mesh visualization
// Hurr, copy and paste for now
struct navMeshDataHeader_t
{
    int version;
    int filesize;
    
    // Poly Mesh
    int vertsOffset;
    int numVerts;
    int polysOffset;
    int areasOffset;
    int flagsOffset;
    int numPolys;
    int numVertsPerPoly;
    vec3_t mins, maxs;
    
    // Detailed Mesh
    int dMeshesOffset;
    int dNumMeshes;
    int dVertsOffset;
    int dNumVerts;
    int dTrisOffset;
    int dNumTris;
    
    // Cell size
    float cellSize;
    float cellHeight;
};

struct navMeshVisual_t
{
    // Poly mesh
    rcPolyMesh polyMesh;
    unsigned short *verts;
    unsigned short *polys;
    unsigned char *areas;
    unsigned short *flags;
    
    // Detailed poly mesh
    rcPolyMeshDetail detailedPolyMesh;
    unsigned int *meshes;
    float *dVerts;
    unsigned char *tris;
};

struct jkgNavmeshDebugDraw : public duDebugDraw
{
private:
    static const int MAX_VERTS = 3000;

    duDebugDrawPrimitives currentPrimitive;
    float size;
    polyVert_t verts[MAX_VERTS];
    int numVerts;

public:
    jkgNavmeshDebugDraw():
        currentPrimitive (DU_DRAW_TRIS),
        numVerts (0)
    {
    }
    
    ~jkgNavmeshDebugDraw(){}
	
	void depthMask(bool state){}

	void texture(bool state){}

	// Begin drawing primitives.
	// Params:
	//  prim - (in) primitive type to draw, one of rcDebugDrawPrimitives.
	//  nverts - (in) number of vertices to be submitted.
	//  size - (in) size of a primitive, applies to point size and line width only.
	void begin(duDebugDrawPrimitives prim, float size = 1.0f)
	{
	    currentPrimitive = prim;
	    this->size = size;
	    numVerts = 0;
	}

	// Submit a vertex
	// Params:
	//  pos - (in) position of the verts.
	//  color - (in) color of the verts.
	void vertex(const float* pos, unsigned int color)
	{
	    if ( numVerts >= MAX_VERTS )
	    {
	        return;
	    }
	    
	    verts[numVerts].xyz[0] = -pos[0];
	    verts[numVerts].xyz[1] = -pos[2];
	    verts[numVerts].xyz[2] = pos[1];
	    
	    VectorClear (verts[numVerts].st);
	    verts[numVerts].modulate[0] = (color >> 0) & 0xff;
	    verts[numVerts].modulate[1] = (color >> 8) & 0xff;
	    verts[numVerts].modulate[2] = (color >> 16) & 0xff;
	    verts[numVerts].modulate[3] = (color >> 24) & 0xff;
	    
	    numVerts++;
	}

	// Submit a vertex
	// Params:
	//  x,y,z - (in) position of the verts.
	//  color - (in) color of the verts.
	void vertex(const float x, const float y, const float z, unsigned int color)
	{
	    float v[3];
	    v[0] = x; v[1] = y; v[2] = z;
	    vertex (v, color);
	}
	
	// Submit a vertex
	// Params:
	//  pos - (in) position of the verts.
	//  color - (in) color of the verts.
	void vertex(const float* pos, unsigned int color, const float* uv)
	{
	    vertex (pos, color);
	}
	
	// Submit a vertex
	// Params:
	//  x,y,z - (in) position of the verts.
	//  color - (in) color of the verts.
	void vertex(const float x, const float y, const float z, unsigned int color, const float u, const float v)
	{
	    float p[3];
	    p[0] = x; p[1] = y; p[2] = z;
	    
	    float uv[2];
	    uv[0] = u; uv[1] = v;
	    
	    vertex (p, color, uv);
	}
	
	// End drawing primitives.
	void end()
	{
	    refEntity_t re;
	    
	    switch ( currentPrimitive )
	    {
	        case DU_DRAW_TRIS:
	            trap_R_AddPolysToScene (cgs.media.whiteShader, 3, verts, numVerts / 3);
	            break;
	            
            case DU_DRAW_QUADS:
	            trap_R_AddPolysToScene (cgs.media.whiteShader, 4, verts, numVerts / 4);
	            break;
	            
	        case DU_DRAW_LINES:
	            memset (&re, 0, sizeof (re));
	            re.reType = RT_LINE;
	            re.radius = size * 0.5f;
	            re.customShader = cgs.media.whiteShader;
	            re.shaderTime = cg.time * 0.001f;
	            for ( int i = 0; i < numVerts; i += 2 )
	            {
	                re.shaderTexCoord[0] = 1.0f;
	                re.shaderTexCoord[1] = 1.0f;
	                VectorCopy (verts[i].xyz, re.origin);
	                VectorCopy (verts[i + 1].xyz, re.oldorigin);
	                VectorCopy4 (verts[i].modulate, re.shaderRGBA);
	                if ( re.shaderRGBA[3] < 50 ) re.shaderRGBA[3] += 50;
	                
	                trap_R_AddRefEntityToScene (&re);
	            }
	            break;

            default:
                break;
	    }
	}
};

static navMeshDataHeader_t navMeshHeader;
static navMeshVisual_t navMeshVisual;
static jkgNavmeshDebugDraw navMeshDrawer;

#endif //__DISABLED__

extern "C"
{
	void JKG_Nav_Init ( const char *mapname )
	{
#ifdef __DISABLED__
		fileHandle_t f;
		char *buffer;
		int size = 0;
		int fileLen = trap_FS_FOpenFile (va ("%s.jnd", mapname), &f, FS_READ);

		if ( fileLen == -1 || !f )
		{
			CG_Printf ("^1No navigation mesh cache found for this map.\n");
			return;
		}

		buffer = (char *)malloc (fileLen + 1);
		trap_FS_Read (buffer, fileLen, f);
		buffer[fileLen] = '\0';
		trap_FS_FCloseFile (f);

		navMeshHeader = *(navMeshDataHeader_t *)buffer;

		size = sizeof (unsigned short) * 3 * navMeshHeader.numVerts;
		navMeshVisual.verts = (unsigned short *)malloc (size);
		memcpy (navMeshVisual.verts, &buffer[navMeshHeader.vertsOffset], size);

		size = sizeof (unsigned short) * navMeshHeader.numPolys * navMeshHeader.numVertsPerPoly * 2;
		navMeshVisual.polys = (unsigned short *)malloc (size);
		memcpy (navMeshVisual.polys, &buffer[navMeshHeader.polysOffset], size);

		size = navMeshHeader.numPolys * sizeof (unsigned char);
		navMeshVisual.areas = (unsigned char *)malloc (size);
		memcpy (navMeshVisual.areas, &buffer[navMeshHeader.areasOffset], size);

		size = navMeshHeader.numPolys * sizeof (unsigned short);
		navMeshVisual.flags = (unsigned short *)malloc (size);
		memcpy (navMeshVisual.flags, &buffer[navMeshHeader.flagsOffset], size);

		size = navMeshHeader.dNumMeshes * sizeof (unsigned int) * 4;
		navMeshVisual.meshes = (unsigned int *)malloc (size);
		memcpy (navMeshVisual.meshes, &buffer[navMeshHeader.dMeshesOffset], size);

		size = navMeshHeader.dNumVerts * sizeof (float) * 3;
		navMeshVisual.dVerts = (float *)malloc (size);
		memcpy (navMeshVisual.dVerts, &buffer[navMeshHeader.dVertsOffset], size);

		size = navMeshHeader.dNumTris * sizeof (unsigned char) * 3;
		navMeshVisual.tris = (unsigned char *)malloc (size);
		memcpy (navMeshVisual.tris, &buffer[navMeshHeader.dTrisOffset], size);

		navMeshVisual.polyMesh.areas = navMeshVisual.areas;
		navMeshVisual.polyMesh.bmax[0] = navMeshHeader.maxs[0];
		navMeshVisual.polyMesh.bmax[1] = navMeshHeader.maxs[1];
		navMeshVisual.polyMesh.bmax[2] = navMeshHeader.maxs[2];
		navMeshVisual.polyMesh.bmin[0] = navMeshHeader.mins[0];
		navMeshVisual.polyMesh.bmin[1] = navMeshHeader.mins[1];
		navMeshVisual.polyMesh.bmin[2] = navMeshHeader.mins[2];
		navMeshVisual.polyMesh.ch = navMeshHeader.cellHeight;
		navMeshVisual.polyMesh.cs = navMeshHeader.cellSize;
		navMeshVisual.polyMesh.flags = navMeshVisual.flags;
		navMeshVisual.polyMesh.maxpolys = navMeshVisual.polyMesh.npolys = navMeshHeader.numPolys;
		navMeshVisual.polyMesh.nverts = navMeshHeader.numVerts;
		navMeshVisual.polyMesh.nvp = navMeshHeader.numVertsPerPoly;
		navMeshVisual.polyMesh.polys = navMeshVisual.polys;
		navMeshVisual.polyMesh.regs = NULL;
		navMeshVisual.polyMesh.verts = navMeshVisual.verts;

		free (buffer);

		CG_Printf ("^2Navigation mesh cache found for this map.\n");
#endif //__DISABLED__
	}

	void JKG_Nav_Destroy ( void )
	{
#ifdef __DISABLED__
		free (navMeshVisual.areas);
		free (navMeshVisual.dVerts);
		free (navMeshVisual.flags);
		free (navMeshVisual.meshes);
		free (navMeshVisual.polys);
		free (navMeshVisual.tris);
		free (navMeshVisual.verts);
#endif //__DISABLED__
	}

#ifdef __DISABLED__
	extern vmCvar_t jkg_debugNavmesh;
#endif //__DISABLED__

	void JKG_Nav_VisualizeMesh ( void )
	{
#ifdef __DISABLED__
		if ( !jkg_debugNavmesh.integer )
		{
			return;
		}

		if ( !navMeshHeader.version )
		{
			return;
		}

		duDebugDrawPolyMesh (&navMeshDrawer, navMeshVisual.polyMesh);
#endif //__DISABLED__
	}
}
