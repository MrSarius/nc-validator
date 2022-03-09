#include <argp.h>
#include <moep/system.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "validator.h"

static struct argp_option options[] = {
    {
        .name = "gen_size",
        .key = 'g',
        .arg = "SIZE",
        .flags = 0,
        .doc = "Set the generation size"
     },
     {
        .name = "nr_iterations",
        .key = 'i',
        .arg = "ITER",
        .flags = 0,
        .doc = "Set the amount of test iterations"
     },
     {
        .name = "loss_rate",
        .key = 'l',
        .arg = "LOSS",
        .flags = 0,
        .doc = "Set propability with which a coded packet is lost during transmission"
     },
     {
        .name = "pkt_size",
        .key = 'p',
        .arg = "SIZE",
        .flags = 0,
        .doc = "Set the packet size"
     },
     {
        .name = "seed",
        .key = 's',
        .arg = "ADDR",
        .flags = 0,
        .doc = "Produce verbose output"
     },
     {NULL}
     };

static error_t parse_opt(int key, char *arg, struct argp_state *state);

static char doc[] = "validation tool for the moeprlcn library";

static struct argp argp = {options, parse_opt, NULL, doc};

struct arguments {
    u64 generation_size;
    u64 nr_iterations;
    float loss_rate;
    u64 packet_size;
    u32 seed;
    bool verbose;
} args;

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct arguments *args = state->input;
    char *endptr = NULL;

    switch (key) {
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
            args->loss_rate = strtol(arg, &endptr, 0);
            if (!endptr && endptr != arg + strlen(arg))
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

            // TODO ARGP_KEY_ARG
            // TODO ARGP_KEY_END

        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

int main(int argc, char **argv) {
    memset(&args, 0, sizeof(args));
    // TODO: realistic numbers
    args.generation_size = 100;
    args.nr_iterations = 10;
    args.loss_rate = 0.0;
    args.packet_size = 1500;
    args.seed = 42;
    args.verbose = false;

    argp_parse(&argp, argc, argv, 0, 0, &args);
    setVerbose(args.verbose);

    logger("Logger works fine! %d\n", 42);

    // TODO call here: validate(args.generation_size, ...);

    return 0;
}
