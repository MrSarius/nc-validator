#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <endian.h>
#include <signal.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <math.h>

#include "util.h"

bool verbose = false;

void setVerbose(bool v) { verbose = v; }

void logger(const char *format, ...)
{
    if (verbose)
    {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }
}

double prop_linear_independent(size_t generation_size, int gftype)
{
    int q;
    size_t i;
    double out = 1.0;

    switch (gftype)
    {
    case 0:
        q = 2;
        break;
    case 1:
        q = 4;
        break;
    case 2:
        q = 16;
        break;
    case 3:
        q = 256;
        break;

    default:
        q = 256;
        break;
    }
    for (i = 1; i <= generation_size; i++)
    {
        out *= (1 - pow(q, i - 1) / pow(q, generation_size));
    }
    return out;
}

void assert(bool exp, const char *format, ...)
{
    if (!exp)
    {
        va_list args;
        va_start(args, format);
        vfprintf(stderr, format, args);
        va_end(args);
        fprintf(stderr, "Exiting...\n");
        fflush(stderr);
        fflush(stdout);
        exit(-1);
    }
}

void set_seed(unsigned int seed)
{
    srandom(seed);
}

float randf()
{
    long int r;

    r = random();
    return (float)r / (RAND_MAX);
}

int randint(int from, int to)
{
    long int r;
    double diff;

    r = random();
    diff = to - from;
    return (int)(diff * r / (RAND_MAX) + from);
}
uint8_t randbyte()
{
    return (uint8_t)randint(0, 255);
}

void randbytes(size_t n, uint8_t *a)
{
    size_t i;
    for (i = 0; i < n; i++)
    {
        a[i] = randbyte();
    }
}

size_t aligned_length(size_t len, size_t alignment)
{
    return (((len + alignment - 1) / alignment) * alignment);
}