// Dynamic arrays header
#ifndef JKG_DYNARRAYS_H
#define JKG_DYNARRAYS_H

#include "q_shared.h"

unsigned int JKG_Arrays_AddArrayElement(void **arry, size_t sze, unsigned int *count);
qboolean JKG_Arrays_AddArrayElement_Location(int index, void **arry, size_t sze, unsigned int *count);
void JKG_Arrays_RemoveAllElements(void **arry);
void JKG_Arrays_RemoveArrayElement(void **arry, unsigned int element, size_t sze, unsigned int *count);

/*
    Here's a short example of how to loop through all the elements in
    a dynamic array:

    int i;
    int *array = (int *)intArray.data;
    for ( i = 0; i < intArray.size; i++ )
    {
        printf ("%d\n", array[i]);
    }

    - Xycaleth
*/

// New implementation
typedef struct jkgArray_s
{
    void *data;
    unsigned int size; // Number of elements in the array
    
    // The fields below are "private".
    unsigned int _elementSize; // Size (in bytes) of a single element
    unsigned int _capacity; // Actual capacity of the array
} jkgArray_t;

void JKG_Array_Init ( jkgArray_t *arr, unsigned int elementSize, unsigned int initialCapacity );
void JKG_Array_Add ( jkgArray_t *arr, void *element );
void JKG_Array_Insert ( jkgArray_t *arr, void *data, unsigned int index );
void JKG_Array_Set ( jkgArray_t *arr, void *data, unsigned int index );
void *JKG_Array_Get ( jkgArray_t *arr, unsigned int index );
void JKG_Array_RemoveRange ( jkgArray_t *arr, unsigned int start, unsigned int count );
void JKG_Array_Remove ( jkgArray_t *arr, unsigned int index );
void JKG_Array_Shrink ( jkgArray_t *arr );
void JKG_Array_RemoveAndShrink ( jkgArray_t *arr, unsigned int index );
void JKG_Array_RemoveRangeAndShrink ( jkgArray_t *arr, unsigned int start, unsigned int count );
void JKG_Array_Free ( jkgArray_t *arr );

#endif
