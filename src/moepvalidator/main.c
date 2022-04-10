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
#include <argp.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "main.h"
#include "util.h"
#include "validator.h"
#include "statistics.h"

/*
 * Argument parsing
 * ---------------------------------------------------------------------------
 */

static struct argp_option options[] = {
    {.name = "csv_stats",
     .key = 'c',
     .arg = "NAME",
     .flags = 0,
     .doc = "Saves statistics with given name to a CVS file in the current working directory"},
    {.name = "field_size",
     .key = 'f',
     .arg = "SIZE",
     .flags = 0,
     .doc = "Set underlying Galois Field size. (0: GF2, 1: GF4, 2: GF16, 3: "
            "GF256)"},
    {.name = "gen_size",
     .key = 'g',
     .arg = "SIZE",
     .flags = 0,
     .doc = "Set the generation size"},
    {.name = "nr_iterations",
     .key = 'i',
     .arg = "ITER",
     .flags = 0,
     .doc = "Set the amount of test iterations"},
    {.name = "loss_rate",
     .key = 'l',
     .arg = "LOSS",
     .flags = 0,
     .doc = "Set probability with which coded data is lost during transmission"},
    {.name = "mode",
     .key = 'm',
     .arg = 0,
     .flags = 0,
     .doc = "Executes the program in 'pre fill' mode."},
    {.name = "pkt_size",
     .key = 'p',
     .arg = "SIZE",
     .flags = 0,
     .doc = "Set the frame size"},
    {.name = "seed",
     .key = 's',
     .arg = "ADDR",
     .flags = 0,
     .doc = "Set the seed which is used to generate random test input"},
    {.name = "verbose",
     .key = 'v',
     .arg = 0,
     .flags = 0,
     .doc = "Produce verbose output"},
    {NULL}};

static error_t parse_opt(int key, char *arg, struct argp_state *state);

static char doc[] = "validation tool for the moeprlcn library";

static struct argp argp = {
    .options = options, .parser = parse_opt, .args_doc = 0, .doc = doc};

struct arguments args;

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
    struct arguments *args = state->input;
    char *endptr = NULL;

    switch (key)
    {
    case 'c':
        args->csv = arg;
        if (!strcmp(args->csv, ""))
        {
            argp_failure(state, 1, errno, "File name cannot be empty.");
        }
        break;

    case 'f':
        args->gftype = strtol(arg, &endptr, 0);
        if ((args->gftype < 0 || args->gftype > 3) ||
            (!endptr && endptr != arg + strlen(arg)))
            argp_failure(state, 1, errno, "Invalid field size: %s", arg);
        break;

    case 'g':
        args->generation_size = strtol(arg, &endptr, 0);
        if (!endptr && endptr != arg + strlen(arg))
            argp_failure(state, 1, errno, "Invalid generation size: %s",
                         arg);
        break;

    case 'i':
        args->nr_iterations = strtol(arg, &endptr, 0);
        if (!endptr && endptr != arg + strlen(arg))
            argp_failure(state, 1, errno,
                         "Invalid number of iterations: %s", arg);
        break;

    case 'l':
        args->loss_rate = strtof(arg, &endptr);
        if ((!endptr && endptr != arg + strlen(arg)) || (args->loss_rate > 1) || (args->loss_rate < 0))
            argp_failure(state, 1, errno, "Invalid loss rate: %s", arg);

        break;

    case 'm':
        args->prefill = true;
        break;

    case 'p':
        args->packet_size = strtol(arg, &endptr, 0);
        if (!endptr && endptr != arg + strlen(arg))
            argp_failure(state, 1, errno, "Invalid packet size: %s", arg);
        break;

    case 's':
        args->seed = strtol(arg, &endptr, 0);
        if (!endptr && endptr != arg + strlen(arg))
            argp_failure(state, 1, errno, "Invalid seed: %s", arg);
        break;

    case 'v':
        args->verbose = true;
        break;

    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

/*
 * ---------------------------------------------------------------------------
 * /Argument parsing
 */

int main(int argc, char **argv)
{
    memset(&args, 0, sizeof(args));
    // set defaults
    args.gftype = 3; // GF256
    args.generation_size = 100;
    args.nr_iterations = 10;
    args.loss_rate = 0.0;
    args.prefill = false;
    args.packet_size = 1500;
    args.seed = 42;
    args.verbose = false;
    args.csv = NULL;

    argp_parse(&argp, argc, argv, 0, 0, &args);
    setVerbose(args.verbose);
    init_stats(args);

    logger(
        "##### Parameters: #####\nField Size: %ld\nGeneration "
        "Size: %ld\nNumber of Iterations: %ld\nLoss Rate: %f\nPacket Size: "
        "%ld\nSeed: %d\nVerbose: %d\n\n",
        args.gftype, args.generation_size, args.nr_iterations,
        args.loss_rate, args.packet_size, args.seed,
        args.verbose);

    bool valid = validate(args.nr_iterations, args.packet_size, args.generation_size, args.loss_rate, args.seed, args.gftype, args.csv, args.prefill) == 0;
    close_stats();
    if (valid)
    {
        logger("\nSuccessful validation!\n");
        return 0;
    }
    else
    {
        fprintf(stderr, "Validation program had some errors, see the log above for more information!\n");
        return -1;
    }
}
