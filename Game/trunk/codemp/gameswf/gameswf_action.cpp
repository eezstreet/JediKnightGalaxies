// gameswf_action.cpp	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Implementation and helpers for SWF actions.


#include "gameswf_action.h"
#include "gameswf_impl.h"
#include "gameswf_log.h"
#include "gameswf_stream.h"
#include "base/tu_random.h"

#include <stdio.h>


#ifdef _WIN32
#define snprintf _snprintf
#endif // _WIN32



// NOTES:
//
// Buttons
// on (press)                 onPress
// on (release)               onRelease
// on (releaseOutside)        onReleaseOutside
// on (rollOver)              onRollOver
// on (rollOut)               onRollOut
// on (dragOver)              onDragOver
// on (dragOut)               onDragOut
// on (keyPress"...")         onKeyDown, onKeyUp      <----- IMPORTANT
//
// Sprites
// onClipEvent (load)         onLoad
// onClipEvent (unload)       onUnload                Hm.
// onClipEvent (enterFrame)   onEnterFrame
// onClipEvent (mouseDown)    onMouseDown
// onClipEvent (mouseUp)      onMouseUp
// onClipEvent (mouseMove)    onMouseMove
// onClipEvent (keyDown)      onKeyDown
// onClipEvent (keyUp)        onKeyUp
// onClipEvent (data)         onData

// Text fields have event handlers too!

// Sprite built in methods:
// play()
// stop()
// gotoAndStop()
// gotoAndPlay()
// nextFrame()
// startDrag()
// getURL()
// getBytesLoaded()
// getBytesTotal()

// Built-in functions: (do these actually exist in the VM, or are they just opcodes?)
// Number()
// String()


namespace gameswf
{
	//
	// action stuff
	//


	void	action_init();

	// Statics.
	bool	s_inited = false;
	stringi_hash<as_value>	s_built_ins;

	fscommand_callback	s_fscommand_handler = NULL;


	void	register_fscommand_callback(fscommand_callback handler)
	// External interface.
	{
		s_fscommand_handler = handler;
	}



	//
	// as_as_function
	//

	void	as_as_function::operator()(
		as_value* result,
		as_object_interface* this_ptr,
		as_environment* caller_env,
		int nargs,
		int first_arg)
	// Dispatch.
	{
		as_environment*	our_env = m_env;
		if (our_env == NULL)
		{
			our_env = caller_env;
		}
		assert(our_env);

		// Set up local stack frame, for parameters and locals.
		int	local_stack_top = our_env->get_local_frame_top();
		our_env->add_frame_barrier();

		// Push the arguments onto the local frame.
		int	args_to_pass = imin(nargs, m_args.size());
		for (int i = 0; i < args_to_pass; i++)
		{
			our_env->add_local(m_args[i], caller_env->bottom(first_arg - i));
		}

		// Execute the actions.
		m_action_buffer->execute(our_env, m_start_pc, m_length, result, m_with_stack);

		// Clean up stack frame.
		our_env->set_local_frame_top(local_stack_top);
	}


	//
	// Function/method dispatch.
	//


	as_value	call_method(
		const as_value& method,
		as_environment* env,
		as_object_interface* this_ptr,
		int nargs,
		int first_arg_bottom_index)
	// first_arg_bottom_index is the stack index, from the bottom, of the first argument.
	// Subsequent arguments are at *lower* indices.  E.g. if first_arg_bottom_index = 7,
	// then arg1 is at env->bottom(7), arg2 is at env->bottom(6), etc.
	{
		as_value	val;

		as_c_function_ptr	func = method.to_c_function();
		if (func)
		{
			// It's a C function.  Call it.
			(*func)(&val, this_ptr, env, nargs, first_arg_bottom_index);
		}
		else if (as_as_function* as_func = method.to_as_function())
		{
			// It's an ActionScript function.  Call it.
			(*as_func)(&val, this_ptr, env, nargs, first_arg_bottom_index);
		}
		else
		{
			log_error("error in call_method(): method is not a function\n");
		}

		return val;
	}


	as_value	call_method0(
		const as_value& method,
		as_environment* env,
		as_object_interface* this_ptr)
	{
		return call_method(method, env, this_ptr, 0, env->get_top_index() + 1);
	}
		
	const char*	call_method_parsed(as_environment* env, as_object_interface* this_ptr, const char* method_call)
	// Big fat slow stringified interface for calling ActionScript.
	// Handy for external binding.
	{
		static const int	BUFSIZE = 1000;
		char	buffer[BUFSIZE];
		array<const char*>	tokens;

		// Brutal crap parsing.  Basically null out any
		// delimiter characters, so that the method name and
		// args sit in the buffer as null-terminated C
		// strings.  Leave an intial ' character as the first
		// char in a string argument.
		// Don't verify parens or matching quotes or anything.
		{
			strncpy(buffer, method_call, BUFSIZE);
			buffer[BUFSIZE - 1] = 0;
			char*	p = buffer;

			char	in_quote = 0;
			bool	in_arg = false;
			for (;; p++)
			{
				char	c = *p;
				if (c == 0)
				{
					// End of string.
					break;
				}
				else if (c == in_quote)
				{
					// End of quotation.
					assert(in_arg);
					*p = 0;
					in_quote = 0;
					in_arg = false;
				}
				else if (in_arg)
				{
					if (in_quote == 0)
					{
						if (c == ')' || c == '(' || c == ',' || c == ' ')
						{
							// End of arg.
							*p = 0;
							in_arg = false;
						}
					}
				}
				else
				{
					// Not in arg.  Watch for start of arg.
					assert(in_quote == 0);
					if (c == '\'' || c == '\"')
					{
						// Start of quote.
						in_quote = c;
						in_arg = true;
						*p = '\'';	// ' at the start of the arg, so later we know this is a string.
						tokens.push_back(p);
					}
					else if (c == ' ' || c == ',')
					{
						// Non-arg junk, null it out.
						*p = 0;
					}
					else
					{
						// Must be the start of a numeric arg.
						in_arg = true;
						tokens.push_back(p);
					}
				}
			}
		}

		array<with_stack_entry>	dummy_with_stack;
		as_value	method = env->get_variable(tokens[0], dummy_with_stack);

		// check method

		// Push args onto env stack (reverse order!).
		int	nargs = tokens.size() - 1;
		for (int i = nargs; i > 0; i--)
		{
			const char*	arg = tokens[i];
			if (arg[0] == '\'')
			{
				// String arg.
				env->push(arg + 1);
			}
			else
			{
				// Arg must be numeric.
				env->push(atof(arg));
			}
		}

		// Do the call.
		as_value	result = call_method(method, env, this_ptr, nargs, env->get_top_index());
		env->drop(nargs);

		// Return pointer to static string for return value.
		static tu_string	s_retval;
		s_retval = result.to_tu_string();
		return s_retval.c_str();
	}


	const char*	call_method_parsed(as_environment* env, as_object_interface* this_ptr, const wchar_t* method_call)
	// Big fat slow stringified interface for calling ActionScript.
	// Handy for external binding.
	{
		tu_string	utf8_method_call;
		tu_string::encode_utf8_from_wchar(&utf8_method_call, method_call);

		return call_method_parsed(env, this_ptr, utf8_method_call.c_str());
	}


	//
	// Built-in objects
	//


	//
	// math object
	//


	// One-argument simple functions.
	#define MATH_WRAP_FUNC1(funcname)											\
	void	math_##funcname(as_value* result, as_object_interface* this_ptr, as_environment* env, int nargs, int first_arg_bottom_index)	\
	{															\
		double	arg = env->bottom(first_arg_bottom_index).to_number();							\
		result->set(funcname(arg));											\
	}

	MATH_WRAP_FUNC1(fabs);
	MATH_WRAP_FUNC1(acos);
	MATH_WRAP_FUNC1(asin);
	MATH_WRAP_FUNC1(atan);
	MATH_WRAP_FUNC1(ceil);
	MATH_WRAP_FUNC1(cos);
	MATH_WRAP_FUNC1(exp);
	MATH_WRAP_FUNC1(floor);
	MATH_WRAP_FUNC1(log);
	MATH_WRAP_FUNC1(sin);
	MATH_WRAP_FUNC1(sqrt);
	MATH_WRAP_FUNC1(tan);

	// Two-argument functions.
	#define MATH_WRAP_FUNC2_EXP(funcname, expr)										\
	void	math_##funcname(as_value* result, as_object_interface* this_ptr, as_environment* env, int nargs, int first_arg_bottom_index)	\
	{															\
		double	arg0 = env->bottom(first_arg_bottom_index).to_number();							\
		double	arg1 = env->bottom(first_arg_bottom_index - 1).to_number();						\
		result->set(expr);												\
	}
	MATH_WRAP_FUNC2_EXP(atan2, (atan2(arg0, arg1)));
	MATH_WRAP_FUNC2_EXP(max, (arg0 > arg1 ? arg0 : arg1));
	MATH_WRAP_FUNC2_EXP(min, (arg0 < arg1 ? arg0 : arg1));
	MATH_WRAP_FUNC2_EXP(pow, (pow(arg0, arg1)));

	// A couple of oddballs.
	void	math_random(as_value* result, as_object_interface* this_ptr, as_environment* env, int nargs, int first_arg_bottom_index)
	{
		// Random number between 0 and 1.
		result->set(tu_random::next_random() / double(Uint32(0x0FFFFFFFF)));
	}
	void	math_round(as_value* result, as_object_interface* this_ptr, as_environment* env, int nargs, int first_arg_bottom_index)
	{
		// round argument to nearest int.
		double	arg0 = env->bottom(first_arg_bottom_index).to_number();
		result->set(floor(arg0 + 0.5));
	}

