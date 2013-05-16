//////////////////////////////////////////////////
//
//  JKG Multi-Threading System
//  Version 1.0 (Win32 only)
//
//  $Id$
//
//////////////////////////////////////////////////

/*
	At the moment, this system is only Win32 compatible. Linux support will
	be given in future revisions. There's no plan for Mac support, and it's
	highly doubtful that Mac support will ever be made.

	This system will be shared across all the modules after thorough testing.

	Expect documentation on the	Databank on how to use this threading system.

	- Didz
*/

#include "gl_enginefuncs.h"
#include "jkg_threading.h"
#include "jkg_libcurl.h"
#include <openssl/crypto.h>
#include "json/cJSON.h"
#include "jkg_threadingsq.h"

#include <assert.h>

/* OpenSSL Thread Safety */
#include <windows.h>

static HANDLE *lock_cs = 0;

void JKG_OPENSSL_ThreadLock(int mode, int type, const char *file, int line) {
    (void)line;
    (void)file;
    
	if (mode & CRYPTO_LOCK)	{
		WaitForSingleObject(lock_cs[type], INFINITE);
	} else {
		ReleaseMutex(lock_cs[type]);
	}
}

void JKG_OPENSSL_ThreadSyncInit(void) {
	int i;

	if (lock_cs) {
		return;
	}

	lock_cs = OPENSSL_malloc(CRYPTO_num_locks() * sizeof(HANDLE));
	for (i = 0; i<CRYPTO_num_locks(); i++) {
		lock_cs[i] = CreateMutex(NULL,FALSE,NULL);
	}

	CRYPTO_set_locking_callback((void (*)(int, int, const char *, int))JKG_OPENSSL_ThreadLock);
}

void JKG_OPENSSL_ThreadSyncCleanup(void) {
	int i;

	if (!lock_cs) {
		return;
	}

	CRYPTO_set_locking_callback(NULL);
	for (i = 0; i < CRYPTO_num_locks(); i++) {
		CloseHandle(lock_cs[i]);
	}
	OPENSSL_free(lock_cs);

	lock_cs = 0;
}


static qboolean shuttingDown = qfalse;

static qboolean			backgrounderActive = qfalse;
static void				*backgrounderHandle = NULL;
static unsigned long	backgrounderId = 0;


/* ----------------------- TASK QUEUES ------------------------------- */

struct jkg_tasks_s {
	asyncTask_t asyncTasks[MAX_ASYNC_TASKS];	// All tasks slots
	int taskCount;								// Running tasks
	jkg_stack_t freeTasks;						// Stack of free tasks
	jkg_queue_t pendingTasks;					// Queue of pending tasks
	jkg_queue_t delayedTasks;					// Queue of delayed tasks (add to pending on next frame)

	jkg_queue_t finishedTasksUI;				// Queue of finished UI tasks (awaiting final callback and cleanup)
	jkg_queue_t finishedTasksCG;				// Queue of finished CGame tasks (awaiting final callback and cleanup)
} jkg_tasks;

void JKG_Task_Init()
{
	int i;
	memset(&jkg_tasks, 0, sizeof(jkg_tasks));
	JKG_Stack_Initialize(&jkg_tasks.freeTasks);
	JKG_Queue_Initialize(&jkg_tasks.pendingTasks);
	JKG_Queue_Initialize(&jkg_tasks.delayedTasks);
	JKG_Queue_Initialize(&jkg_tasks.finishedTasksUI);
	JKG_Queue_Initialize(&jkg_tasks.finishedTasksCG);
	
	// Initialize the free task stack (in reverse order so index 0 is on top)
	for (i=MAX_ASYNC_TASKS - 1; i >= 0; i--) {
		JKG_Stack_Push(&jkg_tasks.freeTasks, &jkg_tasks.asyncTasks[i]);
	}
}

void JKG_Task_Terminate()
{
	JKG_Stack_Destroy(&jkg_tasks.freeTasks);
	JKG_Queue_Destroy(&jkg_tasks.pendingTasks);
	JKG_Queue_Destroy(&jkg_tasks.delayedTasks);
	JKG_Queue_Destroy(&jkg_tasks.finishedTasksUI);
	JKG_Queue_Destroy(&jkg_tasks.finishedTasksCG);
}

// JKG_Task_New - Returns a free task slot
asyncTask_t *JKG_Task_New()
{
	asyncTask_t *task = JKG_Stack_Pop(&jkg_tasks.freeTasks);
	if (task) {
		jkg_tasks.taskCount++;
	}
	return task;
}

