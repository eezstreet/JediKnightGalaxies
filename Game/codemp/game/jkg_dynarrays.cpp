/////////////////////////////////
// Jedi Knight Galaxies Dynamic Arrays
//
// Original code by RoboPhred
// Implemented by BobaFett

#include "g_local.h"
#include "jkg_dynarrays.h"

unsigned int JKG_Arrays_AddArrayElement(void **arry, size_t sze, unsigned int *count){
	*arry = (void *)realloc(*arry, sze * ((*count) + 1));
	if(!arry) {
		G_Error("JKG_Arrays_AddArrayElement: Failed to realloc memory array!\n");
		return -1;
	}
	memset((byte *)(*arry) + (sze * (*count)), 0, sze);
	(*count)++;
	return (*count) - 1; //return the index, for when we start doing the automatic sorting.
}

qboolean JKG_Arrays_AddArrayElement_Location(int index, void **arry, size_t sze, unsigned int *count){
	char *buf;
	int shiftCount;
	*arry = (void *)realloc(*arry, sze * ((*count) + 1));
	if(!arry) {
		G_Error("JKG_Arrays_AddArrayElement_Location: Failed to realloc memory array!\n");
		return qfalse;
	}
	shiftCount = (*count - index) * sze;

	buf = (char *)G_Alloc(shiftCount);
	memcpy(buf, (byte *)(*arry) + (index * sze), shiftCount);
	memcpy((byte *)(*arry) + ((index + 1) * sze), buf, shiftCount);

	G_Free(buf);
	memset((byte *)(*arry) + (index * sze), 0, sze);
	(*count)++;
	return qtrue;
}

void JKG_Arrays_RemoveAllElements(void **arry){
	if(*arry) {
		free(*arry);
	}
	*arry = NULL;
}

void JKG_Arrays_RemoveArrayElement(void **arry, unsigned int element, size_t sze, unsigned int *count){
	if(element == 0 && *count == 1) {
		// Do nothing
	} else if(element+1 == *count) {
		// Do nothing.. again
	} else {
		// Shift
		memcpy((byte *)(*arry) + (element * sze), (byte *)(*arry) + ((element + 1) * sze), (*count - element - 1) * sze);
	}
	(*count)--;
	*arry = (void *)realloc(*arry, sze * (*count));
	if(!arry && (*count) > 0) {
		G_Error("JKG_Arrays_RemoveArrayElement: Failed to realloc memory array!\n");
	}
}

//=========================================================
// JKG_Array_Init
//---------------------------------------------------------
// Description:
// Initialises the dynamic array. All dynamic arrays must
// be initialised using this function.
//=========================================================
void JKG_Array_Init ( jkgArray_t *arr, unsigned int elementSize, unsigned int initialCapacity )
{
    arr->data = NULL;
    initialCapacity = max (initialCapacity, 1);
    
    arr->data = malloc (initialCapacity * elementSize);

    arr->size = 0;
    arr->_elementSize = elementSize;
    arr->_capacity = initialCapacity;
}

//=========================================================
// JKG_Array_EnsureCapacity
//---------------------------------------------------------
// Description:
// Ensures the given dynamic array will have enough space
// to add another n elements.
//=========================================================
static void JKG_Array_EnsureCapacity ( jkgArray_t *arr, int n )
{
    int i = 1;
    while ( (arr->size + n) > (arr->_capacity * i) )
    {
        i *= 2;
    }

    if ( i > 1 )
    {
        arr->data = realloc (arr->data, arr->_elementSize * arr->_capacity * i);
        if ( arr->data == NULL )
        {
            trap_Error ("Failed to reallocate memory for a JKG array.\n");
            return;
        }
        arr->_capacity *= i;
    }
}

//=========================================================
// JKG_Array_Add
//---------------------------------------------------------
// Description:
// Adds a new element to the dynamic array. Note that the
// contents of the pointer to the element provided is
// copied into the array (not the pointer itself).
//=========================================================
void JKG_Array_Add ( jkgArray_t *arr, void *element )
{
    JKG_Array_EnsureCapacity (arr, 1);
    memcpy ((byte *)arr->data + arr->size * arr->_elementSize, element, arr->_elementSize);
    arr->size++;
}

