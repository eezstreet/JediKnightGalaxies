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
// gl_pk3.c
// (c) 2013 Jedi Knight Galaxies

///////////////////////////////////////
//
// Protected Pk3 support - JKG
//
// By BobaFett / eezstreet
//
//////////////////////////////////////


// IMPORTANT: CLASSIFIED ALGORITHM!
// DO NOT LEAK THIS INFORMATION TO *ANYONE*

// This code facilitates the reading of protected pk3s
// The protection works by masking the first 4 bytes of the compressed
// stream with the file's crc, thereby corrupting the data.
// To read it, we simply hook unzReadCurrentFile in the engine
// Before anything, the following assumptions are checked:
// 1. It's the first read of this file (compare remaining compressed data to the size)
// 2. The file is compressed (if its STORE'd, dont bother)
// 3. Check if the file has an extrainfo tag
//
// If all these assumptions are true, we mask the first 4 bytes of the readbuffer
// with the crc, thereby making the stream valid again

#define MASKTAG 0xDEADF00D

#include <windows.h>
#include <stdio.h>
#include "q3unzip.h"
#include "gl_enginefuncs.h"
#define Z_ERRNO        (-1)


int (* _fseek)(FILE* _File, long _Offset, int _Origin) = (int (*)(FILE*,long,int))0x52B755;
size_t (* _fread)(void * _DstBuf, size_t _ElementSize, size_t _Count, FILE * _File) = (size_t (*)(void*,size_t,size_t,FILE*))0x52B820;
long (* _ftell)(FILE * _File) = (long (*)(FILE *))0x52B5FC;
#ifdef OLD_PK3_PROTECTION_ALGO
int unzGetLocalExtrafield (unzFile file,void *buf,unsigned len)
{
	unz_s* s;
	file_in_zip_read_info_s* pfile_in_zip_read_info;
	uInt read_now;
	uLong size_to_read;

	if (file==NULL)
		return UNZ_PARAMERROR;
	s=(unz_s*)file;
    pfile_in_zip_read_info=s->pfile_in_zip_read;

	if (pfile_in_zip_read_info==NULL)
		return UNZ_PARAMERROR;

	size_to_read = (pfile_in_zip_read_info->size_local_extrafield - 
				pfile_in_zip_read_info->pos_local_extrafield);

	if (buf==NULL)
		return (int)size_to_read;
	
	if (len>size_to_read)
		read_now = (uInt)size_to_read;
	else
		read_now = (uInt)len ;

	if (read_now==0)
		return 0;
	// Using our own fseek and fread seems to crash
	// So use the engine's version of these instead
	if (_fseek(pfile_in_zip_read_info->file,
              pfile_in_zip_read_info->offset_local_extrafield + 
			  pfile_in_zip_read_info->pos_local_extrafield,SEEK_SET)!=0)
		return UNZ_ERRNO;

	if (_fread(buf,(uInt)read_now,1,pfile_in_zip_read_info->file)!=1)
		return UNZ_ERRNO;

	return (int)read_now;
}

// PK3 protection master key
static unsigned char MasterKey[128] = { 0xCC, 0xAF, 0xCF, 0x88, 0xC2, 0xD6, 0x81, 0x39, 0x9E, 0xBB, 0xA8, 0x83, 0xBF, 0x92, 0xB6, 0xDF, 0x52, 0x24, 0x61, 0x2, 0xC6, 0x74, 0xAE, 0xA4, 0x80, 0xA1, 0x4F, 0x86, 0xD0, 0xF, 0x2F, 0xB3, 0x25, 0xC1, 0x4A, 0x6, 0x7D, 0xA, 0xC2, 0xEC, 0x1F, 0x64, 0x30, 0x6B, 0x8B, 0x9F, 0x38, 0x64, 0xB1, 0x3F, 0x60, 0x15, 0x63, 0x20, 0x22, 0xE1, 0x6, 0x5C, 0x3F, 0x54, 0x8C, 0x6E, 0x57, 0x62, 0xA6, 0x97, 0xB6, 0xEF, 0x36, 0x81, 0x71, 0x94, 0x3, 0x23, 0xB2, 0xA1, 0xB0, 0xE4, 0x51, 0x5E, 0xEE, 0x4, 0xA4, 0x96, 0xF1, 0x37, 0x97, 0x57, 0x22, 0x91, 0xFE, 0xF2, 0x14, 0xAA, 0x2B, 0x4A, 0xB8, 0xFB, 0xBC, 0x4C, 0xD4, 0x88, 0xB7, 0xB9, 0xB1, 0xC1, 0xD9, 0x2C, 0x15, 0xA9, 0x2B, 0x57, 0x70, 0x38, 0xD5, 0x90, 0x5A, 0xFE, 0x38, 0x8C, 0x3E, 0xA, 0x36, 0x6B, 0x4F, 0xB, 0xD4, 0xF5 };
#endif
typedef struct {
	int idcode;
	char id2;
	char patchlen;
	char offset;
	char displace;
	char key[56];
} filetag_t;

