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
// jkg_gangwars.h
// Contains crap about the TDM/"Gang Wars" gametype proposed by Pande
// Copyright (c) 2013 Jedi Knight Galaxies

#ifndef __JKG_GANGWARS_H
#define __JKG_GANGWARS_H

#pragma once

typedef struct {
	char name[512];			// Display Name
	char refPtr[512];		// Reference/Internal name
	qhandle_t teamIcon;		// Not used by QAGAME
	char modelStore[32][64];	// Parsed separately. Memory is allocated and freed by the Gang Wars handler
	char defaultModel[64];	// The default model for this team
	unsigned char numModelsInStore;
	qboolean useTeamColors;	// Use team colors for this team? (ie force red team/blue team colors

	// STRINGS
	char longname[128];
	char joinstring[128];
	char toomanystring[128];
	char leadstring[128];
	char menujoinstring[128];
	char menustring[128];

	// Scoreboard / Join Screen colors
	vec4_t teamColor;
} gangWarsTeam_t;

// UQ1: Moved to q_shared.c to stop compile header issues...
extern gangWarsTeam_t bgGangWarsTeams[32];
extern int bgnumGangWarTeams;

void JKG_BG_GangWarsInit(void);
void JKG_BG_GangWarsExit(void);

int JKG_GetTeamByReference(char *reference);
gangWarsTeam_t *JKG_GetTeamPtrByReference(char *reference);

#endif