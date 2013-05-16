/**************************************************
* JKG In-Game Console Replacement
*
* Changes:
* - Console (gfx and text) fades in/out
* - Console shader slides down, not expands down
* - Scrollback characters pulsate
* - 'Native resolution' drop-shadow border
*
* Propositions:
* - Resizable with client-side command /con_height
* - Resizable by dragging the edge (double-click to reset)
* - Non-linear (curved) movement instead of linear
* - Mouse-wheel scrolling
* - Mouse-controlled scrollbars on the side
* - Text selection + Ctrl+C support
*   (Precise caret cursor would be needed,
*   plus text entry field activation/deactivation)
* - 
**************************************************/

//#include "q_shared.h"

void __cdecl JKG_Con_DrawSolidConsole(float frac)
{
	
}