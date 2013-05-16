// gameswf_action.h	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Implementation and helpers for SWF actions.


#ifndef GAMESWF_ACTION_H
#define GAMESWF_ACTION_H


#include "gameswf.h"
#include "gameswf_types.h"

#include "base/container.h"


namespace gameswf
{
	struct movie;
	struct as_environment;
	struct as_object_interface;
	struct as_value;


	//
	// with_stack_entry
	//
	// The "with" stack is for Pascal-like with-scoping.

	struct with_stack_entry
	{
		as_object_interface*	m_object;
		int	m_block_end_pc;

		with_stack_entry()
			:
			m_object(NULL),
			m_block_end_pc(0)
		{
		}

		with_stack_entry(as_object_interface* obj, int end)
			:
			m_object(obj),
			m_block_end_pc(end)
		{
		}
	};


	// Base class for actions.
	struct action_buffer
	{
		action_buffer();
		void	read(stream* in);
		void	execute(as_environment* env);
		void	execute(
			as_environment* env,
			int start_pc,
			int exec_bytes,
			as_value* retval,
			const array<with_stack_entry>& initial_with_stack);

		bool	is_null()
		{
			return m_buffer.size() < 1 || m_buffer[0] == 0;
		}

		int	get_length() const { return m_buffer.size(); }

	//private:
		// Don't put these as values in array<>!  They contain
		// internal pointers and cannot be moved or copied.
		// If you need to keep an array of them, keep pointers
		// to new'd instances.
		action_buffer(const action_buffer& a) { assert(0); }
		void operator=(const action_buffer& a) { assert(0); }

		void	process_decl_dict(int start_pc, int stop_pc);

		// data:
		array<unsigned char>	m_buffer;
		array<const char*>	m_dictionary;
		int	m_decl_dict_processed_at;
	};


	typedef void (*as_c_function_ptr)(
		as_value* result,
		as_object_interface* this_ptr,
		as_environment* env,
		int nargs,
		int first_arg_bottom_index);

	//
	// as_as_function
	//
	// ActionScript function.

	struct as_as_function : public ref_counted
	{
		action_buffer*	m_action_buffer;
		as_environment*	m_env;	// @@ might need some kind of ref count here, but beware cycles
		array<with_stack_entry>	m_with_stack;	// initial with-stack on function entry.
		int	m_start_pc;
		int	m_length;
		array<tu_string>	m_args;

		// NULL environment is allowed -- if so, then
		// functions will be executed in the caller's
		// environment, rather than the environment where they
		// were defined.
		as_as_function(action_buffer* ab, as_environment* env, int start, const array<with_stack_entry>& with_stack)
			:
			m_action_buffer(ab),
			m_env(env),
			m_with_stack(with_stack),
			m_start_pc(start),
			m_length(0)
		{
			assert(m_action_buffer);
		}

		void	add_arg(const char* name)
		{
			m_args.push_back(tu_string(name));
		}

		void	set_length(int len) { assert(len >= 0); m_length = len; }

		// Dispatch.
		void	operator()(
			as_value* result,
			as_object_interface* this_ptr,
			as_environment* caller_env,
			int nargs,
			int first_arg);
	};


	// ActionScript value type.
	struct as_value
	{
		enum type
		{
			UNDEFINED,
			STRING,
			NUMBER,
			OBJECT,
			C_FUNCTION,
			AS_FUNCTION,	// ActionScript function.
		};

		type	m_type;
		mutable tu_string	m_string_value;
		union
		{
			mutable	double	m_number_value;	// @@ hm, what about PS2, where double is bad?  should maybe have int&float types.
			as_object_interface*	m_object_value;
			as_c_function_ptr	m_c_function_value;
			as_as_function*	m_as_function_value;
		};

		as_value()
			:
			m_type(UNDEFINED),
			m_number_value(0.0)
		{
		}

		as_value(const as_value& v)
			:
			m_type(UNDEFINED),
			m_number_value(0.0)
		{
			*this = v;
		}

		as_value(const char* str)
			:
			m_type(STRING),
			m_string_value(str),
			m_number_value(0.0)
		{
		}

		as_value(const wchar_t* wstr)
			:
			m_type(STRING),
			m_string_value(""),
			m_number_value(0.0)
		{
			// Encode the string value as UTF-8.
			//
			// Is this dumb?  Alternatives:
			//
			// 1. store a tu_wstring instead of tu_string?
			// Bloats typical ASCII strings, needs a
			// tu_wstring type, and conversion back the
			// other way to interface with char[].
			// 
			// 2. store a tu_wstring as a union with
			// tu_string?  Extra complexity.
			//
			// 3. ??
			//
			// Storing UTF-8 seems like a pretty decent
			// way to do it.  Everything else just
			// continues to work.
			tu_string::encode_utf8_from_wchar(&m_string_value, wstr);
		}

		as_value(bool val)
			:
			m_type(NUMBER),
			m_number_value(double(val))
		{
		}

		as_value(int val)
			:
			m_type(NUMBER),
			m_number_value(double(val))
		{
		}

