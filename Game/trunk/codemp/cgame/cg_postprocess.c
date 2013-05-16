/*============================================================
//
//  Post Processing module
//
//  $Id: cg_postprocess.c 426 2012-08-01 07:20:12Z eezstreet $
//
//============================================================*/

// CAUTION: Utilizes engine hacks, compatible with JA 1.01 win32 ONLY!

#include <GL/glew.h>
#include "jkg_glcommon.h"
#include <math.h>
#include "cg_local.h"
#include "jkg_eshader.h"
#include "cg_postprocess.h"
#include "jkg_pixelshaders.h"
#include "jkg_framebuffers.h"

#pragma comment(lib, "OpenGL32.lib")

vmCvar_t	jkg_postprocess;

typedef struct quadTextureData_s {
    unsigned int numTexturesUsed;
    const texture_t *textures[16];
} quadTextureData_t;

// Have this here to prevent the need to create a new one every pass.
static quadTextureData_t quadTextureData;

void (__cdecl * R_SyncRenderThread)(void) = (void(__cdecl *)(void))0x4915B0;
void (__cdecl * RB_EndSurface)(void) = (void(__cdecl *)(void))0x4AF530;
void (__cdecl * RB_SetGL2D)(void) = (void(__cdecl *)(void))0x48B070;
cvar_t * cl_avidemo;

//qhandle_t mbshader;
//image_t *mbshaderimg;

// Color mod and blur
//qhandle_t cmshader;
//image_t *cmshaderimg;
gshshader_t shColorMod;
gshshader_t shGrayscale;
gshshader_t shSepia;
gshshader_t shBlurX;
gshshader_t shBlurY;
gshshader_t shNoise;
gshshader_t shMotionBlur;
#ifndef BLOOM
gshshader_t shBrightPass;
gshshader_t shBloom;
#endif
#ifdef __GL_ANAGLYPH__
gshshader_t shAnaglyph;
#endif //__GL_ANAGLYPH__
#ifdef __GL_EMBOSS__
gshshader_t shEmboss;
#endif //__GL_EMBOSS__
//GLuint cmshader;

gshshader_t shGenericOutput;

typedef void (*PostProcessFunction)(const void *data);
typedef struct postProcessEffect_s {
    char name[64];
    PostProcessFunction func;
} postProcessEffect_t;

typedef struct postProcessPass_s {
    const postProcessEffect_t *effect;
    const void *data;
} postProcessPass_t;

static void PP_RenderColorMod (const void *data);
static void PP_RenderMotionBlur (const void *data);
static void PP_RenderGaussianBlur (const void *data);
#ifndef BLOOM
static void PP_RenderBloom (const void *data);
#endif
#ifdef __GL_ANAGLYPH__
static void PP_RenderAnaglyph (const void *data);
#endif //__GL_ANAGLYPH__
#ifdef __GL_EMBOSS__
static void PP_RenderEmboss (const void *data);
#endif //__GL_EMBOSS__

static const postProcessEffect_t postProcessEffects[] = {
    { "colormod", PP_RenderColorMod },
    { "motionblur", PP_RenderMotionBlur },
    { "gaussianblur", PP_RenderGaussianBlur },
#ifndef BLOOM
    { "bloom", PP_RenderBloom },
#endif
#ifdef __GL_ANAGLYPH__
	{ "anaglyph", PP_RenderAnaglyph },
#endif //__GL_ANAGLYPH__
#ifdef __GL_EMBOSS__
	{ "emboss", PP_RenderEmboss },
#endif //__GL_EMBOSS__
};
static const unsigned int numEffects = sizeof (postProcessEffects) / sizeof (postProcessEffect_t);

static framebuffer_t *fboMain; // The FBO to hold the original rendered scene.

// For post-processing pipeline
static framebuffer_t *fboPingPong[2]; // Ping pong
static const framebuffer_t *fboLastUsed; // Last FBO used in post-processing system.
static const framebuffer_t *fboToUse;
static framebuffer_t *fboMotionBlur;
static framebuffer_t *fboBrightPass;
static unsigned int currentPingPong; // 0 is main FBO, 1 is the other one.

static postProcessPass_t postProcessQueue[8]; // Circular queue
static unsigned int postProcessQueueSize = sizeof (postProcessQueue) / sizeof (postProcessPass_t);
static unsigned int postProcessQueueHead;
static unsigned int postProcessQueueTail;

