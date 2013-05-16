/*=========================================================
// Framebuffer object handling
//---------------------------------------------------------
// Description:
// Functions for handling framebuffer objects. Used mainly
// for post-processing.
//
// Version:
// $Id: jkg_framebuffers.c 418 2011-10-14 21:11:22Z Xycaleth $
//=========================================================*/

#include <GL/glew.h>
#include "cg_local.h"
#include "jkg_framebuffers.h"
#include "jkg_glcommon.h"

#define MAX_FRAMEBUFFERS (5)
#define MAX_TEXTURES (5 * MAX_FRAMEBUFFERS)
#define MAX_RENDERBUFFERS (2 * MAX_FRAMEBUFFERS)

// Support flags
static qboolean PBOSupported;
static qboolean FramebufferSupported;
static qboolean NativeNPOTSupported;

// Framebuffer, renderbuffer and texture object variables.
static unsigned int numFramebuffers = 0;
static framebuffer_t framebuffers[MAX_FRAMEBUFFERS];

static unsigned int numTextures = 0;
static texture_t textures[MAX_TEXTURES];

static unsigned int numRenderbuffers = 0;
static unsigned int renderbuffers[MAX_RENDERBUFFERS];

// Used to check for unncessary state changes
static const framebuffer_t* currentDrawFBOBound = NULL;
static const framebuffer_t* currentReadFBOBound = NULL;

void CheckGLErrors ( const char* filename, int line )
{
#ifdef _DEBUG
    unsigned int error = glGetError();
    while ( error != GL_NO_ERROR )
    {
        switch ( error )
        {
        case GL_INVALID_ENUM:
            CG_Printf (S_COLOR_RED "GL_INVALID_ENUM in file %s:%d.\n", filename, line);
            break;
        case GL_INVALID_VALUE:
            CG_Printf (S_COLOR_RED "GL_INVALID_VALUE in file %s:%d.\n", filename, line);
            break;
        case GL_INVALID_OPERATION:
            CG_Printf (S_COLOR_RED "GL_INVALID_OPERATION in file %s:%d.\n", filename, line);
            break;
        case GL_STACK_OVERFLOW:
            CG_Printf (S_COLOR_RED "GL_STACK_OVERFLOW in file %s:%d.\n", filename, line);
            break;
        case GL_STACK_UNDERFLOW:
            CG_Printf (S_COLOR_RED "GL_STACK_UNDERFLOW in file %s:%d.\n", filename, line);
            break;
        case GL_OUT_OF_MEMORY:
            CG_Printf (S_COLOR_RED "GL_OUT_OF_MEMORY in file %s:%d.\n", filename, line);
            break;
        default:
            CG_Printf (S_COLOR_RED "Error code 0x%X on line %d.\n", error, line);
            break;
        }
        
        error = glGetError();
    }
#endif
}

static qboolean IsPowerOfTwo ( unsigned int value )
{
    return (qboolean)(value && !(value & (value - 1)));
}

static unsigned int FBO_GetGLInternalFormat ( internalFormat_t internalFormat )
{
    switch ( internalFormat )
    {
    default:
    case IF_RGBA8:
        return GL_RGBA8;
        
    case IF_RGBA16F:
        return GL_RGBA16F_ARB;
        
    case IF_DEPTH_COMPONENT16:
        return GL_DEPTH_COMPONENT16;
        
    case IF_DEPTH_COMPONENT24:
        return GL_DEPTH_COMPONENT24;
        
    case IF_STENCIL_INDEX4:
        return GL_STENCIL_INDEX4_EXT;
    
    case IF_STENCIL_INDEX8:
        return GL_STENCIL_INDEX8_EXT;
    }
}

static unsigned int FBO_GetGLFormat ( internalFormat_t internalFormat )
{
    switch ( internalFormat )
    {
    default:
    case IF_RGBA8:
    case IF_RGBA16F:
        return GL_RGBA;
        
    case IF_DEPTH_COMPONENT16:
    case IF_DEPTH_COMPONENT24:
        return GL_DEPTH_COMPONENT;
        
    case IF_STENCIL_INDEX4:
    case IF_STENCIL_INDEX8:
        return GL_STENCIL_INDEX;
    }
}

unsigned int FBO_GetGLTextureTarget ( textureTarget_t textureTarget )
{
    switch ( textureTarget )
    {
    case TT_POT:
    case TT_NPOT:
        return GL_TEXTURE_2D;
    }
    
    return 0;
}

