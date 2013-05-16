////////////////////////////////
//
//  Pixel Shaders module
//
//  Handles all pixel shader operations
//  as well as loading GSH files
//
////////////////////////////////

#include <GL/glew.h>
#include "jkg_glcommon.h"
#include <math.h>
#include "cg_local.h"
#include <zlib/zlib.h>

#pragma comment(lib, "OpenGL32.lib")

#include "jkg_pixelshaders.h"

// Enable me to switch on the alternative GLSL pipeline.
#define USE_GLSL

static int GLSLSupported = 0;
static int ARBSupported = 0;
static int glslActive = 0;
static int arbActive = 0;
static gshshader_t *currentShader;

void GSH_FreeShader(gshshader_t *shader);

#ifdef USE_GLSL
static void GSH_CompileGLSLShader(gshshader_t *shader) {
	int ret;
	int i;
	int size, type;
	char errlog[1024];
	int numUniforms;
	if (!shader) {
		return;
	}
	// First, create a new program
	if (shader->glsl_vertexShaderSource) {
		// We got vertex shader source, compile it
		shader->glsl_vertexShader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
		glShaderSourceARB(shader->glsl_vertexShader, 1, &shader->glsl_vertexShaderSource, NULL);
		glCompileShaderARB(shader->glsl_vertexShader);
		glGetObjectParameterivARB(shader->glsl_vertexShader, GL_OBJECT_COMPILE_STATUS_ARB, &ret);
		if (ret != GL_TRUE) {
			glGetInfoLogARB(shader->glsl_vertexShader, 1024, NULL, errlog);
			// Shader failed to compile, find out why
			CG_Printf("GSH_CompileGLSLShader: Could not compile vertex shader for shader '%s'!\nLog:\n%s\n", shader->name, errlog);
		}
		
		free ((void *)shader->glsl_vertexShaderSource);
	}
	CheckGLErrors (__FILE__, __LINE__);
	if (shader->glsl_fragmentShaderSource) {
		// We got vertex shader source, compile it
		shader->glsl_fragmentShader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
		glShaderSourceARB(shader->glsl_fragmentShader, 1, &shader->glsl_fragmentShaderSource, NULL);
		glCompileShaderARB(shader->glsl_fragmentShader);
		glGetObjectParameterivARB(shader->glsl_fragmentShader, GL_OBJECT_COMPILE_STATUS_ARB, &ret);
		if (ret != GL_TRUE) {
			glGetInfoLogARB(shader->glsl_fragmentShader, 1024, NULL, errlog);
			// Shader failed to compile, find out why
			CG_Printf("GSH_CompileGLSLShader: Could not compile fragment shader for shader '%s'!\nLog:\n%s\n", shader->name, errlog);
		}
		
		free ((void *)shader->glsl_fragmentShaderSource);
	}
	CheckGLErrors (__FILE__, __LINE__);
	shader->glsl_program = glCreateProgramObjectARB();
	if (shader->glsl_vertexShader) {
		glAttachObjectARB(shader->glsl_program, shader->glsl_vertexShader);
	}
	CheckGLErrors (__FILE__, __LINE__);
	if (shader->glsl_fragmentShader) {
		glAttachObjectARB(shader->glsl_program, shader->glsl_fragmentShader);
	}
	CheckGLErrors (__FILE__, __LINE__);
	glLinkProgramARB(shader->glsl_program);
	CheckGLErrors (__FILE__, __LINE__);
	glGetObjectParameterivARB(shader->glsl_program, GL_OBJECT_LINK_STATUS_ARB, &ret);
	CheckGLErrors (__FILE__, __LINE__);
	if (ret != GL_TRUE) {
		glGetInfoLogARB(shader->glsl_program, 1024, NULL, errlog);
		// Shader failed to compile, find out why
		CG_Printf("GSH_CompileGLSLShader: Could not link program for shader %s!\nLog:\n%s\n", shader->name, errlog);
	}
	glValidateProgramARB(shader->glsl_program);
	CheckGLErrors (__FILE__, __LINE__);
	glGetObjectParameterivARB(shader->glsl_program, GL_OBJECT_VALIDATE_STATUS_ARB, &ret);
	CheckGLErrors (__FILE__, __LINE__);
	if (ret != GL_TRUE) {
		
		glGetInfoLogARB(shader->glsl_program, 1024, NULL, errlog);
		// Shader failed to compile, find out why
		CG_Printf("GSH_CompileGLSLShader: Could not validate program for shader %s!\nLog:\n%s\n", shader->name, errlog);
	}
	
	glGetObjectParameterivARB (shader->glsl_program, GL_OBJECT_ACTIVE_UNIFORMS_ARB, &numUniforms);
	
	shader->varcount = numUniforms;
	shader->vars = malloc (sizeof(gsh_variable_t) * shader->varcount);
	memset (shader->vars, 0, sizeof(gsh_variable_t) * shader->varcount);
	
	for (i = 0; i < numUniforms; i++) {
	    glGetActiveUniformARB (shader->glsl_program, i, sizeof (shader->vars[i].name), NULL, &size, &type, &shader->vars[i].name[0]);
	    shader->vars[i].localidx = i;
	    shader->vars[i].vsfs = 1;
	}
	CheckGLErrors (__FILE__, __LINE__);
}
#endif

