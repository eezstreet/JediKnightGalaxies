#ifndef __JKGADVFMOD
#include "jkg_advsound.h"
#include <errno.h>

#ifdef __JKGADVOGG

#define OGG_SAMPLEWIDTH 2

int __declspec(naked) FS_Read( void *buffer, int len, fileHandle_t f ) {
	(void)buffer;
	(void)len;
	(void)f;
	__asm {
		mov eax, [esp+0xC]
		push [esp+0x8]
		push [esp+0x8]	// +4 due to the previous push
		mov ebx, 0x43C060
		call ebx
		add esp, 0x8
		ret
	}
}

int __declspec(naked) FS_Seek( fileHandle_t f, long offset, int origin ) {
	(void)f;
	(void)offset;
	(void) origin;
	__asm {
		push 0x43C200
		ret
	}
}

// fread() replacement
size_t S_OGG_Callback_read(void *ptr, size_t size, size_t nmemb, void *datasource)
{
	int byteSize = 0;
	int bytesRead = 0;
	size_t nMembRead = 0;
	advSFX_t *file = (advSFX_t *)datasource;

	// check if input is valid
	if(!ptr)
	{
		errno = EFAULT; 
		return 0;
	}
	
	if(!(size && nmemb))
	{
		// It's not an error, caller just wants zero bytes!
		errno = 0;
		return 0;
	}
 
	if(!datasource)
	{
		errno = EBADF; 
		return 0;
	}

	// FS_Read does not support multi-byte elements
	byteSize = nmemb * size;

	// read it with the Q3 function FS_Read()
	bytesRead = FS_Read(ptr, byteSize, file->handle);

	// update the file position
	file->soundFormat.ogg.streampos += bytesRead;

	// this function returns the number of elements read not the number of bytes
	nMembRead = bytesRead / size;

	// even if the last member is only read partially
	// it is counted as a whole in the return value	
	if(bytesRead % size)
	{
		nMembRead++;
	}
	
	return nMembRead;
}

// fseek() replacement
int S_OGG_Callback_seek(void *datasource, ogg_int64_t offset, int whence)
{
	advSFX_t *file = (advSFX_t *)datasource;
	int retVal = 0;

	// check if input is valid
	if(!datasource)
	{
		errno = EBADF; 
		return -1;
	}

	// we must map the whence to its Q3 counterpart
	switch(whence)
	{
		case SEEK_SET :
		{
			// set the file position in the actual file with the Q3 function
			retVal = FS_Seek(file->handle, (long) offset, 2);

			// something has gone wrong, so we return here
			if(retVal < 0)
			{
			 return retVal;
			}

			// keep track of file position
			file->soundFormat.ogg.streampos = (int) offset;
			break;
		}
  
		case SEEK_CUR :
		{
			// set the file position in the actual file with the Q3 function
			retVal = FS_Seek(file->handle, (long) offset, 0);

			// something has gone wrong, so we return here
			if(retVal < 0)
			{
			 return retVal;
			}

			// keep track of file position
			file->soundFormat.ogg.streampos += (int) offset;
			break;
		}
 
		case SEEK_END :
		{
			// Quake 3 seems to have trouble with FS_SEEK_END 
			// so we use the file length and FS_SEEK_SET

			// set the file position in the actual file with the Q3 function
			retVal = FS_Seek(file->handle, (long) file->soundFormat.ogg.streamlength + (long) offset, 2);

			// something has gone wrong, so we return here
			if(retVal < 0)
			{
			 return retVal;
			}

			// keep track of file position
			file->soundFormat.ogg.streampos = file->soundFormat.ogg.streamlength + (int) offset;
			break;
		}
  
		default :
		{
			// unknown whence, so we return an error
			errno = EINVAL;
			return -1;
		}
	}

	// stream->pos shouldn't be smaller than zero or bigger than the filesize
	file->soundFormat.ogg.streampos = (file->soundFormat.ogg.streampos < 0) ? 0 : file->soundFormat.ogg.streampos;
	file->soundFormat.ogg.streampos = (file->soundFormat.ogg.streampos > file->soundFormat.ogg.streamlength) ? file->soundFormat.ogg.streamlength : file->soundFormat.ogg.streampos;

	return 0;
}

// fclose() replacement
int S_OGG_Callback_close(void *datasource)
{
	// we do nothing here and close all things manually in S_OGG_CodecCloseStream()
	return 0;
}

// ftell() replacement
long S_OGG_Callback_tell(void *datasource)
{
	advSFX_t *file = (advSFX_t *)datasource;
	// check if input is valid
	if(!datasource)
	{
		errno = EBADF;
		return -1;
	}

	return (long) file->soundFormat.ogg.streampos;
}

