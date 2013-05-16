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
// gl_screenshotext.c
// Screenshot extension module
// ---------------------------
// This module implements support for making png screenshots, as well as adding support for
// including depth buffer shots.
// (c) 2013 Jedi Knight Galaxies

#include "gl_enginefuncs.h"
#include <libpng/png.h>
#include <windows.h>
#include <gl/gl.h>
#include <stdlib.h>

#pragma comment(lib, "OpenGL32.lib")

typedef enum {
	TC_NONE,
	TC_S3TC,
	TC_S3TC_DXT
} textureCompression_t;

typedef struct {
	const char				*renderer_string;
	const char				*vendor_string;
	const char				*version_string;
	const char				*extensions_string;

	int						maxTextureSize;			// queried from GL
	int						maxActiveTextures;		// multitexture ability
	float					maxTextureFilterAnisotropy;

	int						colorBits, depthBits, stencilBits;

	qboolean				deviceSupportsGamma;
	textureCompression_t	textureCompression;
	qboolean				textureEnvAddAvailable;
	qboolean				clampToEdgeAvailable;

	int						vidWidth, vidHeight;

	int						displayFrequency;

	// synonymous with "does rendering consume the entire screen?", therefore
	// a Voodoo or Voodoo2 will have this set to TRUE, as will a Win32 ICD that
	// used CDS.
	qboolean				isFullscreen;
	qboolean				stereoEnabled;
} glconfig_t;

static glconfig_t *glconfig;

typedef struct {
	char *buffer;
	unsigned int bufferSize;
	unsigned int bufferUsed;
} PNGWriteData_t;

static void PNG_write_data(png_structp png_ptr, png_bytep data, png_size_t length) {
	PNGWriteData_t *ioData = (PNGWriteData_t *)png_get_io_ptr( png_ptr );
	if ( ioData->bufferUsed + length < ioData->bufferSize) {
		memcpy( ioData->buffer + ioData->bufferUsed, data, length );
		ioData->bufferUsed += length;
	}
}

static void PNG_flush_data(png_structp png_ptr) {
	(void)png_ptr;
}