static void GSH_CompileARBShader(gshshader_t *shader) {
	if (!shader) {
		return;
	}
	if (shader->arb_vertexShaderSource) {
		glEnable(GL_VERTEX_PROGRAM_ARB);
		glGenProgramsARB(1, &shader->arb_vertexShader);
		glBindProgramARB(GL_VERTEX_PROGRAM_ARB, shader->arb_vertexShader);
		glProgramStringARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, strlen(shader->arb_vertexShaderSource), shader->arb_vertexShaderSource);
		glBindProgramARB(GL_VERTEX_PROGRAM_ARB, 0);
		glDisable(GL_VERTEX_PROGRAM_ARB);
		
		free ((void *)shader->arb_vertexShaderSource);
	}
	if (shader->arb_fragmentShaderSource) {
		glEnable(GL_FRAGMENT_PROGRAM_ARB);
		glGenProgramsARB(1, &shader->arb_fragmentShader);
		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, shader->arb_fragmentShader);
		glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, strlen(shader->arb_fragmentShaderSource), shader->arb_fragmentShaderSource);
		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, 0);
		glDisable(GL_FRAGMENT_PROGRAM_ARB);
		
		free ((void *)shader->arb_fragmentShaderSource);
	}
	shader->arb_compiled = 1;
}

