///////////////////////////////////
//
// YUV 4:2:0 to RGBA32 conversion code
//
// By BobaFett
//
//////////////////////////////////

///// MMX optimized converter /////

// Lookup tables

#define PRECISION 64		// Changes to this require changes to the assembly as well!


// Y to RGBA conversion macro for lookup table
#define RGBY( Y ) { \
	( short ) ( 1.164 * PRECISION * ( Y - 16 ) + 0.5 ), \
	( short ) ( 1.164 * PRECISION * ( Y - 16 ) + 0.5 ), \
	( short ) ( 1.164 * PRECISION * ( Y - 16 ) + 0.5 ), \
	0x7FFF, \
}

// V to RGBA conversion macro for lookup table
#define RGBV( V ) { \
	( short ) ( 1.596 * PRECISION * ( V - 128 ) + 0.5 ), \
	( short ) (-0.813 * PRECISION * ( V - 128 ) + 0.5 ), \
	0x00, \
	0x00, \
}

// U to RGB conversion macro for lookup table
#define RGBU( U ) { \
	0x00,\
	( short ) (-0.391 * PRECISION * ( U - 128 ) + 0.5 ), \
	( short ) ( 2.018 * PRECISION * ( U - 128 ) + 0.5 ), \
	0x00,\
}

// Y value lookup table [16..235] range
static const short CoefficientRGBY[256][4] = {
	RGBY(0x10), RGBY(0x10), RGBY(0x10), RGBY(0x10),
	RGBY(0x10), RGBY(0x10), RGBY(0x10), RGBY(0x10),
	RGBY(0x10), RGBY(0x10), RGBY(0x10), RGBY(0x10),
	RGBY(0x10), RGBY(0x10), RGBY(0x10), RGBY(0x10),

	RGBY(0x10), RGBY(0x11), RGBY(0x12), RGBY(0x13),
	RGBY(0x14), RGBY(0x15), RGBY(0x16), RGBY(0x17),
	RGBY(0x18), RGBY(0x19), RGBY(0x1A), RGBY(0x1B),
	RGBY(0x1C), RGBY(0x1D), RGBY(0x1E), RGBY(0x1F),

	RGBY(0x20), RGBY(0x21), RGBY(0x22), RGBY(0x23),
	RGBY(0x24), RGBY(0x25), RGBY(0x26), RGBY(0x27),
	RGBY(0x28), RGBY(0x29), RGBY(0x2A), RGBY(0x2B),
	RGBY(0x2C), RGBY(0x2D), RGBY(0x2E), RGBY(0x2F),

	RGBY(0x30), RGBY(0x31), RGBY(0x32), RGBY(0x33),
	RGBY(0x34), RGBY(0x35), RGBY(0x36), RGBY(0x37),
	RGBY(0x38), RGBY(0x39), RGBY(0x3A), RGBY(0x3B),
	RGBY(0x3C), RGBY(0x3D), RGBY(0x3E), RGBY(0x3F),

	RGBY(0x40), RGBY(0x41), RGBY(0x42), RGBY(0x43),
	RGBY(0x44), RGBY(0x45), RGBY(0x46), RGBY(0x47),
	RGBY(0x48), RGBY(0x49), RGBY(0x4A), RGBY(0x4B),
	RGBY(0x4C), RGBY(0x4D), RGBY(0x4E), RGBY(0x4F),

	RGBY(0x50), RGBY(0x51), RGBY(0x52), RGBY(0x53),
	RGBY(0x54), RGBY(0x55), RGBY(0x56), RGBY(0x57),
	RGBY(0x58), RGBY(0x59), RGBY(0x5A), RGBY(0x5B),
	RGBY(0x5C), RGBY(0x5D), RGBY(0x5E), RGBY(0x5F),

	RGBY(0x60), RGBY(0x61), RGBY(0x62), RGBY(0x63),
	RGBY(0x64), RGBY(0x65), RGBY(0x66), RGBY(0x67),
	RGBY(0x68), RGBY(0x69), RGBY(0x6A), RGBY(0x6B),
	RGBY(0x6C), RGBY(0x6D), RGBY(0x6E), RGBY(0x6F),

	RGBY(0x70), RGBY(0x71), RGBY(0x72), RGBY(0x73),
	RGBY(0x74), RGBY(0x75), RGBY(0x76), RGBY(0x77),
	RGBY(0x78), RGBY(0x79), RGBY(0x7A), RGBY(0x7B),
	RGBY(0x7C), RGBY(0x7D), RGBY(0x7E), RGBY(0x7F),

	RGBY(0x80), RGBY(0x81), RGBY(0x82), RGBY(0x83),
	RGBY(0x84), RGBY(0x85), RGBY(0x86), RGBY(0x87),
	RGBY(0x88), RGBY(0x89), RGBY(0x8A), RGBY(0x8B),
	RGBY(0x8C), RGBY(0x8D), RGBY(0x8E), RGBY(0x8F),

	RGBY(0x90), RGBY(0x91), RGBY(0x92), RGBY(0x93),
	RGBY(0x94), RGBY(0x95), RGBY(0x96), RGBY(0x97),
	RGBY(0x98), RGBY(0x99), RGBY(0x9A), RGBY(0x9B),
	RGBY(0x9C), RGBY(0x9D), RGBY(0x9E), RGBY(0x9F),

	RGBY(0xA0), RGBY(0xA1), RGBY(0xA2), RGBY(0xA3),
	RGBY(0xA4), RGBY(0xA5), RGBY(0xA6), RGBY(0xA7),
	RGBY(0xA8), RGBY(0xA9), RGBY(0xAA), RGBY(0xAB),
	RGBY(0xAC), RGBY(0xAD), RGBY(0xAE), RGBY(0xAF),

	RGBY(0xB0), RGBY(0xB1), RGBY(0xB2), RGBY(0xB3),
	RGBY(0xB4), RGBY(0xB5), RGBY(0xB6), RGBY(0xB7),
	RGBY(0xB8), RGBY(0xB9), RGBY(0xBA), RGBY(0xBB),
	RGBY(0xBC), RGBY(0xBD), RGBY(0xBE), RGBY(0xBF),

	RGBY(0xC0), RGBY(0xC1), RGBY(0xC2), RGBY(0xC3),
	RGBY(0xC4), RGBY(0xC5), RGBY(0xC6), RGBY(0xC7),
	RGBY(0xC8), RGBY(0xC9), RGBY(0xCA), RGBY(0xCB),
	RGBY(0xCC), RGBY(0xCD), RGBY(0xCE), RGBY(0xCF),

	RGBY(0xD0), RGBY(0xD1), RGBY(0xD2), RGBY(0xD3),
	RGBY(0xD4), RGBY(0xD5), RGBY(0xD6), RGBY(0xD7),
	RGBY(0xD8), RGBY(0xD9), RGBY(0xDA), RGBY(0xDB),
	RGBY(0xDC), RGBY(0xDD), RGBY(0xDE), RGBY(0xDF),

	RGBY(0xE0), RGBY(0xE1), RGBY(0xE2), RGBY(0xE3),
	RGBY(0xE4), RGBY(0xE5), RGBY(0xE6), RGBY(0xE7),
	RGBY(0xE8), RGBY(0xE9), RGBY(0xEA), RGBY(0xEB),
	RGBY(0xEB), RGBY(0xEB), RGBY(0xEB), RGBY(0xEB),

	RGBY(0xEB), RGBY(0xEB), RGBY(0xEB), RGBY(0xEB),
	RGBY(0xEB), RGBY(0xEB), RGBY(0xEB), RGBY(0xEB),
	RGBY(0xEB), RGBY(0xEB), RGBY(0xEB), RGBY(0xEB),
	RGBY(0xEB), RGBY(0xEB), RGBY(0xEB), RGBY(0xEB),
};