static int PPInited = 0;
static unsigned int sceneDepth;

#define GL_BIND_ADDRESS 0x489FA0
void GL_Bind ( unsigned int textureId )
{
    static image_t image;
    image.texnum = textureId;
    image.frameUsed = 0;

    __asm
    {
        mov esi, offset image
        mov eax, GL_BIND_ADDRESS
        call eax
    }
}

#define GL_SELECTTEXTURE_ADDRESS 0x48A010
int GL_SelectTexture ( int textureUnit )
{
    __asm
    {
        mov esi, textureUnit
        mov eax, GL_SELECTTEXTURE_ADDRESS
        call eax
    }
}

int PP_InitPostProcess() {
    CG_Printf("Initializing Post Processing...\n");
    PPInited = 0;
    
    if ( glewInit() != GLEW_OK )
    {
        CG_Printf (S_COLOR_RED "...Unable to initialize OpenGL extension loader.\n");
        return 0;
    }
    
	if ( !GSH_Init() )
	{
	    CG_Printf (S_COLOR_RED "...GPU shaders not supported. Post processing will not be used.\n");
	    return 0;
	}
	
	if ( !FBO_FramebufferInit() )
	{
	    CG_Printf (S_COLOR_RED "...Framebuffers not supported. Post processing will not be used.\n");
	    return 0;
	}
	
	cl_avidemo = *(cvar_t **)0xB25CFC;
	
	GSH_LoadShader(&shColorMod, "gsh/pp/colormod.gsh");
	GSH_LoadShader(&shGrayscale, "gsh/pp/grayscale.gsh");
	GSH_LoadShader(&shSepia, "gsh/pp/sepia.gsh");
	GSH_LoadShader(&shBlurX, "gsh/pp/blurx.gsh");
	GSH_LoadShader(&shBlurY, "gsh/pp/blury.gsh");
	GSH_LoadShader(&shNoise, "gsh/pp/noise.gsh");
	
	GSH_LoadShader(&shMotionBlur, "gsh/pp/motionblur.gsh");
	GSH_UseShader (&shMotionBlur);
	GSH_SetUniform1i ("sceneTexture", 0);
	GSH_SetUniform1i ("accumTexture", 1);
#ifndef BLOOM
	GSH_LoadShader(&shBrightPass, "gsh/pp/brightpass.fsh");
	
	GSH_LoadShader(&shBloom, "gsh/pp/bloom.gsh");
	GSH_UseShader(&shBloom);
	GSH_SetUniform1i ("sceneTexture", 0);
	GSH_SetUniform1i ("bloomTexture", 1);
#endif
#ifdef __GL_ANAGLYPH__
	GSH_LoadShader(&shAnaglyph, "gsh/pp/anaglyph.gsh");
#endif //__GL_ANAGLYPH__
#ifdef __GL_EMBOSS__
	GSH_LoadShader(&shEmboss, "gsh/pp/emboss.gsh");
#endif //__GL_EMBOSS__
	
	GSH_LoadShader(&shGenericOutput, "gsh/generic.gsh");
	CheckGLErrors (__FILE__, __LINE__);
	GSH_UseShader (&shGenericOutput);
	CheckGLErrors (__FILE__, __LINE__);
	GSH_SetUniform1i ("u_DiffuseMap", 0);
	
	CheckGLErrors (__FILE__, __LINE__);
	
	GSH_UseShader (NULL);
	
	//----------------------
	fboLastUsed = 0;
	fboMain = FBO_CreateFramebuffer();
	FBO_BindFramebuffer (fboMain);
	FBO_AttachColorTextureToFramebuffer (fboMain, FBO_CreateBlankTexture (cgs.glconfig.vidWidth, cgs.glconfig.vidHeight, IF_RGBA8), 0);
	FBO_AttachDepthTextureToFramebuffer (fboMain,
	    FBO_CreateBlankTexture (cgs.glconfig.vidWidth,
	        cgs.glconfig.vidHeight,
	        cgs.glconfig.depthBits == 16 ? IF_DEPTH_COMPONENT16 : IF_DEPTH_COMPONENT24
	    )
	);
	FBO_CheckFramebuffer (fboMain);
	sceneDepth = fboMain->depthTexture;
	
	//----------------------
	fboPingPong[0] = fboMain;
	
	//----------------------
	fboPingPong[1] = FBO_CreateFramebuffer();
	FBO_BindFramebuffer (fboPingPong[1]);	
	FBO_AttachColorTextureToFramebuffer (fboPingPong[1], FBO_CreateBlankTexture (cgs.glconfig.vidWidth, cgs.glconfig.vidHeight, IF_RGBA8), 0);
	FBO_CheckFramebuffer (fboPingPong[1]);
	
	//----------------------
	fboMotionBlur = FBO_CreateFramebuffer();
	FBO_BindFramebuffer (fboMotionBlur);
	FBO_AttachColorTextureToFramebuffer (fboMotionBlur, FBO_CreateBlankTexture (cgs.glconfig.vidWidth, cgs.glconfig.vidHeight, IF_RGBA8), 0);
	FBO_CheckFramebuffer (fboMotionBlur);
	
	//----------------------
	fboBrightPass = FBO_CreateFramebuffer();
	FBO_BindFramebuffer (fboBrightPass);
	FBO_AttachColorTextureToFramebuffer (fboBrightPass, FBO_CreateBlankTexture (cgs.glconfig.vidWidth * 0.25f, cgs.glconfig.vidHeight * 0.25f, IF_RGBA8), 0);
	FBO_CheckFramebuffer (fboBrightPass);
	
	//----------------------
	FBO_BindDefaultFramebuffer();
	
	memset (postProcessQueue, 0, sizeof (postProcessQueue));
	postProcessQueueHead = 0;
	postProcessQueueTail = 0;
	
	PPInited = 1;
	CG_Printf("Initialized successfully\n");
	return 1;
}

