#ifndef JKG_GLCOMMON_H
#define JKG_GLCOMMON_H

#include <windows.h>
#include <gl/gl.h>
#include "glext.h"

int IsExtensionSupported (const char* extension);
void CheckGLErrors ( const char* filename, int line );

#define LOAD_GL_FN(name,type) name = (type)wglGetProcAddress (#name); \
    if ( !name ) return qfalse;

#endif
