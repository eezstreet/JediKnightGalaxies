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
// bg_vmove.c
// Vehicle version of PMove.
// File by eezstreet, (C) 2013 Jedi Knight Galaxies

// Just a note here, this was designed with speeders in mind, not ships.
// I will be upgrading this with proper ship/landspeeder support at a later date.

#include "q_shared.h"
#include "bg_public.h"
#include "bg_local.h"
#include "bg_strap.h"
#include "ghoul2/G2.h"

#ifdef QAGAME
#include "g_local.h" //ahahahahhahahaha@$!$!
#else
#include "../cgame/cg_local.h"
#endif

/*
===========================
Global variables and definitions
===========================
*/

vmove_t *vm;
static bgEntity_t *vm_entSelf = NULL;
static bgEntity_t *vm_entVehicle = NULL;
pml_t	vml;

/*
===========================
Declarations
===========================
*/

void VmoveSingle( vmove_t *vmove );
bgEntity_t *VM_BGEntForNum( int num );

/*
===============================================================
                         Functions
===============================================================
*/

/*
===========================
Vmove

Called instead of Pmove if we're dealing with a vehicle specifically.
This is designed to take the place of PMove entirely, and PMove should not
be using any sort of vehicle code whatsoever.
===========================
*/

void Vmove( vmove_t *vmove )
{
	qboolean locked = qfalse;
	int finalTime = vmove->cmd.serverTime;

	if ( finalTime < vmove->ps->commandTime ) {
		return;	// should not happen
	}

	if ( finalTime > vmove->ps->commandTime + 1000 ) {
		vmove->ps->commandTime = finalTime - 1000;
	}

	if (vmove->ps->fallingToDeath)
	{
		vmove->cmd.forwardmove = 0;
		vmove->cmd.rightmove = 0;
		vmove->cmd.upmove = 0;
		vmove->cmd.buttons = 0;
	}
	if (vmove->ps->pm_type == PM_NOMOVE) {
		vmove->cmd.forwardmove = 0;
		vmove->cmd.rightmove = 0;
		vmove->cmd.upmove = 0;
		vmove->cmd.buttons = 0;
		locked = qtrue;
		vmove->ps->pm_type = PM_NORMAL;		// Hack, i know, but this way we can still do the normal pmove stuff
	}

	vmove->ps->pmove_framecount = (vmove->ps->pmove_framecount+1) & ((1<<PS_PMOVEFRAMECOUNTBITS)-1);

	// chop the move up if it is too long, to prevent framerate
	// dependent behavior
	while ( vmove->ps->commandTime != finalTime ) {
		int		msec;

		msec = finalTime - vmove->ps->commandTime;

		// If its above 1000 we got a huge desync
		if ( msec > 66 && msec < 800 ) {
			msec = 66;
		}

		vmove->cmd.serverTime = vmove->ps->commandTime + msec;
		
		VmoveSingle( vmove );
	}

	if (locked) {
		vmove->ps->pm_type = PM_NOMOVE;		// Restore it
	}
}

/*
===========================
VmoveSingle

Processes a single frame of movement interaction for vehicles
===========================
*/

void VmoveSingle( vmove_t *vmove )
{
	vm = vmove;

	// Get our respective entities --eez
	vm_entSelf = VM_BGEntForNum( vmove->ps->clientNum );
	if(!vmove->ps->m_iVehicleNum)
	{
		assert(!"Attempted to use Vmove instead of Pmove.");
	}
	else
	{
		// what does do when we're a passenger...this will almost certainly not be correct --eez
		vm_entVehicle = VM_BGEntForNum( vmove->ps->m_iVehicleNum );
	}

	// First things first, we aren't allowed to jump --period--
	vm->cmd.upmove = 0;

	// set the talk balloon flag
	if ( vm->cmd.buttons & BUTTON_TALK ) {
		vm->ps->eFlags |= EF_TALK;
	} else {
		vm->ps->eFlags &= ~EF_TALK;
	}

	// clear all pmove local vars
	memset (&pml, 0, sizeof(pml));

	// determine the time
	vml.msec = vmove->cmd.serverTime - vm->ps->commandTime;
	if ( vml.msec < 1 ) {
		vml.msec = 1;
	} else if ( vml.msec > 200 ) {
		vml.msec = 200;
	}

	vm->ps->commandTime = vmove->cmd.serverTime;

	// save old org in case we get stuck
	VectorCopy (vm->ps->origin, vml.previous_origin);

	// save old velocity for crashlanding
	VectorCopy (vm->ps->velocity, vml.previous_velocity);

	vml.frametime = vml.msec * 0.001f;

	// handle viewangles
	AngleVectors (vm->ps->viewangles, vml.forward, vml.right, vml.up);


}

/*
===========================
VM_BGEntForNum

Returns a pointer to the bgEntity for the ent number in question
//rww - Get a pointer to the bgEntity by the index
===========================
*/

bgEntity_t *VM_BGEntForNum( int num )
{
	bgEntity_t *ent;

	if (!vm)
	{
		assert(!"You cannot call VM_BGEntForNum outside of vmove functions!");
		return NULL;
	}

	if (!pm->baseEnt)
	{
		assert(!"Base entity address not set");
		return NULL;
	}

	if (!pm->entSize)
	{
		assert(!"sizeof(ent) is 0, impossible (not set?)");
		return NULL;
	}

	assert(num >= 0 && num < MAX_GENTITIES);

    ent = (bgEntity_t *)((byte *)vm->baseEnt + vm->entSize*(num));

	return ent;
}