static int SavePNG( int compresslevel, int image_width, int image_height, qboolean greyscale, int bitdepth, unsigned char *image_buffer, char *out_buffer, int out_size ) {
	png_structp		png_ptr			= 0;
	png_infop		info_ptr		= 0;
	png_bytep	   *row_pointers	= 0;
	PNGWriteData_t	writeData;
	int				i, rowSize;

	writeData.bufferUsed = 0;
	writeData.bufferSize = out_size;
	writeData.buffer = out_buffer;

	if (!writeData.buffer)
		goto skip_shot;

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (!png_ptr)
		goto skip_shot;

	info_ptr = png_create_info_struct(png_ptr);

	if (!info_ptr)
		goto skip_shot;

	/* Finalize the initing of png library */
    png_set_write_fn(png_ptr, &writeData, PNG_write_data, PNG_flush_data );

	if (compresslevel >= Z_BEST_SPEED && compresslevel <= Z_BEST_COMPRESSION)
		png_set_compression_level(png_ptr, compresslevel );		// Custom compression level
	
	/* set other zlib parameters */
	/*png_set_compression_mem_level(png_ptr, 5);
	png_set_compression_strategy(png_ptr,Z_DEFAULT_STRATEGY);
	png_set_compression_window_bits(png_ptr, 15);
	png_set_compression_method(png_ptr, 8);
	png_set_compression_buffer_size(png_ptr, 8192);*/

	if ( greyscale ) {
		rowSize = image_width * ( bitdepth / 8 );

		png_set_IHDR(png_ptr, info_ptr, image_width, image_height, bitdepth, PNG_COLOR_TYPE_GRAY, 
			          PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

		png_write_info(png_ptr, info_ptr);
	} else {
		rowSize = image_width*3;
		png_set_IHDR(png_ptr, info_ptr, image_width, image_height, bitdepth, PNG_COLOR_TYPE_RGB,
					  PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

		png_write_info(png_ptr, info_ptr );
	}

	// Allocate an array of scanline pointers
	row_pointers = (png_bytep *) malloc( image_height * sizeof(png_bytep) );

	for (i=0; i<image_height; i++)
	{
		row_pointers[i] = ( image_buffer + ( image_height - 1 - i ) * rowSize );
	}

	// Let's start encoding
	png_write_image(png_ptr, row_pointers);
	png_write_end(png_ptr, 0);

skip_shot:
	
	// Do cleanup
	if (png_ptr)
		png_destroy_write_struct(&png_ptr, &info_ptr);
	
	if (row_pointers)
		free( (void *)row_pointers );

	return writeData.bufferUsed;
}

static unsigned short   ShortSwap (unsigned short l)
{
	byte    b1,b2;

	b1 = l&255;
	b2 = (l>>8)&255;

	return (b1<<8) + b2;
}

cvar_t *sse_pngcompression;
cvar_t *sse_zBuffer;
cvar_t *sse_zFar;
cvar_t *sse_zNear;
#define	MAX_OSPATH			256
static void R_ScreenshotFilenamePNG( int lastNumber, char *fileName, char *depthFileName ) {
	int		a,b,c,d;

	if ( lastNumber < 0 || lastNumber > 9999 ) {
		Com_sprintf( fileName, MAX_OSPATH, "screenshots/shot9999.png" );
		Com_sprintf( depthFileName, MAX_OSPATH, "screenshots/shot9999.depth.jpg" );
		return;
	}

	a = lastNumber / 1000;
	lastNumber -= a*1000;
	b = lastNumber / 100;
	lastNumber -= b*100;
	c = lastNumber / 10;
	lastNumber -= c*10;
	d = lastNumber;

	Com_sprintf( fileName, MAX_OSPATH, "screenshots/shot%i%i%i%i.png"
		, a, b, c, d );
	Com_sprintf( depthFileName, MAX_OSPATH, "screenshots/shot%i%i%i%i.depth.png"
		, a, b, c, d );
}

static void GL_SSE_TakeScreenshotPNG( void ) {
	char *outBuf;
	int outSize;
	char *outAlloc;
	unsigned char *outAlign;
	qboolean silent;
	static	int	lastNumber = -1;
	char		filename[MAX_OSPATH];
	char		depthfilename[MAX_OSPATH];
	
	FILE *f;

	if ( !strcmp( Cmd_Argv(1), "silent" ) ) {
		silent = qtrue;
	} else {
		silent = qfalse;
	}

	if ( Cmd_Argc() == 2 && !silent ) {
		// explicit filename
		Com_sprintf( filename, MAX_OSPATH, "screenshots/%s.png", Cmd_Argv( 1 ) );
		Com_sprintf( depthfilename, MAX_OSPATH, "screenshots/%s.depth.png", Cmd_Argv( 1 ) );
	} else {
		// scan for a free filename

		// if we have saved a previous screenshot, don't scan
		// again, because recording demo avis can involve
		// thousands of shots
		if ( lastNumber == -1 ) {
			lastNumber = 0;
		}
		// scan for a free number
		for ( ; lastNumber <= 9999 ; lastNumber++ ) {
			R_ScreenshotFilenamePNG( lastNumber, filename, depthfilename );

			if (!FS_FileExists( filename ))
			{
				break; // file doesn't exist
			}
		}

		if ( lastNumber == 10000 ) {
			Com_Printf ("ScreenShot: Couldn't create a file\n"); 
			return;
		}

		lastNumber++;
	}

	outAlloc = malloc ( glconfig->vidWidth*glconfig->vidHeight * 4 + 8);
	outAlign = (unsigned char *)((((int)(outAlloc))+7) & ~7);

	outSize = glconfig->vidWidth * glconfig->vidHeight * 4;
	outBuf = (char *)malloc( outSize );

	glReadPixels( 0, 0, glconfig->vidWidth, glconfig->vidHeight, GL_RGB, GL_UNSIGNED_BYTE, outAlign ); 
	outSize = SavePNG( sse_pngcompression->integer, glconfig->vidWidth,  glconfig->vidHeight, qfalse, 8, outAlign, outBuf, outSize );

	
	f = fopen( FS_BuildOSPath( Cvar_String( "fs_homepath" ), Cvar_String( "fs_game" ), filename ), "wb" );
	fwrite( outBuf, outSize, 1, f );
	fclose( f );

	if (!Cvar_GetValueInt("cl_avidemo") && !silent) {
		Com_Printf("Wrote %s\n", filename);
	}
	
	if (sse_zBuffer->integer) {
		float zNear, zFar, zMul;
		float zBase, zAdd, zRange;
		int i;
		int pixelcount = glconfig->vidWidth*glconfig->vidHeight;

		zNear = sse_zNear->value; 
		zFar = sse_zFar->value; 
		zMul = 65535.0f / (zFar - zNear);

		#define backEndsceneZfar 1024
		zRange = backEndsceneZfar - Cvar_GetValueFloat( "r_znear" );
        zBase = ( backEndsceneZfar + Cvar_GetValueFloat( "r_znear" ) ) / zRange;
		zAdd =  ( 2 * backEndsceneZfar * Cvar_GetValueFloat( "r_znear" ) ) / zRange;

		glDepthRange( 0.0f, 1.0f );
		glReadPixels( 0, 0, glconfig->vidWidth, glconfig->vidHeight, GL_DEPTH_COMPONENT, GL_FLOAT, outAlign ); 

		for (i=0;i<pixelcount;i++) {
			// Read from the 0 - 1 depth 
			float zVal = ((float *)outAlign)[i];
			int outVal;
			// Back to the original -1 to 1 range 
			zVal = zVal * 2.0f - 1.0f;
			// Back to the original z values 
			zVal = zAdd / ( zBase - zVal );
			// Clip and scale the range that's been selected
			if (zVal <= zNear)
				outVal = 0;
			else if (zVal >= zFar)
				outVal = 65535;
			else 
				outVal = (int)((zVal - zNear) * zMul);
			((unsigned short *)outAlign)[i] = ShortSwap((unsigned short)outVal);
		}
		outSize = SavePNG( Z_BEST_SPEED, glconfig->vidWidth,  glconfig->vidHeight, qtrue, 16, outAlign, outBuf, outSize );
		f = fopen( FS_BuildOSPath( Cvar_String( "fs_homepath" ), Cvar_String( "fs_game" ), depthfilename ), "wb" );
		fwrite( outBuf, outSize, 1, f );
		fclose( f );
		if (!Cvar_GetValueInt("cl_avidemo") && !silent) {
			Com_Printf("Wrote %s\n", depthfilename);
		}
	}

	free(outBuf);
	free(outAlloc);
}

void GL_SSE_Init() {
	// Screenshot Extension (SSE) initialization

	sse_zBuffer = Cvar_Get("sse_zbuffer", "0", CVAR_ARCHIVE);
	sse_zFar = Cvar_Get("sse_zfar", "2000", CVAR_ARCHIVE);
	sse_zNear = Cvar_Get("sse_znear", "4", CVAR_ARCHIVE);
	sse_pngcompression = Cvar_Get("sse_pngcompression", "-1", CVAR_ARCHIVE);

	glconfig = (glconfig_t *)0x914070;
	Cmd_AddCommand("screenshot_png", &GL_SSE_TakeScreenshotPNG);
}