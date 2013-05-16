#include "ui_local.h"

int strap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode )
{
    return trap_FS_FOpenFile (qpath, f, mode);
}

void strap_FS_Read( void *buffer, int len, fileHandle_t f )
{
    trap_FS_Read (buffer, len, f);
}

void strap_FS_FCloseFile( fileHandle_t f )
{
    trap_FS_FCloseFile (f);
}

int strap_FS_GetFileList(  const char *path, const char *extension, char *listbuf, int bufsize )
{
    return trap_FS_GetFileList (path, extension, listbuf, bufsize);
}
