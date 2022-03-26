#ifndef __MAIN_H_
#define __MAIN_H_

#include <stdbool.h>
struct arguments
{
    int gftype;
    size_t generation_size;
    size_t nr_iterations;
    float loss_rate;
    size_t packet_size;
    unsigned int seed;
    bool verbose;
    bool csv;
};

#endif