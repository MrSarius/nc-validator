#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "statistics.h"

FILE *fptr;
struct arguments myargs;

void init_stats(struct arguments a)
{
    if (a.csv == NULL)
    {
        return;
    }

    myargs = a;

    if ((fptr = fopen(myargs.csv, "a")) == NULL)
    {
        printf("Error opening statistics file\n");
        exit(1);
    }
    // todo: before this print a json of the configuration
    fprintf(fptr, "gf,gen_size,frame_size,frames_sent,frames_delivered,frames_dropped,loss_rate,linear_dependent,percentage_linear_dependent\n");
}

void close_stats()
{
    if (fptr)
    {
        fclose(fptr);
    }
}

void update_statistics(size_t frames_delivered, size_t frames_dropped)
{
    if (fptr)
    {
        size_t frames_sent = frames_delivered + frames_dropped;
        size_t linear_dependent = frames_delivered - myargs.generation_size;
        float percentage_linear_dependent = (linear_dependent * 1.0 / frames_delivered) * 100;
        fprintf(fptr, "%d,%ld,%ld,%ld,%ld,%ld,%f,%ld,%f\n", myargs.gftype, myargs.generation_size, myargs.packet_size, frames_sent, frames_delivered, frames_dropped, myargs.loss_rate, linear_dependent, percentage_linear_dependent);
    }
}