void PP_TerminatePostProcess() {
    if (!PPInited || !jkg_postprocess.integer) return;
    
	GSH_UseShader(NULL);
	GSH_FreeShader(&shColorMod);
	GSH_FreeShader(&shGrayscale);
	GSH_FreeShader(&shSepia);
	GSH_FreeShader(&shBlurX);
	GSH_FreeShader(&shBlurY);
	GSH_FreeShader(&shNoise);
	GSH_FreeShader(&shMotionBlur);
	//GSH_FreeShader(&shBrightPass);
	//GSH_FreeShader(&shBloom);
#ifdef __GL_ANAGLYPH__
	GSH_FreeShader(&shAnaglyph);
#endif //__GL_ANAGLYPH__
#ifdef __GL_EMBOSS__
	GSH_FreeShader(&shEmboss);
#endif //__GL_EMBOSS__
	
	GSH_FreeShader(&shGenericOutput);
	
	FBO_FramebufferCleanup();
	
	PPInited = 0;
}

static int VectorCompareT(vec3_t v1, vec3_t v2, double tolerance) {
	if (fabs(v1[0] - v2[0]) > tolerance) return 0;
	if (fabs(v1[1] - v2[1]) > tolerance) return 0;
	if (fabs(v1[2] - v2[2]) > tolerance) return 0;
	return 1;
}

extern trRefEntity_t **backend_currentEntity;
extern vmCvar_t jkg_normalMapping;
void JKG_BeginGenericShader ( void )
{
    trRefEntity_t *ent = *backend_currentEntity;
    if (!PPInited || !jkg_postprocess.integer) return;
    
    if ( !jkg_normalMapping.integer )
    {
        return;
    }
    
    if ( ent && (ent->e.hModel || ent->e.ghoul2) )
    {
        GSH_UseShader (&shGenericOutput);
        
        GSH_SetUniform3f ("u_LightAmbientColor", ent->ambientLight[0] / 255.0f, ent->ambientLight[1] / 255.0f, ent->ambientLight[2] / 255.0f);
        GSH_SetUniform3f ("u_LightColor", ent->directedLight[0] / 255.0f, ent->directedLight[1] / 255.0f, ent->directedLight[2] / 255.0f);
        GSH_SetUniform3f ("u_LightDirection", ent->lightDir[0], ent->lightDir[1], ent->lightDir[2]);
    }
}

void JKG_EndGenericShader ( void )
{
    if (!PPInited || !jkg_postprocess.integer) return;
    
    GSH_UseShader (NULL);
}

