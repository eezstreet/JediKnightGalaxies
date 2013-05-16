//       ____.___________________  .___           ____  __._______  .___  ________  ___ ______________
//      |    |\_   _____/\______ \ |   |         |    |/ _|\      \ |   |/  _____/ /   |   \__    ___/
//      |    | |    __)_  |    |  \|   |         |      <  /   |   \|   /   \  ___/    ~    \|    |   
//  /\__|    | |        \ |    `   \   |         |    |  \/    |    \   \    \_\  \    Y    /|    |   
//  \________|/_______  //_______  /___|         |____|__ \____|__  /___|\______  /\___|_  / |____|   
//                    \/         \/                      \/       \/            \/       \/           
//                         ________    _____  .____       _____  ____  ___.______________ _________   
//                        /  _____/   /  _  \ |    |     /  _  \ \   \/  /|   \_   _____//   _____/   
//                       /   \  ___  /  /_\  \|    |    /  /_\  \ \     / |   ||    __)_ \_____  \    
//                       \    \_\  \/    |    \    |___/    |    \/     \ |   ||        \/        \   
//                        \______  /\____|__  /_______ \____|__  /___/\  \|___/_______  /_______  /   
//                               \/         \/        \/	   \/	   \_/			  \/        \/ (c)
// jkg_joystick.c
// File by eezstreet
// with APIs by Microsoft
// (c) 2013 Jedi Knight Galaxies

#include "cg_local.h"


// Assorted Engine Hacks
// Trying to keep this file as organized as possible. --eez

// FIXME: these are cdecl, so mention it!
static void (*IN_JoyMove)(void) = (void (*)(void))0x0044F5A0;		// JA's default IN_JoyMove. We use a proxied version because ours is always better! (tm)
static void (*Sys_QueEvent)(sysEventType_t type, int value, int value2, int ptrLength, void *ptr) =
		( void (*)(sysEventType_t, int, int, int, void *) )0x00450AC0;
static void (*CL_JoystickMove)(usercmd_t *cmd) = (void (*)(usercmd_t *))0x00419880;		// crap version of the function, in the engine
static void (*_CL_AddReliableCommand)(void) = (void (*)(void))0x41C6F0;
static void (*_Cbuf_ExecuteText)(void) = (void (*)(void))0x436780;

// "because DOLPHINS" ~Raz0r
cvar_t __declspec(naked) * Cvar_Get(const char *cvarname, const char *value, int flags) {
	(void)cvarname;
	(void)value;
	(void)flags;
	__asm
	{
		push 0x439470
		ret
	}
}

// Send commands thru input so it's more reliable --eez
void CL_AddReliableCommand(const char *command)
{
	__asm
	{
		pushad
		pushfd
		mov edi,command
		call _CL_AddReliableCommand
		popfd
		popad
	}
}

void Cbuf_ExecuteText(int when, const char *text)
{
	__asm
	{
		pushad
		pushfd
		mov eax, when
		mov ecx, text
		call _Cbuf_ExecuteText
		popfd
		popad
	}
}

// ===========================
// Standard Definitions, which are to be shared across controller archetypes
// ===========================

enum {
	JOY_NONE,
	JOY_MISC,
#ifdef __XBOX360CONTROLLER
	JOY_XBOX360,
#endif // __XBOX360CONTROLLER
#ifdef __PS3CONTROLLER_DUALSHOCK
	JOY_DUALSHOCK,
#endif // __PS3CONTROLLER_DUALSHOCK
#ifdef __PS3CONTROLLER_SIXAXIS
	JOY_SIXAXIS,
#endif
#ifdef __WINGMANJOYSTICK
	JOY_WINGMAN,
#endif
#ifdef __WIIMOTE
	JOY_WIIMOTE,
#endif // __WIIMOTE
} joystickAPI_e;

// ===========================================================================================================================
//									   __  __  _____  _____  __  __    _____  ____   _____ 
//									  /  \/  \/  _  \/  _  \/  \/  \  /  _  \/  __| /  _  \
//									  >-    -<|  _  <|  |  |>-    -<  >-<_  <|  _  \|  |  |
//									  \__/\__/\_____/\_____/\__/\__/  \_____/\_____/\_____/
//		           
// ===========================================================================================================================

// ===========================================================================================================================
#define __XBOX360CONTROLLER
#ifdef __XBOX360CONTROLLER
// ===========================================================================================================================