// U value lookup table [16..240] range
static const short CoefficientRGBU[256][4] = {
	RGBU(0x10), RGBU(0x10), RGBU(0x10), RGBU(0x10),
	RGBU(0x10), RGBU(0x10), RGBU(0x10), RGBU(0x10),
	RGBU(0x10), RGBU(0x10), RGBU(0x10), RGBU(0x10),
	RGBU(0x10), RGBU(0x10), RGBU(0x10), RGBU(0x10),

	RGBU(0x10), RGBU(0x11), RGBU(0x12), RGBU(0x13),
	RGBU(0x14), RGBU(0x15), RGBU(0x16), RGBU(0x17),
	RGBU(0x18), RGBU(0x19), RGBU(0x1A), RGBU(0x1B),
	RGBU(0x1C), RGBU(0x1D), RGBU(0x1E), RGBU(0x1F),

	RGBU(0x20), RGBU(0x21), RGBU(0x22), RGBU(0x23),
	RGBU(0x24), RGBU(0x25), RGBU(0x26), RGBU(0x27),
	RGBU(0x28), RGBU(0x29), RGBU(0x2A), RGBU(0x2B),
	RGBU(0x2C), RGBU(0x2D), RGBU(0x2E), RGBU(0x2F),

	RGBU(0x30), RGBU(0x31), RGBU(0x32), RGBU(0x33),
	RGBU(0x34), RGBU(0x35), RGBU(0x36), RGBU(0x37),
	RGBU(0x38), RGBU(0x39), RGBU(0x3A), RGBU(0x3B),
	RGBU(0x3C), RGBU(0x3D), RGBU(0x3E), RGBU(0x3F),

	RGBU(0x40), RGBU(0x41), RGBU(0x42), RGBU(0x43),
	RGBU(0x44), RGBU(0x45), RGBU(0x46), RGBU(0x47),
	RGBU(0x48), RGBU(0x49), RGBU(0x4A), RGBU(0x4B),
	RGBU(0x4C), RGBU(0x4D), RGBU(0x4E), RGBU(0x4F),

	RGBU(0x50), RGBU(0x51), RGBU(0x52), RGBU(0x53),
	RGBU(0x54), RGBU(0x55), RGBU(0x56), RGBU(0x57),
	RGBU(0x58), RGBU(0x59), RGBU(0x5A), RGBU(0x5B),
	RGBU(0x5C), RGBU(0x5D), RGBU(0x5E), RGBU(0x5F),

	RGBU(0x60), RGBU(0x61), RGBU(0x62), RGBU(0x63),
	RGBU(0x64), RGBU(0x65), RGBU(0x66), RGBU(0x67),
	RGBU(0x68), RGBU(0x69), RGBU(0x6A), RGBU(0x6B),
	RGBU(0x6C), RGBU(0x6D), RGBU(0x6E), RGBU(0x6F),

	RGBU(0x70), RGBU(0x71), RGBU(0x72), RGBU(0x73),
	RGBU(0x74), RGBU(0x75), RGBU(0x76), RGBU(0x77),
	RGBU(0x78), RGBU(0x79), RGBU(0x7A), RGBU(0x7B),
	RGBU(0x7C), RGBU(0x7D), RGBU(0x7E), RGBU(0x7F),

	RGBU(0x80), RGBU(0x81), RGBU(0x82), RGBU(0x83),
	RGBU(0x84), RGBU(0x85), RGBU(0x86), RGBU(0x87),
	RGBU(0x88), RGBU(0x89), RGBU(0x8A), RGBU(0x8B),
	RGBU(0x8C), RGBU(0x8D), RGBU(0x8E), RGBU(0x8F),

	RGBU(0x90), RGBU(0x91), RGBU(0x92), RGBU(0x93),
	RGBU(0x94), RGBU(0x95), RGBU(0x96), RGBU(0x97),
	RGBU(0x98), RGBU(0x99), RGBU(0x9A), RGBU(0x9B),
	RGBU(0x9C), RGBU(0x9D), RGBU(0x9E), RGBU(0x9F),

	RGBU(0xA0), RGBU(0xA1), RGBU(0xA2), RGBU(0xA3),
	RGBU(0xA4), RGBU(0xA5), RGBU(0xA6), RGBU(0xA7),
	RGBU(0xA8), RGBU(0xA9), RGBU(0xAA), RGBU(0xAB),
	RGBU(0xAC), RGBU(0xAD), RGBU(0xAE), RGBU(0xAF),

	RGBU(0xB0), RGBU(0xB1), RGBU(0xB2), RGBU(0xB3),
	RGBU(0xB4), RGBU(0xB5), RGBU(0xB6), RGBU(0xB7),
	RGBU(0xB8), RGBU(0xB9), RGBU(0xBA), RGBU(0xBB),
	RGBU(0xBC), RGBU(0xBD), RGBU(0xBE), RGBU(0xBF),

	RGBU(0xC0), RGBU(0xC1), RGBU(0xC2), RGBU(0xC3),
	RGBU(0xC4), RGBU(0xC5), RGBU(0xC6), RGBU(0xC7),
	RGBU(0xC8), RGBU(0xC9), RGBU(0xCA), RGBU(0xCB),
	RGBU(0xCC), RGBU(0xCD), RGBU(0xCE), RGBU(0xCF),

	RGBU(0xD0), RGBU(0xD1), RGBU(0xD2), RGBU(0xD3),
	RGBU(0xD4), RGBU(0xD5), RGBU(0xD6), RGBU(0xD7),
	RGBU(0xD8), RGBU(0xD9), RGBU(0xDA), RGBU(0xDB),
	RGBU(0xDC), RGBU(0xDD), RGBU(0xDE), RGBU(0xDF),

	RGBU(0xE0), RGBU(0xE1), RGBU(0xE2), RGBU(0xE3),
	RGBU(0xE4), RGBU(0xE5), RGBU(0xE6), RGBU(0xE7),
	RGBU(0xE8), RGBU(0xE9), RGBU(0xEA), RGBU(0xEB),
	RGBU(0xEC), RGBU(0xED), RGBU(0xEE), RGBU(0xEF),

	RGBU(0xF0), RGBU(0xF0), RGBU(0xF0), RGBU(0xF0),
	RGBU(0xF0), RGBU(0xF0), RGBU(0xF0), RGBU(0xF0),
	RGBU(0xF0), RGBU(0xF0), RGBU(0xF0), RGBU(0xF0),
	RGBU(0xF0), RGBU(0xF0), RGBU(0xF0), RGBU(0xF0),
};