//=========================================================
// PP_RenderTexturedQuad
//---------------------------------------------------------
// Description:
// Renders a textured quad to the screen.
//=========================================================
static void PP_RenderTexturedQuad (const quadTextureData_t *textureData, float x, float y, float width, float height, float s1, float t1, float s2, float t2) {
    int i;
    
    if (!PPInited || !jkg_postprocess.integer) return;

    if (!*(int *)0xFE2A6C) {	// if (!backEnd.projection2D) {
		RB_SetGL2D();			// Set up the proper enviroment for 2D rendering
	}
    
    for (i = 0; i < textureData->numTexturesUsed; i++) {
        GL_SelectTexture (i);
        GL_Bind (textureData->textures[i]->id);
    }

    glBegin (GL_QUADS);
        for (i = 0; i < textureData->numTexturesUsed; i++) {
            glMultiTexCoord2fARB (GL_TEXTURE0_ARB + i, s1, t1);
        }
        glVertex2f (x, y);
        
        for (i = 0; i < textureData->numTexturesUsed; i++) {
            glMultiTexCoord2fARB (GL_TEXTURE0_ARB + i, s2, t1);
        }
        glVertex2f (x + width, y);
        
        for (i = 0; i < textureData->numTexturesUsed; i++) {
            glMultiTexCoord2fARB (GL_TEXTURE0_ARB + i, s2, t2);
        }
        glVertex2f (x + width, y + height);
        
        for (i = 0; i < textureData->numTexturesUsed; i++) {
            glMultiTexCoord2fARB (GL_TEXTURE0_ARB + i, s1, t2);
        }
        glVertex2f (x, y + height);
    glEnd();
    
    GL_SelectTexture (0);
}

//=========================================================
// PP_RenderFullscreenTexturedQuad
//---------------------------------------------------------
// Description:
// Renders a fullscreen textured quad.
//=========================================================
static void PP_RenderFullscreenTexturedQuad (const quadTextureData_t *textureData) {
    float w_ratio;
    float h_ratio;
    
    if (!PPInited || !jkg_postprocess.integer) return;
    
    w_ratio = (float)cgs.glconfig.vidWidth / (float)textureData->textures[0]->width;
    h_ratio = (float)cgs.glconfig.vidHeight / (float)textureData->textures[0]->height;
    
    PP_RenderTexturedQuad (textureData, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, h_ratio, w_ratio, 0.0f);
}

//=========================================================
// PP_RenderEffectToTexture
//---------------------------------------------------------
// Description:
// Renders the effect to the next texture. FBOs are swapped
// internally.
//=========================================================
static void PP_RenderEffectToTexture (const quadTextureData_t *textureData, const framebuffer_t *destFBO) {
    if (!PPInited || !jkg_postprocess.integer) return;

    FBO_BindFramebuffer (destFBO);
    PP_RenderFullscreenTexturedQuad (textureData);

    if (destFBO == fboToUse) {
        fboLastUsed = fboPingPong[currentPingPong];

        currentPingPong = (currentPingPong + 1) % 2;
        fboToUse = fboPingPong[currentPingPong];
    } else {
        fboLastUsed = destFBO;
    }
}

