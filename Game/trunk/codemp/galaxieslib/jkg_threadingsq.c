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
// jkg_threadingsq.c
// JKG Multi-Threading Stack and Queue implementation
// Win32 & Linux compatible
// (c) 2013 Jedi Knight Galaxies

#include "jkg_threadingsq.h"

/*
	These stack and queue implementations are designed for use by the
	multi-threading task management system.

	WARNING: Never push a task onto a stack or queue if it's already in one!
	         Debug builds will contain a failsafe for this, release builds won't!
*/

#include <assert.h>
#ifdef _DEBUG
#define ASSERTTASK(a) assert((a)->inList == 0)
#define FLAGTASK(a) (a)->inList = 1
#define UNFLAGTASK(a) (a)->inList = 0
#else
#define ASSERTTASK(a)
#define FLAGTASK(a)
#define UNFLAGTASK(a)
#endif


#ifdef _WIN32
#define MUTEX_INIT(a) InitializeCriticalSection(&(a)->lock)
#define MUTEX_LOCK(a) EnterCriticalSection(&(a)->lock)
#define MUTEX_UNLOCK(a) LeaveCriticalSection(&(a)->lock);
#define MUTEX_FREE(a) DeleteCriticalSection(&(a)->lock);
#endif
#ifdef __linux__
#define MUTEX_INIT(a) pthread_mutex_init(&(a)->lock, NULL)
#define MUTEX_LOCK(a) pthread_mutex_lock(&(a)->lock)
#define MUTEX_UNLOCK(a) pthread_mutex_unlock(&(a)->lock);
#define MUTEX_FREE(a) pthread_mutex_destroy(&(a)->lock);
#endif

void JKG_Stack_Destroy(jkg_stack_t *stack)		// Clear up the stack
{
	if (!stack->init) {							// If not inited, ignore
		return;
	}
	MUTEX_FREE(stack);							// Free the mutex/critical section
	memset(stack, 0, sizeof(jkg_stack_t));		// Clear the struct
}

void JKG_Stack_Initialize(jkg_stack_t *stack)	// Initialize the stack
{
	if (stack->init) {							// If already inited, just clear the head
		stack->head = 0;
		return;
	}
	memset(stack, 0, sizeof(jkg_stack_t));		// Clear the struct
	MUTEX_INIT(stack);							// Init the mutex
	stack->init = 1;							// Mark as initialized
}

void JKG_Stack_Push(jkg_stack_t *stack, asyncTask_t *task)		// Push a new task on the stack
{
	assert(stack);				// Ensure 'stack' is valid (debug only)
	assert(stack->init);		// Ensure the stack is initialized
	MUTEX_LOCK(stack);			// Lock the mutex/critical section
	ASSERTTASK(task);			// Ensure the task is not already in a stack/queue (debug only)
	FLAGTASK(task);				// Flag the task (as being part of a queue/stack) (debug only)

	task->next = stack->head;	// Link the task to the queue
	stack->head = task;			
	
	MUTEX_UNLOCK(stack);		// Unlock the mutex/CS
}

asyncTask_t *JKG_Stack_Pop(jkg_stack_t *stack)
{
	asyncTask_t *task;

	assert(stack);
	assert(stack->init);
	MUTEX_LOCK(stack);
	
	task = stack->head;			// Get the head item
	if (task) {					// If it's not NULL, there was an item
		stack->head = stack->head->next;	// Update the head item
		task->next = NULL;
		UNFLAGTASK(task);		// Unflag the task (debug only)
	}
	MUTEX_UNLOCK(stack);		// Unlock the mutex/CS
	return task;
}

void JKG_Queue_Destroy(jkg_queue_t *queue)	// Destroys a queue, same process as stack
{
	if (!queue->init) {
		return;
	}
	MUTEX_FREE(queue);
	memset(queue, 0, sizeof(jkg_queue_t));
}

void JKG_Queue_Initialize(jkg_queue_t *queue)	// Initializes a queue, (almost) same process as stack
{
	if (queue->init) {
		queue->head = 0;
		queue->tail = 0;
		return;
	}
	memset(queue, 0, sizeof(jkg_queue_t));
	MUTEX_INIT(queue);
	queue->init = 1;
}

void JKG_Queue_Enqueue(jkg_queue_t *queue, asyncTask_t *task)	// Queue a new task
{
	assert(queue);
	assert(queue->init);
	MUTEX_LOCK(queue);
	ASSERTTASK(task);
	FLAGTASK(task);

	if (queue->head == NULL && queue->tail == NULL)
	{
		// First item in the queue, so it's both the head and tail item
		task->next = NULL;
		queue->head = task;
		queue->tail = task;
	} else {
		task->next = NULL;		// Add the task to the tail
		queue->tail->next = task;
		queue->tail = task;
	}

	MUTEX_UNLOCK(queue);
}

asyncTask_t *JKG_Queue_Dequeue(jkg_queue_t *queue)	// Get the task in front of the queue
{
	asyncTask_t *task;

	assert(queue);
	assert(queue->init);
	MUTEX_LOCK(queue);

	task = queue->head;		// Task to remove is the head task
	if (task) {
		queue->head = queue->head->next;	// Move the head item to the next item
		if (!queue->head) {
			queue->tail = NULL;				// If the head is NULL, we removed the last remaining item, so clear tail as well
		}
		task->next = NULL;
		UNFLAGTASK(task);
	}
	MUTEX_UNLOCK(queue);
	return task;
}