#pragma once

#include "ghoul2/G2.h"

extern cvar_t	*fx_debug;

#ifdef _DEBUG
extern cvar_t	*fx_freeze;
#endif

extern cvar_t	*fx_countScale;
extern cvar_t	*fx_nearCull;

#ifndef ENGINE
typedef vec_t vec2_t[2];	// ya well, I guess this needed redefining for some reason --eez
#endif
#ifdef ENGINE
ID_INLINE void Vector2Clear(vec2_t a)
{
	a[0] = 0.0f;
	a[1] = 0.0f;
}

ID_INLINE void Vector2Set(vec2_t a,float b,float c)
{
	a[0] = b;
	a[1] = c;
}

ID_INLINE void Vector2Copy(vec2_t src,vec2_t dst)
{
	dst[0] = src[0];
	dst[1] = src[1];
}

ID_INLINE void Vector2MA(vec2_t src, float m, vec2_t v, vec2_t dst)
{
	dst[0] = src[0] + (m*v[0]);
	dst[1] = src[1] + (m*v[1]);
}

ID_INLINE void Vector2Scale(vec2_t src,float b,vec2_t dst)
{
	dst[0] = src[0] * b;
	dst[1] = src[1] * b;
}
#endif
class SFxHelper
{
public:
	int		mTime;
	int		mOldTime;
	int		mFrameTime;
	bool	mTimeFrozen;
	float	mRealTime;
	refdef_t*	refdef;
#ifdef _DEBUG
	int		mMainRefs;
	int		mMiniRefs;
#endif

public:
	SFxHelper(void);

	ID_INLINE	int	GetTime(void) { return mTime; }
	ID_INLINE	int	GetFrameTime(void) { return mFrameTime; }

	void	ReInit(refdef_t* pRefdef);
	void	AdjustTime( int time );

	// These functions are wrapped and used by the fx system in case it makes things a bit more portable
	void	Print( const char *msg, ... );

	// File handling
	ID_INLINE	int		OpenFile( const char *path, fileHandle_t *fh, int mode )
	{
#ifdef ENGINE
		return FS_FOpenFileByMode( path, fh, FS_READ );
#else
		return trap_FS_FOpenFile( path, fh, FS_READ );
#endif
	}
	ID_INLINE	int		ReadFile( void *data, int len, fileHandle_t fh )
	{
#ifdef ENGINE
		FS_Read2( data, len, fh );
#else
		trap_FS_Read( data, len, fh );
#endif
		return 1;
	}
	ID_INLINE	void	CloseFile( fileHandle_t fh )
	{
#ifdef ENGINE
		FS_FCloseFile( fh );
#else
		trap_FS_FCloseFile( fh );
#endif
	}

	// Sound
	ID_INLINE	void	PlaySound( vec3_t origin, int entityNum, int entchannel, sfxHandle_t sfxHandle, int volume, int radius )
	{
		//S_StartSound( origin, ENTITYNUM_NONE, CHAN_AUTO, sfxHandle, volume, radius );
#ifdef ENGINE
		S_StartSound( origin, ENTITYNUM_NONE, CHAN_AUTO, sfxHandle );
#else
		trap_S_StartSound( origin, ENTITYNUM_NONE, CHAN_AUTO, sfxHandle );
#endif
	}
	ID_INLINE	void	PlayLocalSound(sfxHandle_t sfxHandle, int entchannel)
	{
		//S_StartSound( origin, ENTITYNUM_NONE, CHAN_AUTO, sfxHandle, volume, radius );
#ifdef ENGINE
		S_StartLocalSound(sfxHandle, entchannel);
#else
		trap_S_StartLocalSound( sfxHandle, entchannel );
#endif
	}
	ID_INLINE	int		RegisterSound( const char *sound )
	{
#ifdef ENGINE
		return S_RegisterSound( sound );
#else
		trap_S_RegisterSound( sound );
#endif
	}

