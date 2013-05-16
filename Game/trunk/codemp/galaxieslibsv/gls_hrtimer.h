////////////////////////////////////////
//
// High resolution timers (Win32 & Linux)
// For Jedi Knight Galaxies
//
// By BobaFett
//
////////////////////////////////////////

// =================================================
// HRT_Start
// -------------------------------------------------
// Starts the timer with the ID specified (range 0-255)
// =================================================
void HRT_Start(int TimerID);

// =================================================
// HRT_Stop
// -------------------------------------------------
// Stops the timer with the ID specified
// =================================================
void HRT_Stop(int TimerID);

// =================================================
// HRT_GetTimingMMS
// -------------------------------------------------
// Gets the timer result in microseconds
// =================================================
double HRT_GetTimingMMS(int TimerID);

// =================================================
// HRT_GetTimingMS
// -------------------------------------------------
// Gets the timer result in milliseconds
// =================================================
double HRT_GetTimingMS(int TimerID);

// =================================================
// HRT_GetTimingS
// -------------------------------------------------
// Gets the timer result in seconds
// =================================================
double HRT_GetTimingS(int TimerID);