		as_value(float val)
			:
			m_type(NUMBER),
			m_number_value(double(val))
		{
		}

		as_value(double val)
			:
			m_type(NUMBER),
			m_number_value(val)
		{
		}

		as_value(as_object_interface* obj)
			:
			m_type(OBJECT),
			m_object_value(obj)
		{
		}

		as_value(as_c_function_ptr func)
			:
			m_type(C_FUNCTION),
			m_c_function_value(func)
		{
			m_c_function_value = func;
		}

		as_value(as_as_function* func)
			:
			m_type(AS_FUNCTION),
			m_as_function_value(func)
		{
			if (m_as_function_value)
			{
				m_as_function_value->add_ref();
			}
		}

		~as_value() { drop_refs(); }

		// Useful when changing types/values.
		void	drop_refs();

		type	get_type() const { return m_type; }

		const char*	to_string() const;
		const tu_string&	to_tu_string() const;
		const tu_stringi&	to_tu_stringi() const;
		double	to_number() const;
		bool	to_bool() const;
		as_object_interface*	to_object() const;
		as_c_function_ptr	to_c_function() const;
		as_as_function*	to_as_function() const;

		void	convert_to_number();
		void	convert_to_string();

		void	set(const tu_string& str) { drop_refs(); m_type = STRING; m_string_value = str; }
		void	set(const char* str) { drop_refs(); m_type = STRING; m_string_value = str; }
		void	set(double val) { drop_refs(); m_type = NUMBER; m_number_value = val; }
		void	set(bool val) { drop_refs(); m_type = NUMBER; m_number_value = val ? 1.0 : 0.0; }
		void	set(int val) { drop_refs(); set(double(val)); }
		void	set(as_object_interface* obj) { drop_refs(); m_type = OBJECT; m_object_value = obj; }
		void	set(as_c_function_ptr func) { drop_refs(); m_type = C_FUNCTION; m_c_function_value = func; }
		void	set(as_as_function* func)
		{
			if (m_type != AS_FUNCTION || m_as_function_value != func)
			{
				drop_refs();
				m_type = AS_FUNCTION;
				m_as_function_value = func;
				if (m_as_function_value)
				{
					m_as_function_value->add_ref();
				}
			}
		}
		void	set_undefined() { drop_refs(); m_type = UNDEFINED; }

		void	operator=(const as_value& v)
		{
			if (v.m_type == UNDEFINED) set_undefined();
			else if (v.m_type == STRING) set(v.m_string_value);
			else if (v.m_type == NUMBER) set(v.m_number_value);
			else if (v.m_type == OBJECT) set(v.m_object_value);
			else if (v.m_type == C_FUNCTION) set(v.m_c_function_value);
			else if (v.m_type == STRING) set(v.m_string_value);
			else if (v.m_type == AS_FUNCTION) set(v.m_as_function_value);
		}

		bool	operator==(const as_value& v) const;
		bool	operator<(const as_value& v) const { return to_number() < v.to_number(); }
		void	operator+=(const as_value& v) { set(this->to_number() + v.to_number()); }
		void	operator-=(const as_value& v) { set(this->to_number() - v.to_number()); }
		void	operator*=(const as_value& v) { set(this->to_number() * v.to_number()); }
		void	operator/=(const as_value& v) { set(this->to_number() / v.to_number()); }	// @@ check for div/0
		void	operator&=(const as_value& v) { set(int(this->to_number()) & int(v.to_number())); }
		void	operator|=(const as_value& v) { set(int(this->to_number()) | int(v.to_number())); }
		void	operator^=(const as_value& v) { set(int(this->to_number()) ^ int(v.to_number())); }
		void	shl(const as_value& v) { set(int(this->to_number()) << int(v.to_number())); }
		void	asr(const as_value& v) { set(int(this->to_number()) >> int(v.to_number())); }
		void	lsr(const as_value& v) { set(double(Uint32(this->to_number()) >> int(v.to_number()))); }

		void	string_concat(const tu_string& str);
	};


	struct as_property_interface
	{
		virtual bool	set_property(int index, const as_value& val) = 0;
	};


	struct as_object_interface : virtual public ref_counted
	{
		virtual ~as_object_interface() {}

		// So that text_character's can return something reasonable.
		virtual const char*	get_text_value() const = 0;

		virtual void	set_member(const tu_string& name, const as_value& val) = 0;
		virtual bool	get_member(const tu_string& name, as_value* val) = 0;

		virtual movie*	to_movie() = 0;
	};


	// ActionScript "environment", essentially VM state?
	struct as_environment
	{
		array<as_value>	m_stack;
		as_value	m_register[4];
		movie*	m_target;
		stringi_hash<as_value>	m_variables;

		// For local vars.  Use empty names to separate frames.
		struct frame_slot
		{
			tu_string	m_name;
			as_value	m_value;

			frame_slot() {}
			frame_slot(const tu_string& name, const as_value& val) : m_name(name), m_value(val) {}
		};
		array<frame_slot>	m_local_frames;


		as_environment()
			:
			m_target(0)
		{
		}

		movie*	get_target() { return m_target; }
		void	set_target(movie* target) { m_target = target; }

