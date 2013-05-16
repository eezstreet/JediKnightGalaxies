///////////////////////////////////
//
// OGM/OGV Playback interface
//
//////////////////////////////////

// This is the interface used to manage the playback
// of OGM/OGV files
// This is linked direcly to the CIN interface defined in gl_cin.c

// -- Code heavilly based on Xreal

#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <theora/theoradec.h>
#include <theora/codec.h>
#include <string.h>
#include <stdlib.h>

#include "gl_enginefuncs.h"

//#include "client.h"
//#include "snd_local.h"

#define NO_YUV_BACKUP 1			// Set to 0 to implement a backup for the yuv converter for non-MMX capable CPU's

// Global optimization causes this code to crash (due to optimization errors), so disable it
#pragma optimize( "g", off )

void HRT_Start(int TimerID);
void HRT_Stop(int TimerID);
double HRT_GetTimingMMS(int TimerID);
double HRT_GetTimingMS(int TimerID);
double HRT_GetTimingS(int TimerID);


// Cin System Flags
#define CIN_system	1
#define CIN_loop	2
#define	CIN_hold	4
#define CIN_silent	8
#define CIN_shader	16

#define MAX_VIDEO_HANDLES	16

#define OGG_BUFFER_SIZE	8*1024	//4096
typedef int fileHandle_t;
typedef char byte;

#define S_COLOR_RED "^1"
#define S_COLOR_YELLOW "^3"

typedef struct
{
	fileHandle_t    ogmFile;

	ogg_sync_state  oy;			/* sync and verify incoming physical bitstream */
	//ogg_stream_state os; /* take physical pages, weld into a logical stream of packets */
	ogg_stream_state os_audio;
	ogg_stream_state os_video;

	vorbis_dsp_state vd;		/* central working state for the packet->PCM decoder */
	vorbis_info     vi;			/* struct that stores all the static vorbis bitstream settings */
	vorbis_comment  vc;			/* struct that stores all the bitstream user comments */

	th_info				ti;	// dump_video.c(example decoder): ti
	th_comment			tc;	// dump_video.c(example decoder): tc
	th_dec_ctx			*td;	// dump_video.c(example decoder): td
	th_setup_info		*ts;

	th_ycbcr_buffer th_yuvbuffer;

	unsigned char  *outputBuffer;
	int             outputWidth;
	int             outputHeight;
	int             outputBufferSize;	// in Pixel (so "real Bytesize" = outputBufferSize*4)
	ogg_int64_t     VFrameCount;	// output video-stream
	ogg_int64_t     Vtime_unit;
	int             currentTime;	// input from Run-function

	int				newFrame;
	int				systemFlags;	// Passed over by Cin system

	qboolean		hasVideo;
	qboolean		hasAudio;

	double			aspectRatio;
} cin_ogm_t;

static cin_ogm_t cin_ogm[MAX_VIDEO_HANDLES];
static cin_ogm_t *currentOgm;

int             nextNeededVFrame();

/* ####################### #######################

  OGG/OGM
  ... also calls to vorbis/theora-libs

*/

/*
  loadBlockToSync

  return:
  !0 -> no data transferred
*/

static int loadBlockToSync()
{
	char           *buffer;
	int             bytes;

	if(currentOgm->ogmFile)
	{
		//Com_DPrintf("OGM - Loading 8192 bytes from file... (handle %i)\n", handle);
		buffer = ogg_sync_buffer(&currentOgm->oy, OGG_BUFFER_SIZE);
		bytes = FS_Read(buffer, OGG_BUFFER_SIZE, currentOgm->ogmFile);
		ogg_sync_wrote(&currentOgm->oy, bytes);

		return !bytes;
	}

	return -1;
}

/*
  loadPagesToStreams

  return:
  !0 -> no data transferred
*/

