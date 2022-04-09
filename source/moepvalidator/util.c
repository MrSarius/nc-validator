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

/**
 * Logs passed string if verbose is set to true
 *
 * @param format format string
 */
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


/**
 * Returns the propability that a generation can be decoded after sending exactly 'generation_size' packets
 *
 * @param generation_size Size of the respective generation
 * @param gftype gftype of respective generation (0,1,2,3)
 */
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
        out *= (1 - pow(q, (int)i - 1 - (int)generation_size));
    }
    return out;
}


/**
 * Small helper that prints message if condition does not hold.
 *
 * @param exp condition
 * @param format format string to be printed
 */
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

/**
 * Sets the seed for the random generator
 *
 * @param seed seed
 */
void set_seed(unsigned int seed)
{
    srandom(seed);
}

/**
 * Returns a random float between 0 and 1
 */
float randf()
{
    long int r;

    r = random();
    return (float)r / (RAND_MAX);
}


/**
 * Returns a random integer between from and to
 * 
 * @param from
 * @param to
 */
int randint(int from, int to)
{
    long int r;
    double diff;

    r = random();
    diff = to - from;
    return (int)(diff * r / (RAND_MAX) + from);
}

/**
 * Return a random byte
 */
uint8_t randbyte()
{
    return (uint8_t)randint(0, 255);
}


/**
 * Fills a with n random bytes
 * 
 * @param n
 * @param a
 */
void randbytes(size_t n, uint8_t *a)
{
    size_t i;
    for (i = 0; i < n; i++)
    {
        a[i] = randbyte();
    }
}

/**
 * Returns the next largest multiple of allignment to length. Intended to allocate alligned memory for the rlnc  functions
 * 
 * @param len length to be alligned
 * @param alignment chosen allignment
 */
size_t aligned_length(size_t len, size_t alignment)
{
    return (((len + alignment - 1) / alignment) * alignment);
}