//=========================================================
// PP_RenderMotionBlur
//---------------------------------------------------------
// Description:
// Carries out the motion blur pass.
//=========================================================
static void PP_RenderMotionBlur (const void *data) {
	float framecount;
	static int prevtime = 0;
	static int prevfaderate = 0;
	
	static int camidle = 0;
	static float camidletime = 0;
	static vec3_t lastcampos = {0,0,0};
	static vec3_t lastcamang = {0,0,0};
	static int IdleFadeRateRedux = 0;
	
	int t;
	int frametime = 0;
	//vec4_t color;
	int skipmb;
	
	// The fade rate is the number of milliseconds a single frame takes to completely
	// disappear in the motion blurred image.
	int FadeRate = *(int *)data;
	
	// Post processing not being used.
	if (!PPInited || !jkg_postprocess.integer) return;
	
	// No fade rate?
	if (!FadeRate) {
		if (prevfaderate) {
			prevtime = 0;
			prevfaderate = 0;
		}
		return;
	}
	
	// Didn't blur last frame. Just draw output current color buffer.
	skipmb = 0;
	if (!prevfaderate) {
		skipmb = 1;
	}
	
	prevfaderate = FadeRate;
	if (!prevtime) {
		prevtime = trap_Milliseconds();
	} else {	
		t = trap_Milliseconds();
		if (cl_avidemo->integer) {
			frametime = 1000.f/cl_avidemo->integer; // Fixed frametime
		} else {
			frametime = t - prevtime;
		}
		prevtime = t;
	}
	
	// Check if the camera is idle (to do screen washing)
	if (VectorCompareT(cg.refdef.vieworg, lastcampos, 0.01f) && VectorCompareT(cg.refdef.viewangles, lastcamang, 0.01f)) {
		// Cam is idle
		if (!camidle) {
			camidle = 1;
		}
	} else {
		if (camidle) {
			camidle = 0;
			camidletime = 0;
			IdleFadeRateRedux = 0;
		}
	}
	VectorCopy(cg.refdef.vieworg, lastcampos);
	VectorCopy(cg.refdef.viewangles, lastcamang);


	if (!skipmb) {
		// Alright here we go, if skipmb is 0, render the motion blur shader over the scene
		// Then we force the renderer to sync up, load the texture, and move the scene into the texture.
		float imageAlpha = 0.0f;

		if (camidle && FadeRate > 1000) {
			if ((FadeRate - IdleFadeRateRedux) > 1000) {
				IdleFadeRateRedux += (frametime *2);
			}
			FadeRate -= IdleFadeRateRedux;
		}
		framecount = ((float)frametime / (float)FadeRate);
		imageAlpha = powf(0.0001, framecount);
		
		GSH_UseShader (&shMotionBlur);
		GSH_SetUniform1f ("alpha", imageAlpha);
		
		quadTextureData.numTexturesUsed = 2;
		quadTextureData.textures[0] = fboLastUsed->colorTextures[0];
		quadTextureData.textures[1] = fboMotionBlur->colorTextures[0];
		
		PP_RenderEffectToTexture (&quadTextureData, fboToUse);
	}
	
	FBO_BlitFramebufferColor (fboLastUsed, fboMotionBlur);
}

//=========================================================
// PP_QueueAdd
//---------------------------------------------------------
// Description:
// Adds a post-process pass to the queue.
//=========================================================
static void PP_QueueAdd (const postProcessPass_t *pass) {
    if ( !PPInited )
    {
        return;
    }
    
    postProcessQueue[postProcessQueueTail] = *pass;
    postProcessQueueTail = (postProcessQueueTail + 1) % postProcessQueueSize;
}

void PP_PreRender (void) {
    if (!PPInited || !jkg_postprocess.integer) {
        return;
    }
    
    //FBO_BindFramebuffer (fboMain);
}

//=========================================================
// PP_BeginPostProcess
//---------------------------------------------------------
// Description:
// Should be called after the world scene is rendered, and
// BEFORE any other render calls. This marks the beginning
// of all post-processing effects.
//=========================================================
void PP_BeginPostProcess (void) {
    if (!PPInited || !jkg_postprocess.integer) {
        return;
    }
    
    // Render the world scene, then blit it to an FBO.
    //GSH_UseShader (&shGenericOutput);
    //trap_R_DrawStretchPic (0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, NULL_HANDLE);
    R_SyncRenderThread();
	RB_EndSurface();
    
    FBO_BlitFramebufferColor (NULL, fboMain);
    //FBO_BlitFramebufferDepth (NULL, fboMain);
    //FBO_BindDefaultFramebuffer();
    
    fboLastUsed = fboMain;
    
    // Clear the queue, and reset.
    memset (postProcessQueue, 0, sizeof (postProcessQueue));
    postProcessQueueTail = 0;
}

#ifdef __SWF__
extern void	gameswf_processor(char *filename);

extern void	gameswf_startswf(char *filename);
extern void	gameswf_finishswf();
extern void	gameswf_continueswf();
extern int gameswf_swfplaying();
extern vmCvar_t	jkg_swf;
#endif //__SWF__