static int loadPagesToStreams()
{

	ogg_page        og;
	int				r = 1;

	while (ogg_sync_pageout(&currentOgm->oy, &og) > 0)
	{
		if (currentOgm->hasAudio) ogg_stream_pagein(&currentOgm->os_audio, &og);
		if (currentOgm->hasVideo) ogg_stream_pagein(&currentOgm->os_video, &og);
		r = 0;
	}

	return r;
}

#define SIZEOF_RAWBUFF 4*1024
static byte     rawBuffer[SIZEOF_RAWBUFF];

#define MIN_AUDIO_PRELOAD 250	// in ms
#define MAX_AUDIO_PRELOAD 350	// in ms

#ifndef NO_YUV_BACKUP

static	long				ROQ_YY_tab[256];
static	long				ROQ_UB_tab[256];
static	long				ROQ_UG_tab[256];
static	long				ROQ_VG_tab[256];
static	long				ROQ_VR_tab[256];

static int ROQ_TablesGenerated = 0;
static void ROQ_GenYUVTables( void )
{
	float t_ub,t_vr,t_ug,t_vg;
	long i;

	if (!ROQ_TablesGenerated) {
		t_ub = (1.77200f/2.0f) * (float)(1<<6) + 0.5f;
		t_vr = (1.40200f/2.0f) * (float)(1<<6) + 0.5f;
		t_ug = (0.34414f/2.0f) * (float)(1<<6) + 0.5f;
		t_vg = (0.71414f/2.0f) * (float)(1<<6) + 0.5f;
		for(i=0;i<256;i++) {
			float x = (float)(2 * i - 255);
		
			ROQ_UB_tab[i] = (long)( ( t_ub * x) + (1<<5));
			ROQ_VR_tab[i] = (long)( ( t_vr * x) + (1<<5));
			ROQ_UG_tab[i] = (long)( (-t_ug * x)		 );
			ROQ_VG_tab[i] = (long)( (-t_vg * x) + (1<<5));
			ROQ_YY_tab[i] = (long)( (i << 6) | (i >> 2) );
		}
		ROQ_TablesGenerated = 1;
	}
}

#endif

/*
Frame_yuv_to_rgb24
is used by the Theora(ogm) code

  moved the convertion into one function, to reduce the number of function-calls
*/
/*
void Frame_yuv_to_rgb24(const unsigned char *y, const unsigned char *u, const unsigned char *v,
						int width, int height, int y_stride, int uv_stride,
						int yWShift, int uvWShift, int yHShift, int uvHShift, unsigned int *output)
{
	int             i, j, uvI;
	long            r, g, b, YY;

	//HRT_Start(88);

	for(j = 0; j < height; ++j)
	{
		for(i = 0; i < width; ++i)
		{

			YY = (long)(ROQ_YY_tab[(y[(i >> yWShift) + (j >> yHShift) * y_stride])]);
			uvI = (i >> uvWShift) + (j >> uvHShift) * uv_stride;

			r = (YY + ROQ_VR_tab[v[uvI]]) >> 6;
			g = (YY + ROQ_UG_tab[u[uvI]] + ROQ_VG_tab[v[uvI]]) >> 6;
			b = (YY + ROQ_UB_tab[u[uvI]]) >> 6;

			if(r < 0)
				r = 0;
			if(g < 0)
				g = 0;
			if(b < 0)
				b = 0;
			if(r > 255)
				r = 255;
			if(g > 255)
				g = 255;
			if(b > 255)
				b = 255;

			*output = (r) | (g << 8) | (b << 16) | 0xFF000000; //(255 << 24);
			++output;
		}
	}
	//HRT_Stop(88);
	//Com_Printf("yuv conv time: %.3f ms\n", HRT_GetTimingMS(88));

}
*/
#ifndef NO_YUV_BACKUP

