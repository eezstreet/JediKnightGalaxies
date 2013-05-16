////////////////////////////////////////
//
// High resolution timers (Win32 & Linux)
// For Jedi Knight Galaxies
//
// By BobaFett
//
////////////////////////////////////////

#ifdef _WIN32
	#include <windows.h>
#else
	#include <sys/time.h>
#endif
#include <stdlib.h>


#ifdef _WIN32
	typedef struct {
		int active;
		LARGE_INTEGER start;
		LARGE_INTEGER stop;
	} timerslot_t;
#else 
	typedef struct {
		int active;
		struct timeval start;
		struct timeval stop;
	} timerslot_t;
#endif

static timerslot_t TimerSlots[256];

void HRT_Start(int TimerID) {
	if (TimerID < 0 || TimerID > 255) {
		return;
	}
#ifdef _WIN32
	QueryPerformanceCounter(&TimerSlots[TimerID].start);
#else
	gettimeofday(&TimerSlots[TimerID].start, NULL);
#endif
	TimerSlots[TimerID].active = 1;
}

void HRT_Stop(int TimerID) {
	if (TimerID < 0 || TimerID > 256) {
		return;
	}
#ifdef _WIN32
	QueryPerformanceCounter(&TimerSlots[TimerID].stop);
#else
	gettimeofday(&TimerSlots[TimerID].stop, NULL);
#endif
	TimerSlots[TimerID].active = 0;
}

double HRT_GetTimingMMS(int TimerID) {
	double starttime, endtime;
#ifdef WIN32
	LARGE_INTEGER	freq;
#endif
	if (TimerID < 0 || TimerID > 256) {
		return 0.0;
	}
#ifdef WIN32
	QueryPerformanceFrequency(&freq);
        QueryPerformanceCounter(&TimerSlots[TimerID].stop);

    starttime = TimerSlots[TimerID].start.QuadPart * (1000000.0 / freq.QuadPart);
    endtime = TimerSlots[TimerID].stop.QuadPart * (1000000.0 / freq.QuadPart);
#else
	
    gettimeofday(&TimerSlots[TimerID].stop, NULL);
	
    //starttime = (TimerSlots[TimerID].start.tv_sec * 1000000.0) + TimerSlots[TimerID].start.tv_usec;
    //endtime = (TimerSlots[TimerID].stop.tv_sec * 1000000.0) + TimerSlots[TimerID].stop.tv_usec;
    // Since these numbers can be extremely high, we'll use an alternative method to work this out
    {
        double temp;
        temp = TimerSlots[TimerID].stop.tv_sec - TimerSlots[TimerID].start.tv_sec;
        temp *= 1000000;
        temp += (TimerSlots[TimerID].stop.tv_usec - TimerSlots[TimerID].start.tv_usec);
        return temp;
    }
#endif
	return endtime - starttime;
}

double HRT_GetTimingMS(int TimerID) {
	return HRT_GetTimingMMS(TimerID) * 0.001;

}

double HRT_GetTimingS(int TimerID) {
	return HRT_GetTimingMMS(TimerID) * 0.000001;
}