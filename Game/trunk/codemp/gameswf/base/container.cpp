// container.cpp	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Some non-inline implementation help for generic containers.


#include "../base/container.h"
#include "../base/utf8.h"
#include "../base/tu_random.h"


void	tu_string::resize(int new_size)
{
	assert(new_size >= 0);

	if (using_heap() == false)
	{
		if (new_size < 15)
		{
			// Stay with internal storage.
			m_local.m_size = (char) (new_size + 1);
			m_local.m_buffer[new_size] = 0;	// terminate
		}
		else
		{
			// need to allocate heap buffer.
			int	capacity = new_size + 1;
			// round up.
			capacity = (capacity + 15) & ~15;
			char*	buf = (char*) tu_malloc(capacity);

			// Copy existing data.
			strcpy(buf, m_local.m_buffer);

			// Set the heap state.
			m_heap.m_buffer = buf;
			m_heap.m_all_ones = char(~0);
			m_heap.m_size = new_size + 1;
			m_heap.m_capacity = capacity;
		}
	}
	else
	{
		// Currently using heap storage.
		if (new_size < 15)
		{
			// Switch to local storage.

			// Be sure to get stack copies of m_heap info, before we overwrite it.
			char*	old_buffer = m_heap.m_buffer;
			int	old_capacity = m_heap.m_capacity;
			UNUSED(old_capacity);

			// Copy existing string info.
			m_local.m_size = (char) (new_size + 1);
			strncpy(m_local.m_buffer, old_buffer, 15);
			m_local.m_buffer[new_size] = 0;	// ensure termination.

			tu_free(old_buffer, old_capacity);
		}
		else
		{
			// Changing size of heap buffer.
			int	capacity = new_size + 1;
			// Round up.
			capacity = (capacity + 15) & ~15;
			if (capacity != m_heap.m_capacity)	// @@ TODO should use hysteresis when resizing
			{
				m_heap.m_buffer = (char*) tu_realloc(m_heap.m_buffer, capacity, m_heap.m_capacity);
				m_heap.m_capacity = capacity;
			}
			// else we're OK with existing buffer.

			m_heap.m_size = new_size + 1;

			// Ensure termination.
			m_heap.m_buffer[new_size] = 0;
		}
	}
}


/*static*/ void	tu_string::encode_utf8_from_wchar(tu_string* result, const wchar_t* wstr)
{
	const wchar_t*	in = wstr;

	// First pass: compute the necessary string length.
	int	bytes_needed = 0;
	char	dummy[10];
	int	offset;
	for (;;)
	{
		Uint32	uc = *in++;
		offset = 0;
		utf8::encode_unicode_character(dummy, &offset, uc);
		bytes_needed += offset;

		assert(offset <= 6);

		if (uc == 0)
		{
			break;
		}
	}

	// Second pass: transfer the data.
	result->resize(bytes_needed - 1);	// resize() adds 1 for the \0 terminator
	in = wstr;
	char*	out = &((*result)[0]);
	offset = 0;
	for (;;)
	{
		assert(offset < bytes_needed);

		Uint32	uc = *in++;
		utf8::encode_unicode_character(out, &offset, uc);

		assert(offset <= bytes_needed);

		if (uc == 0)
		{
			break;
		}
	}

	assert(offset == bytes_needed);
	assert((*result)[offset - 1] == 0);
	assert(result->length() == (int) strlen(result->c_str()));
}


/*static*/ int	tu_string::stricmp(const char* a, const char* b)
{
#ifdef _WIN32
	return ::stricmp(a, b);
#else
	return strcasecmp(a, b);
#endif
}



#ifdef CONTAINER_UNIT_TEST


// Compile this test case with something like:
//
// gcc container.cpp utf8.cpp tu_random.cpp -g -I.. -DCONTAINER_UNIT_TEST -lstdc++ -o container_test
//
//    or
//
// cl container.cpp utf8.cpp tu_random.cpp -Zi -Od -DCONTAINER_UNIT_TEST -I..


void	test_hash()
{
	// Collect a bunch of random key/value pairs.
	array<Uint32>	data;
	for (int i = 0; i < 1000; i++)
	{
		data.push_back(tu_random::next_random());
	}

	// Push into hash.
	hash<Uint32, Uint32>	h;
	{for (int i = 0; i < data.size() / 2; i++)
	{
		h.add(data[i*2], data[i*2 + 1]);

		// Verify the contents of the hash so far.
		for (int j = 0; j < i; j++)
		{
			Uint32	key = data[j*2];
			Uint32	val;
			bool	got = h.get(key, &val);
			assert(got);
			assert(val == data[j*2 + 1]);
		}
	}}

	// Manually copy stuff over to h2, using iterator interface.
	hash<Uint32, Uint32>	h2;
	{for (hash<Uint32, Uint32>::iterator it = h.begin(); it != h.end(); ++it)
	{
		//printf("first = 0x%X, second = 0x%X\n", it->first, it->second);//xxxxx
		assert(h.get(it->first, NULL) == true);

		h2.add(it->first, it->second);

		Uint32	val;
		bool	got = h2.get(it->first, &val);
		assert(got);
		assert(val == it->second);
	}}

	// Verify the contents of h2.
	{for (int i = 0; i < data.size() / 2; i++)
	{
		Uint32	key = data[i*2];
		Uint32	val;
		bool	got = h.get(key, &val);
		assert(got);
		assert(val == data[i*2 + 1]);
	}}

	h.clear();
	assert(h.size() == 0);

	// Verify that h really is missing the stuff it had before, and h2 really has it.
	{for (hash<Uint32, Uint32>::iterator it = h2.begin(); it != h2.end(); ++it)
	{
		assert(h.get(it->first, NULL) == false);
		assert(h2.get(it->first, NULL) == true);
		assert(h.find(it->first) == h.end());
		assert(h2.find(it->first) != h2.end());
	}}
}