// Optimized version, processes 4 pixels (on 2 scanlines) per cycle and auto-assumes YUV 4:2:0 input format
void Frame_yuv_to_rgb24_4px(const unsigned char *y, const unsigned char *u, const unsigned char *v,
						int width, int height, int y_stride, int uv_stride, unsigned int *output)
{
	int             i, j;
	long            r, g, b, Y, U, V;

	
	int y_linedelta = y_stride +  (y_stride - width);
	int uv_linedelta = uv_stride - (width / 2);

	const unsigned char *y0 = y;
	const unsigned char *y1 = y + y_stride;

	unsigned int *rgb0 = output;
	unsigned int *rgb1 = output + width;

	//HRT_Start(88);

	for(j = 0; j < height; j += 2)
	{
		for(i = 0; i < width; i += 2)
		{
			U = *u++;
			V = *v++;

			Y = ROQ_YY_tab[*y0++];

			r = (Y + ROQ_VR_tab[V]) >> 6;
			g = (Y + ROQ_UG_tab[U] + ROQ_VG_tab[V]) >> 6;
			b = (Y + ROQ_UB_tab[U]) >> 6;

			if(r < 0) r = 0; else if(r > 255) r = 255;
			if(g < 0) g = 0; else if(g > 255) g = 255;
			if(b < 0) b = 0; else if(b > 255) b = 255;

			*rgb0++ = (r) | (g << 8) | (b << 16) | 0xFF000000; //(255 << 24);

			Y = ROQ_YY_tab[*y0++];

			r = (Y + ROQ_VR_tab[V]) >> 6;
			g = (Y + ROQ_UG_tab[U] + ROQ_VG_tab[V]) >> 6;
			b = (Y + ROQ_UB_tab[U]) >> 6;

			if(r < 0) r = 0; else if(r > 255) r = 255;
			if(g < 0) g = 0; else if(g > 255) g = 255;
			if(b < 0) b = 0; else if(b > 255) b = 255;

			*rgb0++ = (r) | (g << 8) | (b << 16) | 0xFF000000;

			Y = ROQ_YY_tab[*y1++];

			r = (Y + ROQ_VR_tab[V]) >> 6;
			g = (Y + ROQ_UG_tab[U] + ROQ_VG_tab[V]) >> 6;
			b = (Y + ROQ_UB_tab[U]) >> 6;

			if(r < 0) r = 0; else if(r > 255) r = 255;
			if(g < 0) g = 0; else if(g > 255) g = 255;
			if(b < 0) b = 0; else if(b > 255) b = 255;

			*rgb1++ = (r) | (g << 8) | (b << 16) | 0xFF000000;

			Y = ROQ_YY_tab[*y1++];

			r = (Y + ROQ_VR_tab[V]) >> 6;
			g = (Y + ROQ_UG_tab[U] + ROQ_VG_tab[V]) >> 6;
			b = (Y + ROQ_UB_tab[U]) >> 6;

			if(r < 0) r = 0; else if(r > 255) r = 255;
			if(g < 0) g = 0; else if(g > 255) g = 255;
			if(b < 0) b = 0; else if(b > 255) b = 255;

			*rgb1++ = (r) | (g << 8) | (b << 16) | 0xFF000000;
		}
		rgb0 += width;
		rgb1 += width;
		y0 += y_linedelta;
		y1 += y_linedelta;
		u += uv_linedelta;
		v += uv_linedelta;
	}
	//HRT_Stop(88);
	//Com_Printf("yuv conv time: %.3f ms\n", HRT_GetTimingMS(88));
}

#endif