#define TAGID 0x871F538D

static __inline unsigned int __rotl(unsigned char value, int shift) {
    shift &= 7;
    return (value << shift) | (value >> (8 - shift));
}
#ifdef OLD_PK3_PROTECTION_ALGO
#include "blowfish.h"

void HRT_Start(int TimerID);
void HRT_Stop(int TimerID);
double HRT_GetTimingMS(int TimerID);
#endif

void ProcessFileRead(unzFile unz) {
#ifdef OLD_PK3_PROTECTION_ALGO
	long pos;
	int i;
	filetag_t tag;
	BLOWFISH_CTX bfctx;
	unz_s* s;
	file_in_zip_read_info_s* pfile_in_zip_read_info;
	if (unz==0)
		return;
	s=(unz_s*)unz;
    pfile_in_zip_read_info=s->pfile_in_zip_read;
	// First, check assumption 1: first read in this file
	if (s->cur_file_info.compressed_size != pfile_in_zip_read_info->rest_read_compressed) {
		return;
	}
	// Second assumption: is it compressed?
	if (pfile_in_zip_read_info->compression_method == 0) {
		return;
	}
	// Alright, lookin good, get the extrainfo tag
	// IMPORTANT: unzReadCurrentFile will not fseek again!, so store our current position
	// so we can restore it after this call (since it'll seek too)
	pos = _ftell(pfile_in_zip_read_info->file);
	if (unzGetLocalExtrafield(unz, &tag, sizeof(tag)) != sizeof(tag)) {
		_fseek(pfile_in_zip_read_info->file, pos, SEEK_SET);
		return;
	} else {
		_fseek(pfile_in_zip_read_info->file, pos, SEEK_SET);
	}

	// TODO: Use AES for this, rather than Blowfish
	if ((unsigned int)tag.idcode == ( TAGID ^ pfile_in_zip_read_info->crc32_wait ))
	{
		// Alrighty! This is a protected file, undo the protection so it can be read	
		unsigned long *l, *r;

		*(unsigned int *)&tag.id2 ^= tag.idcode;
		if ((unsigned char)tag.id2 != 0xAB) {
			return;
		}
		// Get ready to decode the blowfish key
		{
			unsigned char *masterkey = &MasterKey[tag.displace];
			for (i=0; i<56; i++) {
				tag.key[i] += masterkey[(i+tag.offset) & 63];
				tag.key[i] = (char)__rotl(tag.key[i], masterkey[(i+1+tag.offset) & 63] & 7);
				tag.key[i] ^= masterkey[(i+2+tag.offset) & 63];
			}
		}

		Blowfish_Init(&bfctx, (unsigned char *)&tag.key[0], 56);
		// Decrypt our data
		l = (unsigned long *)pfile_in_zip_read_info->read_buffer;
		r = l + 1;
		for (i=0; i < tag.patchlen; i++) {
			Blowfish_Decrypt(&bfctx, l, r);
			l+=2; r+=2;
		}
		// We're done!
	}	
#else
#endif
	return;
}
#ifndef OLD_PK3_PROTECTION_ALGO
/*
  Read bytes from the current file.
  buf contain buffer where data must be copied
  len the size of buf.

  return the number of byte copied if somes bytes are copied
  return 0 if the end of file was reached
  return <0 with error code if there is an error
    (UNZ_ERRNO for IO error, or zLib error for uncompress error)
*/
int unzReadCurrentFileNew  (unzFile file, void *buf, unsigned len)
{
	int err=UNZ_OK;
	uInt iRead = 0;
	unz_s* s;
	file_in_zip_read_info_s* pfile_in_zip_read_info;
	if (file==NULL)
		return UNZ_PARAMERROR;
	s=(unz_s*)file;
    pfile_in_zip_read_info=s->pfile_in_zip_read;

	if (pfile_in_zip_read_info==NULL)
		return UNZ_PARAMERROR;


	if ((pfile_in_zip_read_info->read_buffer == NULL))
		return UNZ_END_OF_LIST_OF_FILE;
	if (len==0)
		return 0;

	pfile_in_zip_read_info->stream.next_out = (Byte*)buf;

	pfile_in_zip_read_info->stream.avail_out = (uInt)len;
	
	if (len>pfile_in_zip_read_info->rest_read_uncompressed)
		pfile_in_zip_read_info->stream.avail_out = 
		  (uInt)pfile_in_zip_read_info->rest_read_uncompressed;

	while (pfile_in_zip_read_info->stream.avail_out>0)
	{
		if ((pfile_in_zip_read_info->stream.avail_in==0) &&
            (pfile_in_zip_read_info->rest_read_compressed>0))
		{
			uInt uReadThis = UNZ_BUFSIZE;
			if (pfile_in_zip_read_info->rest_read_compressed<uReadThis)
				uReadThis = (uInt)pfile_in_zip_read_info->rest_read_compressed;
			if (uReadThis == 0)
				return UNZ_EOF;
			if (s->cur_file_info.compressed_size == pfile_in_zip_read_info->rest_read_compressed)
				if (fseek(pfile_in_zip_read_info->file,
						  pfile_in_zip_read_info->pos_in_zipfile + 
							 pfile_in_zip_read_info->byte_before_the_zipfile,SEEK_SET)!=0)
					return UNZ_ERRNO;
			if (fread(pfile_in_zip_read_info->read_buffer,uReadThis,1,
                         pfile_in_zip_read_info->file)!=1)
				return UNZ_ERRNO;
			pfile_in_zip_read_info->pos_in_zipfile += uReadThis;

			pfile_in_zip_read_info->rest_read_compressed-=uReadThis;
			
			pfile_in_zip_read_info->stream.next_in = 
                (Byte*)pfile_in_zip_read_info->read_buffer;
			pfile_in_zip_read_info->stream.avail_in = (uInt)uReadThis;
		}

		if (pfile_in_zip_read_info->compression_method==0)
		{
			uInt uDoCopy,i ;
			if (pfile_in_zip_read_info->stream.avail_out < 
                            pfile_in_zip_read_info->stream.avail_in)
				uDoCopy = pfile_in_zip_read_info->stream.avail_out ;
			else
				uDoCopy = pfile_in_zip_read_info->stream.avail_in ;
				
			for (i=0;i<uDoCopy;i++)
				*(pfile_in_zip_read_info->stream.next_out+i) =
                        *(pfile_in_zip_read_info->stream.next_in+i);
					
//			pfile_in_zip_read_info->crc32 = crc32(pfile_in_zip_read_info->crc32,
//								pfile_in_zip_read_info->stream.next_out,
//								uDoCopy);
			pfile_in_zip_read_info->rest_read_uncompressed-=uDoCopy;
			pfile_in_zip_read_info->stream.avail_in -= uDoCopy;
			pfile_in_zip_read_info->stream.avail_out -= uDoCopy;
			pfile_in_zip_read_info->stream.next_out += uDoCopy;
			pfile_in_zip_read_info->stream.next_in += uDoCopy;
            pfile_in_zip_read_info->stream.total_out += uDoCopy;
			iRead += uDoCopy;
		}
		else
		{
			uLong uTotalOutBefore,uTotalOutAfter;
			const Byte *bufBefore;
			uLong uOutThis;
			int flush=Z_SYNC_FLUSH;

			uTotalOutBefore = pfile_in_zip_read_info->stream.total_out;
			bufBefore = pfile_in_zip_read_info->stream.next_out;

			/*
			if ((pfile_in_zip_read_info->rest_read_uncompressed ==
			         pfile_in_zip_read_info->stream.avail_out) &&
				(pfile_in_zip_read_info->rest_read_compressed == 0))
				flush = Z_FINISH;
			*/
			err=inflate(&pfile_in_zip_read_info->stream,flush);

			uTotalOutAfter = pfile_in_zip_read_info->stream.total_out;
			uOutThis = uTotalOutAfter-uTotalOutBefore;
			
//			pfile_in_zip_read_info->crc32 = 
//                crc32(pfile_in_zip_read_info->crc32,bufBefore,
//                        (uInt)(uOutThis));

			pfile_in_zip_read_info->rest_read_uncompressed -=
                uOutThis;

			iRead += (uInt)(uTotalOutAfter - uTotalOutBefore);
            
			if (err==Z_STREAM_END)
				return (iRead==0) ? UNZ_EOF : iRead;
			if (err!=Z_OK) 
				break;
		}
	}

	if (err==Z_OK)
		return iRead;
	return err;
}
#endif