//=========================================================
// PP_EndPostProcess
//---------------------------------------------------------
// Description:
// This marks the end of all post-processing effects.
//=========================================================
void PP_EndPostProcess (void) {
    int i;
    
    if (!PPInited || !jkg_postprocess.integer) {
        return;
    }
    
    if (postProcessQueueTail > 0) {
        currentPingPong = 1;
        fboToUse = fboPingPong[currentPingPong];
        
        // TODO: I'm not sure if there will be any side effects to using the main FBO as
        // the pong to fboPing. We'll keep it as it is for now - if things need changing
        // then it's easy enough to change it.
        for (i = 0; i < postProcessQueueTail; i++) {
            postProcessQueue[i].effect->func (postProcessQueue[i].data);
        }
    }
    
    GSH_UseShader (NULL);
    FBO_BlitFramebufferColor (fboLastUsed, NULL);
    
    FBO_BindDefaultFramebuffer();
    
    /*quadTextureData.numTexturesUsed = 1;
    quadTextureData.textures[0] = sceneDepth;
    
    PP_RenderTexturedQuad (&quadTextureData, 20.0f, 20.0f, 96.0f, 72.0f, 0.0f, 0.0f, 1.0f, 1.0f);*/
}

//=========================================================
// PP_DoPass
//---------------------------------------------------------
// Description:
// Do a post-process pass with the given data.
//=========================================================
void PP_DoPass (const char *ppName, const void *data) {
    unsigned int i;
    postProcessPass_t pass;
    
    if ( !PPInited || !jkg_postprocess.integer )
    {
        return;
    }
    
    for (i = 0; i < numEffects; i++) {
        if (Q_stricmp (postProcessEffects[i].name, ppName) == 0) {
            pass.data = data;
            pass.effect = &postProcessEffects[i];
            
            PP_QueueAdd (&pass);
            break;
        }
    }
}

//=========================================================
// PP_RenderColorMod
//---------------------------------------------------------
// Description:
// Carries out the Color Mod pass.
//=========================================================
static void PP_RenderColorMod (const void *data) {
    const ppColormod_t *cmod = 0;
	if (!PPInited || !jkg_postprocess.integer) return;
	
	cmod = (const ppColormod_t *)data;
	if (!cmod->active) return;

	if (cmod->red_scale == 1 && cmod->green_scale == 1 && cmod->blue_scale == 1 && cmod->red_bias == 0 && cmod->red_bias == 0 && cmod->red_bias == 0 &&
		cmod->brightness == 0 && cmod->contrast == 1 && cmod->inversion == 0) {
		// Default settings, skip the colormod pass
		goto pass2;
	}
	
	GSH_UseShader(&shColorMod);
	GSH_SetUniform3f("scale", cmod->red_scale, cmod->green_scale, cmod->blue_scale);
	GSH_SetUniform3f("bias", cmod->red_bias, cmod->green_bias, cmod->blue_bias);
	GSH_SetUniform1f("brightness", cmod->brightness);
	GSH_SetUniform1f("contrast", cmod->contrast);
	GSH_SetUniform1f("inversion", cmod->inversion);
	
	quadTextureData.numTexturesUsed = 1;
	quadTextureData.textures[0] = fboLastUsed->colorTextures[0];
	
	PP_RenderEffectToTexture (&quadTextureData, fboToUse);

pass2:
	if (cmod->fx) {
		if (cmod->fx == CMFX_GRAYSCALE) {
			GSH_UseShader(&shGrayscale);
			GSH_SetUniform1f("intensity", cmod->fxintensity);
			GSH_SetUniform1f("brightness", cmod->fxbrightness);
		} else if (cmod->fx == CMFX_SEPIA) {
			GSH_UseShader(&shSepia);
			GSH_SetUniform1f("intensity", cmod->fxintensity);
			GSH_SetUniform1f("brightness", cmod->fxbrightness);
		}
		
		quadTextureData.numTexturesUsed = 1;
	    quadTextureData.textures[0] = fboLastUsed->colorTextures[0];
		
		PP_RenderEffectToTexture (&quadTextureData, fboToUse);
	}
	
	GSH_UseShader(NULL);
}