void	test_stringi()
{
	tu_stringi	a, b;

	// Equality.
	a = "this is a test";
	b = "This is a test";
	assert(a == b);

	b = "tHiS Is a tEsT";
	assert(a == b);

	a += "Hello";
	b += "hellO";
	assert(a == b);

	tu_string	c(b);
	assert(a.to_tu_string() != c);

	// Ordering.
	a = "a";
	b = "B";
	assert(a < b);

	a = "b";
	b = "A";
	assert(a > b);
}


void	test_stringi_hash()
{
	stringi_hash<int>	a;

	assert(a.is_empty());

	a.add("bobo", 1);

	assert(a.is_empty() == false);

	a.add("hello", 2);
	a.add("it's", 3);
	a.add("a", 4);
	a.add("beautiful day!", 5);

	int	result = 0;
	a.get("boBO", &result);
	assert(result == 1);

	a.set("BObo", 2);
	a.get("bObO", &result);
	assert(result == 2);

	assert(a.is_empty() == false);
	a.clear();
	assert(a.is_empty() == true);

	// Hammer on one key that differs only by case.
	tu_stringi	original_key("thisisatest");
	tu_stringi	key(original_key);
	a.add(key, 1234567);

	int	variations = 1 << key.length();
	for (int i = 0; i < variations; i++)
	{
		// Twiddle the case of the key.
		for (int c = 0; c < key.length(); c++)
		{
			if (i & (1 << c))
			{
				key[c] = toupper(key[c]);
			}
			else
			{
				key[c] = tolower(key[c]);
			}
		}

		a.set(key, 7654321);

		// Make sure original entry was modified.
		int	value = 0;
		a.get(original_key, &value);
		assert(value == 7654321);

		// Make sure hash keys are preserving case.
		assert(a.find(key)->first.to_tu_string() == original_key.to_tu_string());

		// Make sure they're actually the same entry.
		assert(a.find(original_key) == a.find(key));
		
		a.set(original_key, 1234567);
		assert(a.find(key)->second == 1234567);
	}
}



int	main()
{
	printf("sizeof(tu_string) == %d\n", sizeof(tu_string));

	array<tu_string>	storage;
	storage.resize(2);

	tu_string&	a = storage[0];
	tu_string&	b = storage[1];
	a = "test1";
	
	printf("&a = 0x%X, &b = 0x%X\n", int(&a), int(&b));

	printf("%s\n", a.c_str());

	assert(a == "test1");
	assert(a.length() == 5);

	a += "2";
	assert(a == "test12");

	a += "this is some more text";
	assert(a.length() == 28);

	assert(a[2] == 's');
	assert(a[3] == 't');
	assert(a[4] == '1');
	assert(a[5] == '2');
	assert(a[7] == 'h');
	assert(a[28] == 0);

	assert(b.length() == 0);
	assert(b[0] == 0);
	assert(b.c_str()[0] == 0);

	tu_string c = a + b;

	assert(c.length() == a.length());

	c.resize(2);
	assert(c == "te");
	assert(c == tu_string("te"));

	assert(tu_string("fourscore and sevent") == "fourscore and sevent");

	b = "#sacrificial lamb";

	// Test growing & shrinking.
	a = "";
	for (int i = 0; i < 1000; i++)
	{
		assert(a.length() == i);

		if (i == 8)
		{
			assert(a == "01234567");
		}
		else if (i == 27)
		{
			assert(a == "012345678901234567890123456");
		}

		a.resize(a.length() + 1);
		a[a.length() - 1] = '0' + (i % 10);
	}

	{for (int i = 999; i >= 0; i--)
	{
		a.resize(a.length() - 1);
		assert(a.length() == i);

		if (i == 8)
		{
			assert(a == "01234567");
		}
		else if (i == 27)
		{
			assert(a == "012345678901234567890123456");
		}
	}}

	// Test larger shrinking across heap/local boundary.
	a = "this is a string longer than 16 characters";
	a = "short";

	// Test larger expand across heap/local boundary.
	a = "another longer string...";

	assert(b == "#sacrificial lamb");

	test_hash();
	test_stringi();
	test_stringi_hash();

	// TODO: unit tests for array<>, tu_string, string_hash<>

	return 0;
}


#endif // CONTAINER_UNIT_TEST