	void math_init()
	{
		// Create built-in math object.
		as_object*	math_obj = new as_object;

		// constants
		math_obj->set_member("e", 2.7182818284590452354);
		math_obj->set_member("ln2", 0.69314718055994530942);
		math_obj->set_member("log2e", 1.4426950408889634074);
		math_obj->set_member("ln10", 2.30258509299404568402);
		math_obj->set_member("log10e", 0.43429448190325182765);
		math_obj->set_member("pi", 3.14159265358979323846);
		math_obj->set_member("sqrt1_2", 0.7071067811865475244);
		math_obj->set_member("sqrt2", 1.4142135623730950488);

		// math methods
		math_obj->set_member("abs", &math_fabs);
		math_obj->set_member("acos", &math_acos);
		math_obj->set_member("asin", &math_asin);
		math_obj->set_member("atan", &math_atan);
		math_obj->set_member("ceil", &math_ceil);
		math_obj->set_member("cos", &math_cos);
		math_obj->set_member("exp", &math_exp);
		math_obj->set_member("floor", &math_floor);
		math_obj->set_member("log", &math_log);
		math_obj->set_member("random", &math_random);
		math_obj->set_member("round", &math_log);
		math_obj->set_member("sin", &math_sin);
		math_obj->set_member("sqrt", &math_sqrt);
		math_obj->set_member("tan", &math_tan);

		s_built_ins.add("math", math_obj);
	}


	//
	// key object
	//


	struct key_as_object : public as_object
	{
		Uint8	m_keymap[key::KEYCOUNT / 8 + 1];	// bit-array
		array<weak_ptr<as_object_interface> >	m_listeners;
		int	m_last_key_pressed;

		key_as_object()
			:
			m_last_key_pressed(0)
		{
			memset(m_keymap, 0, sizeof(m_keymap));
		}

		bool	is_key_down(int code)
		{
			if (code < 0 || code >= key::KEYCOUNT) return false;

			int	byte_index = code >> 3;
			int	bit_index = code - (byte_index << 3);
			int	mask = 1 << bit_index;

			assert(byte_index >= 0 && byte_index < int(sizeof(m_keymap)/sizeof(m_keymap[0])));

			if (m_keymap[byte_index] & mask)
			{
				return true;
			}
			else
			{
				return false;
			}
		}

		void	set_key_down(int code)
		{
			if (code < 0 || code >= key::KEYCOUNT) return;

			m_last_key_pressed = code;

			int	byte_index = code >> 3;
			int	bit_index = code - (byte_index << 3);
			int	mask = 1 << bit_index;

			assert(byte_index >= 0 && byte_index < int(sizeof(m_keymap)/sizeof(m_keymap[0])));

			m_keymap[byte_index] |= mask;

			// Notify listeners.
			for (int i = 0, n = m_listeners.size(); i < n; i++)
			{
				smart_ptr<as_object_interface>	listener = m_listeners[i];
				as_value	method;
				if (listener != NULL
				    && listener->get_member(event_id(event_id::KEY_DOWN).get_function_name(), &method))
				{
					call_method(method, NULL /* or root? */, listener.get_ptr(), 0, 0);
				}
			}
		}

		void	set_key_up(int code)
		{
			if (code < 0 || code >= key::KEYCOUNT) return;

			int	byte_index = code >> 3;
			int	bit_index = code - (byte_index << 3);
			int	mask = 1 << bit_index;

			assert(byte_index >= 0 && byte_index < int(sizeof(m_keymap)/sizeof(m_keymap[0])));

			m_keymap[byte_index] &= ~mask;

			// Notify listeners.
			// @@ TODO need to figure out what environment to use...
		}

		void	cleanup_listeners()
		// Remove dead entries in the listeners list.  (Since
		// we use weak_ptr's, listeners can disappear without
		// notice.)
		{
			for (int i = m_listeners.size() - 1; i >= 0; i--)
			{
				if (m_listeners[i] == NULL)
				{
					m_listeners.remove(i);
				}
			}
		}

		void	add_listener(as_object_interface* listener)
		{
			cleanup_listeners();

			for (int i = 0, n = m_listeners.size(); i < n; i++)
			{
				if (m_listeners[i] == listener)
				{
					// Already in the list.
					return;
				}
			}

			m_listeners.push_back(listener);
		}

		void	remove_listener(as_object_interface* listener)
		{
			cleanup_listeners();

			for (int i = m_listeners.size() - 1; i >= 0; i--)
			{
				if (m_listeners[i] == listener)
				{
					m_listeners.remove(i);
				}
			}
		}

		int	get_last_key_pressed() const { return m_last_key_pressed; }
	};


	void	key_add_listener(as_value* result, as_object_interface* this_ptr, as_environment* env, int nargs, int first_arg)
	// Add a listener (first arg is object reference) to our list.
	// Listeners will have "onKeyDown" and "onKeyUp" methods
	// called on them when a key changes state.
	{
		if (nargs < 1)
		{
			log_error("key_add_listener needs one argument (the listener object)\n");
			return;
		}

		as_object_interface*	listener = env->bottom(first_arg).to_object();
		if (listener == NULL)
		{
			log_error("key_add_listener passed a NULL object; ignored\n");
			return;
		}

		key_as_object*	ko = (key_as_object*) (as_object*) this_ptr;
		assert(ko);

		ko->add_listener(listener);
	}

	void	key_get_ascii(as_value* result, as_object_interface* this_ptr, as_environment* env, int nargs, int first_arg)
	// Return the ascii value of the last key pressed.
	{
		key_as_object*	ko = (key_as_object*) (as_object*) this_ptr;
		assert(ko);

		result->set_undefined();

		int	code = ko->get_last_key_pressed();
		if (code > 0)
		{
			// @@ Crude for now; just jamming the key code in a string, as a character.
			// Need to apply shift/capslock/numlock, etc...
			char	buf[2];
			buf[0] = (char) code;
			buf[1] = 0;

			result->set(buf);
		}
	}

	void	key_get_code(as_value* result, as_object_interface* this_ptr, as_environment* env, int nargs, int first_arg)
	// Returns the keycode of the last key pressed.
	{
		key_as_object*	ko = (key_as_object*) (as_object*) this_ptr;
		assert(ko);

		result->set(ko->get_last_key_pressed());
	}

	void	key_is_down(as_value* result, as_object_interface* this_ptr, as_environment* env, int nargs, int first_arg)
	// Return true if the specified (first arg keycode) key is pressed.
	{
		if (nargs < 1)
		{
			log_error("key_is_down needs one argument (the key code)\n");
			return;
		}

		int	code = (int) env->bottom(first_arg).to_number();

		key_as_object*	ko = (key_as_object*) (as_object*) this_ptr;
		assert(ko);

		result->set(ko->is_key_down(code));
	}

	void	key_is_toggled(as_value* result, as_object_interface* this_ptr, as_environment* env, int nargs, int first_arg)
	// Given the keycode of NUM_LOCK or CAPSLOCK, returns true if
	// the associated state is on.
	{
		// @@ TODO
		result->set(false);
	}

	void	key_remove_listener(as_value* result, as_object_interface* this_ptr, as_environment* env, int nargs, int first_arg)
	// Remove a previously-added listener.
	{
		if (nargs < 1)
		{
			log_error("key_remove_listener needs one argument (the listener object)\n");
			return;
		}

		as_object_interface*	listener = env->bottom(first_arg).to_object();
		if (listener == NULL)
		{
			log_error("key_remove_listener passed a NULL object; ignored\n");
			return;
		}

		key_as_object*	ko = (key_as_object*) (as_object*) this_ptr;
		assert(ko);

		ko->remove_listener(listener);
	}


	void key_init()
	{
		// Create built-in key object.
		as_object*	key_obj = new key_as_object;

		// constants
#define KEY_CONST(k) key_obj->set_member(#k, key::k)
		KEY_CONST(BACKSPACE);
		KEY_CONST(CAPSLOCK);
		KEY_CONST(CONTROL);
		KEY_CONST(DELETEKEY);
		KEY_CONST(DOWN);
		KEY_CONST(END);
		KEY_CONST(ENTER);
		KEY_CONST(ESCAPE);
		KEY_CONST(HOME);
		KEY_CONST(INSERT);
		KEY_CONST(LEFT);
		KEY_CONST(PGDN);
		KEY_CONST(PGUP);
		KEY_CONST(RIGHT);
		KEY_CONST(SHIFT);
		KEY_CONST(SPACE);
		KEY_CONST(TAB);
		KEY_CONST(UP);

		// methods
		key_obj->set_member("addListener", &key_add_listener);
		key_obj->set_member("getAscii", &key_get_ascii);
		key_obj->set_member("getCode", &key_get_code);
		key_obj->set_member("isDown", &key_is_down);
		key_obj->set_member("isToggled", &key_is_toggled);
		key_obj->set_member("removeListener", &key_remove_listener);

		s_built_ins.add("Key", key_obj);
	}


	void	notify_key_event(key::code k, bool down)
	// External interface for the host to report key events.
	{
		action_init();	// @@ put this in some global init somewhere else...

		static tu_string	key_obj_name("Key");

		as_value	kval;
		s_built_ins.get(key_obj_name, &kval);
		if (kval.get_type() == as_value::OBJECT)
		{
			key_as_object*	ko = (key_as_object*) kval.to_object();
			assert(ko);

			if (down) ko->set_key_down(k);
			else ko->set_key_up(k);
		}
		else
		{
			log_error("gameswf::notify_key_event(): no Key built-in\n");
		}
	}
	

	//
	// global init
	//


	void	action_init()
	// Create/hook built-ins.
	{
		if (s_inited == false)
		{
			s_inited = true;

			// @@ hm, should this be per movie_interface
			// instead of truly global?
			s_built_ins.add("_global", new as_object);

			math_init();
			key_init();
		}
	}


	void	action_clear()
	{
		if (s_inited)
		{
			s_inited = false;

			for (stringi_hash<as_value>::iterator it = s_built_ins.begin();
			     it != s_built_ins.end();
			     ++it)
			{
				as_object_interface*	obj = it->second.to_object();
				if (obj)
				{
					delete obj;	// obj->drop_ref()
				}
			}

			s_built_ins.clear();
		}
	}


	//
	// string
	//


