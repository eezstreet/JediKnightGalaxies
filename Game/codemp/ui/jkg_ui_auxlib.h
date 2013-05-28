/*
=======================================================================

	UI Auxiliary library interface functions

=======================================================================
*/

typedef enum {
	TASKSTATE_FREE = 0,		// Free, ready to be created.
	TASKSTATE_CREATING,		// Locked for creation.
	TASKSTATE_QUEUED,		// Waiting in the processing queue.
	TASKSTATE_PROCESSING,	// Processing the task.
	TASKSTATE_DATACOLLECT,	// Task ended, waiting for data to be used.
	TASKSTATE_FINISHED		// Finished, task manager will free memory.
} taskState_t;

typedef struct asyncTask_s {
	taskState_t		state;			// Task state
	int				createTime;		// Make sure tasks are processed in the order they were created
	int				flags;			// Misc flags for the task
	void			*initData;		// Generic data buffer pointer, assigned on creation
	void			*finalData;		// Generic data buffer pointer, assigned in processing
	int				(*threadFunc)(struct asyncTask_s *taskPointer);	// Actual function to process the task
	int				errorCode;		// Error code received from the threadFunc
	void			(*finalCallback)(struct asyncTask_s *taskPointer);	// Callback function gets called from main thread on task finish

#ifdef _DEBUG
	int	inList;		// Debugging purposes only: Whether or not this task is on a queue or list
#endif

	struct asyncTask_s *next;		// Used by stacks/queues
} asyncTask_t;
/*
void JKG_UI_LoadAuxiliaryLibrary();
void JKG_GLUI_PatchEngine();
void JKG_GLUI_BreakLinkup();
void JKG_GLUI_ProcessTasks();

void JKG_GLUI_Task_Test(void (*callback)(asyncTask_t *task));
void JKG_GLUI_Task_GetTermsOfUse(void (*callback)(asyncTask_t *task));
void JKG_GLUI_Task_RegisterUser(const char *username, const char *password, const char *email, void (*callback)(asyncTask_t *task));
void JKG_GLUI_Task_Login(const char *username, const char *password, void (*callback)(asyncTask_t *task));
*/

