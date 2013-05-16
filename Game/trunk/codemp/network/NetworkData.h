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
// NetworkData.h
// Jedi Knight Galaxies MMO Networking Changes
// (c) 2013 Jedi Knight Galaxies

#include "../game/z_global_defines.h"

#ifdef __SECONDARY_NETWORK__

#ifndef NETWORK_DATA
#define NETWORK_DATA

#pragma once
#include <string.h>
#include <iostream>
#include "ByteBuffer.h"

using namespace std;

extern "C"
{
#ifdef CGAME
#include "../cgame/cg_local.h"
#include "../game/q_shared.h"
#else //GAME
#include "../game/g_local.h"
#include "../game/q_shared.h"
#endif //GAME
}

#define MAX_PACKET_SIZE 1000000

struct Packet : public ByteBuffer {
public:
	__inline Packet() : ByteBuffer() { }
	__inline Packet(size_t res) : ByteBuffer(res) { }
	__inline Packet(const Packet &packet) : ByteBuffer(packet) {}
	__inline ~Packet() { clear(); }

	//! Clear packet and set opcode all in one mighty blow
	__inline void Initialize()
	{
		clear();
	}

	/*
	void serialize(char * data) {
	memcpy(data, this, sizeof(Packet));
	}

	void deserialize(char * data) {
	memcpy(this, data, sizeof(Packet));
	}
	*/
};

#endif //NETWORK_DATA

#endif //__SECONDARY_NETWORK__
