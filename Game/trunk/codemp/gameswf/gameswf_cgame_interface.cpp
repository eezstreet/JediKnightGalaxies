// gameswf_test_ogl.cpp	-- Thatcher Ulrich <tu@tulrich.com> 2003 -*- coding: utf-8;-*-

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// A minimal test player app for the gameswf library.

// TODO: UI --eez

//#include "dependencies/SDL/include/sdl.h"		// don't need
#include "gameswf.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#include "base/ogl.h"
#include "libraries/glew/include/GL/glew.h"
#include "base/utility.h"
#include "base/container.h"
#include "base/tu_file.h"
#include "base/tu_types.h"

extern "C"
{
	#include "../cgame/cg_local.h"
}

void	print_usage()
// Brief instructions.
{
	CG_Printf(
		"gameswf_test_ogl -- a test player for the gameswf library.\n"
		"\n"
		"This program has been donated to the Public Domain.\n"
		"See http://tulrich.com/geekstuff/gameswf.html for more info.\n"
		"\n"
		"usage: gameswf_test_ogl [options] movie_file.swf\n"
		"\n"
		"Plays a SWF (Shockwave Flash) movie, using OpenGL and the\n"
		"gameswf library.\n"
		"\n"
		"options:\n"
		"\n"
		"  -h          Print this info.\n"
		"  -s <factor> Scale the movie up/down by the specified factor\n"
		"  -a          Turn antialiasing on/off.  (obsolete)\n"
		"  -v          Be verbose; i.e. print log messages to stdout\n"
		"  -va         Be verbose about movie Actions\n"
		"  -vp         Be verbose about parsing the movie\n"
		"  -p          Run full speed (no sleep) and log frame rate\n"
		"\n"
		"keys:\n"
		"  CTRL-Q          Quit/Exit\n"
		"  CTRL-W          Quit/Exit\n"
		"  ESC             Quit/Exit\n"
		"  CTRL-P          Toggle Pause\n"
		"  CTRL-R          Restart the movie\n"
		"  CTRL-[ or -     Step back one frame\n"
		"  CTRL-] or +     Step forward one frame\n"
		"  CTRL-A          Toggle antialiasing (doesn't work)\n"
		"  CTRL-T          Debug.  Test the set_variable() function\n"
		"  CTRL-G          Debug.  Test the get_variable() function\n"
		"  CTRL-B          Toggle background color\n"
		);
}


#define OVERSIZE	1.0f


static float	s_scale = 1.0f;
static bool	s_antialiased = false;
static bool	s_verbose = false;
static bool	s_background = true;
static bool	s_measure_performance = false;


static void	message_log(const char* message)
// Process a log message.
{
	if (s_verbose)
	{
		fputs(message, stdout);
                fflush(stdout); // needed on osx for some reason
	}
}


static void	log_callback(bool error, const char* message)
// Error callback for handling gameswf messages.
{
	if (error)
	{
		// Log, and also print to stderr.
		message_log(message);
		fputs(message, stderr);
	}
	else
	{
		message_log(message);
	}
}


static tu_file*	file_opener(const char* url)
// Callback function.  This opens files for the gameswf library.
{
	// Made this work with the Q3 file system --eez
	fileHandle_t f;
	int len = trap_FS_FOpenFile(url, &f, FS_READ);
	char buffer[65535];
	if(len >= 65535)
	{
		//Com_Error(ERR_FATAL, "gameswf_cgame_interface.cpp: Buffer overrun (%i >= 65535)", len);
		return NULL;
	}
	else if(len <= 0)
	{
		//Com_Error(ERR_FATAL, "Attempted to load NULL SWF (%s)", url);
		return NULL;
	}
	trap_FS_Read(buffer, len, f);
	trap_FS_FCloseFile(f);
	return new tu_file(tu_file::memory_buffer_enum::memory_buffer, len, buffer);
}


static void	fs_callback(gameswf::movie_interface* movie, const char* command, const char* args)
// For handling notification callbacks from ActionScript.
{
	message_log("fs_callback: '");
	message_log(command);
	message_log("' '");
	message_log(args);
	message_log("'\n");
}

