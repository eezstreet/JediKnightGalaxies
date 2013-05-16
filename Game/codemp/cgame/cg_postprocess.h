//       ____ ___________________   ___           ____  __ _______   ___  ________  ___ ______________
//      |    |\_   _____/\______ \ |   |         |    |/ _|\      \ |   |/  _____/ /   |   \__    ___/
//      |    | |    __)_  |    |  \|   |         |      <  /   |   \|   /   \  ___/    ~    \|    |   
//  /\__|    | |        \ |    `   \   |         |    |  \/    |    \   \    \_\  \    Y    /|    |   
//  \________|/_______  //_______  /___|         |____|__ \____|__  /___|\______  /\___|_  / |____|   
//                    \/         \/                      \/       \/            \/       \/           
//                         ________    _____   ____       _____  ____  ___ ______________ _________   
//                        /  _____/   /  _  \ |    |     /  _  \ \   \/  /|   \_   _____//   _____/   
//                       /   \  ___  /  /_\  \|    |    /  /_\  \ \     / |   ||    __)_ \_____  \    
//                       \    \_\  \/    |    \    |___/    |    \/     \ |   ||        \/        \   
//                        \______  /\____|__  /_______ \____|__  /___/\  \|___/_______  /_______  /   
//                               \/         \/        \/	   \/	   \_/			  \/        \/ (c)
// cg_postprocess.h
// Postprocessing header
// Copyright (c) 2013 Jedi Knight Galaxies

#ifndef _PP_INCLUDED
#define _PP_INCLUDED

typedef enum {
	CMFX_NONE,
	CMFX_GRAYSCALE,
	CMFX_SEPIA,
} Cmfx_e;

typedef struct ppColormod_s {
	int active;
	float red_scale;
	float green_scale;
	float blue_scale;
	float red_bias;
	float green_bias;
	float blue_bias;
	float brightness;
	float contrast;
	float inversion;
	int fx;				// One of the CMFX entries
	float fxintensity;
	float fxbrightness;
} ppColormod_t;

typedef struct ppBlurParams_s {
    float intensity;
    int numPasses;
} ppBlurParams_t;

#ifndef BLOOM
typedef struct ppBloomParams_s {
    float bloomFactor;
    float brightnessThreshold;
} ppBloomParams_t;
#endif

#ifdef __GL_ANAGLYPH__
typedef struct ppAnaglyphParams_s {
	int unused;
} ppAnaglyphParams_t;
#endif //__GL_ANAGLYPH__

#ifdef __GL_EMBOSS__
typedef struct ppEmbossParams_s {
	float embossScale;
} ppEmbossParams_t;
#endif //__GL_EMBOSS__

int PP_InitPostProcess();
void PP_TerminatePostProcess();
void PP_BeginPostProcess (void);
void PP_EndPostProcess (void);
void PP_DoPass (const char *ppName, const void *data);

#endif