#ifndef JKG_FRAMEBUFFERS_H
#define JKG_FRAMEBUFFERS_H

#define MAX_FBO_COLOR_TEXTURES (4)

typedef enum internalFormat_e
{
    IF_RGBA8,
    IF_RGBA16F,
    IF_DEPTH_COMPONENT16,
    IF_DEPTH_COMPONENT24,
    IF_STENCIL_INDEX4,
    IF_STENCIL_INDEX8
} internalFormat_t;

typedef enum textureTarget_e
{
    TT_POT,
    TT_NPOT
} textureTarget_t;

typedef struct texture_s
{
    unsigned int id;
    
    textureTarget_t target;
} texture_t;

// Data type definitions
typedef struct framebuffer_s
{
    int id;
    
    const texture_t* colorTextures[MAX_FBO_COLOR_TEXTURES];
    unsigned int depthTexture;
    
    // will reconsider adding this back if i ever need it
    unsigned int stencilTexture;
} framebuffer_t;

// Function prototypes
//void CheckGLErrors ( const char* filename, int line );

qboolean FBO_FramebufferInit ( void );
void FBO_FramebufferCleanup ( void );
framebuffer_t* FBO_CreateFramebuffer ( void );
void FBO_AttachColorTextureToFramebuffer ( framebuffer_t* framebuffer, const texture_t* texture, unsigned int slot );
void FBO_AttachDepthTextureToFramebuffer ( framebuffer_t* framebuffer, const texture_t* texture );
void FBO_AttachDepthRenderbufferToFramebuffer ( framebuffer_t* framebuffer, unsigned int renderbufferId );
void FBO_AttachStencilRenderbufferToFramebuffer ( framebuffer_t* framebuffer, unsigned int renderbufferId );
void FBO_CheckFramebuffer ( const framebuffer_t* framebuffer );

void FBO_BindDefaultFramebuffer ( void );
void FBO_BindFramebuffer ( const framebuffer_t* framebuffer );
void FBO_BlitFramebufferColor (const framebuffer_t *source, const framebuffer_t *dest);
void FBO_BlitFramebufferDepth (const framebuffer_t *source, const framebuffer_t *dest);

texture_t* FBO_CreateBlankTexture ( unsigned int width, unsigned int height, internalFormat_t internalFormat );
unsigned int FBO_CreateRenderbuffer ( unsigned int width, unsigned int height, internalFormat_t internalFormat );

const texture_t *FBO_GetTextureForId ( unsigned int textureId );

unsigned int FBO_GetGLTextureTarget ( textureTarget_t textureTarget );

#endif
