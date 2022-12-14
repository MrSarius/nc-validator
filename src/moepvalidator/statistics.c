/*
 * Copyright 2021,2022		Tobias Juelg <tobias.juelg@tum.de>
 *				            Ben Riegel <ben.riegel@tum.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * See COPYING for more details.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "statistics.h"

FILE *fptr;
struct arguments myargs;
// TODO: save frames_delivered and frames_delivered_after_full rank in array in order to calculate mean and std of it

/**
 * Call once to initialize the statistics module
 *
 * @param a program arguments
 */
void init_stats(struct arguments a)
{
    if (a.csv == NULL)
    {
        return;
    }

    myargs = a;

    if (access(myargs.csv, F_OK) == 0)
    {
        // csv file already exists
        fprintf(stderr, "Statistics CSV file already exists and would thus be overwritten! Please rename the old file or choose another path.\n");
        exit(1);
    }
    if ((fptr = fopen(myargs.csv, "a")) == NULL)
    {
        fprintf(stderr, "Error opening statistics file\n");
        exit(1);
    }
    fprintf(fptr, "gf,gen_size,frame_size,frames_sent,frames_delivered,frames_dropped,loss_rate,linear_dependent,percentage_linear_dependent,frames_delivered_after_full_rank,prefill\n");
}

/**
 * Closes the open file descriptor of the statistics csv. Note that it is not needed to close the file descriptor for unintended exits as the exit function
 * automatically closes all open file descriptors.
 *
 */
void close_stats()
{
    if (fptr)
    {
        fclose(fptr);
    }
}

/**
 * Adds new line to the statistics csv with data from current iteration
 *
 * @param frames_delivered Frames delivered to destination in current iteration
 * @param frames_dropped Frames dropped in current iteration
 * @param frames_delivered_after_full_rank Number of frames which have been unnecessary delivered as B already had full rank
 */
void update_statistics(size_t frames_delivered, size_t frames_dropped, size_t frames_delivered_after_full_rank)
{
    if (fptr)
    {
        size_t frames_sent = frames_delivered + frames_dropped;
        size_t linear_dependent = frames_delivered - frames_delivered_after_full_rank - myargs.generation_size;
        float percentage_linear_dependent = (linear_dependent * 1.0 / frames_delivered) * 100;
        fprintf(fptr, "%d,%ld,%ld,%ld,%ld,%ld,%f,%ld,%f,%ld,%i\n", myargs.gftype, myargs.generation_size, myargs.packet_size, frames_sent, frames_delivered, frames_dropped, myargs.loss_rate, linear_dependent, percentage_linear_dependent, frames_delivered_after_full_rank, myargs.prefill);
    }
}