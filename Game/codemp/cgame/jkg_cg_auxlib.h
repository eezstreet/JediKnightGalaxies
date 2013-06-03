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
// jkg_cg_auxlib.h
// CGame Auxiliary library interface functions
// Copyright (c) 2013 Jedi Knight Galaxies

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