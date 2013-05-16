// Postprocessing header

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

int PP_InitPostProcess();
void PP_TerminatePostProcess();
void PP_BeginPostProcess (void);
void PP_EndPostProcess (void);
void PP_DoPass (const char *ppName, const void *data);

#endif