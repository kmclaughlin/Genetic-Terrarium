#include "stdafx.h"
#include "Xorshift.h"



uint32_t xor128() {
	uint32_t t;
	t = state[0] ^ (state[0] << 11);
	state[0] = state[1]; state[1] = state[2]; state[2] = state[3];
	return state[3] = state[3] ^ (state[3] >> 19) ^ (t ^ (t >> 8));
}

void initialiseXorState() {
	state[0] = rand();
	state[1] = rand();
	state[2] = rand();
	state[3] = rand();
}