int GSH_LoadShader(gshshader_t *shader, const char *filename) {
	char *buff;
	const char *vars;
	const char *var;
	int i;
	fileHandle_t f;
	gsh_header_t *hdr;
	int len;
	if (!shader) {
		return 0;
	}
	if (shader->contents) {
		GSH_FreeShader(shader);
	}
	len = trap_FS_FOpenFile(filename, &f, FS_READ);
	if (len < 1) {
		CG_Printf("Error while loading pixel shader %s: File not found\n", filename);
		return 0;
	}
	buff = malloc(len);
	trap_FS_Read(buff, len, f);
	trap_FS_FCloseFile(f);
	hdr = (gsh_header_t *)buff;
	if (hdr->header != GSH_HDR) {
		CG_Printf("Error while loading pixel shader %s: Bad header\n", filename);
		return 0;
	}
	if (hdr->version != GSH_VERSION) {
		CG_Printf("Error while loading pixel shader %s: Version mismatch\n", filename);
		return 0;
	}
	Q_strncpyz(shader->name, hdr->name, 64);
	shader->contents = hdr->content;
#ifdef USE_GLSL
	if (shader->contents & GSH_CONTENT_GLSL_VS) {
		// We got a vertex shader, load it
		shader->glsl_vertexShaderSource = malloc(hdr->glsl_vs.size);
		uncompress(( Bytef * ) shader->glsl_vertexShaderSource, ( uLongf * ) &hdr->glsl_vs.size, ( Bytef * ) &buff[hdr->glsl_vs.offset],hdr->glsl_vs.length);
	}
	if (shader->contents & GSH_CONTENT_GLSL_FS) {
		// We got a fragment shader, load it
		shader->glsl_fragmentShaderSource = malloc(hdr->glsl_fs.size);
		uncompress(( Bytef * )shader->glsl_fragmentShaderSource, ( uLongf * ) &hdr->glsl_fs.size, ( Bytef * ) &buff[hdr->glsl_fs.offset],hdr->glsl_fs.length);
	}
	
	if ((shader->contents & (GSH_CONTENT_GLSL_VS | GSH_CONTENT_GLSL_FS)) && GLSLSupported) {
		GSH_CompileGLSLShader (shader);
	}
#endif

	if (shader->contents & GSH_CONTENT_ARB_VS) {
		// We got a vertex shader, load it
		shader->arb_vertexShaderSource = malloc(hdr->arb_vs.size);
		uncompress(( Bytef * )shader->arb_vertexShaderSource, ( uLongf * ) &hdr->arb_vs.size, ( Bytef * ) &buff[hdr->arb_vs.offset],hdr->arb_vs.length);
	}
	if (shader->contents & GSH_CONTENT_ARB_FS) {
		// We got a fragment shader, load it
		shader->arb_fragmentShaderSource = malloc(hdr->arb_fs.size);
		uncompress(( Bytef * )shader->arb_fragmentShaderSource, ( uLongf * ) &hdr->arb_fs.size, ( Bytef * ) &buff[hdr->arb_fs.offset],hdr->arb_fs.length);
	}
	if (shader->contents & GSH_CONTENT_ARB_VARS) {
		vars = malloc(hdr->arb_vars.size);
		uncompress(( Bytef * )vars, ( uLongf * ) &hdr->arb_vars.size, ( Bytef * ) &buff[hdr->arb_vars.offset], hdr->arb_vars.length);
		// Parse the vars
		var = vars;
		shader->varcount = *(int *)var; var += 4;
		shader->vars = malloc(sizeof(gsh_variable_t) * shader->varcount);
		for (i=0; i < shader->varcount; i++) {
			int tSize;
			shader->vars[i].vsfs = *(char *)var; var += 1;
			shader->vars[i].localidx = *(int *)var; var += 4;
			tSize = *(int *)var; var += 4;
			Q_strncpyz(shader->vars[i].name,var, sizeof(shader->vars[i].name));
			var += tSize;
		}
		free(( void * ) vars);
	}
	free(buff);
	
	if ((shader->contents & (GSH_CONTENT_ARB_VS | GSH_CONTENT_ARB_FS)) && ARBSupported) {
		GSH_CompileARBShader (shader);
	}
	
	return 1;
}

//=========================================================
// GSH_GetUniformLocation
//---------------------------------------------------------
// Description:
// Gets the location of the uniform for the given shader.
//=========================================================
static int GSH_GetUniformLocation ( gshshader_t *shader, const char *uniformName )
{
    int i;
    int location = -1;

    for (i = 0; i < shader->varcount; i++)
    {
        if ( shader->vars[i].vsfs != 1 )
        {
            // Not a uniform variable
            continue;
        }
        
        if ( Q_stricmp (uniformName, shader->vars[i].name) == 0 )
        {
            location = shader->vars[i].localidx;
            break;
        }
    }
    
    return location;
}

extern vmCvar_t	r_force_arb_shaders;