//=========================================================
// PP_RenderGaussianBlur
//---------------------------------------------------------
// Description:
// Carries out the gaussian blur pass.
//=========================================================
static void PP_RenderGaussianBlur (const void *data) {
	float pixelx, pixely;
	int i;
	const ppBlurParams_t *blurParams;
	if (!PPInited || !jkg_postprocess.integer) return;
	
	blurParams = (const ppBlurParams_t *)data;
	if (blurParams->intensity == 0.0f || blurParams->numPasses == 0) {	// Intensity must be > 0 to enable blurring
		return;                                                         // And we must have at least 1 pass
	}
	
	pixelx = blurParams->intensity / fboLastUsed->colorTextures[0]->width;
	pixely = blurParams->intensity / fboLastUsed->colorTextures[0]->height;

	for (i=0; i<blurParams->numPasses; i++) {
		GSH_UseShader(&shBlurX);
		GSH_SetUniform1f("pixelSize", pixelx);
		
		quadTextureData.numTexturesUsed = 1;
		quadTextureData.textures[0] = fboLastUsed->colorTextures[0];
		
		PP_RenderEffectToTexture (&quadTextureData, fboToUse);
		
		GSH_UseShader(&shBlurY);
		GSH_SetUniform1f("pixelSize", pixely);
		
		quadTextureData.numTexturesUsed = 1;
		quadTextureData.textures[0] = fboLastUsed->colorTextures[0];

		PP_RenderEffectToTexture (&quadTextureData, fboToUse);
	}
}

#ifndef BLOOM
static void PP_RenderBloom (const void *data) {
    const ppBloomParams_t *bloomParams;
    ppBlurParams_t blurParams;
    const framebuffer_t *colorFBO;
    
    if (!PPInited || !jkg_postprocess.integer) return;
    
    bloomParams = (const ppBloomParams_t *)data;
    GSH_UseShader (&shBrightPass);
    
    GSH_SetUniform1f ("threshold", bloomParams->brightnessThreshold);
    
    quadTextureData.numTexturesUsed = 1;
    quadTextureData.textures[0] = fboLastUsed->colorTextures[0];
    
    colorFBO = fboLastUsed;
    
    PP_RenderEffectToTexture (&quadTextureData, fboBrightPass);
    blurParams.intensity = 0.25f; // to compensate for the fact that the bright pass texture is 4 times smaller than normal size.
    blurParams.numPasses = 3;
    
    PP_RenderGaussianBlur ((const void *)&blurParams);
    
    GSH_UseShader (&shBloom);
    GSH_SetUniform1f ("bloomFactor", bloomParams->bloomFactor);
    
    quadTextureData.numTexturesUsed = 1;
	quadTextureData.textures[0] = fboLastUsed->colorTextures[0];
   // quadTextureData.textures[1] = colorFBO->colorTextures[0];

    
    PP_RenderEffectToTexture (&quadTextureData, fboToUse);
}
#endif

#ifdef __GL_ANAGLYPH__
extern int GSH_GetUniformLocation ( gshshader_t *shader, const char *uniformName );

//=========================================================
// PP_RenderAnaglyph
//---------------------------------------------------------
// Description:
// Carries out the anaglyph pass.
//=========================================================
static void PP_RenderAnaglyph (const void *data) {
	float pixelx, pixely;
	int i;
	const ppAnaglyphParams_t *anaglyphParams;
	int loc_left, loc_right;
	static unsigned int sdr;

	if (!PPInited || !jkg_postprocess.integer) return;
	
	anaglyphParams = (const ppAnaglyphParams_t *)data;

	pixelx = fboLastUsed->colorTextures[0]->width;
	pixely = fboLastUsed->colorTextures[0]->height;

	GSH_UseShader (&shAnaglyph);
	GSH_SetUniform1f ("tex_left", 0);
	GSH_SetUniform1f ("tex_right", 1);

	//loc_left = GSH_GetUniformLocation(&shAnaglyph, "tex_left");
    //loc_right = GSH_GetUniformLocation(&shAnaglyph, "tex_right");
	
#ifdef __0
	/*
	qglUseProgramObjectARB(sdr);
	qglUniform1iARB(loc_left, 0);
	qglUniform1iARB(loc_right, 1);

	qglActiveTextureARB(GL_TEXTURE1_ARB);
	qglBindTexture(GL_TEXTURE_2D, RIGHT_TEX);
	qglEnable(GL_TEXTURE_2D);

	qglActiveTextureARB(GL_TEXTURE0_ARB);
	qglBindTexture(GL_TEXTURE_2D, LEFT_TEX);
	qglEnable(GL_TEXTURE_2D);

	draw_quad(-1, -1, 1, 1);

	qglActiveTextureARB(GL_TEXTURE1_ARB);
	qglDisable(GL_TEXTURE_2D);

	qglActiveTextureARB(GL_TEXTURE0_ARB);
	qglDisable(GL_TEXTURE_2D);

	qglUseProgramObjectARB(0);
	*/

	GSH_UseShader(&shAnaglyph);
	GSH_SetUniform1f("tex_left", 0);
	GSH_SetUniform1f("tex_right", 1);

	glActiveTextureARB(GL_TEXTURE1_ARB);
    glBindTexture(GL_TEXTURE_2D, 0);
    glEnable(GL_TEXTURE_2D);

    glActiveTextureARB(GL_TEXTURE0_ARB);
    glBindTexture(GL_TEXTURE_2D, 1);
    glEnable(GL_TEXTURE_2D);

    //draw_quad(-1, -1, 1, 1);
	glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex2f(-1, -1);
    glTexCoord2f(1, 0); glVertex2f(1, -1);
    glTexCoord2f(1, 1); glVertex2f(1, 1);
    glTexCoord2f(0, 1); glVertex2f(-1, 1);
    glEnd();

    glActiveTextureARB(GL_TEXTURE1_ARB);
    glDisable(GL_TEXTURE_2D);

    glActiveTextureARB(GL_TEXTURE0_ARB);
    glDisable(GL_TEXTURE_2D);

    glUseProgramObjectARB(0);
#endif //_0
		
	quadTextureData.numTexturesUsed = 1;
	quadTextureData.textures[0] = fboLastUsed->colorTextures[0];
		
	PP_RenderEffectToTexture (&quadTextureData, fboToUse);
}
#endif //__GL_ANAGLYPH__

