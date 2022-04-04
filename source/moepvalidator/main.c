#include <argp.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "main.h"
#include "util.h"
#include "validator.h"
#include "statistics.h"

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
     .doc = "Set underlaying Galois Field size. (0: GF2, 1: GF4, 2: GF16, 3: "
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
     .doc = "Set probability with which a coded packet is lost during "
            "transmission"},
    {.name = "pkt_size",
     .key = 'p',
     .arg = "SIZE",
     .flags = 0,
     .doc = "Set the packet size"},
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
        if (!strcmp(args->csv, "")){
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
        if ((!endptr && endptr != arg + strlen(arg)) || (args->loss_rate > 1))
            argp_failure(state, 1, errno, "Invalid loss rate: %s", arg);

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

int main(int argc, char **argv)
{
    memset(&args, 0, sizeof(args));
    // TODO: realistic default values
    args.gftype = 3; // GF256
    args.generation_size = 100;
    args.nr_iterations = 10;
    args.loss_rate = 0.0;
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

    bool valid = validate(args.nr_iterations, args.packet_size, args.generation_size, args.loss_rate, args.seed, args.gftype, args.csv) == 0;
    close_stats();
    if (valid)
    {
        printf("Validation program run through without errors!\n");
        return 0;
    }
    else
    {
        printf("Validation program had some errors, see the log above for more information!\n");
        return -1;
    }
}