// TODO: FIXME. This should be using key events direct from the engine (trap calls are friends, not...er, nevermind) --eez
// Don't need in CGAME --eez
/*static void	key_event(SDLKey key, bool down)
// For forwarding SDL key events to gameswf.
{
	gameswf::key::code	c(gameswf::key::INVALID);

	if (key >= SDLK_a && key <= SDLK_z)
	{
		c = (gameswf::key::code) ((key - SDLK_a) + gameswf::key::A);
	}
	else if (key >= SDLK_F1 && key <= SDLK_F15)
	{
		c = (gameswf::key::code) ((key - SDLK_F1) + gameswf::key::F1);
	}
	else if (key >= SDLK_KP0 && key <= SDLK_KP9)
	{
		c = (gameswf::key::code) ((key - SDLK_KP0) + gameswf::key::KP_0);
	}
	else
	{
		// many keys don't correlate, so just use a look-up table.
		struct
		{
			SDLKey	sdlk;
			gameswf::key::code	gs;
		} table[] =
		{
			{ SDLK_LEFT, gameswf::key::LEFT },
			{ SDLK_UP, gameswf::key::UP },
			{ SDLK_RIGHT, gameswf::key::RIGHT },
			{ SDLK_DOWN, gameswf::key::DOWN },
			// @@ TODO fill this out some more
			{ SDLK_UNKNOWN, gameswf::key::INVALID }
		};

		for (int i = 0; table[i].sdlk != SDLK_UNKNOWN; i++)
		{
			if (key == table[i].sdlk)
			{
				c = table[i].gs;
				break;
			}
		}
	}

	if (c != gameswf::key::INVALID)
	{
		gameswf::notify_key_event(c, down);
	}
}*/


