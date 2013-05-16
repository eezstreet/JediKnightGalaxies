///////////////////////////////////////
//
// Lightmap loader hooks
//
// Increases lightmap quality and quantity
//
///////////////////////////////////////

#include <windows.h>
#include <zlib/zlib.h>

static void *lightmaps[1024];	// All tr.lightmap refs are changed to reference this

static void (*Com_Error)(int level, const char *fmt, ...) = (void (*)(int,const char*,...))0x437290;

typedef struct {
	int		fileofs, filelen;
} lump_t;

#define	LM_MAGIC ('M' << 24 | 'L' << 16 | 'E' << 8 | 'G' )

typedef struct {
	int magic;				// GELM
	int blocksize;
	int blockuncompressed;
	int imgsize;
} lmheader_t;

#define tr_overbrightBits *(int *)0xFE3994
#define tr_numLightmaps *(int *)0xFE3280
#define fileBase *(char* *)0xFE2B90
// Based on Q3's source, changed to JA's equivalent
static	void R_ColorShiftLightingBytes( byte in[4], byte out[4] ) {
	int		shift, r, g, b;

	r = in[0];
	g = in[0];
	b = in[0];

	if (tr_overbrightBits) {
		// shift the color data based on overbright range
		shift = 1 - tr_overbrightBits;
		if (shift) {

			// shift the data based on overbright range
			r = in[0] << shift;
			g = in[1] << shift;
			b = in[2] << shift;
			
			// normalize by color instead of saturating to white
			if ( ( r | g | b ) > 255 ) {
				int		max;

				max = r > g ? r : g;
				max = max > b ? max : b;
				r = r * 255 / max;
				g = g * 255 / max;
				b = b * 255 / max;
			}
		}
	}
	out[0] = (byte)r;
	out[1] = (byte)g;
	out[2] = (byte)b;
	out[3] = (byte)in[3];
}

// Custom lightmap loader, return 0 if loaded, return 1 to use default loader
// and 2 to bail out completely (in case of errors)
// Replica of the original loader, with a lot of extra's :P
// Zlib compression of the lump, support for custom sizes (as long as they're square)
// and overflow checks
int R_LoadLightmaps( lump_t *l, const char* mapname) {
	int lmaps;
	lmheader_t *lmhdr;
	char *buf;
	char *databuff;
	char *imgbuff;

	(void)mapname;
	
	if (!l->filelen) {
		return 2;	// Cancel the loading entirely
	}

	if ((l->filelen % 128) == 0) {
		// Before we return, ensure the size is valid
		lmaps = l->filelen / (128*128*3);
		if (lmaps > 1024) {
			// Too many lightmaps!
			Com_Error(1, "Too many lightmaps (%i > 1024)\n", lmaps);
			return 2;
		}
		// Its a normal 128x128 lightmap, so send to the default handler
		return 1;
	}
	// Alrighty then, its a custom lightmap, kick in our custom loader
	buf = (fileBase + l->fileofs);
	lmhdr = (lmheader_t *)(fileBase + l->fileofs);
	if (lmhdr->magic != LM_MAGIC) {
		Com_Error(1, "Invalid lightmap lump\n");
		return 2;
	}
	databuff = malloc(lmhdr->blockuncompressed);
	if (!databuff) {
		Com_Error(1, "Could not allocate lightmap decompression buffer (%i bytes)\n", lmhdr->blockuncompressed);
		return 2;
	}
	imgbuff = malloc(lmhdr->imgsize * lmhdr->imgsize * 4);
	if (!imgbuff) {
		Com_Error(1, "Could not allocate lightmap image buffer (%i bytes)\n", lmhdr->imgsize * lmhdr->imgsize * 4);
		free(databuff);
		return 2;
	}
	// Alright, time to decompress the image
	if (uncompress((Bytef *)databuff, ( uLongf * ) lmhdr->blockuncompressed, (const Bytef *)(buf+16), (uLong)lmhdr->blocksize) != Z_OK) {
		Com_Error(1, "Could not decompress lightmaps\n");
		free(databuff);
		free(imgbuff);
		return 2;
	}
	// Now that we got the lightmaps decompressed, lets start processing them
	// They are stored in 24 bit, and we need to expand em to 32 bit
	
	// Get the amount of lightmaps we have
	tr_numLightmaps = lmhdr->blockuncompressed / (lmhdr->imgsize * lmhdr->imgsize * 3);
	if (tr_numLightmaps == 1) {
		// Ported from Q3:
		//FIXME: HACK: maps with only one lightmap turn up fullbright for some reason.
		//this avoids this, but isn't the correct solution.
		(tr_numLightmaps)++;
	}

	return 0;	
}


/*
static	void R_LoadLightmaps( lump_t *l ) {
	byte		*buf, *buf_p;
	int			len;
	MAC_STATIC byte		image[LIGHTMAP_SIZE*LIGHTMAP_SIZE*4];
	int			i, j;
	float maxIntensity = 0;
	double sumIntensity = 0;

    len = l->filelen;
	if ( !len ) {
		return;
	}
	buf = fileBase + l->fileofs;

	// we are about to upload textures
	R_SyncRenderThread();

	// create all the lightmaps
	tr.numLightmaps = len / (LIGHTMAP_SIZE * LIGHTMAP_SIZE * 3);
	if ( tr.numLightmaps == 1 ) {
		//FIXME: HACK: maps with only one lightmap turn up fullbright for some reason.
		//this avoids this, but isn't the correct solution.
		tr.numLightmaps++;
	}

	// if we are in r_vertexLight mode, we don't need the lightmaps at all
	if ( r_vertexLight->integer || glConfig.hardwareType == GLHW_PERMEDIA2 ) {
		return;
	}

	for ( i = 0 ; i < tr.numLightmaps ; i++ ) {
		// expand the 24 bit on-disk to 32 bit
		buf_p = buf + i * LIGHTMAP_SIZE*LIGHTMAP_SIZE * 3;

		if ( r_lightmap->integer == 2 )
		{	// color code by intensity as development tool	(FIXME: check range)
			for ( j = 0; j < LIGHTMAP_SIZE * LIGHTMAP_SIZE; j++ )
			{
				float r = buf_p[j*3+0];
				float g = buf_p[j*3+1];
				float b = buf_p[j*3+2];
				float intensity;
				float out[3];

				intensity = 0.33f * r + 0.685f * g + 0.063f * b;

				if ( intensity > 255 )
					intensity = 1.0f;
				else
					intensity /= 255.0f;

				if ( intensity > maxIntensity )
					maxIntensity = intensity;

				HSVtoRGB( intensity, 1.00, 0.50, out );

				image[j*4+0] = out[0] * 255;
				image[j*4+1] = out[1] * 255;
				image[j*4+2] = out[2] * 255;
				image[j*4+3] = 255;

				sumIntensity += intensity;
			}
		} else {
			for ( j = 0 ; j < LIGHTMAP_SIZE * LIGHTMAP_SIZE; j++ ) {
				R_ColorShiftLightingBytes( &buf_p[j*3], &image[j*4] );
				image[j*4+3] = 255;
			}
		}
		tr.lightmaps[i] = R_CreateImage( va("*lightmap%d",i), image, 
			LIGHTMAP_SIZE, LIGHTMAP_SIZE, qfalse, qfalse, GL_CLAMP );
	}

	if ( r_lightmap->integer == 2 )	{
		ri.Printf( PRINT_ALL, "Brightest lightmap value: %d\n", ( int ) ( maxIntensity * 255 ) );
	}
}
*/