// tu_random.cpp	-- Thatcher Ulrich 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Pseudorandom number generator.


#include "../base/tu_random.h"


namespace tu_random
{
	// PRNG code adapted from the complimentary-multiply-with-carry
	// code in the article: George Marsaglia, "Seeds for Random Number
	// Generators", Communications of the ACM, May 2003, Vol 46 No 5,
	// pp90-93.
	//
	// The article says:
	//
	// "Any one of the choices for seed table size and multiplier will
	// provide a RNG that has passed extensive tests of randomness,
	// particularly those in [3], yet is simple and fast --
	// approximately 30 million random 32-bit integers per second on a
	// 850MHz PC.  The period is a*b^n, where a is the multiplier, n
	// the size of the seed table and b=2^32-1.  (a is chosen so that
	// b is a primitive root of the prime a*b^n + 1.)"
	//
	// [3] Marsaglia, G., Zaman, A., and Tsang, W.  Toward a universal
	// random number generator.  _Statistics and Probability Letters
	// 8_ (1990), 35-39.

	const int	SEED_COUNT = 8;
//	const Uint64	a = 123471786;	// for SEED_COUNT=1024
//	const Uint64	a = 123554632;	// for SEED_COUNT=512
//	const Uint64	a = 8001634;	// for SEED_COUNT=255
//	const Uint64	a = 8007626;	// for SEED_COUNT=128
//	const Uint64	a = 647535442;	// for SEED_COUNT=64
//	const Uint64	a = 547416522;	// for SEED_COUNT=32
//	const Uint64	a = 487198574;	// for SEED_COUNT=16
	const Uint64	a = 716514398;	// for SEED_COUNT=8

	// Seeds.  These could be re-initialized from a random source.
	// These particular seeds came out of this perl script, translated
	// from C code in the above article:
	//
	// $j=987654321;
	// for($i=0;$i<8;$i++) {
	//   $j = $j ^ ($j << 13);
	//   $j = $j ^ ($j >> 17);
	//   $j = $j ^ ($j << 5);
	//   print "$j\n";
	// }
        
        // WK: why does this produce warnings under gcc 3.1 / OSX like:
        // tu_random.cpp:58: warning: decimal constant is so large that it is unsigned
        
	static Uint32	Q[8] =
	{
		0x0ECE59F5, //  248404469,
		0x7BE3FAAD, // 2078538413, 
                0x927D4636, // 2457683510,  -1837283786 
		0x6DC8F60B, // 1841886731, 
		0x123C5E6F, //  305946223, 
		0xA7CFA077, // 2815402103, 
		0x2BEC5B77, //  736910199, 
		0xB0DF8DF6, // 2967440886,
	};


	Uint32	next_random()
	// Return the next pseudo-random number in the sequence.
	{
		Uint64	t;
		Uint32	x;

		static Uint32	c = 362436;
		static Uint32	i = SEED_COUNT - 1;
		const Uint32	r = 0xFFFFFFFE;

		i = (i+1) & (SEED_COUNT - 1);
		t = a * Q[i] + c;
		c = (Uint32) (t >> 32);
		x = (Uint32) (t + c);
		if (x < c)
		{
			x++;
			c++;
		}
		
		Uint32	val = r - x;
		Q[i] = val;
		return val;
	}

}	// end namespace tu_random


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
