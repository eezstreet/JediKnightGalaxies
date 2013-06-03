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

#include "g_local.h"
#include "jkg_threading.h"
#include <openssl/crypto.h>
#include "json/cJSON.h"
#include "jkg_threadingsq.h"

/* OpenSSL Thread Safety */
#ifdef _WIN32
static HANDLE *lock_cs = 0;

void JKG_OPENSSL_ThreadLock(int mode, int type, const char *file, int line) {
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

	lock_cs = (HANDLE *)OPENSSL_malloc(CRYPTO_num_locks() * sizeof(HANDLE));
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

#endif

#ifdef PTHREADS
static pthread_mutex_t *lock_cs = 0;
static long *lock_count = 0;

void JKG_OPENSSL_ThreadLock(int mode, int type, const char *file, int line) {

	if (mode & CRYPTO_LOCK) {
		pthread_mutex_lock(&(lock_cs[type]));
		lock_count[type]++;
	} else {
		pthread_mutex_unlock(&(lock_cs[type]));
	}
}

unsigned long JKG_OPENSSL_PThread_ThreadID(void) {
	unsigned long ret;

	ret = (unsigned long)pthread_self();
	return ret;
}

void JKG_OPENSSL_ThreadSyncInit(void) {
	int i;

	if (lock_cs || lock_count) { 
		return;
	}

	lock_cs = OPENSSL_malloc(CRYPTO_num_locks() * sizeof(pthread_mutex_t));
	lock_count = OPENSSL_malloc(CRYPTO_num_locks() * sizeof(long));
	for (i=0; i<CRYPTO_num_locks(); i++) {
		lock_count[i]=0;
		pthread_mutex_init(&(lock_cs[i]),NULL);
	}

	CRYPTO_set_id_callback((unsigned long (*)())JKG_OPENSSL_PThread_ThreadID);
	CRYPTO_set_locking_callback((void (*)(int, int, const char *, int))JKG_OPENSSL_ThreadLock);
}

void JKG_OPENSSL_ThreadSyncCleanup(void) {
	int i;

	if (!lock_cs && !lock_count) { 
		return;
	}

	CRYPTO_set_locking_callback(NULL);

	for (i = 0; i < CRYPTO_num_locks(); i++) {
		pthread_mutex_destroy(&(lock_cs[i]));
	}
	OPENSSL_free(lock_cs);
	OPENSSL_free(lock_count);
}

#endif



#ifdef _WIN32
static void				*backgrounderHandle = NULL;
static unsigned long	backgrounderId = 0;

// Critical sections might be removed later
CRITICAL_SECTION cs_G_Printf;
static qboolean initCriticalSections = qfalse;
#endif

static qboolean shuttingDown = qfalse;
static qboolean			backgrounderActive = qfalse;
#ifdef __linux__
pthread_t backgrounderHandle;

void *cs_G_Printf = NULL;
#endif

/* ----------------------- TASK QUEUES ------------------------------- */

struct jkg_tasks_s {
	asyncTask_t asyncTasks[MAX_ASYNC_TASKS];	// All tasks slots
	int taskCount;								// Running tasks
	jkg_stack_t freeTasks;						// Stack of free tasks
	jkg_queue_t pendingTasks;					// Queue of pending tasks
	jkg_queue_t delayedTasks;					// Queue of delayed tasks (add to pending on next frame)
	jkg_queue_t finishedTasks;					// Queue of finished tasks (awaiting final callback and cleanup)
} jkg_tasks;

void JKG_Task_Init()
{
	int i;
	memset(&jkg_tasks, 0, sizeof(jkg_tasks));
	JKG_Stack_Initialize(&jkg_tasks.freeTasks);
	JKG_Queue_Initialize(&jkg_tasks.pendingTasks);
	JKG_Queue_Initialize(&jkg_tasks.delayedTasks);
	JKG_Queue_Initialize(&jkg_tasks.finishedTasks);
	
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
	JKG_Queue_Destroy(&jkg_tasks.finishedTasks);
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
	JKG_Queue_Enqueue(&jkg_tasks.finishedTasks, task);
}

// JKG_Task_GetFinished - Returns the oldest item on the finished task queue
// Call repeatedly to get all items on the queue. The function will return NULL when the queue is empty.
asyncTask_t *JKG_Task_GetFinished()
{
	return JKG_Queue_Dequeue(&jkg_tasks.finishedTasks);
}

/* ------------------------------------------------------------- */

#ifndef FINAL_BUILD
void JKG_ThreadingDebugPrint( const char *message )
{
	if ( jkg_debugThreading.integer ) {
		G_Printf( "%s", message );
	}
}
#else
#define JKG_ThreadingDebugPrint
#endif

void JKG_ThreadSleep ( int msec ) {
#ifdef _WIN32
	Sleep( msec );
#endif
#ifdef __linux__
	struct timespec timeOut, remains;

	timeOut.tv_sec = 0;
	timeOut.tv_nsec = msec * 10000000;

	nanosleep(&timeOut, &remains); 
#endif
}

void JKG_EnterCriticalSection( void *section )
{
#ifdef _WIN32
	if ( initCriticalSections ) EnterCriticalSection( (LPCRITICAL_SECTION)section );
#endif
}

void JKG_LeaveCriticalSection( void *section )
{
#ifdef _WIN32
	if ( initCriticalSections ) LeaveCriticalSection( (LPCRITICAL_SECTION)section );
#endif
}

void JKG_Printf ( int (QDECL *syscall)( int arg, ... ), const char *message )
{
    JKG_EnterCriticalSection (&cs_G_Printf);
    syscall (G_PRINT, message);
    JKG_LeaveCriticalSection (&cs_G_Printf);
}

void JKG_MainThreadPoller ( void )
{
	asyncTask_t *tempTask = NULL;

	while ((tempTask = JKG_Task_GetFinished()) != NULL) {
		if ( tempTask->finalCallback ) {
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

#ifdef _WIN32
static unsigned long __stdcall JKG_BackgroundWorker ( void *lulz /*Pass something useful if you need*/ )
#endif
#ifdef __linux__
static void *JKG_BackgroundWorker ( void *lulz /*Pass something useful if you need*/ )
#endif
{
	backgrounderActive = qtrue;

#ifndef FINAL_BUILD
	Com_Printf( "^2Background Worker started at %i\n", level.time );
#endif
	while ( !shuttingDown || (jkg_tasks.taskCount > 0) )
	{
		asyncTask_t *workTask = NULL;

		JKG_Task_ProcessDelayedTasks();

		while ((workTask = JKG_Task_GetQueued()) != NULL && workTask != NULL)
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

	shuttingDown = qfalse;
	backgrounderActive = qfalse;

	return 0;
}

void JKG_PrintTasksTable ( int clientNum )
{
	char buf[MAX_STRING_CHARS];
	int i;

	trap_SendServerCommand( clientNum, "print \"----- JKG Async Tasks -----\n\"" );

	Q_strncpyz( buf, "print \"", sizeof(buf) );
	for ( i = 0; i < MAX_ASYNC_TASKS; i++ ) {
		if ( jkg_tasks.asyncTasks[i].state != TASKSTATE_FREE ) {
			Q_strcat( buf, sizeof(buf), va("%5i | %i\n", i, jkg_tasks.asyncTasks[i].state) );
		}
	}
	Q_strcat( buf, sizeof(buf), "\"" );

	trap_SendServerCommand( clientNum, buf );
	trap_SendServerCommand( clientNum, "print \"---------------------------\n\"" );

	return;
}

void JKG_InitThreading ( void )
{
#ifdef __linux__
	int pthErr;
#endif
	Com_Printf( "^5Initializing Background Worker...\n" );

	//memset( asyncTasks, 0, sizeof(*asyncTasks) );

#ifdef _WIN32
	InitializeCriticalSection( &cs_G_Printf );
	initCriticalSections = qtrue;
	Com_Printf( "Initialized critical sections\n" );
#endif

	JKG_OPENSSL_ThreadSyncInit();
	Com_Printf( "Initialized OpenSSL thread synchronisation\n" );

	JKG_Task_Init();
	Com_Printf( "Initialized task queue management\n" );

#ifdef _WIN32
	backgrounderHandle = CreateThread( NULL, 0, JKG_BackgroundWorker, NULL, 0, &backgrounderId );
	if ( !backgrounderHandle ) {
		Com_Error( ERR_DROP, "Failed to initialize thread system!" );
	}
#endif
#ifdef __linux__
	pthErr = pthread_create( &backgrounderHandle, NULL, JKG_BackgroundWorker, NULL );
	if ( pthErr ) {
		Com_Error( ERR_DROP, "Failed to initialize thread system!" );
	}
#endif
}

void JKG_ShutdownThreading ( int maxWaitTime )
{
	int startMurder = trap_Milliseconds();
	int killDuration;

	if (!backgrounderActive) return;

#ifndef FINAL_BUILD
	Com_Printf( "^5Shutting down Background Worker...\n" );
#endif

	shuttingDown = qtrue;

	while ( shuttingDown ) {
		JKG_MainThreadPoller();		// Keep processing queries
		if ( trap_Milliseconds() > startMurder + maxWaitTime ) {
#ifdef _WIN32
			TerminateThread( backgrounderHandle, 0 );
			CloseHandle( backgrounderHandle );
#endif
#ifdef __linux__
			pthread_cancel( backgrounderHandle );
#endif
#ifndef FINAL_BUILD
			Com_Printf( "^1Thread system terminated forcefully!\n" );
#endif
			break;
		}

		JKG_ThreadSleep( 1 );
	}

	killDuration = trap_Milliseconds() - startMurder;
#ifndef FINAL_BUILD
	Com_Printf( "^5Took %i milliseconds\n", killDuration );
#endif

#ifdef _WIN32
	initCriticalSections = qfalse;
	DeleteCriticalSection( &cs_G_Printf );
#endif

	JKG_OPENSSL_ThreadSyncCleanup();
	JKG_Task_Terminate();
}

asyncTask_t *JKG_NewAsyncTask ( int (*threadFunc)(struct asyncTask_s *taskPointer), void *initData, int flags, void (*finalCallback)(struct asyncTask_s *taskPointer) ) {
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
	newTask->flags = flags;
	newTask->finalCallback = finalCallback;
	newTask->createTime = level.time;
	newTask->state = TASKSTATE_QUEUED;

	JKG_Task_Queue(newTask);

	return newTask;
}

qboolean JKG_ThreadingInitialized(void)
{
	if(!jkg_tasks.freeTasks.init)
	{
		return qfalse;
	}
	return qtrue;
}
