#pragma once

#include "ghoul2/G2.h"

extern cvar_t	*fx_debug;

#ifdef _DEBUG
extern cvar_t	*fx_freeze;
#endif

extern cvar_t	*fx_countScale;
extern cvar_t	*fx_nearCull;

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
		return FS_FOpenFileByMode( path, fh, FS_READ );
	}
	ID_INLINE	int		ReadFile( void *data, int len, fileHandle_t fh )
	{
		FS_Read2( data, len, fh );
		return 1;
	}
	ID_INLINE	void	CloseFile( fileHandle_t fh )
	{
		FS_FCloseFile( fh );
	}

	// Sound
	ID_INLINE	void	PlaySound( vec3_t origin, int entityNum, int entchannel, sfxHandle_t sfxHandle, int volume, int radius )
	{
		//S_StartSound( origin, ENTITYNUM_NONE, CHAN_AUTO, sfxHandle, volume, radius );
		S_StartSound( origin, ENTITYNUM_NONE, CHAN_AUTO, sfxHandle );
	}
	ID_INLINE	void	PlayLocalSound(sfxHandle_t sfxHandle, int entchannel)
	{
		//S_StartSound( origin, ENTITYNUM_NONE, CHAN_AUTO, sfxHandle, volume, radius );
		S_StartLocalSound(sfxHandle, entchannel);
	}
	ID_INLINE	int		RegisterSound( const char *sound )
	{
		return S_RegisterSound( sound );
	}

	// Physics/collision
	ID_INLINE	void	Trace( trace_t &tr, vec3_t start, vec3_t min, vec3_t max, vec3_t end, int skipEntNum, int flags )
	{
		TCGTrace		*td = (TCGTrace *)cl.mSharedMemory;

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

		VM_Call( cgvm, CG_TRACE );

		tr = td->mResult;
	}

	ID_INLINE	void	G2Trace( trace_t &tr, vec3_t start, vec3_t min, vec3_t max, vec3_t end, int skipEntNum, int flags )
	{
		TCGTrace		*td = (TCGTrace *)cl.mSharedMemory;

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

		VM_Call( cgvm, CG_G2TRACE );

		tr = td->mResult;
	}

	ID_INLINE	void	AddGhoul2Decal(int shader, vec3_t start, vec3_t dir, float size)
	{
		TCGG2Mark		*td = (TCGG2Mark *)cl.mSharedMemory;

		td->size = size;
		td->shader = shader;
		VectorCopy(start, td->start);
		VectorCopy(dir, td->dir);

		VM_Call(cgvm, CG_G2MARK);
	}

	ID_INLINE	void	AddFxToScene( refEntity_t *ent )
	{
#ifdef _DEBUG
		mMainRefs++;

		assert(!ent || ent->renderfx >= 0);
#endif
		re.AddRefEntityToScene( ent );
	}
	ID_INLINE	void	AddFxToScene( miniRefEntity_t *ent )
	{
#ifdef _DEBUG
		mMiniRefs++;

		assert(!ent || ent->renderfx >= 0);
#endif
		re.AddMiniRefEntityToScene( ent );
	}
#ifndef VV_LIGHTING
	ID_INLINE	void	AddLightToScene( vec3_t org, float radius, float red, float green, float blue )
	{
		re.AddLightToScene(	org, radius, red, green, blue );
	}
#endif

	ID_INLINE	int		RegisterShader( const char *shader )
	{
		return re.RegisterShader( shader );
	}
	ID_INLINE	int		RegisterModel( const char *model )
	{
		return re.RegisterModel( model );
	}

	ID_INLINE	void	AddPolyToScene( int shader, int count, polyVert_t *verts )
	{
		re.AddPolyToScene( shader, count, verts, 1 );
	}

	ID_INLINE void AddDecalToScene ( qhandle_t shader, const vec3_t origin, const vec3_t dir, float orientation, float r, float g, float b, float a, qboolean alphaFade, float radius, qboolean temporary )
	{
		re.AddDecalToScene ( shader, origin, dir, orientation, r, g, b, a, alphaFade, radius, temporary );
	}

	void	CameraShake( vec3_t origin, float intensity, int radius, int time );
	qboolean GetOriginAxisFromBolt(CGhoul2Info_v *pGhoul2, int mEntNum, int modelNum, int boltNum, vec3_t /*out*/origin, vec3_t /*out*/axis[3]);
};

extern SFxHelper	theFxHelper;
