#include "utils.h"

#define X 123456789
#define Y 987654321
#define Z 43219876

static uint32_t seed = 6543217;
static uint32_t x = X;
static uint32_t y = Y;
static uint32_t z = Z;

void srand(u_int _seed) {
    seed = _seed;
    x = X;
    y = Y;
    z = Z;
}

// Implementation of Marsaglia JKISS RNG uses 64-bit value and 2x multiplies
// Source: https://github.com/SpiNNakerManchester/spinn_common/tree/master
u_int rand() {
    x = 314527869 * x + 1234567;
    y ^= y << 5;
    y ^= y >> 7;
    y ^= y << 22;
    uint64_t t = 4294584393ULL * z + seed;
    seed = t >> 32;
    z = t;

    return (uint32_t) (x + y + z);
}

void memset(void *dst, uint8_t value, u_int count) {
    uint8_t *d = dst;
    while (count--) {
        *d++ = value;
    }
}

void memcpy(void *dst, void *src, u_int count) {
    uint8_t *d = dst;
    uint8_t *s = src;
    while (count--)
        *d++ = *s++;
}