	as_value	string_method(
		as_environment* env,
		const tu_string& this_string,
		const tu_stringi& method_name,
		int nargs,
		int first_arg_bottom_index)
	{
		if (method_name == "charCodeAt")
		{
			int	index = (int) env->bottom(first_arg_bottom_index).to_number();
			if (index >= 0 && index < this_string.length())
			{
				return as_value(this_string[index]);
			}

			return as_value(0);
		}
		else if (method_name == "indexOf")
		{
			if (nargs < 1)
			{
				return as_value(-1);
			}
			else
			{
				int	start_index = 0;
				if (nargs > 1)
				{
					start_index = (int) env->bottom(first_arg_bottom_index - 1).to_number();
				}
				const char*	str = this_string.c_str();
				const char*	p = strstr(
					str + start_index,
					env->bottom(first_arg_bottom_index).to_string());
				if (str == NULL)
				{
					return -1;
				}

				return p - str;
			}
		}
		else if (method_name == "substring")
		{
			// Pull a slice out of this_string.
			int	start = 0;
			int	end = this_string.length();
			if (nargs >= 1)
			{
				start = (int) env->bottom(first_arg_bottom_index).to_number();
				start = iclamp(start, 0, this_string.length());
			}
			if (nargs >= 2)
			{
				end = (int) env->bottom(first_arg_bottom_index - 1).to_number();
				end = iclamp(end, 0, this_string.length());
			}

			if (end < start) swap(&start, &end);	// dumb, but that's what the docs say
			assert(end >= start);

			tu_string	result(this_string.c_str() + start);
			result.resize(end - start);	// @@ check this!

			return as_value(result);
		}

		return as_value();
	}


	//
	// properties by number
	//

	static const tu_string	s_property_names[] =
	{
		tu_string("_x"),
		tu_string("_y"),
		tu_string("_xscale"),
		tu_string("_yscale"),
		tu_string("_currentframe"),
		tu_string("_totalframes"),
		tu_string("_alpha"),
		tu_string("_visible"),
		tu_string("_width"),
		tu_string("_height"),
		tu_string("_rotation"),
		tu_string("_target"),
		tu_string("_framesloaded"),
		tu_string("_name"),
		tu_string("_droptarget"),
		tu_string("_url"),
		tu_string("_highquality"),
		tu_string("_focusrect"),
		tu_string("_soundbuftime"),
		tu_string("@@ mystery quality member"),
		tu_string("_xmouse"),
		tu_string("_ymouse"),
	};


	static as_value	get_property(as_object_interface* obj, int prop_number)
	{
		as_value	val;
		if (prop_number >= 0 && prop_number < int(sizeof(s_property_names)/sizeof(s_property_names[0])))
		{
			obj->get_member(s_property_names[prop_number], &val);
		}
		else
		{
			log_error("error: invalid property query, property number %d\n", prop_number);
		}
		return val;
	}

	static void	set_property(as_object_interface* obj, int prop_number, const as_value& val)
	{
		if (prop_number >= 0 && prop_number < int(sizeof(s_property_names)/sizeof(s_property_names[0])))
		{
			obj->set_member(s_property_names[prop_number], val);
		}
		else
		{
			log_error("error: invalid set_property, property number %d\n", prop_number);
		}
	}


	//
	// do_action
	//


	// Thin wrapper around action_buffer.
	struct do_action : public execute_tag
	{
		action_buffer	m_buf;

		void	read(stream* in)
		{
			m_buf.read(in);
		}

		void	execute(movie* m)
		{
			m->add_action_buffer(&m_buf);
		}

		void	execute_state(movie* m)
		{
			// left empty because actions don't have to be replayed when seeking the movie.
		}

		virtual bool	is_action_tag() const
		// Tell the caller that we are an action tag.
		{
			return true;
		}
	};

	void	do_action_loader(stream* in, int tag_type, movie_definition_sub* m)
	{
		IF_VERBOSE_PARSE(log_msg("tag %d: do_action_loader\n", tag_type));

		IF_VERBOSE_ACTION(log_msg("-- actions in frame %d\n", m->get_loading_frame()));

		assert(in);
		assert(tag_type == 12);
		assert(m);
		
		do_action*	da = new do_action;
		da->read(in);

		m->add_execute_tag(da);
	}


	//
	// action_buffer
	//

	// Disassemble one instruction to the log.
	static void	log_disasm(const unsigned char* instruction_data);

	action_buffer::action_buffer()
		:
		m_decl_dict_processed_at(-1)
	{
	}


	void	action_buffer::read(stream* in)
	{
		// Read action bytes.
		for (;;)
		{
			int	instruction_start = m_buffer.size();

			int	pc = m_buffer.size();

			int	action_id = in->read_u8();
			m_buffer.push_back(action_id);

			if (action_id & 0x80)
			{
				// Action contains extra data.  Read it.
				int	length = in->read_u16();
				m_buffer.push_back(length & 0x0FF);
				m_buffer.push_back((length >> 8) & 0x0FF);
				for (int i = 0; i < length; i++)
				{
					unsigned char	b = in->read_u8();
					m_buffer.push_back(b);
				}
			}

			IF_VERBOSE_ACTION(log_msg("%4d\t", pc); log_disasm(&m_buffer[instruction_start]); );

			if (action_id == 0)
			{
				// end of action buffer.
				break;
			}
		}
	}


	void	action_buffer::process_decl_dict(int start_pc, int stop_pc)
	// Interpret the decl_dict opcode.  Don't read stop_pc or
	// later.  A dictionary is some static strings embedded in the
	// action buffer; there should only be one dictionary per
	// action buffer.
	//
	// NOTE: Normally the dictionary is declared as the first
	// action in an action buffer, but I've seen what looks like
	// some form of copy protection that amounts to:
	//
	// <start of action buffer>
	//          push true
	//          branch_if_true label
	//          decl_dict   [0]   // this is never executed, but has lots of orphan data declared in the opcode
	// label:   // (embedded inside the previous opcode; looks like an invalid jump)
	//          ... "protected" code here, including the real decl_dict opcode ...
	//          <end of the dummy decl_dict [0] opcode>
	//
	// So we just interpret the first decl_dict we come to, and
	// cache the results.  If we ever hit a different decl_dict in
	// the same action_buffer, then we log an error and ignore it.
	{
		assert(stop_pc <= m_buffer.size());

		if (m_decl_dict_processed_at == start_pc)
		{
			// We've already processed this decl_dict.
			int	count = m_buffer[start_pc + 3] | (m_buffer[start_pc + 4] << 8);
			assert(m_dictionary.size() == count);
			return;
		}

		if (m_decl_dict_processed_at != -1)
		{
			log_error("error: process_decl_dict(%d, %d): decl_dict was already processed at %d\n",
				  start_pc,
				  stop_pc,
				  m_decl_dict_processed_at);
			return;
		}

		m_decl_dict_processed_at = start_pc;

		// Actual processing.
		int	i = start_pc;
		int	length = m_buffer[i + 1] | (m_buffer[i + 2] << 8);
		int	count = m_buffer[i + 3] | (m_buffer[i + 4] << 8);
		i += 2;

		assert(start_pc + 3 + length == stop_pc);

		m_dictionary.resize(count);

		// Index the strings.
		for (int ct = 0; ct < count; ct++)
		{
			// Point into the current action buffer.
			m_dictionary[ct] = (const char*) &m_buffer[3 + i];

			while (m_buffer[3 + i])
			{
				// safety check.
				if (i >= stop_pc)
				{
					log_error("error: action buffer dict length exceeded\n");

					// Jam something into the remaining (invalid) entries.
					while (ct < count)
					{
						m_dictionary[ct] = "<invalid>";
						ct++;
					}
					return;
				}
				i++;
			}
			i++;
		}
	}


	void	action_buffer::execute(as_environment* env)
	// Interpret the actions in this action buffer, and evaluate
	// them in the given environment.  Execute our whole buffer,
	// without any arguments passed in.
	{
		int	local_stack_top = env->get_local_frame_top();
		env->add_frame_barrier();

		array<with_stack_entry>	empty_with_stack;
		execute(env, 0, m_buffer.size(), NULL, empty_with_stack);

		env->set_local_frame_top(local_stack_top);
	}