/*

  return: audio wants more packets
*/
static qboolean loadAudio()
{
	float         **pcm;
	float          *right, *left;
	int             samples, samplesNeeded;
	int             i;
	short          *ptr;
	ogg_packet      op;
	vorbis_block    vb;

	if (!currentOgm->hasAudio || (currentOgm->systemFlags & CIN_silent)) {
		return qfalse;
	}

	memset(&op, 0, sizeof(op));
	memset(&vb, 0, sizeof(vb));
	vorbis_block_init(&currentOgm->vd, &vb);

	while(currentOgm->currentTime + MAX_AUDIO_PRELOAD > (int)(currentOgm->vd.granulepos * 1000 / currentOgm->vi.rate))
	{
		if((samples = vorbis_synthesis_pcmout(&currentOgm->vd, &pcm)) > 0)
		{
			// vorbis -> raw
			ptr = (short *)rawBuffer;
			samplesNeeded = (SIZEOF_RAWBUFF) / (2 * 2);	// (width*channel)
			if(samples < samplesNeeded)
				samplesNeeded = samples;

			if (currentOgm->vi.channels > 1) {
				// Do stereo conversion
				left = pcm[0];
				right = pcm[1];
				for(i = 0; i < samplesNeeded; ++i)
				{
					ptr[0] = (short)(left[i] >= -1.0f &&
							  left[i] <= 1.0f) ? left[i] * 32767.f : 32767 * ((left[i] > 0.0f) - (left[i] < 0.0f));
					ptr[1] = (short)(right[i] >= -1.0f &&
							  right[i] <= 1.0f) ? right[i] * 32767.f : 32767 * ((right[i] > 0.0f) - (right[i] < 0.0f));
					ptr += 2;
				}
			} else {
				// Do mono conversion
				left = pcm[0];
				for(i = 0; i < samplesNeeded; ++i)
				{
					ptr[0] = (short)(left[i] >= -1.0f &&
							  left[i] <= 1.0f) ? left[i] * 32767.f : 32767 * ((left[i] > 0.0f) - (left[i] < 0.0f));
					ptr += 1;
				}
			}
			if(samplesNeeded > 0)
			{
				// tell libvorbis how many samples we actually consumed
				vorbis_synthesis_read(&currentOgm->vd, samplesNeeded);

				S_RawSamples( samplesNeeded, currentOgm->vi.rate, 2, currentOgm->vi.channels, rawBuffer, 1.0f, qtrue);

			}
		} else {
			// op -> vorbis
			if(ogg_stream_packetout(&currentOgm->os_audio, &op))
			{
				if(vorbis_synthesis(&vb, &op) == 0)
					vorbis_synthesis_blockin(&currentOgm->vd, &vb);
			} else {
				break;
			}
		}
	}

	vorbis_block_clear(&vb);

	if(currentOgm->currentTime + MIN_AUDIO_PRELOAD > (int)(currentOgm->vd.granulepos * 1000 / currentOgm->vi.rate))
		return qtrue;
	else
		return qfalse;
}



/*
how many >> are needed to make y==x (shifting y>>i)
return: -1	-> no match
		>=0	-> number of shifts
*/
static int findSizeShift(int x, int y)
{
	int             i;

	for(i = 0; (y >> i); ++i)
		if(x == (y >> i))
			return i;

	return -1;
}

/*

  return:	1	-> loaded a new Frame ( currentOgm->outputBuffer points to the actual frame )
			0	-> no new Frame
			<0	-> error
*/

static int loadVideoFrameTheora()
{
	int             r = 0;
	ogg_packet      op;
	int				neededVFrame = nextNeededVFrame();
	
	if (neededVFrame == currentOgm->VFrameCount)
	{
		return 1;
	}

	memset(&op, 0, sizeof(op));

	while(!r && (ogg_stream_packetout(&currentOgm->os_video, &op)))
	{
		ogg_int64_t   th_frame;
		ogg_int64_t   th_granulepos;

		th_decode_packetin(currentOgm->td, &op, &th_granulepos);

		th_frame = th_granule_frame(currentOgm->td, th_granulepos);
		if(th_frame >= neededVFrame)
		{			
			if(th_decode_ycbcr_out(currentOgm->td, currentOgm->th_yuvbuffer))
				continue;

			if(currentOgm->outputWidth != (int)currentOgm->ti.frame_width || currentOgm->outputHeight != (int)currentOgm->ti.frame_height)
			{
				currentOgm->outputWidth = currentOgm->ti.frame_width;
				currentOgm->outputHeight = currentOgm->ti.frame_height;
				Com_DPrintf("[Theora(ogg)] new resolution %dx%d\n", currentOgm->outputWidth, currentOgm->outputHeight);
			}

			if(( unsigned int ) currentOgm->outputBufferSize < currentOgm->ti.frame_width * currentOgm->ti.frame_height)
			{

				currentOgm->outputBufferSize = currentOgm->ti.frame_width * currentOgm->ti.frame_height;

				/* Free old output buffer */
				if(currentOgm->outputBuffer)
					free(currentOgm->outputBuffer);

				/* Allocate the new buffer */
				currentOgm->outputBuffer = (unsigned char *)malloc(currentOgm->outputBufferSize * 4);
				if(currentOgm->outputBuffer == NULL)
				{
					currentOgm->outputBufferSize = 0;
					r = -2;
					currentOgm->newFrame = 0;
					break;
				}
			}
			
			currentOgm->newFrame = 1;

			r = 1;
			currentOgm->VFrameCount = th_frame;
			
		}
	}
	return r;
}