void GSH_UseShader(gshshader_t *shader) {
#ifdef USE_GLSL
	int shaderHasGlsl;
#endif
	
	if (!shader) {
#ifdef USE_GLSL
	    // Release activated shaders
	    if (glslActive) {
		    glUseProgramObjectARB(0);
		    glslActive = 0;
	    }
#endif
	    if (arbActive) {
		    glBindProgramARB(GL_VERTEX_PROGRAM_ARB, 0);
		    glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, 0);
		    glDisable(GL_VERTEX_PROGRAM_ARB);
		    glDisable(GL_FRAGMENT_PROGRAM_ARB);			
		    arbActive = 0;
	    }
	    currentShader = 0;
		return;
	}
	
	if (currentShader == shader) {
	    // Why bind the shader if it's already done? :D
	    return;
	}
	
#ifdef USE_GLSL
	shaderHasGlsl = (shader->contents & (GSH_CONTENT_GLSL_VS | GSH_CONTENT_GLSL_FS));
	// Load GLSL if possible, otherwise fall back to ARB (unless its forced)
	if (GLSLSupported && !r_force_arb_shaders.integer && shaderHasGlsl) { // <-- TODO: checks 'n all
		glUseProgramObjectARB(shader->glsl_program);
		currentShader = shader;
		glslActive = 1;
	} else
#endif
	if (ARBSupported) {
		// Use ARB
		if (shader->arb_vertexShader) {
			glEnable(GL_VERTEX_PROGRAM_ARB);
			glBindProgramARB(GL_VERTEX_PROGRAM_ARB, shader->arb_vertexShader);
			currentShader = shader;
			arbActive = 1;
		}
		
		if (shader->arb_fragmentShader) {
			glEnable(GL_FRAGMENT_PROGRAM_ARB);
			glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, shader->arb_fragmentShader);
			currentShader = shader;
			arbActive = 1;
		}
	}
}
/* Defined in jkg_pixelshaders.h

#define GSH_SetUniform1f( uniform, f1 ) GSH_SetUniformf( uniform, 1, f1, 0, 0, 0)
#define GSH_SetUniform2f( uniform, f1, f2 ) GSH_SetUniformf( uniform, 2, f1, f2, 0, 0)
#define GSH_SetUniform3f( uniform, f1, f2, f3 ) GSH_SetUniformf( uniform, 3, f1, f2, f3, 0)
#define GSH_SetUniform4f( uniform, f1, f2, f3, f4 ) GSH_SetUniformf( uniform, 4, f1, f2, f3, f4)
*/
void GSH_SetUniformf(const char *uniform, int varcount, float f1, float f2, float f3, float f4) {
	if (!currentShader || !uniform || !uniform[0]) {
		return;
	}
#ifdef USE_GLSL
	if (glslActive) {
		// We got GLSL for this, nice :)
		int loc = GSH_GetUniformLocation (currentShader, uniform);
		if (loc != -1) {
			if (varcount == 1) {
				glUniform1fARB(loc, f1);
			} else if (varcount == 2) {
				glUniform2fARB(loc, f1, f2);
			} else if (varcount == 3) {
				glUniform3fARB(loc, f1, f2, f3);
			} else if (varcount == 4) {
				glUniform4fARB(loc, f1, f2, f3, f4);
			}
		}
	}
#endif

    if (arbActive) {
		// Seems we got ARB goin, use our local variable list to find the indexes for both
		// the vertex and fragment programs
		int locv = -1, locf = -1, i;
		for (i=0; i < currentShader->varcount; i++) {
			if (!Q_stricmp(uniform, currentShader->vars[i].name)) {
				if (currentShader->vars[i].vsfs == 0) { // Vertex shader
					locv = currentShader->vars[i].localidx;
				} else if (currentShader->vars[i].vsfs == 1) { // Fragment shader
					locf = currentShader->vars[i].localidx;
				}
				
				break;
			}
		}
		if (locv != -1) {
			// Got a vertex shader entry here
			// ARB doesnt care what data type it is, it always passes 4 floats
			glProgramLocalParameter4fARB(GL_VERTEX_PROGRAM_ARB, locv, f1, f2, f3, f4);
		}
		if (locf != -1) {
			glProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, locf, f1, f2, f3, f4);
		}
	}
}