//=========================================================
// JKG_Array_Insert
//---------------------------------------------------------
// Description:
// Inserts an element at the specified index. Note that the
// contents of the pointer to the element provided is
// copied into the array (not the pointer itself).
//=========================================================
void JKG_Array_Insert ( jkgArray_t *arr, void *data, unsigned int index )
{
    assert (index < arr->size);
    
    JKG_Array_EnsureCapacity (arr, 1);
    
    // Copy all data starting from index
    memmove (
        (byte *)arr->data + (index + 1) * arr->_elementSize, // destination
        (byte *)arr->data + index * arr->_elementSize, // source
        arr->_elementSize * (arr->size - index - 1) // num bytes
    );
    
    // Copy new data into index
    memcpy ((byte *)arr->data + arr->_elementSize * index, data, arr->_elementSize);
}

//=========================================================
// JKG_Array_Get
//---------------------------------------------------------
// Description:
// Gets the element at the specified index.
//=========================================================
void *JKG_Array_Get ( jkgArray_t *arr, unsigned int index )
{
    assert (index < arr->size);
    
    return (byte *)arr->data + index * arr->_elementSize;
}

//=========================================================
// JKG_Array_Set
//---------------------------------------------------------
// Description:
// Sets the element at the specified index.
//=========================================================
void JKG_Array_Set ( jkgArray_t *arr, void *data, unsigned int index )
{
    assert (index < arr->size);
    
    memcpy ((byte *)arr->data + index * arr->_elementSize, data, arr->_elementSize);
}

//=========================================================
// JKG_Array_RemoveRange
//---------------------------------------------------------
// Description:
// Removes a range of elements from the dynamic array.
//=========================================================
void JKG_Array_RemoveRange ( jkgArray_t *arr, unsigned int start, unsigned int count )
{
    assert ((start + count) < arr->size);

    memmove (
        (byte *)arr->data + start * arr->_elementSize,
        (byte *)arr->data + (start + count) * arr->_elementSize,
        arr->_elementSize * (arr->size - start - count)
    );

    arr->size -= count;
}

//=========================================================
// JKG_Array_Remove
//---------------------------------------------------------
// Description:
// Removes the element at the specified index.
//=========================================================
void JKG_Array_Remove ( jkgArray_t *arr, unsigned int index )
{
    JKG_Array_RemoveRange (arr, index, 1);
}

//=========================================================
// JKG_Array_Shrink
//---------------------------------------------------------
// Description:
// Shrinks the dynamic array so that it uses only enough
// memory to contain the data.
//=========================================================
void JKG_Array_Shrink ( jkgArray_t *arr )
{
    arr->data = realloc (arr->data, arr->size * arr->_elementSize);
    if ( arr->data == NULL )
    {
        trap_Error ("Failed to reallocate memory for a JKG array.\n");
        return;
    }
    
    arr->_capacity = arr->size;
}

//=========================================================
// JKG_Array_RemoveAndShrink
//---------------------------------------------------------
// Description:
// Removes the element at the specified index, and then
// shrinks the array.
//=========================================================
void JKG_Array_RemoveAndShrink ( jkgArray_t *arr, unsigned int index )
{
    JKG_Array_RemoveRange (arr, index, 1);
    JKG_Array_Shrink (arr);
}

//=========================================================
// JKG_Array_RemoveRangeAndShrink
//---------------------------------------------------------
// Description:
// Removes the elements in the specified range, and then
// shrinks the array.
//=========================================================
void JKG_Array_RemoveRangeAndShrink ( jkgArray_t *arr, unsigned int start, unsigned int count )
{
    JKG_Array_RemoveRange (arr, start, count);
    JKG_Array_Shrink (arr);
}

//=========================================================
// JKG_Array_Free
//---------------------------------------------------------
// Description:
// Cleans up the dynamic array. All dynamic arrays must be
// freed when they are no longer used.
//=========================================================
void JKG_Array_Free ( jkgArray_t *arr )
{
    if ( arr->data )
    {
        free (arr->data);
        arr->data = NULL;
    }
    
    arr->size = 0;
    arr->_capacity = 0;
    arr->_elementSize = 0;
}