#ifdef __GL_EMBOSS__
static void PP_RenderEmboss (const void *data) {
	const ppEmbossParams_t *EmbossParams;

	if (!PPInited || !jkg_postprocess.integer) return;
	
	EmbossParams = (const ppEmbossParams_t *)data;

	GSH_UseShader (&shEmboss);
	//SetParameter4f( 0, m_varEmbossScale, 0, 0, 0 );
	//GSH_SetUniform1f ("embossScale", EmbossParams->embossScale);
	GSH_SetUniform1f ("0", EmbossParams->embossScale);
	GSH_SetUniform1f ("1", EmbossParams->embossScale);

	quadTextureData.numTexturesUsed = 1;
	quadTextureData.textures[0] = fboLastUsed->colorTextures[0];
		
	PP_RenderEffectToTexture (&quadTextureData, fboToUse);
}
#endif //__GL_EMBOSS__

float Q_flrand(float min, float max);

// TODO: Reimplement this properly.
/*void PP_RenderNoise(float colorIntensity, float distortionIntensity) {
	float seed[4];
	if (!PPInited || !jkg_postprocess.integer) return;
	if (colorIntensity == 0 && distortionIntensity == 0) {	
		return;
	}
	seed[0] = Q_flrand(0, 99999);
	seed[1] = Q_flrand(0, 99999);
	seed[2] = Q_flrand(0, 99999);
	seed[3] = Q_flrand(0, 99999);

	trap_R_DrawStretchPic(0,0,0,0,0,0,0,0,cgs.media.whiteShader);
	R_SyncRenderThread();

	
	glBindTexture(GL_TEXTURE_2D, cmshaderimg->texnum);
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, cgs.glconfig.vidWidth, cgs.glconfig.vidHeight, 0);
	trap_R_SetColor(NULL);
	
	GSH_UseShader(&shNoise);
	GSH_SetUniform4f("seed", seed[0], seed[1], seed[2], seed[3]);
	GSH_SetUniform1f("cintensity", colorIntensity);
	GSH_SetUniform1f("dintensity", distortionIntensity);

	//PP_RenderQuad(cmshaderimg,0,0,640,480,0,1,1,0,1,1,1,1);

	GSH_UseShader(NULL);
}*/

//================================================
//
// JKG_CheckIfIntel
// Little hack I implemented to make sure that Intel Mobility
// garbage is taken care of. --eez
//
//================================================

qboolean JKG_CheckIfIntel(void)
{
	const char *graphicVendor = glGetString(GL_VENDOR);
	if(!Q_strncmp(graphicVendor, "Intel", 5))	// God have mercy on your soul...
	{
		return qtrue;
	}
	return qfalse;
}