qboolean FBO_FramebufferInit ( void )
{
    memset (framebuffers, 0, sizeof (framebuffers));
    memset (textures, 0, sizeof (textures));
    memset (renderbuffers, 0, sizeof (renderbuffers));
    
    FramebufferSupported = (qboolean)(GLEW_EXT_framebuffer_object && GLEW_EXT_framebuffer_blit);
    
    // NPOT textures were introduced into the OpenGL 2.0 core specification.
    NativeNPOTSupported = (qboolean)GLEW_VERSION_2_0;
    if ( !NativeNPOTSupported )
    {
        CG_Printf (S_COLOR_YELLOW "...Non-power of two textures not supported\n");
    }
    
    PBOSupported = (qboolean)GLEW_ARB_pixel_buffer_object;
    
    CheckGLErrors (__FILE__, __LINE__);
    
    return FramebufferSupported;
}

void FBO_FramebufferCleanup ( void )
{
    unsigned int i = 0;
    framebuffer_t* fbo = framebuffers;
    texture_t* texture = textures;
    
    for ( ; i < numFramebuffers; i++, fbo++ )
    {
        if ( fbo->id == 0 )
        {
            continue;
        }
        
        glDeleteFramebuffers (1, &fbo->id);
    }
    
    for ( i = 0; i < numTextures; i++, texture++ )
    {
        if ( texture->id == 0 )
        {
            continue;
        }
        
        glDeleteTextures (1, &texture->id);
    }
    
    glDeleteRenderbuffers (numRenderbuffers, renderbuffers);
    
    FBO_BindDefaultFramebuffer();
    
    CheckGLErrors (__FILE__, __LINE__);
}

framebuffer_t* FBO_CreateFramebuffer ( void )
{
    framebuffer_t* fbo = NULL;
    
    if (!FramebufferSupported) {
        return NULL;
    }

    if ( numFramebuffers >= MAX_FRAMEBUFFERS )
    {
        CG_Printf ("Maximum number of framebuffers exeeded.\n");
        return NULL;
    }
    
    fbo = &framebuffers[numFramebuffers];
    glGenFramebuffers (1, &fbo->id);
    
    if ( fbo->id == 0 )
    {
        CG_Printf ("Failed to create framebuffer with internal ID %d.\n", numFramebuffers);
        return NULL;
    }
    
    numFramebuffers++;
    
    CheckGLErrors (__FILE__, __LINE__);
    
    return fbo;
}

void FBO_AttachColorTextureToFramebuffer ( framebuffer_t* framebuffer, const texture_t* texture, unsigned int slot )
{
    if (!framebuffer) {
        return;
    }

    if ( slot >= MAX_FBO_COLOR_TEXTURES )
    {
        CG_Printf ("Invalid slot number given (%d), valid range is 0 - %d.\n", slot, MAX_FBO_COLOR_TEXTURES - 1);
        return;
    }
    
    glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + slot, FBO_GetGLTextureTarget (texture->target), texture->id, 0);
    framebuffer->colorTextures[slot] = texture;
    
    CheckGLErrors (__FILE__, __LINE__);
}

void FBO_AttachDepthRenderbufferToFramebuffer ( framebuffer_t* framebuffer, unsigned int renderbufferId )
{
    if (!framebuffer) {
        return;
    }

    glFramebufferRenderbuffer (GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderbufferId);
    framebuffer->depthTexture = renderbufferId;
    
    CheckGLErrors (__FILE__, __LINE__);
}

void FBO_AttachDepthTextureToFramebuffer ( framebuffer_t* framebuffer, const texture_t* texture )
{
    if (!framebuffer) {
        return;
    }

    glFramebufferTexture2D (GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, FBO_GetGLTextureTarget (texture->target), texture->id, 0);
    framebuffer->depthTexture = texture->id;
    
    CheckGLErrors (__FILE__, __LINE__);
}

void FBO_CheckFramebuffer ( const framebuffer_t* framebuffer )
{
    unsigned int status;
    
    if (!framebuffer) {
        return;
    }
    
    status = glCheckFramebufferStatus (GL_FRAMEBUFFER);
    switch ( status )
    {
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        CG_Printf ("One or more framebuffer attachment points are not complete.\n");
        break;
        
    /*case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
        CG_Printf ("One or more attached images have different dimensions.\n");
        break;*/
        
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
        CG_Printf ("Invalid framebuffer attachment object type used.\n");
        break;
        
    /*case GL_FRAMEBUFFER_INCOMPLETE_FORMATS:
        CG_Printf ("More than one internal format was used in the color attachments.\n");
        break;*/
        
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
        CG_Printf ("Missing a read buffer.\n");
        break;
        
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        CG_Printf ("No images were attached to the framebuffer.\n");
        break;
        
    case GL_FRAMEBUFFER_COMPLETE:
        break;
    }
    
	if ( status != GL_FRAMEBUFFER_COMPLETE )
	{
		CG_Printf ("Creation of framebuffer %d could not be completed.\n", framebuffer->id);
	}
	
	CheckGLErrors (__FILE__, __LINE__);
}