// Definitions, includes, and library inclusions
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <XInput.h>
#pragma comment(lib, "XInput.lib")

// States
static XINPUT_STATE state;
static XINPUT_VIBRATION XBOX_rumble;

// Stuff pertaining to buttons
static int lastDPad;
static int lastBButton;
static qboolean crouchToggle;

// Rumble
static int rumbleTime;
static int rumbleStartTime;
static int rumbleIntensity;

// ===========================================================================================================================

void XboxUpdate(void)
{
	// This acts as a portal for traffic to flow through.
	// Vibration goes into the XBOX controller through this function, and we send commands to the game
	// from input received in the controller itself.
	DWORD dwResult;

	ZeroMemory( &state, sizeof(XINPUT_STATE) );
	dwResult = XInputGetState(0, &state); //Get the XInput state

	if(dwResult != ERROR_SUCCESS)
	{
		// Bye.
		CG_CenterPrint( "Please connect your XBOX 360 controller to your USB drive.", SCREEN_HEIGHT * 0.30, BIGCHAR_WIDTH );
		return;
	}

	// Send the events about our thumbsticks to the game. (Ships: use triggers for roll?)

	// Left Sticks control movement
	Sys_QueEvent(SE_JOYSTICK_AXIS, AXIS_SIDE, state.Gamepad.sThumbLX, 0, NULL);
	Sys_QueEvent(SE_JOYSTICK_AXIS, AXIS_FORWARD, state.Gamepad.sThumbLY, 0, NULL);

	// Right Sticks control angles
	Sys_QueEvent(SE_JOYSTICK_AXIS, AXIS_YAW, state.Gamepad.sThumbRX, 0, NULL);
	Sys_QueEvent(SE_JOYSTICK_AXIS, AXIS_PITCH, state.Gamepad.sThumbRY, 0, NULL);

	// Now handle the vibration stuff
	if( (rumbleStartTime + rumbleTime) < cg.time )
	{
		ZeroMemory( &XBOX_rumble, sizeof( XINPUT_VIBRATION ) );
		XInputSetState(0, &XBOX_rumble);
	}
}

void XboxInit(void)
{
	// Called whenever the latched cvar in_joystick changes on the cgame end.
	// Controllers of all kinds cannot be used for UI, period.
	lastDPad = 0;
	lastBButton = 0;
	crouchToggle = qfalse;

	ZeroMemory( &XBOX_rumble, sizeof( XINPUT_VIBRATION ) );
}