// the callback structure
const ov_callbacks S_OGG_Callbacks =
{
 &S_OGG_Callback_read,
 &S_OGG_Callback_seek,
 &S_OGG_Callback_close,
 &S_OGG_Callback_tell
};

/*
=================
S_OGG_CodecCloseStream
=================
*/
void S_OGG_CodecCloseStream(advSFX_t *file)
{
	// check if input is valid
	if(!file)
	{
		return;
	}
	
	// let the OGG codec cleanup its stuff
	ov_clear((OggVorbis_File *) file->soundFormat.ogg.streamptr);

	// free the OGG codec control struct
	free(file->soundFormat.ogg.streamptr);

	if(file->handle)
		trap_FS_FCloseFile(file->handle);
}

/*
=================
S_OGG_CodecReadStream
=================
*/
int S_OGG_CodecReadStream(advSFX_t *file, int bytes, void *buffer)
{
	// buffer handling
	int bytesRead, bytesLeft, c;
	char *bufPtr;
	
	// Bitstream for the decoder
	int BS = 0;

	// big endian machines want their samples in big endian order
	int IsBigEndian = 0;

#	ifdef Q3_BIG_ENDIAN
	IsBigEndian = 1;
#	endif // Q3_BIG_ENDIAN

	// check if input is valid
	if(!(file && buffer))
	{
		return 0;
	}

	if(bytes <= 0)
	{
		return 0;
	}

	bytesRead = 0;
	bytesLeft = bytes;
	bufPtr = buffer;

	// cycle until we have the requested or all available bytes read
	while(-1)
	{
		// read some bytes from the OGG codec
		c = ov_read((OggVorbis_File *) file->soundFormat.ogg.streamptr, bufPtr, bytesLeft, IsBigEndian, OGG_SAMPLEWIDTH, 1, &BS);
		
		// no more bytes are left
		if(c <= 0)
		{
			break;
		}

		bytesRead += c;
		bytesLeft -= c;
		bufPtr += c;
  
		// we have enough bytes
		if(bytesLeft <= 0)
		{
			break;
		}
	}

	return bytesRead;
}

/*
=====================================================================
S_OGG_CodecLoad

We handle S_OGG_CodecLoad as a special case of the streaming functions 
where we read the whole stream at once.
======================================================================
*/
void *S_OGG_Load(advSFX_t *file)
{
	byte *buffer;
	int bytesRead;
	OggVorbis_File *vf;
	vorbis_info *OGGInfo;
	ogg_int64_t numSamples;
	
	// open the file as a stream
	file->soundFormat.ogg.streamlength = file->len;

	vf = malloc(sizeof(OggVorbis_File));
	if(!vf)
	{
		Com_Error(ERR_FATAL, "Could not allocate %i bytes for VORBIS file: %s", sizeof(OggVorbis_File), file->fileName);
		return NULL;
	}

	if(ov_open_callbacks(file, vf, NULL, 0, S_OGG_Callbacks) != 0)
	{
		free(vf);
		return NULL;
	}

	if(!ov_seekable(vf))
	{
		assert(NULL);
		ov_clear(vf);
		free(vf);
		return NULL;
	}

	if(ov_streams(vf) != 1)
	{
		Com_Error(ERR_FATAL, "Vorbis/Theora: Multiple stream detection! (@%s)", file->fileName);
		ov_clear(vf);
		free(vf);
		return NULL;
	}

	OGGInfo = ov_info(vf, 0);
	if(!OGGInfo)
	{
		Com_Printf("^1Malformed OGG-Vorbis file: %s", file->fileName);
		ov_clear(vf);
		free(vf);
		return NULL;
	}

	numSamples = ov_pcm_total(vf, 0);
	file->sampleRate = OGGInfo->rate;
	file->channels = OGGInfo->channels;
	file->soundFormat.ogg.streampos = 0;
	file->soundFormat.ogg.streamptr = vf;
	file->soundFormat.ogg.size = numSamples * file->channels * OGG_SAMPLEWIDTH;
	
	// copy over the info
	/*info->rate = stream->info.rate;
	info->width = stream->info.width;
	info->channels = stream->info.channels;
	info->samples = stream->info.samples;
	info->size = stream->info.size;
	info->dataofs = stream->info.dataofs;*/

	// allocate a buffer
	// this buffer must be free-ed by the caller of this function
    	buffer = malloc(file->soundFormat.ogg.size);
	if(!buffer)
	{
		S_OGG_CodecCloseStream(file);
	
		return NULL;	
	}

	// fill the buffer
	bytesRead = S_OGG_CodecReadStream(file, file->soundFormat.ogg.size, buffer);
	
	// we don't even have read a single byte
	if(bytesRead <= 0)
	{
		free(buffer);
		S_OGG_CodecCloseStream(file);

		return NULL;	
	}

	S_OGG_CodecCloseStream(file);
	
	return buffer;
}

#endif
#endif