	void	action_buffer::execute(
		as_environment* env,
		int start_pc,
		int exec_bytes,
		as_value* retval,
		const array<with_stack_entry>& initial_with_stack)
	// Interpret the specified subset of the actions in our
	// buffer.  Caller is responsible for cleaning up our local
	// stack frame (it may have passed its arguments in via the
	// local stack frame).
	{
		action_init();	// @@ stick this somewhere else; need some global static init function

		assert(env);

		array<with_stack_entry>	with_stack(initial_with_stack);

		movie*	original_target = env->get_target();
		UNUSED(original_target);		// Avoid warnings.

		int	stop_pc = start_pc + exec_bytes;

		for (int pc = start_pc; pc < stop_pc; )
		{
			// Cleanup any expired "with" blocks.
			while (with_stack.size() > 0
			       && pc >= with_stack.back().m_block_end_pc)
			{
				with_stack.resize(with_stack.size() - 1);
			}

			// Get the opcode.
			int	action_id = m_buffer[pc];
			if ((action_id & 0x80) == 0)
			{
				IF_VERBOSE_ACTION(log_msg("EX:\t"); log_disasm(&m_buffer[pc]));

				// Simple action; no extra data.
				switch (action_id)
				{
				default:
					break;

				case 0x00:	// end of actions.
					return;

				case 0x04:	// next frame.
					env->get_target()->goto_frame(env->get_target()->get_current_frame() + 1);
					break;

				case 0x05:	// prev frame.
					env->get_target()->goto_frame(env->get_target()->get_current_frame() - 1);
					break;

				case 0x06:	// action play
					env->get_target()->set_play_state(movie::PLAY);
					break;

				case 0x07:	// action stop
					env->get_target()->set_play_state(movie::STOP);
					break;

				case 0x08:	// toggle quality
				case 0x09:	// stop sounds
					break;

				case 0x0A:	// add
				{
					env->top(1) += env->top(0);
					env->drop(1);
					break;
				}
				case 0x0B:	// subtract
				{
					env->top(1) -= env->top(0);
					env->drop(1);
					break;
				}
				case 0x0C:	// multiply
				{
					env->top(1) *= env->top(0);
					env->drop(1);
					break;
				}
				case 0x0D:	// divide
				{
					env->top(1) /= env->top(0);
					env->drop(1);
					break;
				}
				case 0x0E:	// equal
				{
					env->top(1).set(env->top(1) == env->top(0));
					env->drop(1);
					break;
				}
				case 0x0F:	// less than
				{
					env->top(1).set(env->top(1) < env->top(0));
					env->drop(1);
					break;
				}
				case 0x10:	// logical and
				{
					env->top(1).set(env->top(1).to_bool() && env->top(0).to_bool());
					env->drop(1);
					break;
				}
				case 0x11:	// logical or
				{
					env->top(1).set(env->top(1).to_bool() && env->top(0).to_bool());
					env->drop(1);
					break;
				}
				case 0x12:	// logical not
				{
					env->top(0).set(! env->top(0).to_bool());
					break;
				}
				case 0x13:	// string equal
				{
					env->top(1).set(env->top(1).to_tu_string() == env->top(0).to_tu_string());
					env->drop(1);
					break;
				}
				case 0x14:	// string length
				{
					env->top(0).set(env->top(0).to_tu_string().length());
					break;
				}
				case 0x15:	// substring
				{
					int	size = int(env->top(0).to_number());
					int	base = int(env->top(1).to_number()) - 1;
					const tu_string&	str = env->top(2).to_tu_string();

					// Keep base within range.
					base = iclamp(base, 0, str.length());

					// Truncate if necessary.
					size = imin(str.length() - base, size);

					// @@ This can be done without new allocations if we get dirtier w/ internals
					// of as_value and tu_string...
					tu_string	new_string = str.c_str() + base;
					new_string.resize(size);

					env->drop(2);

					break;
				}
				case 0x17:	// pop
				{
					env->drop(1);
					break;
				}
				case 0x18:	// int
				{
					env->top(0).set(int(floor(env->top(0).to_number())));
					break;
				}
				case 0x1C:	// get variable
				{
					as_value	varname = env->pop();
					env->push(env->get_variable(varname.to_tu_string(), with_stack));
					break;
				}
				case 0x1D:	// set variable
				{
					env->set_variable(env->top(1).to_tu_string(), env->top(0), with_stack);
					env->drop(2);
					break;
				}
				case 0x20:	// set target expression
				{
					// @@ TODO
					break;
				}
				case 0x21:	// string concat
				{
					env->top(1).string_concat(env->top(0).to_tu_string());
					env->drop(1);
					break;
				}
				case 0x22:	// get property
				{
					movie*	target = env->find_target(env->top(1));
					if (target)
					{
						env->top(1) = get_property(target, (int) env->top(0).to_number());
					}
					else
					{
						env->top(1) = as_value(as_value::UNDEFINED);
					}
					env->drop(1);
					break;
				}

				case 0x23:	// set property
				{
					movie*	target = env->find_target(env->top(2));
					if (target)
					{
						set_property(target, (int) env->top(1).to_number(), env->top(0));
					}
					env->drop(3);
					break;
				}

				case 0x24:	// duplicate clip (sprite?)
				{
					env->get_target()->clone_display_object(
						env->top(2).to_tu_string(),
						env->top(1).to_tu_string(),
						(int) env->top(0).to_number());
					env->drop(3);
					break;
				}

				case 0x25:	// remove clip
					env->get_target()->remove_display_object(env->top(0).to_tu_string());
					env->drop(1);
					break;

				case 0x26:	// trace
				{
					// Log the stack val.
					const char*	message = env->top(0).to_string();
					log_msg("%s\n", message);
					env->drop(1);
					break;
				}

				case 0x27:	// start drag movie
				{
					movie::drag_state	st;

					st.m_character = env->find_target(env->top(0));
					if (st.m_character == NULL)
					{
						log_error("error: start_drag of invalid target '%s'.\n",
							  env->top(0).to_string());
					}

					st.m_lock_center = env->top(1).to_bool();
					st.m_bound = env->top(2).to_bool();
					if (st.m_bound)
					{
						st.m_bound_x0 = (float) env->top(6).to_number();
						st.m_bound_y0 = (float) env->top(5).to_number();
						st.m_bound_x1 = (float) env->top(4).to_number();
						st.m_bound_y1 = (float) env->top(3).to_number();
						env->drop(4);
					}
					env->drop(3);

					movie*	root_movie = env->get_target()->get_root_movie();
					assert(root_movie);

					if (root_movie && st.m_character)
					{
						root_movie->set_drag_state(st);
					}
					
					break;
				}

				case 0x28:	// stop drag movie
				{
					movie*	root_movie = env->get_target()->get_root_movie();
					assert(root_movie);

					root_movie->stop_drag();

					break;
				}

				case 0x29:	// string less than
				{
					env->top(1).set(env->top(1).to_tu_string() < env->top(0).to_tu_string());
					break;
				}
				case 0x30:	// random
				{
					int	max = int(env->top(0).to_number());
					if (max < 1) max = 1;
					env->top(0).set(double(tu_random::next_random() % max));
					break;
				}
				case 0x31:	// mb length
				{
					// @@ TODO
					break;
				}
				case 0x32:	// ord
				{
					// ASCII code of first character
					env->top(0).set(env->top(0).to_string()[0]);
					break;
				}
				case 0x33:	// chr
				{
					char	buf[2];
					buf[0] = int(env->top(0).to_number());
					buf[1] = 0;
					env->top(0).set(buf);
					break;
				}

				case 0x34:	// get timer
					// Push milliseconds since we started playing.
					env->push(floorf(env->m_target->get_timer() * 1000.0f));
					break;

				case 0x35:	// mb substring
				{
					// @@ TODO
				}
				case 0x37:	// mb chr
				{
					// @@ TODO
					break;
				}
				case 0x3A:	// delete
				{
					// @@ TODO
					break;
				}
				case 0x3B:	// delete all
				{
					// @@ TODO
					break;
				}

				case 0x3C:	// set local
				{
					as_value	value = env->pop();
					as_value	varname = env->pop();
					env->set_local(varname.to_tu_string(), value);
					break;
				}

				case 0x3D:	// call function
				{
					as_value	function;
					if (env->top(0).get_type() == as_value::STRING)
					{
						// Function is a string; lookup the function.
						const tu_string&	function_name = env->top(0).to_tu_string();
						function = env->get_variable(function_name, with_stack);

						if (function.get_type() != as_value::C_FUNCTION
						    && function.get_type() != as_value::AS_FUNCTION)
						{
							log_error("error in call_function: '%s' is not a function\n",
								  function_name.c_str());
						}
					}
					else
					{
						// Hopefully the actual function object is here.
						function = env->top(0);
					}
					int	nargs = (int) env->top(1).to_number();
					as_value	result = call_method(function, env, NULL, nargs, env->get_top_index() - 2);
					env->drop(nargs + 1);
					env->top(0) = result;
					break;
				}
				case 0x3E:	// return
				{
					// Put top of stack in the provided return slot, if
					// it's not NULL.
					if (retval)
					{
						*retval = env->top(0);
					}
					env->drop(1);

					// Skip the rest of this buffer (return from this action_buffer).
					pc = stop_pc;

					break;
				}
				case 0x3F:	// modulo
				{
					// @@ TODO
					break;
				}
				case 0x40:	// new
				{
					// @@ TODO
					break;
				}
				case 0x41:	// declare local
				{
					const tu_string&	varname = env->top(0).to_tu_string();
					env->declare_local(varname);
					env->drop(1);
					break;
				}
				case 0x42:	// declare array
				{
					// @@ TODO
					break;
				}
				case 0x43:	// declare object
				{
					// @@ TODO
					break;
				}
				case 0x44:	// type of
				{
					// @@ TODO
					break;
				}
				case 0x45:	// get target
				{
					// @@ TODO
					break;
				}
				case 0x46:	// enumerate
				{
					// @@ TODO
					break;
				}
				case 0x47:	// add (typed)
				{
					if (env->top(1).get_type() == as_value::STRING)
					{
						env->top(1).string_concat(env->top(0).to_tu_string());
					}
					else
					{
						env->top(1) += env->top(0);
					}
					env->drop(1);
					break;
				}
				case 0x48:	// less than (typed)
				{
					if (env->top(1).get_type() == as_value::STRING)
					{
						env->top(1).set(env->top(1).to_tu_string() < env->top(0).to_tu_string());
					}
					else
					{
						env->top(1).set(env->top(1) < env->top(0));
					}
					env->drop(1);
					break;
				}
				case 0x49:	// equal (typed)
				{
					// @@ identical to untyped equal, as far as I can tell...
					env->top(1).set(env->top(1) == env->top(0));
					env->drop(1);
					break;
				}
				case 0x4A:	// to number
				{
					env->top(0).convert_to_number();
					break;
				}
				case 0x4B:	// to string
				{
					env->top(0).convert_to_string();
					break;
				}
				case 0x4C:	// dup
					env->push(env->top(0));
					break;
				
				case 0x4D:	// swap
				{
					as_value	temp = env->top(1);
					env->top(1) = env->top(0);
					env->top(0) = temp;
					break;
				}
				case 0x4E:	// get member
				{
					as_object_interface*	obj = env->top(1).to_object();

					// Special case: String has a member "length"
					if (obj == NULL
					    && env->top(1).get_type() == as_value::STRING
					    && env->top(0).to_tu_stringi() == "length")
					{
						int	len = env->top(1).to_tu_string().length();
						env->top(1).set(len);
					}
					else
					{
						env->top(1).set_undefined();
						if (obj)
						{
							obj->get_member(env->top(0).to_tu_string(), &(env->top(1)));
						}
						else
						{
							// @@ log error?
						}
					}
					env->drop(1);
					break;
				}
				case 0x4F:	// set member
				{
					as_object_interface*	obj = env->top(2).to_object();
					if (obj)
					{
						obj->set_member(env->top(1).to_tu_string(), env->top(0));
					}
					else
					{
						// @@ invalid object: log error?
					}
					env->drop(3);
					break;
				}
				case 0x50:	// increment
					env->top(0) += 1;
					break;
				case 0x51:	// decrement
					env->top(0) -= 1;
					break;
				case 0x52:	// call method
				{
					as_object_interface*	obj = env->top(1).to_object();
					int	nargs = (int) env->top(2).to_number();
					as_value	result;
					if (obj)
					{
						as_value	method;
						const tu_string&	method_name = env->top(0).to_tu_string();
						if (obj->get_member(method_name, &method))
						{
							if (method.get_type() != as_value::C_FUNCTION
							    && method.get_type() != as_value::AS_FUNCTION)
							{
								log_error("error: call_method: '%s' is not a method\n",
									  method_name.c_str());
							}
							else
							{
								result = call_method(
									method,
									env,
									obj,
									nargs,
									env->get_top_index() - 3);
							}
						}
						else
						{
							log_error("error: call_method can't find method %s\n", method_name.c_str());
						}
					}
					else if (env->top(1).get_type() == as_value::STRING)
					{
						// Hack to call String methods.  as_value
						// should maybe be subclassed from as_object_interface
						// instead, or have as_value::to_object() make a proxy
						// or something.
						result = string_method(
							env,
							env->top(1).to_tu_string(),
							env->top(0).to_tu_stringi(),
							nargs,
							env->get_top_index() - 3);
					}
					else
					{
						log_error("error: call_method op on invalid object.\n");
					}
					env->drop(nargs + 2);
					env->top(0) = result;
					break;
				}
				case 0x53:	// new method
					// @@ TODO
					break;
				case 0x54:	// instance of
					// @@ TODO
					break;
				case 0x55:	// enumerate object
					// @@ TODO
					break;
				case 0x60:	// bitwise and
					env->top(1) &= env->top(0);
					env->drop(1);
					break;
				case 0x61:	// bitwise or
					env->top(1) |= env->top(0);
					env->drop(1);
					break;
				case 0x62:	// bitwise xor
					env->top(1) ^= env->top(0);
					env->drop(1);
					break;
				case 0x63:	// shift left
					env->top(1).shl(env->top(0));
					env->drop(1);
					break;
				case 0x64:	// shift right (signed)
					env->top(1).asr(env->top(0));
					env->drop(1);
					break;
				case 0x65:	// shift right (unsigned)
					env->top(1).lsr(env->top(0));
					env->drop(1);
					break;
				case 0x66:	// strict equal
					if (env->top(1).get_type() != env->top(0).get_type())
					{
						// Types don't match.
						env->top(1).set(false);
						env->drop(1);
					}
					else
					{
						env->top(1).set(env->top(1) == env->top(0));
						env->drop(1);
					}
					break;
				case 0x67:	// gt (typed)
					if (env->top(1).get_type() == as_value::STRING)
					{
						env->top(1).set(env->top(1).to_tu_string() > env->top(0).to_tu_string());
					}
					else
					{
						env->top(1).set(env->top(1).to_number() > env->top(0).to_number());
					}
					env->drop(1);
					break;
				case 0x68:	// string gt
					env->top(1).set(env->top(1).to_tu_string() > env->top(0).to_tu_string());
					env->drop(1);
					break;
				}
				pc++;	// advance to next action.
			}
			else
			{
				IF_VERBOSE_ACTION(log_msg("EX:\t"); log_disasm(&m_buffer[pc]));

				// Action containing extra data.
				int	length = m_buffer[pc + 1] | (m_buffer[pc + 2] << 8);
				int	next_pc = pc + length + 3;

				switch (action_id)
				{
				default:
					break;

				case 0x81:	// goto frame
				{
					int	frame = m_buffer[pc + 3] | (m_buffer[pc + 4] << 8);
					// 0-based already?
					//// Convert from 1-based to 0-based
					//frame--;
					env->get_target()->goto_frame(frame);
					break;
				}

				case 0x83:	// get url
				{
					// If this is an FSCommand, then call the callback
					// handler, if any.

					// Two strings as args.
					const char*	url = (const char*) &(m_buffer[pc + 3]);
					int	url_len = strlen(url);
					const char*	target = (const char*) &(m_buffer[pc + 3 + url_len + 1]);

					// If the url starts with "FSCommand:", then this is
					// a message for the host app.
					if (strncmp(url, "FSCommand:", 10) == 0)
					{
						if (s_fscommand_handler)
						{
							// Call into the app.
							(*s_fscommand_handler)(env->get_target()->get_root_interface(), url + 10, target);
						}
					}
					// else just ignore this.  Maybe log it?

					break;
				}

				case 0x87:	// store_register
				{
					int	reg = m_buffer[pc + 3];
					if (reg >= 0 && reg < 4)
					{
						// Save top of stack in specified register.
						env->m_register[reg] = env->top(0);
						
						IF_VERBOSE_ACTION(
							log_msg("-------------- reg[%d] = '%s'\n",
								reg,
								env->top(0).to_string()));
					}
					else
					{
						log_error("store_register[%d] -- register out of bounds!", reg);
					}

					break;
				}

				case 0x88:	// decl_dict: declare dictionary
				{
					int	i = pc;
					//int	count = m_buffer[pc + 3] | (m_buffer[pc + 4] << 8);
					i += 2;

					process_decl_dict(pc, next_pc);

					break;
				}

				case 0x8A:	// wait for frame
				{
					// @@ TODO I think this has to deal with incremental loading
					break;
				}

				case 0x8B:	// set target
				{
					// Change the movie we're working on.
					const char* target_name = (const char*) &m_buffer[pc + 3];
					if (target_name[0] == 0) { env->set_target(original_target); }
					else {
//						env->set_target(env->get_target()->find_labeled_target(target_name));
//						if (env->get_target() == NULL) env->set_target(original_target);
					}
					break;
				}

				case 0x8C:	// go to labeled frame
				{
					char*	frame_label = (char*) &m_buffer[pc + 3];
					env->get_target()->goto_labeled_frame(frame_label);
					break;
				}

				case 0x8D:	// wait for frame expression (?)
					break;

				case 0x94:	// with
				{
					if (with_stack.size() < 8)
					{
 						int	block_length = m_buffer[pc + 3] | (m_buffer[pc + 4] << 8);
 						int	block_end = next_pc + block_length;
 						as_object_interface*	with_obj = env->top(0).to_object();
 						with_stack.push_back(with_stack_entry(with_obj, block_end));
					}
					env->drop(1);
					break;
				}
				case 0x96:	// push_data
				{
					int i = pc;
					while (i - pc < length)
					{
						int	type = m_buffer[3 + i];
						i++;
						if (type == 0)
						{
							// string
							const char*	str = (const char*) &m_buffer[3 + i];
							i += strlen(str) + 1;
							env->push(str);

							IF_VERBOSE_ACTION(log_msg("-------------- pushed '%s'\n", str));
						}
						else if (type == 1)
						{
							// float (little-endian)
							union {
								float	f;
								Uint32	i;
							} u;
							compiler_assert(sizeof(u) == sizeof(u.i));

							memcpy(&u.i, &m_buffer[3 + i], 4);
							u.i = swap_le32(u.i);
							i += 4;

							env->push(u.f);

							IF_VERBOSE_ACTION(log_msg("-------------- pushed '%f'\n", u.f));
						}
						else if (type == 2)
						{
							// NULL
							env->push(as_value());	// @@???

							IF_VERBOSE_ACTION(log_msg("-------------- pushed NULL\n"));
						}
						else if (type == 3)
						{
							env->push(as_value(as_value::UNDEFINED));

							IF_VERBOSE_ACTION(log_msg("-------------- pushed UNDEFINED\n"));
						}
						else if (type == 4)
						{
							// contents of register
							int	reg = m_buffer[3 + i];
							UNUSED(reg);
							i++;
							if (reg < 0 || reg >= 4)
							{
								env->push(as_value(as_value::UNDEFINED));
								log_error("push register[%d] -- register out of bounds!\n", reg);
							}
							else
							{
								env->push(env->m_register[reg]);
								IF_VERBOSE_ACTION(
									log_msg("-------------- pushed register[%d] = '%s'\n",
										reg,
										env->top(0).to_string()));
							}

						}
						else if (type == 5)
						{
							bool	bool_val = m_buffer[3 + i] ? true : false;
							i++;
//							log_msg("bool(%d)\n", bool_val);
							env->push(bool_val);

							IF_VERBOSE_ACTION(log_msg("-------------- pushed %s\n", bool_val ? "true" : "false"));
						}
						else if (type == 6)
						{
							// double
							// wacky format: 45670123
							union {
								double	d;
								Uint64	i;
								struct {
									Uint32	lo;
									Uint32	hi;
								} sub;
							} u;
							compiler_assert(sizeof(u) == sizeof(u.i));

							memcpy(&u.sub.hi, &m_buffer[3 + i], 4);
							memcpy(&u.sub.lo, &m_buffer[3 + i + 4], 4);
							u.i = swap_le64(u.i);
							i += 8;

							env->push(u.d);

							IF_VERBOSE_ACTION(log_msg("-------------- pushed double %f\n", u.d));
						}
						else if (type == 7)
						{
							// int32
							Sint32	val = m_buffer[3 + i]
								| (m_buffer[3 + i + 1] << 8)
								| (m_buffer[3 + i + 2] << 16)
								| (m_buffer[3 + i + 3] << 24);
							i += 4;
						
							env->push(val);

							IF_VERBOSE_ACTION(log_msg("-------------- pushed int32 %d\n", val));
						}
						else if (type == 8)
						{
							int	id = m_buffer[3 + i];
							i++;
							if (id < m_dictionary.size())
							{
								env->push(m_dictionary[id]);

								IF_VERBOSE_ACTION(log_msg("-------------- pushed '%s'\n", m_dictionary[id]));
							}
							else
							{
								log_error("error: dict_lookup(%d) is out of bounds!\n", id);
								env->push(0);
								IF_VERBOSE_ACTION(log_msg("-------------- pushed 0 @@\n"));
							}
						}
						else if (type == 9)
						{
							int	id = m_buffer[3 + i] | (m_buffer[4 + i] << 8);
							i += 2;
							if (id < m_dictionary.size())
							{
								env->push(m_dictionary[id]);
								IF_VERBOSE_ACTION(log_msg("-------------- pushed '%s'\n", m_dictionary[id]));
							}
							else
							{
								log_error("error: dict_lookup(%d) is out of bounds!\n", id);
								env->push(0);

								IF_VERBOSE_ACTION(log_msg("-------------- pushed 0 @@"));
							}
						}
					}
					
					break;
				}
				case 0x99:	// branch always (goto)
				{
					Sint16	offset = m_buffer[pc + 3] | (m_buffer[pc + 4] << 8);
					next_pc += offset;
					// @@ TODO range checks
					break;
				}
				case 0x9A:	// get url 2
				{
					int	method = m_buffer[pc + 3];
					UNUSED(method);

					const char*	target = env->top(0).to_string();
					const char*	url = env->top(1).to_string();

					// If the url starts with "FSCommand:", then this is
					// a message for the host app.
					if (strncmp(url, "FSCommand:", 10) == 0)
					{
						if (s_fscommand_handler)
						{
							// Call into the app.
							(*s_fscommand_handler)(env->get_target()->get_root_interface(), url + 10, target);
						}
					}
					// else nothing.  Maybe log it?

					// Drop the args.
					env->drop(2);

					break;
				}

				case 0x9B:	// declare function
				{
					as_as_function*	func = new as_as_function(this, env, next_pc, with_stack);

					int	i = pc;
					i += 3;

					// Extract name.
					tu_string	name = (const char*) &m_buffer[i];
					i += name.length() + 1;

					// Get number of arguments.
					int	nargs = m_buffer[i] | (m_buffer[i + 1] << 8);
					i += 2;

					// Get the names of the arguments.
					for (int n = 0; n < nargs; n++)
					{
						func->add_arg((const char*) &m_buffer[i]);
						i += func->m_args.back().length() + 1;
					}

					// Get the length of the actual function code.
					int	length = m_buffer[i] | (m_buffer[i + 1] << 8);
					i += 2;
					func->set_length(length);

					// Skip the function body (don't interpret it now).
					next_pc += length;

					// If we have a name, then save the function in this
					// environment under that name.
					as_value	function_value(func);
					if (name.length() > 0)
					{
						// @@ NOTE: should this be m_target->set_variable()???
						env->set_member(name, function_value);
					}

					// Also leave it on the stack.
					env->push_val(function_value);

					break;
				}
				case 0x9D:	// branch if true
				{
					Sint16	offset = m_buffer[pc + 3] | (m_buffer[pc + 4] << 8);
					
					bool	test = env->top(0).to_bool();
					env->drop(1);
					if (test)
					{
						next_pc += offset;

						if (next_pc > stop_pc)
						{
							log_error("branch to offset %d -- this section only runs to %d\n",
								  next_pc,
								  stop_pc);
						}
					}
					break;
				}
				case 0x9E:	// call frame
				{
					// Note: no extra data in this instruction!
					assert(env->m_target);
					env->m_target->call_frame_actions(env->top(0));
					env->drop(1);

					break;
				}

				case 0x9F:	// goto frame expression (?)
				{
					//assert(0);
					//log_error("error: unimplemented opcode 0x9F, goto_frame_exp\n");

					// From Alexi's SWF ref:
					//
					// Pop a value or a string and jump to the specified
					// frame. When a string is specified, it can include a
					// path to a sprite as in:
					// 
					//   /Test:55
					// 
					// When f_play is ON, the action is to play as soon as
					// that frame is reached. Otherwise, the
					// frame is shown in stop mode.
					unsigned char	play_flag = m_buffer[pc + 3];
					movie::play_state	state = play_flag ? movie::PLAY : movie::STOP;

					if (env->top(0).get_type() == as_value::STRING)
					{
						// @@ TODO: parse possible sprite path...
						
						// Also, if the frame spec is actually a number (not a label), then
						// we need to do the conversion...

						env->get_target()->goto_labeled_frame(env->top(0).to_string());
					}
					else
					{
						// @@ are frame numbers here 1-based or 0-based???
						// @@ guessing 0-based for now.
						env->get_target()->goto_frame(int(env->top(0).to_number()));
					}
					env->get_target()->set_play_state(state);
					
					env->drop(1);

					break;
				}
				
				}
				pc = next_pc;
			}
		}

		env->set_target(original_target);
	}


