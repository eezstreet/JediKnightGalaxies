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
// jkg_eshader.h
// Engine shader structs
//
// This header contains the definition (as far as known)
// of the engine's shader structures
//
// Reverse engineered by BobaFett
// Copyright (c) 2013 Jedi Knight Galaxies

typedef struct image_s {
	char 	name[64];
	short 	width;
	short 	height;
	int 	texnum;
	int		frameUsed;
	int		imageFormat;
	int		glWrapClampMode;
	char 	mipmap;
	char	allowPicmip;
	short 	levused;
} image_t;

typedef struct waveForm_s {
	int		func;
	float	base;
	float	amplitude;
	float	phase;
	float	frequency;
} waveForm_t;

typedef struct textureBundle_s {
	union {
		image_t		**images;				// if numImageAnimations > 1
		image_t		*image;					// if numImageAnimations == 1
	};
	int			tcGen;
	vec3_t		(*tcGenVectors)[2];
	void		*texMods;
	short		numTexMods;
	short		numImageAnimations;
	float		imageAnimationSpeed;
	char		isLightmap;				// bool
	char		oneshotanimMap;			// bool
	char		vertexLightmap;			// bool (not verified)
	char		isVideoMap;				// bool
	int			videoMapHandle;			// RoQ handle
} textureBundle_t;

typedef struct shaderStage_s {	/* INCOMPLETE */
	char	active;				// bool
	char	isDetail;
	short	/* UNMAPPED */__unk;
	textureBundle_t bundle[2];
	waveForm_t	rgbWave;
	int		rgbGen;
	waveForm_t	alphaWave;
	int		alphaGen;
	char	constantColor[4];
	int		stateBits;
	int		adjustColorsForFog;
	int		/* UNMAPPED */__unk2;
	void	*surfaceSprite;
	char	glow;				// bool
	char	/* UNMAPPED */__unk3[3];
} shaderStage_t;


typedef struct fogParms_s {
	vec3_t color;
	float depthForOpaque;
} fogParms_t;


typedef struct skyParms_s {
	float		cloudHeight;
	image_t *	skybox[6];
} skyParms_t;

typedef struct deformStage_s {
	int			deformation;
	vec3_t		moveVector;
	waveForm_t	deformationWave;
	float		deformationSpread;
	float		bulgeWidth;
	float		bulgeHeight;
	float		bulgeSpeed;
} deformStage_t;

typedef struct shader_s {  /* INCOMPLETE */
	char	name[64];
	int		lightmapIndex;
	int		lightstyles[4];			// Unsure how these work
	int		index;
	int		sortedIndex;
	float	sort;
	int		surfaceFlags;				
	int		contentFlags;
	char	defaultshader;			// bool
	char	explicitlyDefined;		// bool
	char	entityMergable;			// bool
	char	/* UNMAPPED */__unk;
	skyParms_t *sky;				// NULL if not used
	fogParms_t *fogParms;			// NULL if not used
	float	portalRange;
	int		/* UNMAPPED */__unk2;
	char	cullType;
	char	/* UNMAPPED */__unk3[3];
	char	polygonOffset;			// bool
	char	noMipMaps;				// bool
	char	noPicMip;				// bool
	char	noTC;					// bool
	int		fogPass;
	char	/* UNMAPPED */__unk4[12];
	deformStage_t *deforms[3];
	short	numDeforms;
	short	numUnfoggedPasses;
	shaderStage_t *stages; //[8];
	float	clampTime;
	float	timeOffset;
	char	glow;					// bool
	char	/* UNMAPPED */__unk5[3];
	struct shader_s *remappedshader;
	struct shader_s *next;
} shader_t;