/*

  return:	1	-> loaded a new Frame ( currentOgm->outputBuffer points to the actual frame )
			0	-> no new Frame
			<0	-> error
*/
static int loadVideoFrame() {
	currentOgm->newFrame = 0;

	if(currentOgm->hasVideo)
		return loadVideoFrameTheora();

	return 1;
}

/*
  return: qtrue => noDataTransferred (end of file)
*/

static qboolean loadFrame()
{
	qboolean        needVOutputData = qtrue;
	qboolean        audioWantsMoreData = qtrue;
	int             status;

	while(needVOutputData || audioWantsMoreData)
	{

		if(needVOutputData)
		{
			status = loadVideoFrame();
			if (status) {
				needVOutputData = qfalse;
			}
		}

		audioWantsMoreData = loadAudio();

		if(needVOutputData || audioWantsMoreData)
		{
			// try to transfer Pages to the audio- and video-Stream
			if (loadBlockToSync()) {
				return qtrue; // EOF
			} else {
				loadPagesToStreams();
			}
		}
	}

	return qfalse;
}

//from VLC ogg.c ( http://trac.videolan.org/vlc/browser/trunk/modules/demux/ogg.c )
typedef struct
{
	char            streamtype[8];
	char            subtype[4];

	ogg_int32_t     size;		/* size of the structure */

	ogg_int64_t     time_unit;	/* in reference time */// in 10^-7 seconds (dT between frames)
	ogg_int64_t     samples_per_unit;
	ogg_int32_t     default_len;	/* in media time */

	ogg_int32_t     buffersize;
	ogg_int16_t     bits_per_sample;

	union
	{
		struct
		{
			ogg_int32_t     width;
			ogg_int32_t     height;
		} stream_header_video;

		struct
		{
			ogg_int16_t     channels;
			ogg_int16_t     blockalign;
			ogg_int32_t     avgbytespersec;
		} stream_header_audio;
	} sh;
} stream_header_t;

qboolean isPowerOf2(int x)
{
	//int             bitsSet = 0;
	//int             i;

	//for(i = 0; i < sizeof(int) * 8; ++i)
	//	if(x & (1 << i))
	//		++bitsSet;

	//return (bitsSet <= 1);
	return !(x & (x-1));
}

#ifndef NO_YUV_BACKUP
static int MMXSupported = 0;
void Yuv2rgb32_MMX(const unsigned char *y, const unsigned char *u, const unsigned char *v,
						int width, int height, int y_stride, int uv_stride, unsigned int *output);