	//
	// as_value -- ActionScript value type
	//


	const char*	as_value::to_string() const
	// Conversion to string.
	{
		return to_tu_string().c_str();
	}


	const tu_stringi&	as_value::to_tu_stringi() const
	{
		return reinterpret_cast<const tu_stringi&>(to_tu_string());
	}


	const tu_string&	as_value::to_tu_string() const
	// Conversion to const tu_string&.
	{
		if (m_type == STRING) { /* don't need to do anything */ }
		else if (m_type == NUMBER)
		{
			char buffer[50];
			snprintf(buffer, 50, "%g", m_number_value);
			m_string_value = buffer;
		}
		else if (m_type == UNDEFINED)
		{
			m_string_value = "undefined";
		}
		else if (m_type == OBJECT)
		{
			const char*	val = NULL;
			if (m_object_value)
			{
				val = m_object_value->get_text_value();
			}

			if (val)
			{
				m_string_value = val;
			}
			else
			{
				// @@ actually, we need to return our full path.
				char buffer[50];
				snprintf(buffer, 50, "<object 0x%X>", (unsigned) m_object_value);
				m_string_value = buffer;
			}
		}
		else if (m_type == C_FUNCTION)
		{
			char buffer[50];
			snprintf(buffer, 50, "<c_function 0x%X>", (unsigned) m_c_function_value);
			m_string_value = buffer;
		}
		else if (m_type == AS_FUNCTION)
		{
			char buffer[50];
			snprintf(buffer, 50, "<as_function 0x%X>", (unsigned) m_as_function_value);
			m_string_value = buffer;
		}
		else
		{
			m_string_value = "<bad type>";
			assert(0);
		}
		
		return m_string_value;
	}

	
	double	as_value::to_number() const
	// Conversion to double.
	{
		if (m_type == STRING)
		{
			m_number_value = atof(m_string_value.c_str());
			return m_number_value;
		}
		else if (m_type == NUMBER)
		{
			return m_number_value;
		}
		else if (m_type == OBJECT && m_object_value != NULL)
		{
			// Text characters with var names could get in
			// here.
			return atof(m_object_value->get_text_value());
		}
		else
		{
			return 0.0;
		}
	}


