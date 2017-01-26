#ifndef XORSHIFT_H
#define XORSHIFT_H

uint32_t xor128();
void initialiseXorState();

static uint32_t state[4] = { 123456789, 362436069, 521288629, 88675123 };

#endif