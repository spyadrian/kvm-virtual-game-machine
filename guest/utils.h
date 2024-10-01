#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdint.h>
#include <sys/types.h>

void srand(u_int _seed);
u_int rand();

void memset(void *dst, uint8_t value, u_int count);
void memcpy(void *dst, void *src, u_int count);

#endif