	bool	as_value::to_bool() const
	// Conversion to boolean.
	{
		if (m_type == STRING)
		{
			if (m_string_value == "false")
			{
				return false;
			}
			else if (m_string_value == "true")
			{
				return true;
			}
			else
			{
				return to_number() != 0.0;
			}
		}
		else if (m_type == NUMBER)
		{
			return m_number_value != 0.0;
		}
		else if (m_type == OBJECT)
		{
			return m_object_value != NULL;
		}
		else if (m_type == C_FUNCTION)
		{
			return m_c_function_value != NULL;
		}
		else if (m_type == AS_FUNCTION)
		{
			return m_as_function_value != NULL;
		}
		else
		{
			return false;
		}
	}

	
	as_object_interface*	as_value::to_object() const
	// Return value as an object.
	{
		if (m_type == OBJECT)
		{
			// OK.
			return m_object_value;
		}
		else
		{
			return NULL;	// @@ or return a valid "null object"?
		}
	}


	as_c_function_ptr	as_value::to_c_function() const
	// Return value as a C function ptr.  Returns NULL if value is
	// not a C function.
	{
		if (m_type == C_FUNCTION)
		{
			// OK.
			return m_c_function_value;
		}
		else
		{
			return NULL;	// @@ or return a valid "null function"?
		}
	}

	as_as_function*	as_value::to_as_function() const
	// Return value as an ActionScript function.  Returns NULL if value is
	// not an ActionScript function.
	{
		if (m_type == AS_FUNCTION)
		{
			// OK.
			return m_as_function_value;
		}
		else
		{
			return NULL;
		}
	}


	void	as_value::convert_to_number()
	// Force type to number.
	{
		set(to_number());
	}


	void	as_value::convert_to_string()
	// Force type to string.
	{
		to_tu_string();	// init our string data.
		m_type = STRING;	// force type.
	}


	bool	as_value::operator==(const as_value& v) const
	// Return true if operands are equal.
	{
		if (m_type == STRING)
		{
			return m_string_value == v.to_tu_string();
		}
		else if (m_type == NUMBER)
		{
			return m_number_value == v.to_number();
		}
		else
		{
			return m_type == v.m_type;
		}
	}

	
	void	as_value::string_concat(const tu_string& str)
	// Sets *this to this string plus the given string.
	{
		to_tu_string();	// make sure our m_string_value is initialized
		m_type = STRING;
		m_string_value += str;
	}

	void	as_value::drop_refs()
	// Drop any ref counts we have; this happens prior to changing our value.
	{
		if (m_type == AS_FUNCTION)
		{
			if (m_as_function_value)
			{
				m_as_function_value->drop_ref();
			}
		}
	}


	//
	// as_environment
	//


	as_value	as_environment::get_variable(const tu_string& varname, const array<with_stack_entry>& with_stack) const
	// Return the value of the given var, if it's defined.
	{
		// Path lookup rigamarole.
		movie*	target = m_target;
		tu_string	path;
		tu_string	var;
		if (parse_path(varname, &path, &var))
		{
			target = find_target(path);	// @@ Use with_stack here too???  Need to test.
			if (target)
			{
				as_value	val;
				target->get_member(var, &val);
				return val;
			}
			else
			{
				log_error("find_target(\"%s\") failed\n", path.c_str());
				return as_value(as_value::UNDEFINED);
			}
		}
		else
		{
			return this->get_variable_raw(varname, with_stack);
		}
	}


