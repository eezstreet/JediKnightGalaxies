// Copyright (C) 1999-2000 Id Software, Inc.
//
//
// g_mem.c
//
// Rewritten based on RoboPhred's code


#include "g_local.h"
#include "jkg_dynarrays.h"

typedef struct {
	int size;
	void *ptr;
} JKGMemBlock_t;

// These static entries may not be used outside of g_mem.c
// G_Alloc and G_Free are the wrappers for these

static unsigned int BlockCount;
static JKGMemBlock_t* MemBlocks = NULL;

// All allocs get 4 bytes extra
// These 4 bytes will contain the following alloc marker
#define ALLOCMARKER 0xDEADC0DE
// Basically all blocks look like this:
// <alloc marker>|<data>, where | is the ptr returned;
// Due to this, its easy to determine if the alloc is made by this system or not
// Which will speed up G_Free significantly as it wont have to search the array
// if the alloc doesnt have the marker.

static JKGMemBlock_t *JKG_Mem_AddBlock(int size) {
	int idx;
	// Add new entry in the MemBlocks dynamic array
	idx = JKG_Arrays_AddArrayElement((void **)&MemBlocks, sizeof(JKGMemBlock_t), &BlockCount);

	// Since these allocs are static, we'll use trap_TrueMalloc for it
	// This will make the alloc in the engine's zone memory

	// Allocs are fitted with a marker, so we can quickly determine if an alloc is made by this (in G_Free)
	trap_TrueMalloc(&MemBlocks[idx].ptr, size+4);
	MemBlocks[idx].size = size;
	if (!MemBlocks[idx].ptr) {
		G_Error("G_Alloc: Failed to allocate %i bytes of memory\n", size);
	}
	memset(MemBlocks[idx].ptr,0,size);
	*(int *)MemBlocks[idx].ptr = ALLOCMARKER;
        #ifndef __linux__
		*(int **)MemBlocks[idx].ptr += 4;
        #else
        { // linux doesnt like += 4 on pointers >.>
            int tmp = (int)MemBlocks[idx].ptr;
            tmp += 4;
            MemBlocks[idx].ptr = (void *)tmp;
        }
        #endif
	return &MemBlocks[idx];
}

static void JKG_Mem_RemoveBlock(void* ptr) {
	unsigned int i, idx = -1;
	if (*(int *)((int)ptr-4) != ALLOCMARKER) {
		// Alloc aint ours for sure
		return;
	}
	for(i = 0; i<BlockCount; i++){
		if(ptr == MemBlocks[i].ptr){
			idx = i;
			break;
		}
	}
	if(idx == -1)
		return; // Not one of our blocks, dont try to free

	// Free the block and remove the entry from the array
	#ifndef __linux__
        *(int **)MemBlocks[idx].ptr -= 4;
        #else
        { // linux doesnt like -= 4 on pointers >.>
            int tmp = (int)MemBlocks[idx].ptr;
            tmp -= 4;
            MemBlocks[idx].ptr = (void *)tmp;
        }
        #endif
	trap_TrueFree(&MemBlocks[idx].ptr);
	JKG_Arrays_RemoveArrayElement((void **)&MemBlocks, idx, sizeof(JKGMemBlock_t), &BlockCount);
}

// All functions below this point are available outside of this file


void *G_Alloc( int size ) {
	if (size <= 0) {
		return NULL; // -.-'
	}
	return JKG_Mem_AddBlock(size)->ptr;
}

void G_Free(void *ptr) {
	if (!ptr) {
		return;
	}
	JKG_Mem_RemoveBlock(ptr);
}

void G_InitMemory( void ) {
	// Dont do anything, lol
}

void G_TerminateMemory( void ) {
	// Called in G_ShutdownGame, frees all allocations
	// Must be done to avoid memory leaking through map changes
	unsigned int i;
	for (i=0; i<BlockCount; i++) {
        #ifndef __linux__
		*(int **)MemBlocks[i].ptr -= 4;
        #else
        { // linux doesnt like -= 4 on pointers >.>
            int tmp = (int)MemBlocks[i].ptr;
            tmp -= 4;
            MemBlocks[i].ptr = (void *)tmp;
        }
        #endif
		trap_TrueFree(&MemBlocks[i].ptr);
	}
	BlockCount = 0;
	JKG_Arrays_RemoveAllElements((void **)&MemBlocks);
}

void Svcmd_GameMem_f( void ) {
	unsigned int i, sz = 0;
	for (i=0; i<BlockCount; i++) {
		sz += MemBlocks[i].size;
	}
	G_Printf("Game memory status: Using %i bytes of memory in %i blocks\n", sz, BlockCount);
}


/*
#define POOLSIZE	(256 * 1024)

static char		memoryPool[POOLSIZE];
static int		allocPoint;

void *G_Alloc( int size ) {
	
	char	*p;

	if ( g_debugAlloc.integer ) {
		G_Printf( "G_Alloc of %i bytes (%i left)\n", size, POOLSIZE - allocPoint - ( ( size + 31 ) & ~31 ) );
	}

	if ( allocPoint + size > POOLSIZE ) {
	  G_Error( "G_Alloc: failed on allocation of %i bytes\n", size ); // bk010103 - was %u, but is signed
		return NULL;
	}

	p = &memoryPool[allocPoint];

	allocPoint += ( size + 31 ) & ~31;

	return p;
}

void G_InitMemory( void ) {
	//allocPoint = 0;
}

void Svcmd_GameMem_f( void ) {
	G_Printf( "Game memory status: %i out of %i bytes allocated\n", allocPoint, POOLSIZE );
}
*/