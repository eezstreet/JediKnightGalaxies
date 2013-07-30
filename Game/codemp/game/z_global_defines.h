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
// z_global_defines.h
// Global Defines for JKG. Enable/disable features here!
// (c) 2013 Jedi Knight Galaxies


// ----------------------------------------------------------------------------------------
// Global defines for JKG. Enable/disable features here!
// ----------------------------------------------------------------------------------------

//
// ----------------------------------------------------------------------------------------
//

#ifndef __JKG_OPTIONS__
#define __JKG_OPTIONS__

#ifndef _DEBUG
#define _CRT_SECURE_NO_DEPRECATE // UQ1: Stop VC8 compile warnings...
#endif //_DEBUG

//
// ----------------------------------------------------------------------------------------
//

// UQ1: Disabled debug cvars by default, enable if you need them...
//#define __DEBUGGING__

// MMO Stuff...
// NOTE: __MMO__ should be disabled for _ALL_ Phase 1 and possibly Phase 2 releases
//#define __MMO__									// UQ1: Sets compile to MMO mode. 1000 player support...
//#define __RPG__									// UQ1: For the new gametypes that combine all the others into a single instance... Need to update gametypes file and menus to match when we enable...
//#define __PHASE_2__								// UQ1: Stuff that needs to be fixed for phase 2...
#define __PTR										// eez: Public Test Realm

// AI Stuff...
#define __NPC_MINPLAYERS__							// UQ1: Like bot_minplayers but for NPCs... For fun...
#define __DOMINANCE_NPC__							// UQ1: DominancE Advanced NPCs...
#define __AUTOWAYPOINT__							// UQ1: Auto-Waypointing System...
#define __TEMPORARY_HACK_FOR_BOT_LIGHTSABERS__	// UQ1: Just forces all bots to use a saber for testing/fun...
#define __CHECK_ROUTING_BEFORE_WAYPOINT_SPAWNS__	// UQ1: Checks for useable routes before spawning anything at a waypoint...
//#define __BASIC_RANDOM_MOVEMENT_AI__				// UQ1: For testing large numbers of players without using much CPU.
#define __ALWAYS_TWO_TRAVELLINGVENDORS			// eez: Always spawn two travelling vendors

// Joystick support
#define __XBOX360CONTROLLER							// eez: XBOX 360 controller support. Do /in_joystick 2 to activate.
//#define __PS3CONTROLLER_DUALSHOCK					// eez: at request of szico		// DOES NOT WORK (I can't get it to work properly)

// Generic Stuff...
#define __DISABLE_UNUSED_SPAWNS__				// UQ1: Disable spawning of map ammo/weapons... (since we can't pick them up anyway, they are a waste of entities and cpu time)
#define __WEAPON_HOLSTER__						// UQ1: Weapon Holsterring System... (Under Construction)
//#define __WAYPOINT_SPAWNS__						// UQ1: Use waypoints as spawn locations...
#define __ENTITY_OVERRIDES__						// UQ1: Override/Add-To map entity spawns with an external file...
//#define VEH_CONTROL_SCHEME_4						// Vehicle Control System...
//#define _G_FRAME_PERFANAL

// OpenGL FX...
//#define __GL_ANAGLYPH__							// UQ1: Red/Blue 3D Glasses... (Not Functional)
//#define __GL_EMBOSS__								// UQ1: Emboss... (Not Functional)
//#define __SWF__									// UQ1: Experimental SWF renderer... // eez: this is almost done!
//#define __EXPERIMENTAL_SHADOWS__					// UQ1: Experimental Shadow System... // laggy

// New gametypes --eez
// 9lives FFA. All players have 9 lives and gain one life per each kill, until the first person is out.
// Items are not bought, rather, players are given better weapons with killstreaks. Thus, there are no credits.
//#define __JKG_NINELIVES__
// Ticketing-based TFFA. Both sides have a limited set of lives and shops are the same as normal.
//#define __JKG_TICKETING__
// Round-based TFFA. Each player only has one life (or a limited amount of lives)
// Items are bought between rounds
//#define __JKG_ROUNDBASED__

//
// ----------------------------------------------------------------------------------------
//

#if defined (_WIN32)

// UQ1: let's add some #pragma's there to disable unimportant extra warnings...
// FIXME: move this all to disablewarnings.h

