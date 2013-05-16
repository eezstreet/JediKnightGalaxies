// Memory management module for lua
// Instead of mallocing all the memory, we store it here
// Based on Q3's zone memory system

/*
==============================================================================

						ZONE MEMORY ALLOCATION

There is never any space between memblocks, and there will never be two
contiguous free memblocks.

The rover can be left pointing at a non-empty block

The zone calls are pretty much only used for small strings and structures,
all big things are allocated on the hunk.
==============================================================================
*/
#if 0  // Deprecated
#include <windows.h>

#define	ZONEID	0xDEADF00D // XD
#define MINFRAGMENT	48

typedef struct zonedebug_s {
	char *label;
	char *file;
	int line;
	int allocSize;
} zonedebug_t;

typedef struct memblock_s {
	int		size;           // including the header and possibly tiny fragments
	int     tag;            // a tag of 0 is a free block
	struct memblock_s       *next, *prev;
	int     id;        		// should be ZONEID
} memblock_t;

typedef struct {
	int		size;			// total bytes malloced, including header
	int		used;			// total bytes used
	memblock_t	blocklist;	// start / end cap for linked list
	memblock_t	*rover;
} memzone_t;

// main zone for all "dynamic" memory allocation
memzone_t	*mainzone;

#define LUAMEM_SIZE 1024*1024*16 // 16 MB of memory

char MemBuffer[LUAMEM_SIZE + 32]; //32 extra bytes for safety

void LuaZ_CheckHeap( void );
void LuaZ_ClearZone( memzone_t *zone, int size );

void LuaZ_Initialize() {
	mainzone = (memzone_t *)&MemBuffer[0];
	LuaZ_ClearZone(mainzone, LUAMEM_SIZE);
}

/*
========================
Z_ClearZone
========================
*/
void LuaZ_ClearZone( memzone_t *zone, int size ) {
	memblock_t	*block;
	
	// set the entire zone to one free block

	zone->blocklist.next = zone->blocklist.prev = block =
		(memblock_t *)( (byte *)zone + sizeof(memzone_t) );
	zone->blocklist.tag = 1;	// in use block
	zone->blocklist.id = 0;
	zone->blocklist.size = 0;
	zone->rover = block;
	zone->size = size;
	zone->used = 0;
	
	block->prev = block->next = &zone->blocklist;
	block->tag = 0;			// free block
	block->id = ZONEID;
	block->size = size - sizeof(memzone_t);
}

/*
========================
Z_AvailableZoneMemory
========================
*/
int LuaZ_AvailableZoneMemory( memzone_t *zone ) {
	return zone->size - zone->used;
}

/*
========================
Z_AvailableMemory
========================
*/
int LuaZ_AvailableMemory( void ) {
	return LuaZ_AvailableZoneMemory( mainzone );
}

/*
========================
Z_Free
========================
*/
void LuaZ_Free( void *ptr ) {
	memblock_t	*block, *other;
	memzone_t *zone;
	
	if (!ptr) {
		//Com_Error( ERR_DROP, "Z_Free: NULL pointer" );
	}

	zone = mainzone;
	if (!zone) return;

	block = (memblock_t *) ( (byte *)ptr - sizeof(memblock_t));
	if (block->id != ZONEID) {
		//Com_Error( ERR_FATAL, "Z_Free: freed a pointer without ZONEID" );
		return;
	}
	if (block->tag == 0) {
		//Com_Error( ERR_FATAL, "Z_Free: freed a freed pointer" );
		return;
	}
	// check the memory trash tester
	if ( *(int *)((byte *)block + block->size - 4 ) != ZONEID ) {
		//Com_Error( ERR_FATAL, "Z_Free: memory block wrote past end" );
		return;
	}

	zone->used -= block->size;
	// set the block to something that should cause problems
	// if it is referenced...
	memset( ptr, 0xaa, block->size - sizeof( *block ) );

	block->tag = 0;		// mark as free
	
	other = block->prev;
	if (!other->tag) {
		// merge with previous free block
		other->size += block->size;
		other->next = block->next;
		other->next->prev = other;
		if (block == zone->rover) {
			zone->rover = other;
		}
		block = other;
	}

	zone->rover = block;

	other = block->next;
	if ( !other->tag ) {
		// merge the next free block onto the end
		block->size += other->size;
		block->next = other->next;
		block->next->prev = block;
		if (other == zone->rover) {
			zone->rover = block;
		}
	}
}
/*
================
Z_TagMalloc
================
*/

void *LuaZ_Malloc( int size ) {
	int		extra, allocSize;
	memblock_t	*start, *rover, *new, *base;
	memzone_t *zone;

	zone = mainzone;
	if (!zone) return 0;

	LuaZ_CheckHeap();

	allocSize = size;
	//
	// scan through the block list looking for the first free block
	// of sufficient size
	//
	size += sizeof(memblock_t);	// account for size of block header
	size += 4;					// space for memory trash tester
	size = (size + 3) & ~3;		// align to 32 bit boundary
	
	base = rover = zone->rover;
	start = base->prev;
	
	do {
		if (rover == start)	{
			// scaned all the way around the list
			//Com_Error( ERR_FATAL, "Z_Malloc: failed on allocation of %i bytes from the %s zone",
								//size, zone == smallzone ? "small" : "main");
			return NULL;
		}
		if (rover->tag) {
			base = rover = rover->next;
		} else {
			rover = rover->next;
		}
	} while (base->tag || base->size < size);
	
	//
	// found a block big enough
	//
	extra = base->size - size;
	if (extra > MINFRAGMENT) {
		// there will be a free fragment after the allocated block
		new = (memblock_t *) ((byte *)base + size );
		new->size = extra;
		new->tag = 0;			// free block
		new->prev = base;
		new->id = ZONEID;
		new->next = base->next;
		new->next->prev = new;
		base->next = new;
		base->size = size;
	}
	
	base->tag = 1;			// no longer a free block
	
	zone->rover = base->next;	// next allocation will start looking here
	zone->used += base->size;	//
	
	base->id = ZONEID;

	// marker for memory trash testing
	*(int *)((byte *)base + base->size - 4) = ZONEID;

	return (void *) ((byte *)base + sizeof(memblock_t));
}

/*
========================
Z_CheckHeap
========================
*/
void LuaZ_CheckHeap( void ) {
	memblock_t	*block;
	
	for (block = mainzone->blocklist.next ; ; block = block->next) {
		if (block->next == &mainzone->blocklist) {
			break;			// all blocks have been hit
		}
		if ( (byte *)block + block->size != (byte *)block->next)
			//Com_Error( ERR_FATAL, "Z_CheckHeap: block size does not touch the next block\n" );
			return;
		if ( block->next->prev != block) {
			// autofix time
			block->next->prev = block;
			//Com_Error( ERR_FATAL, "Z_CheckHeap: next block doesn't have proper back link\n" );
		}
		if ( !block->tag && !block->next->tag ) {
			//Com_Error( ERR_FATAL, "Z_CheckHeap: two consecutive free blocks\n" );
			return;
		}
	}
}
#endif