void GSH_SetUniform1i(const char *uniform, int i1) {
	if (!currentShader || !uniform || !uniform[0]) {
		return;
	}
#ifdef USE_GLSL
	if (glslActive) {
		// We got GLSL for this, nice :)
		int loc = GSH_GetUniformLocation (currentShader, uniform);
		if (loc != -1) {
			glUniform1iARB (loc, i1);
		}
	}
#endif

    /*if (arbActive) {
		// Seems we got ARB goin, use our local variable list to find the indexes for both
		// the vertex and fragment programs
		int locv = -1, locf = -1, i;
		for (i=0; i < currentShader->varcount; i++) {
			if (!Q_stricmp(uniform, currentShader->vars[i].name)) {
				if (currentShader->vars[i].vsfs == 0) { // Vertex shader
					locv = currentShader->vars[i].localidx;
				} else if (currentShader->vars[i].vsfs == 1) { // Fragment shader
					locf = currentShader->vars[i].localidx;
				}
			}
		}
		if (locv != -1) {
			// Got a vertex shader entry here
			// ARB doesnt care what data type it is, it always passes 4 floats
			glProgramLocalParameter4fARB(GL_VERTEX_PROGRAM_ARB, locv, f1, f2, f3, f4);
		}
		if (locf != -1) {
			glProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, locf, f1, f2, f3, f4);
		}
	}*/
}

void GSH_FreeShader(gshshader_t *shader) {
	if (!shader) {
		return;
	}
#ifdef USE_GLSL
	// Free all GLSL related stuff
	if (shader->glsl_program) {
		if (shader->glsl_vertexShader) {
			glDetachObjectARB(shader->glsl_program, shader->glsl_vertexShader);
			glDeleteObjectARB(shader->glsl_vertexShader);
			shader->glsl_vertexShader = 0;
		}
		if (shader->glsl_fragmentShader) {
			glDetachObjectARB(shader->glsl_program, shader->glsl_fragmentShader);
			glDeleteObjectARB(shader->glsl_fragmentShader);
			shader->glsl_fragmentShader = 0;
		}
		glDeleteObjectARB(shader->glsl_fragmentShader);
		shader->glsl_fragmentShader = 0;
	}
#endif
	// Free all ARB related stuff
	if (shader->arb_vertexShader) {
		glDeleteProgramsARB(1, &shader->arb_vertexShader);
		shader->arb_vertexShader = 0;
	}
	if (shader->arb_fragmentShader) {
		glDeleteProgramsARB(1, &shader->arb_fragmentShader);
		shader->arb_fragmentShader = 0;
	}

    // Shader source is deleted after it's first uploaded to the GPU.

	if (shader->vars) {
		free((void *)shader->vars);
	}
	shader->varcount = 0;
	shader->contents = 0;
	memset(shader->name, 0, 64);
	shader->arb_compiled = 0;
}

qboolean GSH_Init() {
#ifdef USE_GLSL
    GLSLSupported = (qboolean)(GLEW_ARB_vertex_shader && GLEW_ARB_fragment_shader && GLEW_ARB_shading_language_100 && GLEW_ARB_shader_objects);
#else
    trap_Cvar_Set ("r_force_arb_shaders", "1");
#endif
    ARBSupported = (qboolean)(GLEW_ARB_vertex_program && GLEW_ARB_fragment_program);
	
	if ( !GLSLSupported && ARBSupported )
	{
	    CG_Printf (S_COLOR_YELLOW "...GLSL not supported, falling back to ARB assembly shaders.\n");
	}
	
	return (qboolean)(ARBSupported || GLSLSupported);
}