// JKG_Task_Free - Frees the task
void JKG_Task_Free(asyncTask_t *task)
{
	assert(task);
	JKG_Stack_Push(&jkg_tasks.freeTasks, task);
	jkg_tasks.taskCount--;
}

// JKG_Task_Queue - Queues the task for execution
void JKG_Task_Queue(asyncTask_t *task)
{
	assert(task);
	JKG_Queue_Enqueue(&jkg_tasks.pendingTasks, task);
}

// JKG_Task_GetQueued - Returns the oldest queued task (returns NULL if no more tasks remain)
// Call repeatedly to get all queued tasks
asyncTask_t *JKG_Task_GetQueued()
{
	return JKG_Queue_Dequeue(&jkg_tasks.pendingTasks);
}

// JKG_Task_DelayTask - Puts the task on the delayed queue, so that it can be processed again
// after a call to JKG_Task_ProcessDelayedTasks (used for tasks that want to reset themselves)
void JKG_Task_DelayTask(asyncTask_t *task)
{
	assert(task);
	JKG_Queue_Enqueue(&jkg_tasks.delayedTasks, task);
}

// JKG_Task_ProcessDelayedTasks - Puts all tasks on the delayed queue back on the pending queue
void JKG_Task_ProcessDelayedTasks()
{
	asyncTask_t *task;
	while ((task = JKG_Queue_Dequeue(&jkg_tasks.delayedTasks)) != NULL)
		JKG_Queue_Enqueue(&jkg_tasks.pendingTasks, task);
}

// JKG_Task_Finished - Puts the task on the finished queue (so its callback can be called and the task can be cleaned)
void JKG_Task_Finished(asyncTask_t *task)
{
	assert(task);
	JKG_Queue_Enqueue( ( task->flags & TASKFLAG_UITASK ) ? &jkg_tasks.finishedTasksUI : &jkg_tasks.finishedTasksCG, task);
}

// JKG_Task_GetFinished - Returns the oldest item on the finished task queue
// Call repeatedly to get all items on the queue. The function will return NULL when the queue is empty.
asyncTask_t *JKG_Task_GetFinished(int ui)
{
	return JKG_Queue_Dequeue(ui ? &jkg_tasks.finishedTasksUI : &jkg_tasks.finishedTasksCG);
}

/* ------------------------------------------------------------- */

#ifndef FINAL_BUILD

void JKG_ThreadingDebugPrint( const char *message )
{
	if ( Cvar_GetValueInt("jkg_debugThreading") ) {		// TODO: Clean this up
		Com_Printf( "%s", message );
	}
}

#else
#define JKG_ThreadingDebugPrint
#endif

void JKG_ThreadSleep ( int msec ) {
	Sleep( msec );
}

void JKG_MainThreadPoller ( int ui, int purge )
{
	asyncTask_t *tempTask = NULL;

	while ((tempTask = JKG_Task_GetFinished(ui)) != NULL) {
		if ( !purge && tempTask->finalCallback ) {
			tempTask->finalCallback( tempTask );
		}

		// Free these if they were malloced
		if ( tempTask->initData ) {
			if ( tempTask->flags & TASKFLAG_MALLOCED_INITDATA ) {
				free( tempTask->initData );
			}
			if ( tempTask->flags & TASKFLAG_JSON_INITDATA) {
				cJSON_Delete( (cJSON *)tempTask->initData );
			}
			tempTask->initData = NULL;
		}

		if ( tempTask->finalData ) {
			if ( tempTask->flags & TASKFLAG_MALLOCED_FINALDATA ) {
				free( tempTask->finalData );
			}
			if ( tempTask->flags & TASKFLAG_JSON_FINALDATA) {
				cJSON_Delete( (cJSON *)tempTask->finalData );
			}
			tempTask->finalData = NULL;
		}

		tempTask->state = TASKSTATE_FREE;
		JKG_Task_Free(tempTask);	
	}

	return;
}