static void ProcessVideoFrame()
{
	int             yWShift, uvWShift;
	int             yHShift, uvHShift;

	if (currentOgm->newFrame) {
		currentOgm->newFrame = 0;
		if (!MMXSupported) {
			yWShift = findSizeShift(currentOgm->th_yuvbuffer.y_width, currentOgm->th_info.width);
			uvWShift = findSizeShift(currentOgm->th_yuvbuffer.uv_width, currentOgm->th_info.width);
			yHShift = findSizeShift(currentOgm->th_yuvbuffer.y_height, currentOgm->th_info.height);
			uvHShift = findSizeShift(currentOgm->th_yuvbuffer.uv_height, currentOgm->th_info.height);

			if(yWShift < 0 || uvWShift < 0 || yHShift < 0 || uvHShift < 0)
			{
				Com_Printf("[Theora] unexpected resolution in a yuv-Frame\n");
			}
			else
			{
				Frame_yuv_to_rgb24_4px(currentOgm->th_yuvbuffer.y, currentOgm->th_yuvbuffer.u, currentOgm->th_yuvbuffer.v,
									   currentOgm->th_info.width, currentOgm->th_info.height, currentOgm->th_yuvbuffer.y_stride,
									   currentOgm->th_yuvbuffer.uv_stride,  (unsigned int *)currentOgm->outputBuffer);
									   
				
			}
		} else {
			Yuv2rgb32_MMX(currentOgm->th_yuvbuffer.y, currentOgm->th_yuvbuffer.u, currentOgm->th_yuvbuffer.v,
								   currentOgm->th_info.width, currentOgm->th_info.height,currentOgm->th_yuvbuffer.y_stride,
								   currentOgm->th_yuvbuffer.uv_stride, (unsigned int *)currentOgm->outputBuffer);
		}
	}
}
#else
static int MMXSupported = 0;
void Yuv2rgb32_MMX(const unsigned char *y, const unsigned char *u, const unsigned char *v,
						int width, int height, int y_stride, int uv_stride, unsigned int *output);
static void ProcessVideoFrame()
{
	if (currentOgm->newFrame) {
		currentOgm->newFrame = 0;
			Yuv2rgb32_MMX(currentOgm->th_yuvbuffer[0].data, currentOgm->th_yuvbuffer[1].data, currentOgm->th_yuvbuffer[2].data,
								   currentOgm->ti.frame_width, currentOgm->ti.frame_height, currentOgm->th_yuvbuffer[0].stride,
								   currentOgm->th_yuvbuffer[1].stride, (unsigned int *)currentOgm->outputBuffer);
	}
}
#endif // NO_YUV_BACKUP

void Cin_OGM_Shutdown();
/*

  return: 0 -> no problem
*/
//TODO: vorbis/theora-header&init in sub-functions
//TODO: "clean" error-returns ...

#ifndef NO_YUV_BACKUP

int __declspec(naked) IsMMXSupported(void) {
	__asm
	{
		pushad
		mov eax, 1
		cpuid
		and edx, 0x800000		// Check for MMX support
		jnz present
		popad
		xor eax,eax
		ret
present:
		popad
		mov eax, 1
		ret
	}
}

#endif