	as_value	as_environment::get_variable_raw(
		const tu_string& varname,
		const array<with_stack_entry>& with_stack) const
	// varname must be a plain variable name; no path parsing.
	{
		assert(strchr(varname.c_str(), ':') == NULL);
		assert(strchr(varname.c_str(), '/') == NULL);
		assert(strchr(varname.c_str(), '.') == NULL);

		as_value	val;

		// Check the with-stack.
		for (int i = with_stack.size() - 1; i >= 0; i--)
		{
			as_object_interface*	obj = with_stack[i].m_object;
			if (obj && obj->get_member(varname, &val))
			{
				// Found the var in this context.
				return val;
			}
		}

		// Check locals.
		int	local_index = find_local(varname);
		if (local_index >= 0)
		{
			// Get local var.
			return m_local_frames[local_index].m_value;
		}

		// Looking for "this"?
		if (varname == "this")
		{
			val.set(m_target);
			return val;
		}

		// Check movie members.
		if (m_target->get_member(varname, &val))
		{
			return val;
		}

		// Check built-in constants.
		if (varname == "_root" || varname == "_level0")
		{
			return as_value(m_target->get_root_movie());
		}
		if (s_built_ins.get(varname, &val))
		{
			return val;
		}
	
		// Fallback.
		IF_VERBOSE_ACTION(log_msg("get_variable_raw(\"%s\") failed, returning UNDEFINED.\n", varname.c_str()));
		return as_value(as_value::UNDEFINED);
	}


	void	as_environment::set_variable(
		const tu_string& varname,
		const as_value& val,
		const array<with_stack_entry>& with_stack)
	// Given a path to variable, set its value.
	{
		IF_VERBOSE_ACTION(log_msg("-------------- %s = %s\n", varname.c_str(), val.to_string()));//xxxxxxxxxx

		// Path lookup rigamarole.
		movie*	target = m_target;
		tu_string	path;
		tu_string	var;
		if (parse_path(varname, &path, &var))
		{
			target = find_target(path);
			if (target)
			{
				target->set_member(var, val);
			}
		}
		else
		{
			this->set_variable_raw(varname, val, with_stack);
		}
	}


	void	as_environment::set_variable_raw(
		const tu_string& varname,
		const as_value& val,
		const array<with_stack_entry>& with_stack)
	// No path rigamarole.
	{
		// Check the with-stack.
		for (int i = with_stack.size() - 1; i >= 0; i--)
		{
			as_object_interface*	obj = with_stack[i].m_object;
			as_value	dummy;
			if (obj && obj->get_member(varname, &dummy))
			{
				// This object has the member; so set it here.
				obj->set_member(varname, val);
				return;
			}
		}

		// Check locals.
		int	local_index = find_local(varname);
		if (local_index >= 0)
		{
			// Set local var.
			m_local_frames[local_index].m_value = val;
			return;
		}

		assert(m_target);

		m_target->set_member(varname, val);
	}


	void	as_environment::set_local(const tu_string& varname, const as_value& val)
	// Set/initialize the value of the local variable.
	{
		// Is it in the current frame already?
		int	index = find_local(varname);
		if (index < 0)
		{
			// Not in frame; create a new local var.

			assert(varname.length() > 0);	// null varnames are invalid!
    			m_local_frames.push_back(frame_slot(varname, val));
		}
		else
		{
			// In frame already; modify existing var.
			m_local_frames[index].m_value = val;
		}
	}

	
	void	as_environment::add_local(const tu_string& varname, const as_value& val)
	// Add a local var with the given name and value to our
	// current local frame.  Use this when you know the var
	// doesn't exist yet, since it's faster than set_local();
	// e.g. when setting up args for a function.
	{
		assert(varname.length() > 0);
		m_local_frames.push_back(frame_slot(varname, val));
	}


	void	as_environment::declare_local(const tu_string& varname)
	// Create the specified local var if it doesn't exist already.
	{
		// Is it in the current frame already?
		int	index = find_local(varname);
		if (index < 0)
		{
			// Not in frame; create a new local var.
			assert(varname.length() > 0);	// null varnames are invalid!
    			m_local_frames.push_back(frame_slot(varname, as_value()));
		}
		else
		{
			// In frame already; don't mess with it.
		}
	}

	
	bool	as_environment::get_member(const tu_string& varname, as_value* val) const
	{
		return m_variables.get(varname, val);
	}


	void	as_environment::set_member(const tu_string& varname, const as_value& val)
	{
		m_variables.set(varname, val);
	}


	int	as_environment::find_local(const tu_string& varname) const
	// Search the active frame for the named var; return its index
	// in the m_local_frames stack if found.
	// 
	// Otherwise return -1.
	{
		// Linear search sucks, but is probably fine for
		// typical use of local vars in script.  There could
		// be pathological breakdowns if a function has tons
		// of locals though.  The ActionScript bytecode does
		// not help us much by using strings to index locals.

		for (int i = m_local_frames.size() - 1; i >= 0; i--)
		{
			if (m_local_frames[i].m_name.length() == 0)
			{
				// End of local frame; stop looking.
				return -1;
			}
			else if (m_local_frames[i].m_name == varname)
			{
				// Found it.
				return i;
			}
		}
		return -1;
	}


	bool	as_environment::parse_path(const tu_string& var_path, tu_string* path, tu_string* var) const
	// See if the given variable name is actually a sprite path
	// followed by a variable name.  These come in the format:
	//
	// 	/path/to/some/sprite/:varname
	//
	// (or same thing, without the last '/')
	//
	// or
	//	path.to.some.var
	//
	// If that's the format, puts the path part (no colon or
	// trailing slash) in *path, and the varname part (no color)
	// in *var and returns true.
	//
	// If no colon, returns false and leaves *path & *var alone.
	{
		// Search for colon.
		int	colon_index = 0;
		int	var_path_length = var_path.length();
		for ( ; colon_index < var_path_length; colon_index++)
		{
			if (var_path[colon_index] == ':')
			{
				// Found it.
				break;
			}
		}

		if (colon_index >= var_path_length)
		{
			// No colon.  Is there a '.'?  Find the last
			// one, if any.
			for (colon_index = var_path_length - 1; colon_index >= 0; colon_index--)
			{
				if (var_path[colon_index] == '.')
				{
					// Found it.
					break;
				}
			}
			if (colon_index < 0) return false;
		}

		// Make the subparts.

		// Var.
		*var = &var_path[colon_index + 1];

		// Path.
		if (colon_index > 0)
		{
			if (var_path[colon_index - 1] == '/')
			{
				// Trim off the extraneous trailing slash.
				colon_index--;
			}
		}
		// @@ could be better.  This whole usage of tu_string is very flabby...
		*path = var_path;
		path->resize(colon_index);

		return true;
	}


	movie*	as_environment::find_target(const as_value& val) const
	// Find the sprite/movie represented by the given value.  The
	// value might be a reference to the object itself, or a
	// string giving a relative path name to the object.
	{
		if (val.get_type() == as_value::OBJECT)
		{
			return val.to_object()->to_movie();
		}
		else if (val.get_type() == as_value::STRING)
		{
			return find_target(val.to_tu_string());
		}
		else
		{
			log_error("error: invalid path; neither string nor object");
			return NULL;
		}
	}


	static const char*	next_slash_or_dot(const char* word)
	// Search for next '.' or '/' character in this word.  Return
	// a pointer to it, or to NULL if it wasn't found.
	{
		for (const char* p = word; *p; p++)
		{
			if (*p == '.' && p[1] == '.')
			{
				p++;
			}
			else if (*p == '.' || *p == '/')
			{
				return p;
			}
		}

		return NULL;
	}


	movie*	as_environment::find_target(const tu_string& path) const
	// Find the sprite/movie referenced by the given path.
	{
		if (path.length() <= 0)
		{
			return m_target;
		}

		assert(path.length() > 0);

		movie*	env = m_target;
		assert(env);
		
		const char*	p = path.c_str();
		tu_string	subpart;

		if (*p == '/')
		{
			// Absolute path.  Start at the root.
			env = env->get_relative_target("_level0");
			p++;
		}


		for (;;)
		{
			const char*	next_slash = next_slash_or_dot(p);
			subpart = p;
			if (next_slash == p)
			{
				log_error("error: invalid path '%s'\n", path.c_str());
				break;
			}
			else if (next_slash)
			{
				// Cut off the slash and everything after it.
				subpart.resize(next_slash - p);
			}

			env = env->get_relative_target(subpart);
			//@@   _level0 --> root, .. --> parent, . --> this, other == character

			if (env == NULL || next_slash == NULL)
			{
				break;
			}

			p = next_slash + 1;
		}
		return env;
	}


	//
	// event_id
	//

	const tu_string&	event_id::get_function_name() const
	{
		static tu_string	s_function_names[EVENT_COUNT] =
		{
			"",		// INVALID
			"",		// PRESS
			"",		// RELEASE
			"",		// RELEASE_OUTSIDE
			"", 		// ROLL_OVER
			"",		// ROLL_OUT
			"",		// DRAG_OVER
			"",		// DRAG_OUT
			"",		// KEY_PRESS
			"",		// INITIALIZE
			"onLoad",	// LOAD
			"onUnload",	// UNLOAD
			"onEnterFrame",	// ENTER_FRAME
			"onMouseDown",	// MOUSE_DOWN
			"onMouseUp",	// MOUSE_UP
			"onMouseMove",	// MOUSE_MOVE
			"onKeyDown",	// KEY_DOWN
			"onKeyUp",	// KEY_UP
			"onData",	// DATA
		};

		assert(m_id > INVALID && m_id < EVENT_COUNT);
		return s_function_names[m_id];
	}


	//
	// Disassembler
	//


#define COMPILE_DISASM 1

#ifndef COMPILE_DISASM

	void	log_disasm(const unsigned char* instruction_data)
	// No disassembler in this version...
	{
		log_msg("<no disasm>\n");
	}

#else // COMPILE_DISASM