const texture_t *FBO_GetTextureForId ( unsigned int textureId )
{
    int i = 0;
    for ( ; i < MAX_TEXTURES; i++ )
    {
        if ( textures[i].id == textureId )
        {
            return &textures[i];
        }
    }
    
    return NULL;
}

void FBO_BindDefaultFramebuffer ( void )
{
    if (!FramebufferSupported) {
        return;
    }

    if ( currentDrawFBOBound != NULL || currentReadFBOBound != NULL )
    {
        glBindFramebuffer (GL_FRAMEBUFFER, 0);
        currentDrawFBOBound = NULL;
        currentReadFBOBound = NULL;
        
        cg.framebufferActive = qfalse;
    }
    
    CheckGLErrors (__FILE__, __LINE__);
}

void FBO_BindFramebuffer ( const framebuffer_t* framebuffer )
{
    if (!framebuffer) {
        return;
    }

    if ( currentDrawFBOBound != framebuffer || currentReadFBOBound != framebuffer )
    {
        glBindFramebuffer (GL_FRAMEBUFFER, framebuffer->id);
        currentDrawFBOBound = framebuffer;
        currentReadFBOBound = framebuffer;
        
        cg.framebufferActive = qtrue;
    }
    
    CheckGLErrors (__FILE__, __LINE__);
}

void FBO_BlitFramebufferDepth (const framebuffer_t *source, const framebuffer_t *dest) {
    if (!FramebufferSupported) {
        return;
    }

    if (currentReadFBOBound != source) {
        if (source == NULL) {
            glBindFramebuffer (GL_READ_FRAMEBUFFER, 0);
        } else {
            glBindFramebuffer (GL_READ_FRAMEBUFFER, source->id);
        }
    }
    
    if (currentDrawFBOBound != dest) {
        if (dest == NULL) {
            glBindFramebuffer (GL_DRAW_FRAMEBUFFER, 0);
        } else {
            glBindFramebuffer (GL_DRAW_FRAMEBUFFER, dest->id);
        }
    }
    
    glBlitFramebuffer (0, 0, cgs.glconfig.vidWidth, cgs.glconfig.vidHeight,
                            0, 0, cgs.glconfig.vidWidth, cgs.glconfig.vidHeight,
                            GL_DEPTH_BUFFER_BIT,
                            GL_NEAREST);
                            
    currentReadFBOBound = source;
    currentDrawFBOBound = dest;
    cg.framebufferActive = qtrue;
    
    CheckGLErrors (__FILE__, __LINE__);
}

void FBO_BlitFramebufferColor (const framebuffer_t *source, const framebuffer_t *dest) {
    int sw, sh, dw, dh;

    if (!FramebufferSupported) {
        return;
    }
    
    sw = cgs.glconfig.vidWidth;
    sh = cgs.glconfig.vidHeight;
    dw = cgs.glconfig.vidWidth;
    dh = cgs.glconfig.vidHeight;

    if (currentReadFBOBound != source) {
        if (source == NULL) {
            glBindFramebuffer (GL_READ_FRAMEBUFFER, 0);
        } else {
            glBindFramebuffer (GL_READ_FRAMEBUFFER, source->id);
            sw = source->colorTextures[0]->width;
            sh = source->colorTextures[0]->height;
        }
    }
    
    if (currentDrawFBOBound != dest) {
        if (dest == NULL) {
            glBindFramebuffer (GL_DRAW_FRAMEBUFFER, 0);
        } else {
            glBindFramebuffer (GL_DRAW_FRAMEBUFFER, dest->id);
            dw = dest->colorTextures[0]->width;
            dh = dest->colorTextures[0]->height;
        }
    }
    
    glBlitFramebufferEXT (0, 0, sw, sh, 0, 0, dw, dh, GL_COLOR_BUFFER_BIT, GL_LINEAR);
                            
    currentReadFBOBound = source;
    currentDrawFBOBound = dest;
    cg.framebufferActive = qtrue;
    
    CheckGLErrors (__FILE__, __LINE__);
}