		// stack access/manipulation
		// @@ TODO do more checking on these
		template<class T>
		void	push(T val) { push_val(as_value(val)); }
		void	push_val(const as_value& val) { m_stack.push_back(val); }
		as_value	pop() { as_value result = m_stack.back(); m_stack.pop_back(); return result; }
		as_value&	top(int dist) { return m_stack[m_stack.size() - 1 - dist]; }
		as_value&	bottom(int index) { return m_stack[index]; }
		void	drop(int count) { m_stack.resize(m_stack.size() - count); }

		int	get_top_index() const { return m_stack.size() - 1; }

		as_value	get_variable(const tu_string& varname, const array<with_stack_entry>& with_stack) const;
		// no path stuff:
		as_value	get_variable_raw(const tu_string& varname, const array<with_stack_entry>& with_stack) const;

		void	set_variable(const tu_string& path, const as_value& val, const array<with_stack_entry>& with_stack);
		// no path stuff:
		void	set_variable_raw(const tu_string& path, const as_value& val, const array<with_stack_entry>& with_stack);

		void	set_local(const tu_string& varname, const as_value& val);
		void	add_local(const tu_string& varname, const as_value& val);	// when you know it doesn't exist.
		void	declare_local(const tu_string& varname);	// Declare varname; undefined unless it already exists.

		bool	get_member(const tu_string& varname, as_value* val) const;
		void	set_member(const tu_string& varname, const as_value& val);

		// Parameter/local stack frame management.
		int	get_local_frame_top() const { return m_local_frames.size(); }
		void	set_local_frame_top(int t) { assert(t <= m_local_frames.size()); m_local_frames.resize(t); }
		void	add_frame_barrier() { m_local_frames.push_back(frame_slot()); }

		// Internal.
		int	find_local(const tu_string& varname) const;
		bool	parse_path(const tu_string& var_path, tu_string* path, tu_string* var) const;
		movie*	find_target(const tu_string& path) const;
		movie*	find_target(const as_value& val) const;
	};


	//
	// as_object
	//
	// A generic bag of attributes.  Base-class for ActionScript
	// script-defined objects.
	struct as_object : public as_object_interface
	{
		stringi_hash<as_value>	m_members;

		virtual const char*	get_text_value() const { return NULL; }

		virtual void	set_member(const tu_string& name, const as_value& val)
		{
			m_members.set(name, val);
		}

		virtual bool	get_member(const tu_string& name, as_value* val)
		{
			return m_members.get(name, val);
		}

		virtual movie*	to_movie()
		// This object is not a movie; no conversion.
		{
			return NULL;
		}
	};


	//
	// Some handy helpers
	//

	// Clean up any stray heap stuff we've allocated.
	void	action_clear();

	// Dispatching methods from C++.
	as_value	call_method0(const as_value& method, as_environment* env, as_object_interface* this_ptr);
	as_value	call_method1(
		const as_value& method, as_environment* env, as_object_interface* this_ptr,
		const as_value& arg0);
	as_value	call_method2(
		const as_value& method, as_environment* env, as_object_interface* this_ptr,
		const as_value& arg0, const as_value& arg1);
	as_value	call_method3(
		const as_value& method, as_environment* env, as_object_interface* this_ptr,
		const as_value& arg0, const as_value& arg1, const as_value& arg2);


	const char*	call_method_parsed(as_environment* env, as_object_interface* this_ptr, const char* method_call);
	const char*	call_method_parsed(as_environment* env, as_object_interface* this_ptr, const wchar_t* method_call);

	//
	// event_id
	//
	// For keyDown and stuff like that.

	struct event_id
	{
		// These must match the function names in event_id::get_function_name()
		enum id_code
		{
			INVALID,

			// These are for buttons & sprites.
			PRESS,
			RELEASE,
			RELEASE_OUTSIDE,
			ROLL_OVER,
			ROLL_OUT,
			DRAG_OVER,
			DRAG_OUT,
			KEY_PRESS,

			// These are for sprites only.
			INITIALIZE,
			LOAD,
			UNLOAD,
			ENTER_FRAME,
			MOUSE_DOWN,
			MOUSE_UP,
			MOUSE_MOVE,
			KEY_DOWN,
			KEY_UP,
			DATA,

			EVENT_COUNT
		};

		unsigned char	m_id;
		unsigned char	m_key_code;

		event_id() : m_id(INVALID), m_key_code(key::INVALID) {}

		event_id(id_code id, key::code c = key::INVALID)
			:
			m_id((unsigned char) id),
			m_key_code((unsigned char) c)
		{
			// For the button key events, you must supply a keycode.
			// Otherwise, don't.
			assert((m_key_code == key::INVALID && (m_id != KEY_PRESS))
				|| (m_key_code != key::INVALID && (m_id == KEY_PRESS)));
		}

		bool	operator==(const event_id& id) const { return m_id == id.m_id && m_key_code == id.m_key_code; }

		// Return the name of a method-handler function corresponding to this event.
		const tu_string&	get_function_name() const;
	};


}	// end namespace gameswf


#endif // GAMESWF_ACTION_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
