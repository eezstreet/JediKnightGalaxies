////////////////////////////
//
//  Stress level indicator
//
////////////////////////////

static double frametimes[16];
static int frameidx = 0;
static double maxframetime = -1;
static double minframetime = -1;

#include "gls_enginefuncs.h"

void GLS_StressAddTime(double frametimeMs) {
	frametimes[frameidx] = frametimeMs;
	frameidx++;
	if (frameidx > 15) {
		frameidx = 0;
	}
	if (maxframetime == -1 || maxframetime < frametimeMs) {
		maxframetime = frametimeMs;
	}
	if (minframetime == -1 || minframetime > frametimeMs) {
		minframetime = frametimeMs;
	}
}

double GLS_GetStressLevel() {
	double avg;
	int i;
	for (i=0, avg = 0.0; i<16; i++) {
		avg += frametimes[i];
	}
	avg /= 16;
	return avg;
}

void GLS_DisplayStressLevel() {
	double avg;
	double frametime;
	double pct, pct1, pct2;
	const char *strlevel;
	int i;
	for (i=0, avg = 0.0; i<16; i++) {
		avg += frametimes[i];
	}
	avg /= 16;
	frametime = 1000 / 20; // sv_fps
	pct = (avg / frametime) * 100.0;
	pct1 = (minframetime / frametime) * 100.0;
	pct2 = (maxframetime / frametime) * 100.0;
	if (pct < 10) {
		strlevel = "Minimal";
	} else if (pct < 25) {
		strlevel = "Very Low";
	} else if (pct < 50) {
		strlevel = "Low";
	} else if (pct < 75) {
		strlevel = "Medium";
	} else if (pct < 100) {
		strlevel = "High";
	} else if (pct < 200) {
		strlevel = "Very high";
	} else {
		strlevel = "Extremely high";
	}
	Com_Printf("Server stress level: %s (%.2f pct) - %.3f ms avg frametime (out of %.3f)\nServer frametime spikes since last check: Lowest - %.3f ms (%.3f pct), Highest - %.3f (%.3f pct)\n", strlevel, pct, avg, frametime, minframetime, pct1, maxframetime, pct2);
	minframetime = -1;
	maxframetime = -1;
}