int Cin_OGM_Init(int handle, const char *filename, int systemFlags)
{
	int             status;
	ogg_page        og;
	ogg_packet      op;
	int             i;

#ifndef NO_YUV_BACKUP
	MMXSupported = IsMMXSupported();
#endif

	currentOgm = &cin_ogm[handle];

	if(currentOgm->ogmFile)
	{
		Com_Printf(S_COLOR_YELLOW "WARNING: It seems there was already a video running in slot %i, it will be killed to start %s\n", handle, filename);
		Cin_OGM_Shutdown(handle);
	}

	memset(currentOgm, 0, sizeof(cin_ogm_t));

	FS_FOpenFileRead(filename, &currentOgm->ogmFile, qtrue);
	if(!currentOgm->ogmFile)
	{
		Com_Printf(S_COLOR_YELLOW "WARNING: Can't open ogm file for reading (%s)\n", filename);
		return -1;
	}
	
	currentOgm->systemFlags = systemFlags;

	ogg_sync_init(&currentOgm->oy);	/* Now we can read pages */

	status = 0;

	vorbis_info_init(&currentOgm->vi);
	vorbis_comment_init(&currentOgm->vc);				

	th_info_init(&currentOgm->ti);
	th_comment_init(&currentOgm->tc);

	while (!status) {
		if (loadBlockToSync())
			break;

		while (ogg_sync_pageout(&currentOgm->oy, &og) >= 0) {
			ogg_stream_state temp;
			if(!ogg_page_bos(&og)){
				/* don't leak the page; get it into the appropriate stream */
				if(currentOgm->hasVideo) ogg_stream_pagein(&currentOgm->os_video, &og);
				if(currentOgm->hasAudio) ogg_stream_pagein(&currentOgm->os_audio, &og);
				status = 1;
				break;
			}

			ogg_stream_init(&temp, ogg_page_serialno(&og));
			ogg_stream_pagein(&temp,&og);
			ogg_stream_packetout(&temp,&op);

			/* identify the codec: try theora */
			if(!currentOgm->hasVideo && th_decode_headerin(&currentOgm->ti, &currentOgm->tc, &currentOgm->ts, &op) >= 0){
				/* it is theora */
				memcpy(&currentOgm->os_video, &temp, sizeof(temp));
				currentOgm->hasVideo = qtrue;
			} else if (!currentOgm->hasAudio && vorbis_synthesis_headerin(&currentOgm->vi, &currentOgm->vc, &op) >= 0){
				/* it is vorbis */
				memcpy(&currentOgm->os_audio, &temp, sizeof(temp));
				currentOgm->hasAudio = qtrue;
			} else {
				/* whatever it is, we don't care about it */
				ogg_stream_clear(&temp);
			}
		}
	}

	if (currentOgm->hasAudio) {
		//load vorbis header

		i = 1;
		while(i < 3)
		{
			status = ogg_stream_packetout(&currentOgm->os_audio, &op);
			if(status < 0)
			{
				Com_Printf(S_COLOR_YELLOW "WARNING: Corrupt ogg packet while loading vorbis-headers, ogm file(%s)\n", filename);
				return -8;
			}
			if(status > 0)
			{
				status = vorbis_synthesis_headerin(&currentOgm->vi, &currentOgm->vc, &op);
				if(i == 0 && status < 0)
				{
					Com_Printf(S_COLOR_YELLOW "WARNING: This Ogg bitstream does not contain Vorbis audio data, ogm file (%s)\n", filename);
					return -9;
				}
				++i;
			}
			else if(loadPagesToStreams(handle))
			{
				if(loadBlockToSync(handle))
				{
					Com_Printf(S_COLOR_YELLOW "WARNING: Couldn't find all vorbis headers before end of ogm file (%s)\n", filename);
					return -10;
				}
			}
		}

		vorbis_synthesis_init(&currentOgm->vd, &currentOgm->vi);
	}/* else {
		Com_Printf(S_COLOR_YELLOW "WARNING: Haven't found an audio (vorbis) stream in ogm file (%s)\n", filename);
	}*/

	if (currentOgm->hasVideo) {
	#ifndef NO_YUV_BACKUP
		ROQ_GenYUVTables();
	#endif

		i = 1;
		while(i < 3)
		{
			status = ogg_stream_packetout(&currentOgm->os_video, &op);
			if(status < 0)
			{
				Com_Printf(S_COLOR_YELLOW "WARNING: Corrupt ogg packet while loading theora-headers, ogm file (%s)\n", filename);
				return -8;
			}
			if(status > 0)
			{
				status = th_decode_headerin(&currentOgm->ti, &currentOgm->tc, &currentOgm->ts, &op);
				if(status < 0)
				{
					Com_Printf(S_COLOR_YELLOW "WARNING: This Ogg bitstream does not contain theora data, ogm file (%s)\n", filename);
					return -9;
				}
				++i;
			}
			else if(loadPagesToStreams(handle))
			{
				if(loadBlockToSync(handle))
				{
					Com_Printf(S_COLOR_YELLOW "WARNING: Couldn't find all theora headers before end of ogm file (%s)\n", filename);
					return -10;
				}
			}
		}

		currentOgm->td = th_decode_alloc(&currentOgm->ti, currentOgm->ts);

		th_setup_free(currentOgm->ts);
		currentOgm->ts = 0;

		if(!isPowerOf2(currentOgm->ti.frame_width))
		{
			Com_Printf(S_COLOR_RED "ERROR: VideoWidth of the ogm file isn't a power of 2 value (%s)\n", filename);
			return -5;
		}
		if(!isPowerOf2(currentOgm->ti.frame_height))
		{
			Com_Printf(S_COLOR_RED "ERROR: VideoHeight of the ogm-file isn't a power of 2 value (%s)\n", filename);
			return -6;
		}
		if (currentOgm->ti.pixel_fmt != TH_PF_420) {
			Com_Printf(S_COLOR_RED "ERROR: Unsupported pixel format, only YUV 4:2:0 is currently supported. (%s)\n", filename);
			return -7;
		}

		currentOgm->Vtime_unit = ((ogg_int64_t) currentOgm->ti.fps_denominator * 1000 * 10000 / currentOgm->ti.fps_numerator);
		if (currentOgm->ti.aspect_numerator == 0 || currentOgm->ti.aspect_denominator == 0) {
			currentOgm->aspectRatio = 0;	// Not provided
		} else {
			currentOgm->aspectRatio = (double)currentOgm->ti.aspect_numerator * (double)currentOgm->ti.pic_width / ((double)currentOgm->ti.aspect_denominator * (double)currentOgm->ti.pic_height);
		}
	} else {
		currentOgm->aspectRatio = 0;
		currentOgm->outputWidth = 1;
		currentOgm->outputWidth = 1;
		currentOgm->outputBufferSize = 4;
		currentOgm->outputBuffer = malloc(4);
		*(unsigned int *)currentOgm->outputBuffer = 0x000000FF; // Black
	}
	Com_DPrintf("OGM Initialisation done (%i) (%s)\n", handle, filename);

	return 0;
}