	// Physics/collision
	ID_INLINE	void	Trace( trace_t &tr, vec3_t start, vec3_t min, vec3_t max, vec3_t end, int skipEntNum, int flags )
	{
#ifdef ENGINE
		TCGTrace		*td = (TCGTrace *)cl.mSharedMemory;
#else
		TCGTrace		*td = (TCGTrace *)trap_FX_GetSharedMemory();
#endif

		if ( !min )
		{
			min = vec3_origin;
		}

		if ( !max )
		{
			max = vec3_origin;
		}

		memset(td, sizeof(*td), 0);
		VectorCopy(start, td->mStart);
		VectorCopy(min, td->mMins);
		VectorCopy(max, td->mMaxs);
		VectorCopy(end, td->mEnd);
		td->mSkipNumber = skipEntNum;
		td->mMask = flags;

#ifdef ENGINE
		VM_Call( cgvm, CG_TRACE );
#else
		vmMain( CG_TRACE, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
#endif

		tr = td->mResult;
	}

	ID_INLINE	void	G2Trace( trace_t &tr, vec3_t start, vec3_t min, vec3_t max, vec3_t end, int skipEntNum, int flags )
	{
#ifdef ENGINE
		TCGTrace		*td = (TCGTrace *)cl.mSharedMemory;
#else
		TCGTrace		*td = (TCGTrace *)trap_FX_GetSharedMemory();
#endif

		if ( !min )
		{
			min = vec3_origin;
		}

		if ( !max )
		{
			max = vec3_origin;
		}

		memset(td, sizeof(*td), 0);
		VectorCopy(start, td->mStart);
		VectorCopy(min, td->mMins);
		VectorCopy(max, td->mMaxs);
		VectorCopy(end, td->mEnd);
		td->mSkipNumber = skipEntNum;
		td->mMask = flags;

#ifdef ENGINE
		VM_Call( cgvm, CG_G2TRACE );
#else
		vmMain( CG_G2TRACE, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
#endif

		tr = td->mResult;
	}

	ID_INLINE	void	AddGhoul2Decal(int shader, vec3_t start, vec3_t dir, float size)
	{
#ifdef ENGINE
		TCGG2Mark		*td = (TCGG2Mark *)cl.mSharedMemory;
#else
		TCGG2Mark		*td = (TCGG2Mark *)trap_FX_GetSharedMemory();
#endif

		td->size = size;
		td->shader = shader;
		VectorCopy(start, td->start);
		VectorCopy(dir, td->dir);

#ifdef ENGINE
		VM_Call(cgvm, CG_G2MARK);
#else
		vmMain( CG_G2MARK, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
#endif
	}

	ID_INLINE	void	AddFxToScene( refEntity_t *ent )
	{
#ifdef _DEBUG
		mMainRefs++;

		assert(!ent || ent->renderfx >= 0);
#endif
#ifdef ENGINE
		re.AddRefEntityToScene( ent );
#else
		trap_R_AddRefEntityToScene( ent );
#endif
	}
	ID_INLINE	void	AddFxToScene( miniRefEntity_t *ent )
	{
#ifdef _DEBUG
		mMiniRefs++;

		assert(!ent || ent->renderfx >= 0);
#endif
#ifdef ENGINE
		re.AddMiniRefEntityToScene( ent );
#else
		trap_R_AddMiniRefEntityToScene( ent );
#endif
	}
#ifndef VV_LIGHTING
	ID_INLINE	void	AddLightToScene( vec3_t org, float radius, float red, float green, float blue )
	{
#ifdef ENGINE
		re.AddLightToScene(	org, radius, red, green, blue );
#else
		trap_R_AddLightToScene( org, radius, red, green, blue );
#endif
	}
#endif

	ID_INLINE	int		RegisterShader( const char *shader )
	{
#ifdef ENGINE
		return re.RegisterShader( shader );
#else
		return (int)trap_R_RegisterShader( shader );
#endif
	}
	ID_INLINE	int		RegisterModel( const char *model )
	{
#ifdef ENGINE
		return re.RegisterModel( model );
#else
		return (int)trap_R_RegisterModel( model );
#endif
	}

	ID_INLINE	void	AddPolyToScene( int shader, int count, polyVert_t *verts )
	{
#ifdef ENGINE
		re.AddPolyToScene( shader, count, verts, 1 );
#else
		trap_R_AddPolyToScene( shader, count, verts );
#endif
	}

	ID_INLINE void AddDecalToScene ( qhandle_t shader, const vec3_t origin, const vec3_t dir, float orientation, float r, float g, float b, float a, qboolean alphaFade, float radius, qboolean temporary )
	{
#ifdef ENGINE
		re.AddDecalToScene ( shader, origin, dir, orientation, r, g, b, a, alphaFade, radius, temporary );
#else
		trap_R_AddDecalToScene ( shader, origin, dir, orientation, r, g, b, a, alphaFade, radius, temporary );
#endif
	}

	void	CameraShake( vec3_t origin, float intensity, int radius, int time );
	qboolean GetOriginAxisFromBolt(CGhoul2Info_v *pGhoul2, int mEntNum, int modelNum, int boltNum, vec3_t /*out*/origin, vec3_t /*out*/axis[3]);
};

extern SFxHelper	theFxHelper;
