/*
 * Copyright 2013, 2014		Maurice Leclaire <leclaire@in.tum.de>
 *				Stephan M. Guenther <moepi@moepi.net>
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
#include <unistd.h>
#include <string.h>
#include <argp.h>
#include <endian.h>
#include <signal.h>

#include <sys/timerfd.h>

#include <arpa/inet.h>

#include <net/ethernet.h>

#include <netinet/in.h>

#include <moep/system.h>

#include <moep/modules/moep80211.h>
#include <moep/modules/ieee8023.h>

// #include "util.h"
// #include "timeout.h"


#define PACKAGE_VERSION		"0.1"
#define PACKAGE_BUGREPORT


const char *argp_program_version = "ptm " PACKAGE_VERSION;
const char *argp_program_bug_address = "<" PACKAGE_BUGREPORT ">";

static char args_doc[] = "IF FREQ";

static char doc[] =
"ptm - a packet transfer module for moep80211\n\n"
"  IF                         Use the radio interface with name IF\n"
"  FREQ                       Use the frequency FREQ [in Hz] for the radio\n"
"                             interface; You can use M for MHz.";

enum fix_args {
	FIX_ARG_IF = 0,
	FIX_ARG_FREQ = 1,
	FIX_ARG_CNT
};

static struct argp_option options[] = {
	{"hwaddr", 'a', "ADDR", 0, "Set the hardware address to ADDR"},
	{"beacon", 'b', "INTERVAL", 0, "Transmit beacons and use an interval of INTERVAL milliseconds"},
	{"gi", 'g', "s|l", 0, "Use [s]hort guard interval or [l]ong guard interval"},
	{"ht", 'h', "BANDWIDTH", OPTION_ARG_OPTIONAL, "Enable HT and optionally set the bandwidth to BANDWIDTH"},
	{"ipaddr", 'i', "ADDR", 0, "Set the ip address to ADDR"},
	{"mtu", 'm', "SIZE", 0, "Set the mtu to SIZE"},
	{"rate", 'r', "RATE|MCS", 0, "Set the legacy rate to RATE*500kbit/s or the mcs index to MCS if HT is used"},
	{}
};

static error_t parse_opt(int key, char *arg, struct argp_state *state);

static struct argp argp = {
	options,
	parse_opt,
	args_doc,
	doc
};


struct arguments {
    u64 nr_iterations;
    u64 packet_size;
    u64 generation_size;
    float loss_rate;
    u32 seed;
    int verbose; // -> logger into util
} args;

// TODO: rewrite parser
static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
	struct arguments *args = state->input;
	char *endptr = NULL;
	int chan_width;
	long long int freq;

	switch (key) {
	case 'a':
		if (!(args->addr = ieee80211_aton(arg)))
			argp_failure(state, 1, errno, "Invalid hardware address");
		break;
	case 'g':
		if (strlen(arg) != 1)
			argp_failure(state, 1, errno, "Invalid guard interval: %s", arg);
		switch (*arg) {
		case 's':
			args->gi = IEEE80211_RADIOTAP_MCS_SGI;
			break;
		case 'l':
			args->gi = 0;
			break;
		default:
			argp_failure(state, 1, errno, "Invalid guard interval: %s", arg);
		}
		break;
	case 'h':
		if (!arg) {
			args->chan_width = MOEP80211_CHAN_WIDTH_20;
			break;
		}
		chan_width = strtol(arg, &endptr, 0);
		switch (chan_width) {
		case 20:
			args->chan_width = MOEP80211_CHAN_WIDTH_20;
			break;
		case 40:
			args->chan_width = MOEP80211_CHAN_WIDTH_40;
			if (!endptr)
				argp_failure(state, 1, errno, "Invalid HT mode: %s", arg);
			switch (*(endptr++)) {
			case '+':
				args->freq1 += 10;
				break;
			case '-':
				args->freq1 += 10;
				break;
			default:
				argp_failure(state, 1, errno, "Invalid HT mode: %s", arg);
			}
			break;
		default:
			argp_failure(state, 1, errno, "Invalid HT mode: %s", arg);
		}
		if (endptr != NULL && endptr != arg + strlen(arg))
			argp_failure(state, 1, errno, "Invalid HT mode: %s", arg);
		break;
	case 'i':
		if (!(inet_aton(arg, &args->ip)))
			argp_failure(state, 1, errno, "Invalid ip address");
		break;
	case 'm':
		args->mtu = strtol(arg, &endptr, 0);
		if (endptr != NULL && endptr != arg + strlen(arg))
			argp_failure(state, 1, errno, "Invalid mtu: %s", arg);
		if (args->mtu <= 0)
			argp_failure(state, 1, errno, "Invalid mtu: %d", args->mtu);
		break;
	case 'b':
		args->beacon = strtol(arg, &endptr, 0);
		if (endptr != NULL && endptr != arg + strlen(arg))
			argp_failure(state, 1, errno, "Invalid beacon interval: %s", arg);
		if (args->beacon <= 0)
			argp_failure(state, 1, errno, "Invalid beacon interval: %d", args->beacon);
		break;
	case 'r':
		args->rate = strtol(arg, &endptr, 0);
		if (endptr != NULL && endptr != arg + strlen(arg))
			argp_failure(state, 1, errno, "Invalid data rate: %s", arg);
		if (args->rate < 0)
			argp_failure(state, 1, errno, "Invalid data rate: %d", args->rate);
		break;
	case ARGP_KEY_ARG:
		switch (state->arg_num) {
		case FIX_ARG_IF:
			args->rad = arg;
			break;
		case FIX_ARG_FREQ:
			freq = strtoll(arg, &endptr, 0);
			while (endptr != NULL && endptr != arg + strlen(arg)) {
				switch (*endptr) {
				case 'k':
				case 'K':
					freq *= 1000;
					break;
				case 'm':
				case 'M':
					freq *= 1000000;
					break;
				case 'g':
				case 'G':
					freq *= 1000000000;
					break;
				default:
					argp_failure(state, 1, errno, "Invalid frequency: %s", arg);
				}
				endptr++;
			}
			if (freq < 0)
				argp_failure(state, 1, errno, "Invalid frequency: %lld", freq);
			args->freq = freq / 1000000;
			args->freq1 += args->freq;
			break;
		default:
			argp_usage(state);
		}
		break;
	case ARGP_KEY_END:
		if (state->arg_num < FIX_ARG_CNT)
			argp_usage(state);
		break;
	default:
		return ARGP_ERR_UNKNOWN;
	}

	return 0;
}



int main(int argc, char **argv)
{

	memset(&args, 0, sizeof(args));
    // TODO: realistic numbers
	args.nr_iterations = 100;
	args.packet_size = 1500;
	args.generation_size = 10;
	args.loss_rate = 0.0;
	args.seed = 42;
    // 0 - 5 where 5 gives you the most output
	args.verbose = 3;

	argp_parse(&argp, argc, argv, 0, 0, &args);

	return 0;
}
