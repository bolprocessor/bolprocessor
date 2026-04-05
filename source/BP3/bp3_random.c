#include "bp3_random.h"

static unsigned int bp3_rng_state = 1;

void bp3_srand(unsigned int seed) {
    bp3_rng_state = seed;
}

int bp3_rand(void) {
    bp3_rng_state = bp3_rng_state * 214013 + 2531011;
    return (bp3_rng_state >> 16) & 0x7fff;
}