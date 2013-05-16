
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
//#define __MMO__									// UQ1: Sets compile to MMO mode. 1000 player support...
#define __NOT_MMO__									// UQ1: Disables stuff that does not work in MMO mode (TODO stuff)...
//#define __PHASE_2__								// UQ1: Stuff that needs to be fixed for phase 2...

// AI Stuff...
#define __NPC_MINPLAYERS__						// UQ1: Like bot_minplayers but for NPCs... For fun...
#define __DOMINANCE_NPC__						// UQ1: DominancE Advanced NPCs...
#define __AUTOWAYPOINT__						// UQ1: Auto-Waypointing System...
#define __TEMPORARY_HACK_FOR_BOT_LIGHTSABERS__  // UQ1: Just forces all bots to use a saber for testing/fun...

// Joystick support
#define __XBOX360CONTROLLER

// Generic Stuff...
#define __WEAPON_HOLSTER__						// UQ1: Weapon Holsterring System... (Under Construction)
//#define VEH_CONTROL_SCHEME_4					// Vehicle Control System...


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

#endif // _WIN32

//
// ----------------------------------------------------------------------------------------
//

#endif //__JKG_OPTIONS__
