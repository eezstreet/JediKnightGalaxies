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
// gl_console.c
// (c) 2013 Jedi Knight Galaxies

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