extern "C"
{
	//
	// UQ1: All these should be converted to arrays so multiple swf files can play on screen at once???
	// probably
	gameswf::movie_definition*	md = NULL;
	gameswf::movie_interface*	m = NULL;
	gameswf::render_handler*	render = NULL;
#ifdef __SDL_SOUND__ // UQ1: Ummm bass???
	gameswf::sound_handler*	sound = NULL;
#endif //__SDL_SOUND__

	int	width = 0;
	int	height = 0;

	// Mouse state.
	int		mouse_x = 0;
	int		mouse_y = 0;
	int		mouse_buttons = 0;

	bool	paused = false;
	float	speed_scale = 1.0f;
	Uint32	last_ticks = 0;
	int		frame_counter = 0;
	int		last_logged_fps = 0;

	//
	// UQ1: Call this code to begin playing an swf file...
	//
	void	gameswf_startswf(char *filename)
	{
		assert(tu_types_validate());

		const char* infile = filename;

		if (md && m) return; // UQ1: already playing one... FIXME: Add support for a list of instances...

		if (infile == NULL)
		{
			CG_Printf("no input file\n");
			print_usage();
			return;
		}

		//
		// UQ1: *BEGIN* Re-init all vars...
		//
		md = NULL;
		m = NULL;
		render = NULL;
#ifdef __SDL_SOUND__ // UQ1: Ummm bass???
		sound = NULL;
#endif //__SDL_SOUND__

		width = 0;
		height = 0;

		// Mouse state.
		mouse_x = 0;
		mouse_y = 0;
		mouse_buttons = 0;

		paused = false;
		speed_scale = 1.0f;
		last_ticks = 0;
		frame_counter = 0;
		last_logged_fps = 0;
		//
		// UQ1: *END* Re-init all vars...
		//

		gameswf::register_file_opener_callback(file_opener);
		gameswf::register_fscommand_callback(fs_callback);
		gameswf::register_log_callback(log_callback);
		//gameswf::set_antialiased(s_antialiased);

#ifdef __SDL_SOUND__ // UQ1: Ummm bass???
		sound = gameswf::create_sound_handler_sdl();
		gameswf::set_sound_handler(sound);
#endif //__SDL_SOUND__

		render = gameswf::create_render_handler_ogl();
		gameswf::set_render_handler(render); 

		// Get info about the width & height of the movie.
		int	movie_version = 0, movie_width = 0, movie_height = 0;
		
		gameswf::get_movie_info(infile, &movie_version, &movie_width, &movie_height, NULL, NULL);
		
		if (movie_version == 0)
		{
			CG_Printf("error: can't get info about %s\n", infile);
			return;
		}

		// Initialize the SDL subsystems we're using.
		//if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO /* | SDL_INIT_JOYSTICK | SDL_INIT_CDROM*/)) // UQ1: Original...
		/*if (SDL_Init(0)) // UQ1: Because I don't think we need any of those functions atm... In fact, I think VIDEO might screw up screen...
		{
			CG_Printf("Unable to init SDL: %s\n", SDL_GetError());
			return;
		}*/
		// SDL sux --eez

		//atexit(SDL_Quit);

/*
		//	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
		//	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
		//	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
		//	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 5);
		//	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1);
	*/

		width = int(movie_width * s_scale);
		height = int(movie_height * s_scale);

		// Turn on alpha blending.
		// Moot point. Engine handles all of the rendering now. --eez
		/*glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Turn on line smoothing.  Antialiased lines can be used to
		// smooth the outsides of shapes.
		glEnable(GL_LINE_SMOOTH);
		glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);	// GL_NICEST, GL_FASTEST, GL_DONT_CARE

		glMatrixMode(GL_PROJECTION);
		glOrtho(-OVERSIZE, OVERSIZE, OVERSIZE, -OVERSIZE, -1, 1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();*/

		// Load the actual movie.
		md = gameswf::create_movie(infile);
		if (md == NULL)
		{
			CG_Printf("error: can't create a movie from '%s'\n", infile);
			return;
		}
		m = md->create_instance();
		if (m == NULL)
		{
			CG_Printf("error: can't create movie instance\n");
			return;
		}

		// Mouse state.
		mouse_x = 0;
		mouse_y = 0;
		mouse_buttons = 0;

		paused = false;
		speed_scale = 1.0f;
//		last_ticks = SDL_GetTicks();
		frame_counter = 0;
		last_logged_fps = last_ticks;
	}

	//
	// UQ1: Call this code to finish playing an swf file and clean up... (actually it should be called by the display porcedure below anyway)
	//
	void	gameswf_finishswf()
	{
		assert(tu_types_validate());

		if (md) md->drop_ref();
		if (m) m->drop_ref();

#ifdef __SDL_SOUND__ // UQ1: Ummm bass???
		delete sound;
#endif //__SDL_SOUND__
		delete render;

		// For testing purposes, throw some keypresses into gameswf,
		// to make sure the key handler is properly using weak
		// references to listeners.
		gameswf::notify_key_event(gameswf::key::A, true);
		gameswf::notify_key_event(gameswf::key::B, true);
		gameswf::notify_key_event(gameswf::key::C, true);

		// Clean up gameswf as much as possible, so valgrind will help find actual leaks.
		gameswf::clear();

		md = NULL;
		m = NULL;
	}

	//
	// UQ1: Call this each cgame frame to continue playing the swf file...
	//
	void	gameswf_continueswf()
	{
		
		/*	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
			SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
			SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
			SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 5);
			SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1);
		

		// Turn on alpha blending.
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Turn on line smoothing.  Antialiased lines can be used to
		// smooth the outsides of shapes.
		glEnable(GL_LINE_SMOOTH);
		glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);	// GL_NICEST, GL_FASTEST, GL_DONT_CARE

		glMatrixMode(GL_PROJECTION);
		glOrtho(-OVERSIZE, OVERSIZE, OVERSIZE, -OVERSIZE, -1, 1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		// This is the old loop...
		Uint32	ticks = SDL_GetTicks();
		int	delta_ticks = ticks - last_ticks;
		float	delta_t = delta_ticks / 1000.f;
		last_ticks = ticks;

		if (paused == true)
		{
			delta_t = 0.0f;
		}

		// Handle input.
		SDL_Event	event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_KEYDOWN:
				{
					SDLKey	key = event.key.keysym.sym;
					bool	ctrl = (event.key.keysym.mod & KMOD_CTRL) != 0;

					if (key == SDLK_ESCAPE
						|| (ctrl && key == SDLK_q)
						|| (ctrl && key == SDLK_w))
					{
						gameswf_finishswf();
						return;
					}
					else if (ctrl && key == SDLK_p)
					{
						// Toggle paused state.
						paused = ! paused;
						CG_Printf("paused = %d\n", int(paused));
					}
					else if (ctrl && key == SDLK_r)
					{
						// Restart the movie.
						m->restart();
					}
					else if (ctrl && (key == SDLK_LEFTBRACKET || key == SDLK_KP_MINUS))
					{
						paused = true;
						//delta_t = -0.1f;
						m->goto_frame(m->get_current_frame()-1);
					}
					else if (ctrl && (key == SDLK_RIGHTBRACKET || key == SDLK_KP_PLUS))
					{
						paused = true;
						//delta_t = +0.1f;
						m->goto_frame(m->get_current_frame()+1);
					}
					else if (ctrl && key == SDLK_a)
					{
						// Toggle antialiasing.
						s_antialiased = !s_antialiased;
						//gameswf::set_antialiased(s_antialiased);
					}
					else if (ctrl && key == SDLK_t)
					{
						// test text replacement / variable setting:
						m->set_variable("test.text", "set_edit_text was here...\nanother line of text for you to see in the text box\nSome UTF-8: ñö£ç°ÄÀÔ¿");
					}
					else if (ctrl && key == SDLK_g)
					{
						// test get_variable.
						message_log("testing get_variable: '");
						message_log(m->get_variable("test.text"));
						message_log("'\n");
					}
					else if (ctrl && key == SDLK_b)
					{
						// toggle background color.
						s_background = !s_background;
					}

					key_event(key, true);

					break;
				}

			case SDL_KEYUP:
				{
					SDLKey	key = event.key.keysym.sym;
					key_event(key, false);
					break;
				}

			case SDL_MOUSEMOTION:
				mouse_x = (int) (event.motion.x / s_scale);
				mouse_y = (int) (event.motion.y / s_scale);
				break;

			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				{
					int	mask = 1 << (event.button.button);
					if (event.button.state == SDL_PRESSED)
					{
						mouse_buttons |= mask;
					}
					else
					{
						mouse_buttons &= ~mask;
					}
					break;
				}

			case SDL_QUIT:
				gameswf_finishswf();
				return;

			default:
				break;
			}
		}*/

		m->set_display_viewport(0, 0, width, height);
		m->set_background_alpha(s_background ? 1.0f : 0.05f);

		m->notify_mouse_state(mouse_x, mouse_y, mouse_buttons);

		m->advance(cg.time-cg.oldTime);

		//glDisable(GL_DEPTH_TEST);	// Disable depth testing.
		//glDrawBuffer(GL_BACK);

		m->display();
		frame_counter++;

		//SDL_GL_SwapBuffers();

		/*if (s_measure_performance == false)
		{
			// Don't hog the CPU.
			SDL_Delay(10);
		}
		else*/
		{
			// Log the frame rate every second or so.
			if (last_ticks - last_logged_fps > 1000)
			{
				float	delta = (last_ticks - last_logged_fps) / 1000.f;

				if (delta > 0)
				{
					CG_Printf("fps = %3.1f\n", frame_counter / delta);
				}
				else
				{
					CG_Printf("fps = *inf*\n");
				}

				last_logged_fps = last_ticks;
				frame_counter = 0;
			}
		}
	
		// TODO: rewrite renderer so this actually works
		// this will likely run just like a texture in cgame and function like a 
		// videomap / ROQ would. Just have to make it not eat performance hardcore.

		// Lemme walk through the steps involved:
		// 1. tr_shader.c hook: ParseStage
		// ParseStage needs to have a "swfMap" phase. Should act near identical in concept, with one
		// minor exception: it sets bundle->isVideoMap to -1.
		// 2. tr_shade.c hook: R_BindAnimatedImage
		// Before the first check: check bundle->isVideoMap with -1. If so, do the render.
		return;
	}

	int gameswf_swfplaying()
	{
		if (md && m) return 1;

		return 0;
	}
}