void XboxMovement(usercmd_t *cmd)
{
	// Handles all input of the client's XBOX 360 controller.
	int *joystickAxis = (int *)0x97D6C0;
	float rightratio = joystickAxis[AXIS_SIDE] / 32767.0f;
	float fwdratio = joystickAxis[AXIS_FORWARD] / 32767.0f;
	float pitchratio = joystickAxis[AXIS_PITCH] / 32767.0f;
	float yawratio = joystickAxis[AXIS_YAW] / 32767.0f;
	cvar_t *cl_yawspeed = Cvar_Get("cl_yawspeed", "140", 0);

	float *ptrPitch = (float *)0x97DF88;
	float *ptrYaw = (float *)0x97DF8C;

	// Invert controls for R stick with certain cvars --eez
	if(in_invertYLook.integer) pitchratio *= -1.0f;
	if(in_invertXLook.integer) yawratio *= -1.0f;

	if(fabs(rightratio) > 0.3)
	{
		// WIP deadzone
		cmd->rightmove = ClampChar(rightratio * 128);
	}

	if( fabs(fwdratio) > 0.3 )
	{
		cmd->forwardmove = ClampChar(fwdratio * 128);
	}

	if( crouchToggle )
	{
		cmd->upmove = -128;
	}

	// so going down is not faster than going up, and same with l/r
	if( fabs(pitchratio) > 0.25 )
	{
		if(pitchratio < 0)
		{
			pitchratio += 0.1f;
			pitchratio *= -pitchratio;
		}
		else
		{
			pitchratio -= 0.1f;
			pitchratio *= pitchratio;
		}
		CLAMP(pitchratio, -1.0f, 1.0f);

		*ptrPitch -= (cl_yawspeed->value*pitchratio)*0.01;
	}

	if( fabs(yawratio) > 0.25 )
	{
		if(yawratio < 0)
		{
			yawratio += 0.1f;
			yawratio *= -yawratio;
		}
		else
		{
			yawratio -= 0.1f;
			yawratio *= yawratio;
		}
		CLAMP(yawratio, -1.0f, 1.0f);

		*ptrYaw -= (cl_yawspeed->value*yawratio)*0.01;
	}

	// Now here's something that I absolutely DREAD doing..
	// Have to make a few presets based on what I like. JA doesn't allow for controls on this one :(

	if( in_controlScheme.integer == 0 )
	{
		// AKA generic FPS

		// --General--
		// Select = Open Inventory
		// A button = Jump/Force Jump
		// B button = toggle crouch (!!)
		// L/R D-Pad = select guns in ACI
		// L3 = sprint
		// Y button = use

		// -- Guns --
		// Left Trigger = ADS
		// Right Trigger = Shoot
		// R3 = Change Firing Mode
		// X button = reload

		// -- Sabers --
		// Left Trigger = Blocking Mode
		// Right Trigger = Attack
		// Left Trigger + Right Trigger = Projectile Blocking Mode
		// R3 = Change saber style
		// X button = kick/melee

		// -- No Use --
		// Left Bumper = Reserved
		// Right Bumper = Reserved
		// Up/Down D-Pad = Reserved
		// Start = Reserved

		// -- Sabers 
		// 
		if( state.Gamepad.wButtons & XINPUT_GAMEPAD_A )
		{
			// A button = Jump
			cmd->upmove += 127;
		}
		
		if( state.Gamepad.wButtons & XINPUT_GAMEPAD_Y )
		{
			cmd->buttons |= BUTTON_USE;
		}

		if( state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB )
		{
			cmd->buttons |= BUTTON_SPRINT;
		}

		if( state.Gamepad.bRightTrigger )
		{
			cmd->buttons |= BUTTON_ATTACK;
		}

		if( state.Gamepad.bLeftTrigger )
		{
			cmd->buttons |= BUTTON_IRONSIGHTS;
		}

		if( state.Gamepad.wButtons & XINPUT_GAMEPAD_X )
		{
			CL_AddReliableCommand("reload");
		}

		if( state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB )
		{
			Cbuf_ExecuteText(0, "saberAttackCycle");
		}

		if( cg.time > lastDPad )
		{
			if( state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT )
			{
				Cbuf_ExecuteText(0, "weapprev");
				lastDPad = cg.time + 250;
			}
			else if( state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT )
			{
				Cbuf_ExecuteText(0, "weapnext");
				lastDPad = cg.time + 250;
			}
		}

		if( cg.time > lastBButton )
		{
			if( state.Gamepad.wButtons & XINPUT_GAMEPAD_B )
			{
				crouchToggle = !crouchToggle;
				lastBButton = cg.time + 200;
			}
		}
	}

}

// Specific rumble handler for this controller
void Xbox_DoRumble( int duration, int intensity )
{
	CAP(intensity, 65535);

	rumbleStartTime = cg.time;
	rumbleTime = duration;
	rumbleIntensity = intensity;

	XBOX_rumble.wLeftMotorSpeed = intensity;
	XBOX_rumble.wRightMotorSpeed = intensity;

	XInputSetState(0, &XBOX_rumble);
}

// ===========================================================================================================================
#endif // __XBOX360CONTROLLER
// ===========================================================================================================================

// ===========================================================================================================================
// Sony Playstation 3 -- "Dualshock 3" controller
#ifdef __PS3CONTROLLER_DUALSHOCK
// I don't have one of these yet. Will add support at some point --eez
#endif // __PS3CONTROLLER_DUALSHOCK

// Sony Playstation 3 -- "Sixaxis" controller
#ifdef __PS3CONTROLLER_SIXAXIS
#endif // __PS3CONTROLLER_SIXAXIS

// Logitech "Wingman" series joystick
#ifdef __WINGMANJOYSTICK
#endif // __WINGMANJOYSTICK

// Nintendo Wii "Wiimote"
#ifdef __WIIMOTE
#endif // __WIIMOTE
// ===========================================================================================================================