// warning C4706: assignment within conditional expression
#pragma warning( disable : 4706 )

// warning C4710: function 'XXXXX' not inlined
#pragma warning( disable : 4710 )

// warning C4221: nonstandard extension used : 'XXXXX' : cannot be initialized using address of automatic variable 'name'
#pragma warning( disable : 4221 )

// warning C4701: local variable 'XXXXX' may be used without having been initialized
#pragma warning( disable : 4701 )

// warning C4211: nonstandard extension used : redefined extern to static
#pragma warning( disable : 4211 )

// warning C4204: nonstandard extension used : non-constant aggregate initializer
#pragma warning( disable : 4204 )

// warning C4130: '==' : logical operation on address of string constant
#pragma warning( disable : 4130 )

// warning C4090: 'function' : different 'const' qualifiers
#pragma warning( disable : 4090 )

//
// Added: Vis Studio 2010 Code Analysis warnings...
//

// warning C6262: Function uses 'XXXXX' bytes of stack
#pragma warning( disable : 6262 )

// warning C6011: Dereferencing NULL pointer
#pragma warning( disable : 6011 )

// warning C6385: Invalid data: accessing 'xxx', the readable size is 'xxx' bytes, but 'xxx' bytes might be read
#pragma warning( disable : 6385 )

// warning C6387: 'argument x' might be '0': this does not adhere to the specification for the function 'xxx'
#pragma warning( disable : 6387 )

#pragma warning(disable : 4018)     // signed/unsigned mismatch
#pragma warning(disable : 4032)
#pragma warning(disable : 4051)
#pragma warning(disable : 4057)		// slightly different base types
#pragma warning(disable : 4100)		// unreferenced formal parameter
#pragma warning(disable : 4115)
#pragma warning(disable : 4125)		// decimal digit terminates octal escape sequence
#pragma warning(disable : 4127)		// conditional expression is constant
#pragma warning(disable : 4136)
#pragma warning(disable : 4152)		// nonstandard extension, function/data pointer conversion in expression
#pragma warning(disable : 4201)
#pragma warning(disable : 4214)
#pragma warning(disable : 4244)		// conversion from double to float
#pragma warning(disable : 4284)		// return type not UDT
#pragma warning(disable : 4305)		// truncation from const double to float
#pragma warning(disable : 4310)		// cast truncates constant value
#pragma warning(disable : 4389)		// signed/unsigned mismatch
#pragma warning(disable : 4503)		// decorated name length truncated
//#pragma warning(disable:  4505)!!!remove these to reduce vm size!! // unreferenced local function has been removed
#pragma warning(disable : 4511)		//copy ctor could not be genned
#pragma warning(disable : 4512)		//assignment op could not be genned
#pragma warning(disable : 4514)		// unreffed inline removed
#pragma warning(disable : 4663)		// c++ lang change
#pragma warning(disable : 4702)		// unreachable code
#pragma warning(disable : 4710)		// not inlined
#pragma warning(disable : 4711)		// selected for automatic inline expansion
#pragma warning(disable : 4220)		// varargs matches remaining parameters
#pragma warning(disable : 4786)		//identifier was truncated

//rww (for vc.net, warning numbers changed apparently):
#pragma warning(disable : 4213)		//nonstandard extension used : cast on l-value
#pragma warning(disable : 4245)		//signed/unsigned mismatch

#pragma warning(disable : 4996)     //deprecated warning

// Disable warnings that now occur since we're using OpenJK --eez
#pragma warning(disable : 4710)		// function not inlined


// Time to enable some warnings that probably should be left in
//#pragma warning(2 : 4242)			// Return type does not match the variable type
#pragma warning(2 : 4062)			// Switch does not contain a default label when it should
#pragma warning(2 : 4191)			// Function pointer calling convention, args, or return type do not match assignment
#pragma warning(2 : 4287)			// Unsigned variable was compared against a signed value
#pragma warning(2 : 4289)			// Variable declared in for loop is used outside scope
#pragma warning(2 : 4296)			// Operator expression is always false
#pragma warning(2 : 4555)			// Expression with no effect
#pragma warning(2 : 4619)			// No warning exists for #pragma warning
#pragma warning(2 : 4710)			// Function was not inlined
#pragma warning(2 : 4711)			// Function was inlined


#endif

//
// ----------------------------------------------------------------------------------------
//

#endif //__JKG_OPTIONS__