// Based on code seen at: http://en.wikipedia.org/wiki/Power_of_two#Algorithm_to_find_the_next-highest_power_of_two
static unsigned int NextPowerOfTwo (unsigned int number) {
    int i = 1;
    number--;
    for (; i < 16; i *= 2) {
        number |= (number >> i);
    }
    number++;
    
    return number;
}

// Based on code seen at: http://en.wikipedia.org/wiki/Power_of_two#Algorithm_to_convert_any_number_into_nearest_power_of_two_number
static unsigned int NearestPowerOfTwo (unsigned int number) {
    int i;

    if (number == 0) {
        return 1;
    }
    
    for (i = sizeof (int) * 8 - 1; ((1u << i) & number) == 0; i--);
    
    if (((1u << (i - 1)) & number) == 0) {
        return 1u << i;
    }
    
    return 1u << (i + 1);
}

static unsigned int FBO_GetDataType ( internalFormat_t internalFormat )
{
    switch ( internalFormat )
    {
    case IF_RGBA16F:
    case IF_DEPTH_COMPONENT16:
    case IF_DEPTH_COMPONENT24:
        return GL_FLOAT;
        
    default:
        return GL_UNSIGNED_BYTE;
    }
}

extern vmCvar_t r_force_pot_textures;
texture_t* FBO_CreateBlankTexture ( unsigned int width, unsigned int height, internalFormat_t internalFormat )
{
    unsigned int textureId = 0;
    unsigned int glTexTarget = 0;
    texture_t* texture = NULL;
    
    if (!FramebufferSupported) {
        return NULL;
    }

    if ( numTextures >= MAX_TEXTURES )
    {
        CG_Printf ("Exceeded maximum number of textures.\n");
        return NULL;
    }

    glGenTextures (1, &textureId);
    if ( textureId == 0 )
    {
        CG_Printf ("Failed to create texture with internal ID %d.\n", numTextures);
        return NULL;
    }
    
    texture = &textures[numTextures];
    texture->id = textureId;
    texture->target = TT_POT;//IsPowerOfTwo (width) && IsPowerOfTwo (height) ? TT_POT : TT_NPOT;
    
    glTexTarget = FBO_GetGLTextureTarget (texture->target);
    
    glBindTexture (glTexTarget, textureId);
    
    glTexParameteri (glTexTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri (glTexTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri (glTexTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri (glTexTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    if ( r_force_pot_textures.integer == 2 )
    {
        width = NearestPowerOfTwo (width);
        height = NearestPowerOfTwo (height);
    }
    else if ( r_force_pot_textures.integer == 1 || !NativeNPOTSupported )
    {
        width = NextPowerOfTwo (width);
        height = NextPowerOfTwo (height);
    }
    
    texture->width = width;
    texture->height = height;
    
    glTexImage2D (glTexTarget, 0, FBO_GetGLInternalFormat (internalFormat), width, height, 0, FBO_GetGLFormat (internalFormat), FBO_GetDataType (internalFormat), NULL);
    
    glBindTexture (glTexTarget, 0);
    
    numTextures++;
    
    CheckGLErrors (__FILE__, __LINE__);
    
    return texture;
}

unsigned int FBO_CreateRenderbuffer ( unsigned int width, unsigned int height, internalFormat_t internalFormat )
{
    unsigned int renderbufferId = 0;
    
    if (!FramebufferSupported) {
        return 0;
    }
    
    if ( numRenderbuffers >= MAX_RENDERBUFFERS )
    {
        CG_Printf ("Exceeded maximum number of renderbuffers.\n");
        return 0;
    }
    
    glGenRenderbuffers (1, &renderbufferId);
    if ( renderbufferId == 0 )
    {
        CG_Printf ("Failed to create renderbuffer with internal ID %d.\n", numRenderbuffers);
        return 0;
    }
    
    if ( r_force_pot_textures.integer == 2 )
    {
        width = NearestPowerOfTwo (width);
        height = NearestPowerOfTwo (height);
    }
    else if ( r_force_pot_textures.integer == 1 || !NativeNPOTSupported )
    {
        width = NextPowerOfTwo (width);
        height = NextPowerOfTwo (height);
    }
    
    glBindRenderbuffer (GL_RENDERBUFFER, renderbufferId);
    glRenderbufferStorage (GL_RENDERBUFFER, FBO_GetGLInternalFormat (internalFormat), width, height);
    
    renderbuffers[numRenderbuffers] = renderbufferId;
    numRenderbuffers++;
    
    CheckGLErrors (__FILE__, __LINE__);
    
    return renderbufferId;
}