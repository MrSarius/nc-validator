#ifndef __UTIL_H_
#define __UTIL_H_

#include <stdbool.h>

void setVerbose(bool v);

void logger(const char *format, ...);

double prop_linear_independent(size_t generation_size, int gftype);

void assert(bool exp, const char *format, ...);

void set_seed(unsigned int seed);

float randf();

int randint(int from, int to);

uint8_t randbyte();

void randbytes(size_t n, uint8_t *a);

size_t aligned_length(size_t len, size_t alignment);

#endif