int nextNeededVFrame()
{
	return (int)(currentOgm->currentTime * (ogg_int64_t) 10000.0 / currentOgm->Vtime_unit);
}

/*

  time ~> time in ms to which the movie should run
  return:	0 => nothing special
			1 => eof
*/

int Cin_OGM_Run(int handle, int time)
{

	currentOgm->currentTime = time;

	currentOgm = &cin_ogm[handle];

	if(loadFrame())
		return 1;

	ProcessVideoFrame();
	return 0;
}

/*
  Gives a Pointer to the current Output-Buffer
  and the Resolution
*/
unsigned char  *Cin_OGM_GetOutput(int handle, int *outWidth, int *outHeight)
{
	if(outWidth != NULL)
		*outWidth = cin_ogm[handle].outputWidth;
	if(outHeight != NULL)
		*outHeight = cin_ogm[handle].outputHeight;

	return cin_ogm[handle].outputBuffer;
}

double  Cin_OGM_GetAspect(int handle)
{
	return cin_ogm[handle].aspectRatio;
}

int Cin_OGM_GetFlags(int handle)
{
	return cin_ogm[handle].systemFlags;
}

void Cin_OGM_Shutdown(int handle) {

	currentOgm = &cin_ogm[handle];

	if (currentOgm->td) th_decode_free(currentOgm->td), currentOgm->td = 0 ;
	if (currentOgm->ts) th_setup_free(currentOgm->ts);
	th_comment_clear(&currentOgm->tc);
	th_info_clear(&currentOgm->ti);

	if(currentOgm->outputBuffer)
		free(currentOgm->outputBuffer);
	currentOgm->outputBuffer = NULL;

	vorbis_dsp_clear(&currentOgm->vd);
	vorbis_comment_clear(&currentOgm->vc);
	vorbis_info_clear(&currentOgm->vi);	/* must be called last (comment from vorbis example code) */


	ogg_stream_clear(&currentOgm->os_audio);
	ogg_stream_clear(&currentOgm->os_video);

	ogg_sync_clear(&currentOgm->oy);

	FS_FCloseFile(currentOgm->ogmFile);
	currentOgm->ogmFile = 0;
}