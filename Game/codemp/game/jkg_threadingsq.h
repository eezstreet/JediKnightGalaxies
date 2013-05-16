//////////////////////////////////////////////////
//
//  JKG Multi-Threading Stack and Queue implementation
//  Version 1.0 (Win32 and Linux compatible)
//
//  $Id$
//
//////////////////////////////////////////////////

#ifndef _JKG_THREADINGSQ
#define _JKG_THREADINGSQ

#include "g_local.h"
#include "jkg_threading.h"

/*
	These stack and queue implementations are designed for use by the
	multi-threading task management system.
*/

typedef struct jkg_stack_s
{
#ifdef __linux__
	pthread_mutex_t lock;
#endif
#ifdef _WIN32
	CRITICAL_SECTION lock;
#endif
	int init;
	asyncTask_t *head;
} jkg_stack_t;

typedef struct jkg_queue_s
{
#ifdef __linux__
	pthread_mutex_t lock;
#endif
#ifdef _WIN32
	CRITICAL_SECTION lock;
#endif
	int init;
	asyncTask_t *head;
	asyncTask_t *tail;
} jkg_queue_t;

// Stack functions (LIFO-based)
void		JKG_Stack_Destroy(jkg_stack_t *stack);				// Clear up the stack
void		JKG_Stack_Initialize(jkg_stack_t *stack);			// Initialize the stack
void		JKG_Stack_Push(jkg_stack_t *stack, asyncTask_t *task);		// Push a task on the stack
asyncTask_t *JKG_Stack_Pop(jkg_stack_t *stack);							// Pop a task from the stack

// Queue functions
void		JKG_Queue_Destroy(jkg_queue_t *queue);			// Clears up a queue
void		JKG_Queue_Initialize(jkg_queue_t *queue);		// Initializes a queue
void		JKG_Queue_Enqueue(jkg_queue_t *queue, asyncTask_t *task);	// Queue a new task
asyncTask_t *JKG_Queue_Dequeue(jkg_queue_t *queue);			// Get the task in front of the queue

#endif