// V value lookup table [16..240] range
static const short CoefficientRGBV[256][4] = {
	RGBV(0x10), RGBV(0x10), RGBV(0x10), RGBV(0x10),
	RGBV(0x10), RGBV(0x10), RGBV(0x10), RGBV(0x10),
	RGBV(0x10), RGBV(0x10), RGBV(0x10), RGBV(0x10),
	RGBV(0x10), RGBV(0x10), RGBV(0x10), RGBV(0x10),

	RGBV(0x10), RGBV(0x11), RGBV(0x12), RGBV(0x13),
	RGBV(0x14), RGBV(0x15), RGBV(0x16), RGBV(0x17),
	RGBV(0x18), RGBV(0x19), RGBV(0x1A), RGBV(0x1B),
	RGBV(0x1C), RGBV(0x1D), RGBV(0x1E), RGBV(0x1F),

	RGBV(0x20), RGBV(0x21), RGBV(0x22), RGBV(0x23),
	RGBV(0x24), RGBV(0x25), RGBV(0x26), RGBV(0x27),
	RGBV(0x28), RGBV(0x29), RGBV(0x2A), RGBV(0x2B),
	RGBV(0x2C), RGBV(0x2D), RGBV(0x2E), RGBV(0x2F),

	RGBV(0x30), RGBV(0x31), RGBV(0x32), RGBV(0x33),
	RGBV(0x34), RGBV(0x35), RGBV(0x36), RGBV(0x37),
	RGBV(0x38), RGBV(0x39), RGBV(0x3A), RGBV(0x3B),
	RGBV(0x3C), RGBV(0x3D), RGBV(0x3E), RGBV(0x3F),

	RGBV(0x40), RGBV(0x41), RGBV(0x42), RGBV(0x43),
	RGBV(0x44), RGBV(0x45), RGBV(0x46), RGBV(0x47),
	RGBV(0x48), RGBV(0x49), RGBV(0x4A), RGBV(0x4B),
	RGBV(0x4C), RGBV(0x4D), RGBV(0x4E), RGBV(0x4F),

	RGBV(0x50), RGBV(0x51), RGBV(0x52), RGBV(0x53),
	RGBV(0x54), RGBV(0x55), RGBV(0x56), RGBV(0x57),
	RGBV(0x58), RGBV(0x59), RGBV(0x5A), RGBV(0x5B),
	RGBV(0x5C), RGBV(0x5D), RGBV(0x5E), RGBV(0x5F),

	RGBV(0x60), RGBV(0x61), RGBV(0x62), RGBV(0x63),
	RGBV(0x64), RGBV(0x65), RGBV(0x66), RGBV(0x67),
	RGBV(0x68), RGBV(0x69), RGBV(0x6A), RGBV(0x6B),
	RGBV(0x6C), RGBV(0x6D), RGBV(0x6E), RGBV(0x6F),

	RGBV(0x70), RGBV(0x71), RGBV(0x72), RGBV(0x73),
	RGBV(0x74), RGBV(0x75), RGBV(0x76), RGBV(0x77),
	RGBV(0x78), RGBV(0x79), RGBV(0x7A), RGBV(0x7B),
	RGBV(0x7C), RGBV(0x7D), RGBV(0x7E), RGBV(0x7F),

	RGBV(0x80), RGBV(0x81), RGBV(0x82), RGBV(0x83),
	RGBV(0x84), RGBV(0x85), RGBV(0x86), RGBV(0x87),
	RGBV(0x88), RGBV(0x89), RGBV(0x8A), RGBV(0x8B),
	RGBV(0x8C), RGBV(0x8D), RGBV(0x8E), RGBV(0x8F),

	RGBV(0x90), RGBV(0x91), RGBV(0x92), RGBV(0x93),
	RGBV(0x94), RGBV(0x95), RGBV(0x96), RGBV(0x97),
	RGBV(0x98), RGBV(0x99), RGBV(0x9A), RGBV(0x9B),
	RGBV(0x9C), RGBV(0x9D), RGBV(0x9E), RGBV(0x9F),

	RGBV(0xA0), RGBV(0xA1), RGBV(0xA2), RGBV(0xA3),
	RGBV(0xA4), RGBV(0xA5), RGBV(0xA6), RGBV(0xA7),
	RGBV(0xA8), RGBV(0xA9), RGBV(0xAA), RGBV(0xAB),
	RGBV(0xAC), RGBV(0xAD), RGBV(0xAE), RGBV(0xAF),

	RGBV(0xB0), RGBV(0xB1), RGBV(0xB2), RGBV(0xB3),
	RGBV(0xB4), RGBV(0xB5), RGBV(0xB6), RGBV(0xB7),
	RGBV(0xB8), RGBV(0xB9), RGBV(0xBA), RGBV(0xBB),
	RGBV(0xBC), RGBV(0xBD), RGBV(0xBE), RGBV(0xBF),

	RGBV(0xC0), RGBV(0xC1), RGBV(0xC2), RGBV(0xC3),
	RGBV(0xC4), RGBV(0xC5), RGBV(0xC6), RGBV(0xC7),
	RGBV(0xC8), RGBV(0xC9), RGBV(0xCA), RGBV(0xCB),
	RGBV(0xCC), RGBV(0xCD), RGBV(0xCE), RGBV(0xCF),

	RGBV(0xD0), RGBV(0xD1), RGBV(0xD2), RGBV(0xD3),
	RGBV(0xD4), RGBV(0xD5), RGBV(0xD6), RGBV(0xD7),
	RGBV(0xD8), RGBV(0xD9), RGBV(0xDA), RGBV(0xDB),
	RGBV(0xDC), RGBV(0xDD), RGBV(0xDE), RGBV(0xDF),

	RGBV(0xE0), RGBV(0xE1), RGBV(0xE2), RGBV(0xE3),
	RGBV(0xE4), RGBV(0xE5), RGBV(0xE6), RGBV(0xE7),
	RGBV(0xE8), RGBV(0xE9), RGBV(0xEA), RGBV(0xEB),
	RGBV(0xEC), RGBV(0xED), RGBV(0xEE), RGBV(0xEF),

	RGBV(0xF0), RGBV(0xF0), RGBV(0xF0), RGBV(0xF0),
	RGBV(0xF0), RGBV(0xF0), RGBV(0xF0), RGBV(0xF0),
	RGBV(0xF0), RGBV(0xF0), RGBV(0xF0), RGBV(0xF0),
	RGBV(0xF0), RGBV(0xF0), RGBV(0xF0), RGBV(0xF0),
};

 void __declspec(naked) __cdecl Yuv2rgb32_MMX(const unsigned char *y, const unsigned char *u, const unsigned char *v,
						int width, int height, int y_stride, int uv_stride, unsigned int *output)
{
	(void)y; (void)u; (void)v; (void)width; (void)height; (void)y_stride; (void)uv_stride; (void)output; // Supress warnings
	__asm
	{
		pushad
		finit
		sub esp, 0x8

		// Set up strides

		// Y-stride
		mov eax,	[esp + 0x28 + 0x18]		// Load y_stride
		sub eax,	[esp + 0x28 + 0x10]		// Subtract width from it
		mov [esp + 0x20], eax				// Save y_stride extra

		// UV-stride
		mov eax,	[esp + 0x28 + 0x1C]		// Load uv_stride
		mov ebx,	[esp + 0x28 + 0x10]		// Load width
		shr ebx,	1						// ebx >> 1 (divide by 2)
		sub eax,	ebx						// Subtract width from it
		mov [esp + 0x24], eax				// Save uv_stride extra

		// Load arguments
		mov edx,	[esp + 0x28 + 0x4]		// Load Y into edx
		mov edi,	[esp + 0x28 + 0x8]		// Load U into edi
		mov esi,	[esp + 0x28 + 0xC]		// Load V into esi

		mov ebx,	[esp + 0x28 + 0x14]		// Load height into ebx
		//mov ecx,	[esp + 0x28 + 0x10]		// Load width into ecx

		mov ebp,	[esp + 0x28 + 0x20]		// Load output into ebp

		xor eax,	eax

hloop:
		push ebx							// Push height
		push eax							// Push even/uneven

		mov eax, [esp + 0x28 + 0x10 + 0x08]	// Move width in eax
		shr eax, 1							// Divide by 2

		xor ebx,	ebx						// Set ebx and ecx to 0
		xor ecx,	ecx
wloop:

			// Conversion
			mov bl,		byte ptr [edi]					// Load value of U into eax
			mov cl,		byte ptr [esi]					// Load value of V into ebx

			movq mm0,	[CoefficientRGBU + 8 * ebx]	// Load the RGBU coefficient for the U into mm0 (64-bit)
			paddw mm0,	[CoefficientRGBV + 8 * ecx]	// Add the  RGBV coefficient for the V to mm0 (64-bit)

			mov bl,		byte ptr [edx]					// Load value of Y into eax
			mov cl,		byte ptr [edx+1]				// Load next value of Y into ebx

			movq mm1,	[CoefficientRGBY + 8 * ebx]	// Load RGBY coefficients for both Y values into mm1 and mm2
			movq mm2,	[CoefficientRGBY + 8 * ecx]


			paddw mm1,	mm0		// Add the Y coefficients to the UV coefficients for both pixels
			paddw mm2,	mm0

			psraw mm1,	6		// Shift 6 places (/ 64)
			psraw mm2,	6

			packuswb mm1,	mm2	// Pack the two mm registers together to form the bytes of 2 pixels

			movq [ebp],	mm1		// Store the two pixels in the buffer

			add ebp, 8
			add edx, 2
			inc edi
			inc esi

			dec eax
		jnz wloop

		// Advance to the next line
		pop eax
		pop ebx

		dec ebx			// Next line
		jz done

		add edx, [esp + 0x20]	// Add y-stride
		add edi, [esp + 0x24]	// Add uv-stride
		add esi, [esp + 0x24]	// Add uv-stride

		xor eax, 1		// Invert eax
		jne hloop

		sub edi, [esp + 0x28 + 0x1C]	// Subtract uv-stride (start over on this line)
		sub esi, [esp + 0x28 + 0x1C]
		jmp hloop
done:

		add esp, 0x8
		emms			// Shut down MMX
		popad			// Restore registers
		xor eax,eax		// This is a void function, so clear eax
		ret
	}
}