	void	log_disasm(const unsigned char* instruction_data)
	// Disassemble one instruction to the log.
	{
		enum arg_format {
			ARG_NONE = 0,
			ARG_STR,
			ARG_HEX,	// default hex dump, in case the format is unknown or unsupported
			ARG_U8,
			ARG_U16,
			ARG_S16,
			ARG_PUSH_DATA,
			ARG_DECL_DICT };
		struct inst_info
		{
			int	m_action_id;
			const char*	m_instruction;

			arg_format	m_arg_format;
		};

		static inst_info	s_instruction_table[] = {
			{ 0x04, "next_frame", ARG_NONE },
			{ 0x05, "prev_frame", ARG_NONE },
			{ 0x06, "play", ARG_NONE },
			{ 0x07, "stop", ARG_NONE },
			{ 0x08, "toggle_qlty", ARG_NONE },
			{ 0x09, "stop_sounds", ARG_NONE },
			{ 0x0A, "add", ARG_NONE },
			{ 0x0B, "sub", ARG_NONE },
			{ 0x0C, "mul", ARG_NONE },
			{ 0x0D, "div", ARG_NONE },
			{ 0x0E, "equ", ARG_NONE },
			{ 0x0F, "lt", ARG_NONE },
			{ 0x10, "and", ARG_NONE },
			{ 0x11, "or", ARG_NONE },
			{ 0x12, "not", ARG_NONE },
			{ 0x13, "str_eq", ARG_NONE },
			{ 0x14, "str_len", ARG_NONE },
			{ 0x15, "substr", ARG_NONE },
			{ 0x17, "pop", ARG_NONE },
			{ 0x18, "floor", ARG_NONE },
			{ 0x1C, "get_var", ARG_NONE },
			{ 0x1D, "set_var", ARG_NONE },
			{ 0x20, "set_target_exp", ARG_NONE },
			{ 0x21, "str_cat", ARG_NONE },
			{ 0x22, "get_prop", ARG_NONE },
			{ 0x23, "set_prop", ARG_NONE },
			{ 0x24, "dup_sprite", ARG_NONE },
			{ 0x25, "rem_sprite", ARG_NONE },
			{ 0x26, "trace", ARG_NONE },
			{ 0x27, "start_drag", ARG_NONE },
			{ 0x28, "stop_drag", ARG_NONE },
			{ 0x29, "str_lt", ARG_NONE },
			{ 0x30, "random", ARG_NONE },
			{ 0x31, "mb_length", ARG_NONE },
			{ 0x32, "ord", ARG_NONE },
			{ 0x33, "chr", ARG_NONE },
			{ 0x34, "get_timer", ARG_NONE },
			{ 0x35, "substr_mb", ARG_NONE },
			{ 0x36, "ord_mb", ARG_NONE },
			{ 0x37, "chr_mb", ARG_NONE },
			{ 0x3A, "delete", ARG_NONE },
			{ 0x3B, "delete_all", ARG_NONE },
			{ 0x3C, "set_local", ARG_NONE },
			{ 0x3D, "call_func", ARG_NONE },
			{ 0x3E, "return", ARG_NONE },
			{ 0x3F, "mod", ARG_NONE },
			{ 0x40, "new", ARG_NONE },
			{ 0x41, "decl_local", ARG_NONE },
			{ 0x42, "decl_array", ARG_NONE },
			{ 0x43, "decl_obj", ARG_NONE },
			{ 0x44, "type_of", ARG_NONE },
			{ 0x45, "get_target", ARG_NONE },
			{ 0x46, "enumerate", ARG_NONE },
			{ 0x47, "add_t", ARG_NONE },
			{ 0x48, "lt_t", ARG_NONE },
			{ 0x49, "eq_t", ARG_NONE },
			{ 0x4A, "number", ARG_NONE },
			{ 0x4B, "string", ARG_NONE },
			{ 0x4C, "dup", ARG_NONE },
			{ 0x4D, "swap", ARG_NONE },
			{ 0x4E, "get_member", ARG_NONE },
			{ 0x4F, "set_member", ARG_NONE },
			{ 0x50, "inc", ARG_NONE },
			{ 0x51, "dec", ARG_NONE },
			{ 0x52, "call_method", ARG_NONE },
			{ 0x53, "new_method", ARG_NONE },
			{ 0x54, "is_inst_of", ARG_NONE },
			{ 0x55, "enum_object", ARG_NONE },
			{ 0x60, "bit_and", ARG_NONE },
			{ 0x61, "bit_or", ARG_NONE },
			{ 0x62, "bit_xor", ARG_NONE },
			{ 0x63, "shl", ARG_NONE },
			{ 0x64, "asr", ARG_NONE },
			{ 0x65, "lsr", ARG_NONE },
			{ 0x66, "eq_strict", ARG_NONE },
			{ 0x67, "gt_t", ARG_NONE },
			{ 0x68, "gt_str", ARG_NONE },
			
			{ 0x81, "goto_frame", ARG_U16 },
			{ 0x83, "get_url", ARG_STR },
			{ 0x87, "store_register", ARG_U8 },
			{ 0x88, "decl_dict", ARG_DECL_DICT },
			{ 0x8A, "wait_for_frame", ARG_HEX },
			{ 0x8B, "set_target", ARG_STR },
			{ 0x8C, "goto_frame_lbl", ARG_STR },
			{ 0x8D, "wait_for_fr_exp", ARG_HEX },
			{ 0x94, "with", ARG_U16 },
			{ 0x96, "push_data", ARG_PUSH_DATA },
			{ 0x99, "goto", ARG_S16 },
			{ 0x9A, "get_url2", ARG_HEX },
			{ 0x9B, "func", ARG_HEX },
			{ 0x9D, "branch_if_true", ARG_S16 },
			{ 0x9E, "call_frame", ARG_HEX },
			{ 0x9F, "goto_frame_exp", ARG_HEX },
			{ 0x00, "<end>", ARG_NONE }
		};

		int	action_id = instruction_data[0];
		inst_info*	info = NULL;

		for (int i = 0; ; i++)
		{
			if (s_instruction_table[i].m_action_id == action_id)
			{
				info = &s_instruction_table[i];
			}

			if (s_instruction_table[i].m_action_id == 0)
			{
				// Stop at the end of the table and give up.
				break;
			}
		}

		arg_format	fmt = ARG_HEX;

		// Show instruction.
		if (info == NULL)
		{
			log_msg("<unknown>[0x%02X]", action_id);
		}
		else
		{
			log_msg("%-15s", info->m_instruction);
			fmt = info->m_arg_format;
		}

		// Show instruction argument(s).
		if (action_id & 0x80)
		{
			assert(fmt != ARG_NONE);

			int	length = instruction_data[1] | (instruction_data[2] << 8);

			// log_msg(" [%d]", length);

			if (fmt == ARG_HEX)
			{
				for (int i = 0; i < length; i++)
				{
					log_msg(" 0x%02X", instruction_data[3 + i]);
				}
				log_msg("\n");
			}
			else if (fmt == ARG_STR)
			{
				log_msg(" \"");
				for (int i = 0; i < length; i++)
				{
					log_msg("%c", instruction_data[3 + i]);
				}
				log_msg("\"\n");
			}
			else if (fmt == ARG_U8)
			{
				int	val = instruction_data[3];
				log_msg(" %d\n", val);
			}
			else if (fmt == ARG_U16)
			{
				int	val = instruction_data[3] | (instruction_data[4] << 8);
				log_msg(" %d\n", val);
			}
			else if (fmt == ARG_S16)
			{
				int	val = instruction_data[3] | (instruction_data[4] << 8);
				if (val & 0x8000) val |= ~0x7FFF;	// sign-extend
				log_msg(" %d\n", val);
			}
			else if (fmt == ARG_PUSH_DATA)
			{
				log_msg("\n");
				int i = 0;
				while (i < length)
				{
					int	type = instruction_data[3 + i];
					i++;
					log_msg("\t\t");	// indent
					if (type == 0)
					{
						// string
						log_msg("\"");
						while (instruction_data[3 + i])
						{
							log_msg("%c", instruction_data[3 + i]);
							i++;
						}
						i++;
						log_msg("\"\n");
					}
					else if (type == 1)
					{
						// float (little-endian)
						union {
							float	f;
							Uint32	i;
						} u;
						compiler_assert(sizeof(u) == sizeof(u.i));

						memcpy(&u.i, instruction_data + 3 + i, 4);
						u.i = swap_le32(u.i);
						i += 4;

						log_msg("(float) %f\n", u.f);
					}
					else if (type == 2)
					{
						log_msg("NULL\n");
					}
					else if (type == 3)
					{
						log_msg("undef\n");
					}
					else if (type == 4)
					{
						// contents of register
						int	reg = instruction_data[3 + i];
						i++;
						log_msg("reg[%d]\n", reg);
					}
					else if (type == 5)
					{
						int	bool_val = instruction_data[3 + i];
						i++;
						log_msg("bool(%d)\n", bool_val);
					}
					else if (type == 6)
					{
						// double
						// wacky format: 45670123
						union {
							double	d;
							Uint64	i;
							struct {
								Uint32	lo;
								Uint32	hi;
							} sub;
						} u;
						compiler_assert(sizeof(u) == sizeof(u.i));

						memcpy(&u.sub.hi, instruction_data + 3 + i, 4);
						memcpy(&u.sub.lo, instruction_data + 3 + i + 4, 4);
						u.i = swap_le64(u.i);
						i += 8;

						log_msg("(double) %f\n", u.d);
					}
					else if (type == 7)
					{
						// int32
						Sint32	val = instruction_data[3 + i]
							| (instruction_data[3 + i + 1] << 8)
							| (instruction_data[3 + i + 2] << 16)
							| (instruction_data[3 + i + 3] << 24);
						i += 4;
						log_msg("(int) %d\n", val);
					}
					else if (type == 8)
					{
						int	id = instruction_data[3 + i];
						i++;
						log_msg("dict_lookup[%d]\n", id);
					}
					else if (type == 9)
					{
						int	id = instruction_data[3 + i] | (instruction_data[3 + i + 1] << 8);
						i += 2;
						log_msg("dict_lookup_lg[%d]\n", id);
					}
				}
			}
			else if (fmt == ARG_DECL_DICT)
			{
				int	i = 0;
				int	count = instruction_data[3 + i] | (instruction_data[3 + i + 1] << 8);
				i += 2;

				log_msg(" [%d]\n", count);

				// Print strings.
				for (int ct = 0; ct < count; ct++)
				{
					log_msg("\t\t");	// indent

					log_msg("\"");
					while (instruction_data[3 + i])
					{
						// safety check.
						if (i >= length)
						{
							log_msg("<disasm error -- length exceeded>\n");
							break;
						}

						log_msg("%c", instruction_data[3 + i]);
						i++;
					}
					log_msg("\"\n");
					i++;
				}
			}
		}
		else
		{
			log_msg("\n");
		}
	}

#endif // COMPILE_DISASM


};


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