static unsigned long __stdcall JKG_BackgroundWorker ( void *lulz /*Pass something useful if you need*/ )
{
	int errcode = 0;

	errcode = JKG_Libcurl_Init();
	if ( errcode ) return errcode;

	backgrounderActive = qtrue;

#ifndef FINAL_BUILD
	Com_Printf( "^2Background Worker started at %i\n", Sys_Milliseconds() );
#endif

	while ( !shuttingDown )
	{
		asyncTask_t *workTask = NULL;

		JKG_Libcurl_Poller();

		JKG_Task_ProcessDelayedTasks();

		while ((workTask = JKG_Task_GetQueued()) != NULL)
		{
			JKG_ThreadingDebugPrint( va( "^2Task ID %i is about to be processed!", workTask-jkg_tasks.asyncTasks ) );
			
			workTask->state = TASKSTATE_PROCESSING;
			assert( workTask->threadFunc ); // Make sure we can call something...
			workTask->errorCode = workTask->threadFunc( workTask ); // RUN THE TASK, collecting the error code
			
			if (workTask->flags & TASKFLAG_RESET) {
				workTask->flags &= ~TASKFLAG_RESET;
				workTask->state = TASKSTATE_QUEUED;		// Run it again in the next frame
				JKG_Task_DelayTask(workTask);
				continue;
			}
			if (!(workTask->flags & TASKFLAG_PENDING)) {	// Check if this task is still pending
				workTask->state = TASKSTATE_DATACOLLECT;	// If not, mark it as complete
				JKG_Task_Finished(workTask);				// Put the task on the finished task queue if it's not pending
			}
		}

		JKG_ThreadSleep( 1 );
	}

	JKG_Libcurl_Shutdown();

	shuttingDown = qfalse;
	backgrounderActive = qfalse;

	return 0;
}

void JKG_PrintTasksTable ( )
{
	char buf[MAX_STRING_CHARS];
	int i;

	Com_Printf("\"----- JKG Async Tasks -----\n\"" );

	Q_strncpyz( buf, "\"", sizeof(buf) );
	for ( i = 0; i < MAX_ASYNC_TASKS; i++ ) {
		if ( jkg_tasks.asyncTasks[i].state != TASKSTATE_FREE ) {
			Q_strcat( buf, sizeof(buf), va("%5i | %i\n", i, jkg_tasks.asyncTasks[i].state) );
		}
	}
	Q_strcat( buf, sizeof(buf), "\"" );

	Com_Printf( buf );
	Com_Printf( "\"---------------------------\n\"" );

	return;
}

void JKG_InitThreading ( void )
{
	if (backgrounderActive) {
		return;
	}

	Com_Printf( "^5Initializing Background Worker...\n" );

	JKG_OPENSSL_ThreadSyncInit();
	Com_Printf( "Initialized OpenSSL thread synchronisation\n" );

	JKG_Task_Init();
	Com_Printf( "Initialized task queue management\n" );

	backgrounderHandle = CreateThread( NULL, 0, JKG_BackgroundWorker, NULL, 0, &backgrounderId );
	if ( !backgrounderHandle ) {
		Com_Error( ERR_FATAL, "Failed to initialize thread system!" );
	}

}

void JKG_ShutdownThreading ( int maxWaitTime )
{
	int startMurder = Sys_Milliseconds();
	int killDuration;

	if (!backgrounderActive) return;

#ifndef FINAL_BUILD
	Com_Printf( "^5Shutting down Background Worker...\n" );
#endif

	shuttingDown = qtrue;

	while ( shuttingDown ) {
		
		if ( Sys_Milliseconds() > startMurder + maxWaitTime ) {

			TerminateThread( backgrounderHandle, 0 );
			CloseHandle( backgrounderHandle );
#ifndef FINAL_BUILD
			Com_Printf( "^1Thread system terminated forcefully!\n" );
#endif
			break;
		}

		JKG_ThreadSleep( 1 );
	}

	killDuration = Sys_Milliseconds() - startMurder;
#ifndef FINAL_BUILD
	Com_Printf( "^5Took %i milliseconds\n", killDuration );
#endif

	JKG_OPENSSL_ThreadSyncCleanup();
	JKG_Task_Terminate();
}

asyncTask_t *JKG_NewAsyncTask ( int (*threadFunc)(struct asyncTask_s *taskPointer), void *initData, int flags, void (*finalCallback)(struct asyncTask_s *taskPointer), int ui ) {
	asyncTask_t *newTask = NULL;

	newTask = JKG_Task_New();

	if ( !newTask ) {
		return NULL;
	}

	assert( threadFunc );

	memset( newTask, 0, sizeof(*newTask) );
	newTask->state = TASKSTATE_CREATING;
	newTask->threadFunc = threadFunc;
	newTask->initData = initData;
	newTask->flags = ui ? flags | TASKFLAG_UITASK : flags;
	newTask->finalCallback = finalCallback;
	newTask->createTime = Sys_Milliseconds();
	newTask->state = TASKSTATE_QUEUED;

	JKG_Task_Queue(newTask);

	return newTask;
}