void __declspec(naked) __cdecl Yuv2rgb32luma_MMX(const unsigned char *y, const unsigned char *u, const unsigned char *v,
						int width, int height, int y_stride, int uv_stride, unsigned int *output)
{
	(void)y; (void)u; (void)v; (void)width; (void)height; (void)y_stride; (void)uv_stride; (void)output; // Supress warnings
	__asm
	{
		pushad
		finit
		sub esp, 0x8

		// Set up strides

		// Y-stride
		mov eax,	[esp + 0x28 + 0x18]		// Load y_stride
		sub eax,	[esp + 0x28 + 0x10]		// Subtract width from it
		mov [esp + 0x20], eax				// Save y_stride extra

		// UV-stride
		mov eax,	[esp + 0x28 + 0x1C]		// Load uv_stride
		mov ebx,	[esp + 0x28 + 0x10]		// Load width
		shr ebx,	1						// ebx >> 1 (divide by 2)
		sub eax,	ebx						// Subtract width from it
		mov [esp + 0x24], eax				// Save uv_stride extra

		// Load arguments
		mov edx,	[esp + 0x28 + 0x4]		// Load Y into edx

		mov ebx,	[esp + 0x28 + 0x14]		// Load height into ebx
		mov ecx,	[esp + 0x28 + 0x10]		// Load width into ecx

		mov ebp,	[esp + 0x28 + 0x20]		// Load output into ebp

		xor eax,	eax

hloop:
		push ebx							// Push height
		mov ebx,	ecx						// Move width in ebx
		push eax							// Push even/uneven
wloop:
		push ebx							// Push width
		xor ebx,	ebx						// Set ebx to 0
		xor eax,	eax
		
		// Conversion
		mov al,		byte ptr [edx]					// Load value of Y into eax
		mov bl,		byte ptr [edx+1]				// Load next value of Y into ebx

		movq mm0,	[CoefficientRGBY + 8 * eax]	// Load RGBY coefficients for both Y values into mm1 and mm2
		movq mm1,	[CoefficientRGBY + 8 * ebx]

		psraw mm0,	6		// Shift 6 places (/ 64)
		psraw mm1,	6

		packuswb mm0,	mm1	// Pack the two mm registers together to form the bytes of 2 pixels

		movq [ebp],	mm0		// Store the two pixels in the buffer

		add ebp, 8
		add edx, 2

		pop ebx
		sub ebx,2
		jnz wloop

		// Advance to the next line
		pop eax
		pop ebx

		dec ebx			// Next line
		jz done

		add edx, [esp + 0x20]	// Add y-stride

		xor eax, 1		// Invert eax
		jne hloop

		jmp hloop
done:

		add esp, 0x8
		emms			// Shut down MMX
		popad			// Restore registers
		xor eax,eax		// This is a void function, so clear eax
		ret
	}
}


