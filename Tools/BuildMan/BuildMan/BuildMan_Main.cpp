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
// BuildMan_Main.cpp
// Part of BuildMan, Build Number Management tool for Jedi Knight Galaxies
// Copyright (c) 2013 Jedi Knight Galaxies
// File by eezstreet

// FIXME: Unix support
#ifdef _WIN32
#include <Windows.h>
#include <direct.h>
#endif
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <sstream>

#define FILE_WE_NEED "game\\q_shared.h"

enum DesiredProjects_e
{
	DP_CGAME,
	DP_GAME,
	DP_UI,
	DP_AUX,
	DP_AUXSV,
	DP_MAX
};

using namespace std;

static DesiredProjects_e desiredProject = DP_MAX;
static FILE *buildFilePtr;
static string projectName;
static vector<string> fileContents;

/*
==================
ConvertToUppercase

Converts string to uppercase
Whoever came up with the name of this func is a real creative dude.
==================
*/

char *ConvertToUppercase( char *text )
{
	int i = 0;
	while( text[i] != '\0' )
	{
		text[i] = (char)toupper(text[i]);
		i++;
	}
	return text;
}

/*
==================
GetProjectType

Returns enumerated project type from string
==================
*/

DesiredProjects_e GetProjectType( char *proj )
{
	if( !strcmp(proj, "cgame") ) return DP_CGAME;
	else if( !strcmp(proj, "game") ) return DP_GAME;
	else if( !strcmp(proj, "ui") ) return DP_UI;
	else if( !strcmp(proj, "aux") ) return DP_AUX;
	else if( !strcmp(proj, "auxsv") ) return DP_AUXSV;
	else return DP_MAX;
}

/*
==================
GetProjectDefineName

Retrieves the name of the define used for this project
==================
*/

void GetProjectDefineName( char *proj )
{
	string defineText = "BUILDNUM_";
	defineText += ConvertToUppercase(proj);
	projectName = defineText.c_str();
}

/*
==================
FindLineThatStartsWith

...yeah, another one of these funcs.
==================
*/

string FindLineThatStartsWith( const char *text, int *vectorIndex )
{
	vector<string>::iterator it;
	for(it = fileContents.begin(); it != fileContents.end(); it++)
	{
		if(!strncmp(it->c_str(), text, strlen(text)))
		{
			*vectorIndex = it-fileContents.begin();
			return *it;
		}
	}

	return "";
}

/*
==================
AdjustDefine

Adjusts a #define in a file.
==================
*/

void AdjustDefine( const char *defineName, bool incrementValue = true, int newValue = -1 )
{
	int vectorIndex = 0;
	string ourLine = "#define ";
	ourLine += defineName;

	string defineLine = FindLineThatStartsWith( ourLine.c_str(), &vectorIndex );
	string newLine = "#define ";
	newLine += defineName;
	newLine += " ";
	
	if( incrementValue )
	{
		// increment the current value
		int outputValue = 0;
		string defineFormat = newLine;
		defineFormat += "%i";

		sscanf(defineLine.c_str(), defineFormat.c_str(), &outputValue);
		outputValue++;

		stringstream ss;

		ss << newLine << outputValue << endl;

		newLine = ss.str();
	}
	else
	{
		stringstream ss;

		ss << newLine << newValue << endl;
		newLine = ss.str();
	}

	// set the new value in the vector so that when we write it later, it all gets updated
	fileContents[vectorIndex] = newLine;
}

/*
==================
ShouldWeBeIncrementing

Returns true if we're allowed to increment (#define INCREMENT_BUILD_NUMBERS	1)
==================
*/

bool ShouldWeBeIncrementing( void )
{
	int dummy = 0;
	string defineLine = FindLineThatStartsWith("#define INCREMENT_BUILD_NUMBERS", &dummy);
	sscanf(defineLine.c_str(), "#define INCREMENT_BUILD_NUMBERS %i", &dummy);
	
	if(dummy > 0) return true;
	return false;
}

/*
==================
ScanFile

Scans the file and puts everything into a clean little vector
==================
*/

void ScanFile( void )
{
	// loop loop loop
	char line[1024];

	while( fgets( line, 1024, buildFilePtr ) && !feof(buildFilePtr) && !ferror(buildFilePtr) )
	{
		fileContents.push_back(line);
	}
}

/*
==================
DoBuildIncrement

All of our basic criteria has been met with main, so let's go ahead and start the increment process.
==================
*/

void DoBuildIncrement( void )
{
	long long offset = -1;
	ScanFile();

	if(!ShouldWeBeIncrementing())
	{
		return;
	}
	AdjustDefine( projectName.c_str() );
}

/*
==================
FileWrite

Final step: writes each line to the file
zzzz
==================
*/

void FileWrite( void )
{
	string concatLines = "";

	for(vector<string>::iterator it = fileContents.begin(); it != fileContents.end(); it++)
		concatLines += *it;

	fwrite(concatLines.c_str(), sizeof(byte), concatLines.length(), buildFilePtr);
}

/*
==================
main
==================
*/

// Application entry point --eez
int main(int argc, char *argv[])
{
	printf("BuildMan build number manager for Jedi Knight Galaxies\n");
	printf("Copyright (c) 2013 Jedi Knight Galaxies / eezstreet\n\n");

	// Check arg
	if(argc < 2)
	{
		printf("Usage: buildman <project>\n");
		printf("Possible projects: cgame game ui aux auxsv\n");
		return 0;
	}

	// I guess now, just figure out what project we want.
	char *projectText = argv[1];
	desiredProject = GetProjectType( projectText );

	if( desiredProject == DP_MAX )
	{
		printf( "ERROR: Invalid project '%s'.\n", projectText );
		return 0;
	}

	// Great, now let's check if the file with the #defines exists..
	// Grab our working directory because C file operations are buttfrustrated
	char working_directory[1024];
	string dir;

	getcwd(working_directory, sizeof(working_directory)-1);
	working_directory[1023] = 0;
	dir = working_directory;
	dir += "\\";
	dir += FILE_WE_NEED;

	buildFilePtr = fopen(dir.c_str(), "r");
	if(!buildFilePtr)
	{
		printf( "ERROR: Could not find/open build file '%s' for reading, aborting...\n", FILE_WE_NEED );
		return 0;
	}

	GetProjectDefineName( argv[1] );
	DoBuildIncrement( );

	fclose(buildFilePtr);

	buildFilePtr = fopen(dir.c_str(), "w");
	if(!buildFilePtr)
	{
		printf( "ERROR: Could not open build file '%s' for writing, aborting...\n", FILE_WE_NEED );
		return 0;
	}

	FileWrite();

	fclose(buildFilePtr);

	return 0;
}