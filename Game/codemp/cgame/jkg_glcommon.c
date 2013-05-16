/*=========================================================
// Common OpenGL functions
//---------------------------------------------------------
// Description:
// Common OpenGL helper functions that are used by other
// code.
//
// Version:
// $Id: jkg_glcommon.c 111 2010-02-12 15:57:57Z Xycaleth $
//=========================================================*/

#include "jkg_glcommon.h"
#include "cg_local.h"

//=========================================================
// IsExtensionSupported
//---------------------------------------------------------
// Description:
// Checks if the given extension is supported by the
// current GPU.
//=========================================================
int IsExtensionSupported (const char* extension) {
	if (strstr (cgs.glconfig.extensions_string, extension)) {
		return 1;
	}

	return 0;
}