/*
void __declspec(naked) __cdecl Yuv2rgb32_MMX(const unsigned char *y, const unsigned char *u, const unsigned char *v,
						int width, int height, int y_stride, int uv_stride, unsigned int *output)
{
	(void)y; (void)u; (void)v; (void)width; (void)height; (void)y_stride; (void)uv_stride; (void)output; // Supress warnings
__asm
	{
		pushad
		finit
		sub esp, 0x20

		// Stack layout:
		// [esp + 0x0] - y_stride extra
		// [esp + 0x4] - uv_stride extra
		// [esp + 0x8] - uv_stride rollback
		// [esp + 0xC] - Scanlines remaining
		// [esp + 0x10] - Even/Odd line (1/0)
		// [esp + 0x14] - Scanline pixels remaining
		// [esp + 0x18] - Unused
		// [esp + 0x1C] - Unused

		// Set up strides

		// Y-stride
		mov eax,	[esp + 0x40 + 0x18]		// Load y_stride
		sub eax,	[esp + 0x40 + 0x10]		// Subtract width from it
		mov [esp + 0x0], eax				// Save y_stride extra

		// UV-stride
		mov eax,	[esp + 0x40 + 0x1C]		// Load uv_stride
		mov ebx,	[esp + 0x40 + 0x10]		// Load width
		shr ebx,	1						// ebx >> 1 (divide by 2)
		mov	[esp + 0x8], ebx				// Save uv_stride rollback
		sub eax,	ebx						// Subtract width from it
		mov [esp + 0x4], eax				// Save uv_stride extra

		// Load arguments
		mov edx,	[esp + 0x40 + 0x4]		// Load Y into edx
		mov edi,	[esp + 0x40 + 0x8]		// Load U into edi
		mov esi,	[esp + 0x40 + 0xC]		// Load V into esi

		mov ebx,	[esp + 0x40 + 0x14]		// Load height into ebx
		mov ecx,	[esp + 0x40 + 0x10]		// Load width into ecx

		mov ebp,	[esp + 0x40 + 0x20]		// Load output into ebp

		xor eax,	eax
		mov [esp + 0x10], eax				// Scanline odd/even
		
		mov [esp + 0xC], ebx				// Scanlines remaining

		
hloop:
		mov [esp + 0x14], ecx				// Scanline pixels remaining
		
		xor ebx,	ebx						// Set ebx to 0
		xor eax,	eax
wloop:

		// Conversion
		mov al,		byte ptr [edi]					// Load value of U into eax
		mov bl,		byte ptr [esi]					// Load value of V into ebx

		movq mm0,	[CoefficientRGBU + 8 * eax]	// Load the RGBU coefficient for the U into mm0 (64-bit)
		paddw mm0,	[CoefficientRGBV + 8 * ebx]	// Add the  RGBV coefficient for the V to mm0 (64-bit)

		mov al,		byte ptr [edx]					// Load value of Y into eax
		mov bl,		byte ptr [edx+1]				// Load next value of Y into ebx

		movq mm1,	[CoefficientRGBY + 8 * eax]	// Load RGBY coefficients for both Y values into mm1 and mm2
		movq mm2,	[CoefficientRGBY + 8 * ebx]


		paddw mm1,	mm0		// Add the Y coefficients to the UV coefficients for both pixels
		paddw mm2,	mm0

		psraw mm1,	6		// Shift 6 places (/ 64)
		psraw mm2,	6

		packuswb mm1,	mm2	// Pack the two mm registers together to form the bytes of 2 pixels

		movq [ebp],	mm1		// Store the two pixels in the buffer

		add ebp, 8
		add edx, 2
		add edi, 1
		add esi, 1

		sub dword ptr [esp + 0x14], 4
		jnz wloop

		dec dword ptr [esp + 0xC]			// Next line
		jz done

		add edx, [esp + 0x0]	// Add y-stride

		xor dword ptr [esp + 0x10], 1
		jz evenline
		
		sub edi, [esp + 0x8]	// Subtract uv-stride (start over on this line)
		sub esi, [esp + 0x8]
		jmp hloop
evenline:
		add edi, [esp + 0x4]	// Add uv-stride
		add esi, [esp + 0x4]	// Add uv-stride
		jmp hloop	
done:
		add esp, 0x20
		emms			// Shut down MMX
		popad			// Restore registers
		xor eax,eax		// This is a void function, so clear eax
		ret
	}
}
*/