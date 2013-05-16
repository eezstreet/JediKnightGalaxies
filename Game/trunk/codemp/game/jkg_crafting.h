#pragma once
#include "jkg_dynarrays.h"

#ifndef __linux__
typedef enum
#else
enum
#endif
{
	CRAFTING_WORKBENCH,
	CRAFTING_WELDING,
	CRAFTING_CHEMICAL,
	CRAFTING_MAX
} workbenches_t;

typedef struct
{
	const char *internalName;
	unsigned int itemID;
	unsigned int quantity;
} craftInput_t;

typedef struct
{
	workbenches_t craftBench;
	qboolean freestyle;
	int group;

	jkgArray_t input;
	jkgArray_t output;
} craftRecipe_t;

#define MAX_CRAFTING_FILE_SIZE	16384
#define MAX_INPUTS				8