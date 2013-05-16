////////////////////////////////
//
//  Pixel Shaders module
//
//  Handles all pixel shader operations
//  as well as loading GSH files
//
////////////////////////////////

#define GSH_HDR				(('F'<<24)+('H'<<16)+('S'<<8)+'G')		// GSHF
#define GSH_VERSION			1

#define GSH_CONTENT_GLSL_VS		1
#define GSH_CONTENT_GLSL_FS		2
#define GSH_CONTENT_ARB_VS		4
#define GSH_CONTENT_ARB_FS		8
#define GSH_CONTENT_ARB_VARS	16

#define GSH_SetUniform1f( uniform, f1 ) GSH_SetUniformf( uniform, 1, f1, 0, 0, 0)
#define GSH_SetUniform2f( uniform, f1, f2 ) GSH_SetUniformf( uniform, 2, f1, f2, 0, 0)
#define GSH_SetUniform3f( uniform, f1, f2, f3 ) GSH_SetUniformf( uniform, 3, f1, f2, f3, 0)
#define GSH_SetUniform4f( uniform, f1, f2, f3, f4 ) GSH_SetUniformf( uniform, 4, f1, f2, f3, f4)

typedef struct {
	char name[64];
	char vsfs;				// 0 = vertex, 1 = fragment
	int localidx;
} gsh_variable_t;


typedef struct {
	char name[64];
	int contents;
	
	unsigned int glsl_program;
	unsigned int glsl_vertexShader;
	const char *glsl_vertexShaderSource;
	unsigned int glsl_fragmentShader;
	const char *glsl_fragmentShaderSource;

	unsigned int arb_vertexShader;
	const char *arb_vertexShaderSource;
	unsigned int arb_fragmentShader;
	const char *arb_fragmentShaderSource;
	int arb_compiled;
	
	int	varcount;
	gsh_variable_t *vars;
} gshshader_t;

typedef struct {
	int offset;
	int length;
	int size;
} gsh_data_entry_t;

typedef struct {
	int header;
	int version;
	char content;			//1 or more GSH_CONTENT_xxx constants
	char name[64];

	gsh_data_entry_t glsl_vs;
	gsh_data_entry_t glsl_fs;

	gsh_data_entry_t arb_vs;
	gsh_data_entry_t arb_fs;
	gsh_data_entry_t arb_vars;
} gsh_header_t;

int GSH_LoadShader(gshshader_t *shader, const char *filename);
qboolean GSH_Init();
void GSH_FreeShader(gshshader_t *shader);
void GSH_SetUniformf(const char *uniform, int varcount, float f1, float f2, float f3, float f4);
void GSH_SetUniform1i(const char *uniform, int i1);
void GSH_UseShader(gshshader_t *shader);