void __cdecl JKG_ControllerUpdate(void)
{
	cvar_t *in_joystick = Cvar_Get("in_joystick", "0", 0);
	int joyNum = in_joystick->integer;
	// Calls the various things for joystick support, based on in_joystick

	if(joyNum != cg.last_joy)
	{
		// Do an init function for each thingie
		cg.last_joy = joyNum;
		switch( joyNum )
		{
#ifdef __XBOX360CONTROLLER
			case JOY_XBOX360:
				XboxInit();
				break;
#endif // __XBOX360CONTROLLER
#ifdef __PS3CONTROLLER_DUALSHOCK
			case JOY_DUALSHOCK:
				break;
#endif // __PS3CONTROLLER_DUALSHOCK
#ifdef __PS3CONTROLLER_SIXAXIS
			case JOY_SIXAXIS:
				break;
#endif // __PS3CONTROLLER_SIXAXIS
#ifdef __WINGMANJOYSTICK
			case JOY_WINGMAN:
				break;
#endif // __WINGMANJOYSTICK
#ifdef __WIIMOTE
			case JOY_WIIMOTE:
				break;
#endif // __WIIMOTE
		}
	}

	if(joyNum == 0 || joyNum == 1)
	{
		// Use JA default. Useful if you're a masochist, I guess. --eez
		IN_JoyMove();
		return;
	}
	else
	{
		switch( joyNum )
		{
#ifdef __XBOX360CONTROLLER
			case JOY_XBOX360:
				XboxUpdate();
				break;
#endif // __XBOX360CONTROLLER
#ifdef __PS3CONTROLLER_DUALSHOCK
			case JOY_DUALSHOCK:
				break;
#endif // __PS3CONTROLLER_DUALSHOCK
#ifdef __PS3CONTROLLER_SIXAXIS
			case JOY_SIXAXIS:
				break;
#endif // __PS3CONTROLLER_SIXAXIS
#ifdef __WINGMANJOYSTICK
			case JOY_WINGMAN:
				break;
#endif // __WINGMANJOYSTICK
#ifdef __WIIMOTE
			case JOY_WIIMOTE:
				break;
#endif // __WIIMOTE
		}
	}
}

void __cdecl JKG_CL_JoystickMovement( usercmd_t *cmd ) // you can't hook like this, is what i tried to write. so i need to intercept the address then, or use a naked func, or? naked func with asm bridge :p k lemme write one which should work
{
	cvar_t *in_joystick = Cvar_Get("in_joystick", "0", 0);
	int joyNum = in_joystick->integer;

	switch(joyNum)
	{
#ifdef __XBOX360CONTROLLER
			case JOY_XBOX360:
				XboxMovement(cmd);
				break;
#endif // __XBOX360CONTROLLER
#ifdef __PS3CONTROLLER_DUALSHOCK
			case JOY_DUALSHOCK:
				break;
#endif // __PS3CONTROLLER_DUALSHOCK
#ifdef __PS3CONTROLLER_SIXAXIS
			case JOY_SIXAXIS:
				break;
#endif // __PS3CONTROLLER_SIXAXIS
#ifdef __WINGMANJOYSTICK
			case JOY_WINGMAN:
				break;
#endif // __WINGMANJOYSTICK
#ifdef __WIIMOTE
			case JOY_WIIMOTE:
				break;
#endif // __WIIMOTE
			default:
				// Use the original version of the function
				CL_JoystickMove(cmd);
				break;
	}
}

void __declspec(naked) _Hook_CL_JoystickMovement()
{
	__asm
	{
		add esp,4 
		pushad
		pushfd
		push ecx
		call JKG_CL_JoystickMovement
		add esp, 4
		popfd
		popad
		mov esi, 0x0041A1C7//push 0x0041A1C7
		jmp esi
	}
}

void JKG_DoControllerRumble( int duration, int intensity )
{
	cvar_t *in_joystick = Cvar_Get("in_joystick", "0", 0);

	duration *= in_rumbleIntensity.value;
	intensity *= in_rumbleIntensity.value;

	switch(in_joystick->integer)
	{
#ifdef __XBOX360CONTROLLER
			case JOY_XBOX360:
				Xbox_DoRumble(duration, intensity);
				break;
#endif // __XBOX360CONTROLLER
#ifdef __PS3CONTROLLER_DUALSHOCK
			case JOY_DUALSHOCK:
				break;
#endif // __PS3CONTROLLER_DUALSHOCK
#ifdef __PS3CONTROLLER_SIXAXIS
			case JOY_SIXAXIS:
				break;
#endif // __PS3CONTROLLER_SIXAXIS
#ifdef __WINGMANJOYSTICK
			case JOY_WINGMAN:
				break;
#endif // __WINGMANJOYSTICK
#ifdef __WIIMOTE
			case JOY_WIIMOTE:
				break;
#endif // __WIIMOTE
	}
}