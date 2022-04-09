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
#include <unistd.h>
#include <string.h>
#include <endian.h>
#include <signal.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <moep/system.h>
#include <moep/modules/moep80211.h>
#include <moep/modules/ieee8023.h>
#include <moeprlnc/rlnc.h>
#include <stdarg.h>
#include <stdbool.h>

#include "validator.h"
#include "util.h"
#include "statistics.h"

#define MEMORY_ALIGNMENT 32

/**
 * Abstraction of a generation
 */
struct generation
{
    size_t packet_size;
    size_t n_packets;
    size_t generation_size;
    // size: n_packets x packet_size
    uint8_t *data;
};

/**
 * Free generation
 *
 * @param gen generation to be freed
 */
void free_gen(struct generation *gen)
{
    free(gen->data);
    free(gen);
}

/**
 * Frees both generations and rlnc blocks
 *
 * @param gen_a
 * @param gen_b
 * @param rlnc_block_a
 * @param rlnc_block_b
 */
void free_everything(struct generation *gen_a, struct generation *gen_b, rlnc_block_t rlnc_block_a, rlnc_block_t rlnc_block_b)
{
    free_gen(gen_a);
    free_gen(gen_b);
    rlnc_block_free(rlnc_block_a);
    rlnc_block_free(rlnc_block_b);
}

/**
 * Returns true if both generations are equal
 *
 * @param gen_a
 * @param gen_b
 */
bool cmp_gen(const struct generation *gen_a, const struct generation *gen_b)
{
    if (gen_a->packet_size != gen_b->packet_size || gen_a->n_packets != gen_b->n_packets)
        return false;
    if (memcmp(gen_a->data, gen_b->data, gen_a->n_packets * gen_a->packet_size) == 0)
        return true;
    else
        return false;
}

/**
 * Create new empty generation containing random bytes
 *
 * @param packet_size packet size of new generation
 * @param generation_size generation size of new generation
 */
struct generation *empty_generation(size_t packet_size, size_t generation_size)
{
    size_t len;
    uint8_t *data;
    struct generation *gen_new;

    len = packet_size * generation_size;
    data = malloc(sizeof(uint8_t) * len);
    gen_new = malloc(sizeof(struct generation));
    gen_new->data = data;
    gen_new->packet_size = packet_size;
    gen_new->generation_size = generation_size;
    gen_new->n_packets = 0;
    return gen_new;
}

/**
 * Create new generation containing random bytes
 *
 * @param packet_size packet size of new generation
 * @param generation_size generation size of new generation
 */
struct generation *create_generation(size_t packet_size, size_t generation_size)
{
    size_t len;
    struct generation *gen_new;

    len = packet_size * generation_size;
    gen_new = empty_generation(packet_size, generation_size);
    randbytes(len, gen_new->data);
    gen_new->n_packets = generation_size;
    return gen_new;
}

/**
 * Take the ith data from the generation and add it to the rlnc_block
 *
 * @param gen_a generation
 * @param rlnc_block_a rlnc_block
 * @param ith index of frame in generation
 */
int create_at_A(struct generation *gen_a, rlnc_block_t rlnc_block_a, size_t ith)
{
    size_t ps;

    ps = gen_a->packet_size;
    return rlnc_block_add(rlnc_block_a, (int)ith, (const uint8_t *)(gen_a->data + ith * ps), ps);
}

/**
 * Transmits frame from rlnc_block_a to rlnc_block_b.
 *
 * @param loss_rate loss rate with which a frame might be lost in transmission
 * @param rlnc_block_a
 * @param rlnc_block_b
 * @param frames_created frames created in current iteration
 */
int transmit_A2B(float loss_rate, rlnc_block_t rlnc_block_a, rlnc_block_t rlnc_block_b, size_t frames_created)
{
    ssize_t frame_size;
    size_t sz;
    uint8_t *dst;
    ssize_t re;
    float r;

    frame_size = rlnc_block_current_frame_len(rlnc_block_a);
    sz = aligned_length(frame_size, MEMORY_ALIGNMENT);
    dst = malloc(sizeof(uint8_t) * sz);
    re = rlnc_block_encode(rlnc_block_a, dst, sz, 0);

    assert(!(re > sz && re != -1), "transmit_A2B: rlnc_block_encode returned incoherent size!\n");
    assert(!(frames_created == 0 && re != -1), "Return of rlnc_block_encode in transmit_A2B was %i instead of -1 (no packets previously added)", re);
    if (re == -1)
    {
        // empty block
        free(dst);
        return -1;
    }

    r = randf();
    if (r <= loss_rate)
    {
        // simulated packet loss
        free(dst);
        return -2;
    }
    re = rlnc_block_decode(rlnc_block_b, dst, re);
    assert(re == 0, "Return of rlnc_block_decode in transmit_A2B was %i instead of 0", re);

    // moeprln copies data so we have to free it
    free(dst);
    return 0;
}

/**
 * Consume frame from rlnc_block_b and add it to gen_b, if available
 *
 * @param rlnc_block_b loss rate with which a frame might be lost in transmission
 * @param gen_b
 * @param packet_size
 * @param consumed_packets packets consumed so far in the current iteration
 */
int consume_at_B(rlnc_block_t rlnc_block_b, struct generation *gen_b, size_t packet_size, size_t consumed_packets)
{
    size_t sz;
    uint8_t *dst;
    ssize_t re;

    sz = aligned_length(packet_size, MEMORY_ALIGNMENT);
    dst = malloc(sizeof(uint8_t) * sz);
    re = rlnc_block_get(rlnc_block_b, (int)consumed_packets, (uint8_t *)dst, sz);

    assert(re <= sz, "consume_at_B: rlnc_block_get returned incoherent size! max=%lu, return=%li\n", sz, re);

    if (re == 0)
    {
        // no new packet could be decoded
        free(dst);
        return -1;
    }
    // if next packet could be decoded then the returned data should have the size of a packet
    assert(re == packet_size, "consume_at_B: return size is unequal packet size! packet=%lu, return=%li\n", packet_size, re);
    memcpy(gen_b->data + consumed_packets * packet_size, dst, re);
    gen_b->n_packets++;
    free(dst);
    return 0;
}

/**
 * Prints the indexes and byte data of the positions which do not match in the ith frame of the two given generations.
 * 
 * @param gen_a generation A
 * @param gen_b generation B to which A should be compared
 * @param ith position of the frame in the generation. Only this frame is beeing compared
 * @param packet_size 
 */
void print_pkt_diff(const struct generation *gen_a, const struct generation *gen_b, size_t ith, size_t packet_size)
{
    uint8_t *data_a;
    uint8_t *data_b;

    data_a = gen_a->data + ith * packet_size;
    data_b = gen_b->data + ith * packet_size;
    fprintf(stderr, "Frame diff in detail:\n");
    for (size_t i = 0; i < packet_size; i++)
    {
        if (data_a[i] != data_b[i])
            fprintf(stderr, "%zu: %i != %i\n", i, data_a[i], data_b[i]);
    }
}

/**
 * Prints ith frame (the whole frame) from generation a and b, both in a separate line such that one can compare them easily.
 * 
 * @param gen_a 
 * @param gen_b 
 * @param packet_size 
 * @param i ith frame to print.
 */
void print_ith_frame_ab(const struct generation *gen_a, const struct generation *gen_b, size_t packet_size, size_t i)
{
    char a[packet_size + 1];
    char b[packet_size + 1];

    memcpy(a, gen_a->data + i * packet_size, packet_size);
    memcpy(b, gen_b->data + i * packet_size, packet_size);
    a[packet_size] = '\0';
    b[packet_size] = '\0';
    fprintf(stderr, "Packet a: %s\n", a);
    fprintf(stderr, "Packet b: %s\n", b);
}

/**
 * One of two modes that define the order how frames are generated, transmitted and decoded. This mode uses the following predefined order of these events:
 * First, all frames are added to A, then randomly coded frames are transmitted to B until B has full rank. Finally, B decodes/observes the frames and compares
 * them to the originally added ones. This mode is mainly used for statistical purposes as we can exactly calculate the decoding probability after N linear coded
 * frames and compare it to the measured probability in order to detect any problems with the pseudorandomness with libmoeprlnc. Note that the comparison only
 * works when the seed of the rlnc_block is set. See the readme for further details.
 * 
 * 
 * @param i 
 * @param packet_size 
 * @param loss_rate 
 * @param generation_size 
 * @param gen_a 
 * @param gen_b 
 * @param rlnc_block_a 
 * @param rlnc_block_b 
 * @return size_t 
 */
size_t pre_fill(size_t i, size_t packet_size, float loss_rate, size_t generation_size, struct generation *gen_a, struct generation *gen_b, rlnc_block_t rlnc_block_a, rlnc_block_t rlnc_block_b)
{
    int re_val;
    size_t frames_created;
    size_t frames_consumed;
    size_t frames_delivered;
    size_t frames_dropped;
    size_t j;

    frames_created = 0;
    frames_consumed = 0;
    frames_delivered = 0;
    frames_dropped = 0;

    // pre fill network buffer at A
    for (j = 0; j < generation_size; j++)
    {
        re_val = create_at_A(gen_a, rlnc_block_a, frames_created++);
        assert(re_val == 0, "rlnc_block_add should have returned 0 but return %i instead\n", re_val);
    }

    // send until B has full rank
    while (rlnc_block_rank_decode(rlnc_block_b) != generation_size)
    {
        re_val = transmit_A2B(loss_rate, rlnc_block_a, rlnc_block_b, frames_created);
        if (re_val == 0)
        {
            frames_delivered++;
        }
        else if (re_val == -2)
        {
            frames_dropped++;
        }
    }

    for (j = 0; j < generation_size; j++)
    {

        re_val = consume_at_B(rlnc_block_b, gen_b, packet_size, frames_consumed);
        assert(re_val == 0, "consume_at_B returned %i instead of 0 which means decoding was not possible. However, in this postion it should be possbile as the matrix already as full rank\n", re_val);

        // check if generations match
        if (memcmp(gen_a->data + frames_consumed * packet_size, gen_b->data + frames_consumed * packet_size, packet_size) != 0)
        {
            fprintf(stderr, "Generations do not match!\n");
            print_ith_frame_ab(gen_a, gen_b, packet_size, i);
            print_pkt_diff(gen_a, gen_b, frames_consumed, packet_size);
            free_everything(gen_a, gen_b, rlnc_block_a, rlnc_block_b);
            return -1;
        }
        // the next packet could be decoded and is in gen_b
        frames_consumed++;
        // print_ith_frame_ab(gen_a, gen_b, packet_size, i);
    }
    update_statistics(frames_delivered, frames_dropped, 0);
    size_t linear_dependent = frames_delivered - generation_size;
    return linear_dependent == 0;
}

/**
 * One of two modes that define the order how frames are generated, transmitted and decoded. This mode uses a pseudo random (defined by the seed) order of of these
 * events. Thus, it can happen we try to send decoded frames even though we haven't add any to the rlnc_block in A. This will result in a warning from libmoeprlnc.
 * However, this warning can be ignored as it is handled correctly by this function. Furthermore, it can also happen that more frames are transmitted than actually
 * needed by B, e.g. when A transmitts even though B has already full rank. These frames are denoted as frames_delivered_after_full_rank in the statistics csv.
 * Another problem that comes with random order is that in the beginning when not all frames have been added to A, the frames send to B are coded only from
 * a subset of all frames (namely the ones that A already knows of). Thus, this invalidates the statistics of decidability after N frames, which makes
 * this mode more or less useless for statistical analysis.
 * 
 * @param i 
 * @param packet_size 
 * @param loss_rate 
 * @param generation_size 
 * @param gen_a 
 * @param gen_b 
 * @param rlnc_block_a 
 * @param rlnc_block_b 
 * @return size_t 
 */
size_t random_order(size_t i, size_t packet_size, float loss_rate, size_t generation_size, struct generation *gen_a, struct generation *gen_b, rlnc_block_t rlnc_block_a, rlnc_block_t rlnc_block_b)
{
    int re_val;
    float r;
    size_t frames_created;
    size_t frames_consumed;
    size_t frames_delivered;
    size_t frames_dropped;
    size_t frames_delivered_after_full_rank;
    int tmp_rank;
    size_t failed_decodings;

    failed_decodings = 0;
    frames_created = 0;
    frames_consumed = 0;
    frames_delivered = 0;
    frames_dropped = 0;

    frames_delivered_after_full_rank = 0;

    while (gen_a->n_packets != gen_b->n_packets)
    {
        r = randf();
        if (r < 1. / 3 && frames_created < generation_size)
        {
            re_val = create_at_A(gen_a, rlnc_block_a, frames_created++);
        }
        else if (r > 1. / 3 && r < 2. / 3)
        {
            tmp_rank = rlnc_block_rank_decode(rlnc_block_b);
            re_val = transmit_A2B(loss_rate, rlnc_block_a, rlnc_block_b, frames_created);
            if (re_val == 0)
            {
                frames_delivered++;
                if (tmp_rank == (int)generation_size)
                    frames_delivered_after_full_rank++;
            }
            else if (re_val == -2)
            {
                frames_dropped++;
            }
        }
        else
        {
            re_val = consume_at_B(rlnc_block_b, gen_b, packet_size, frames_consumed);

            if (re_val != 0)
            {
                // the next packet could not be decoded
                failed_decodings++;
            }
            else
            {
                // check if generations match
                if (memcmp(gen_a->data + frames_consumed * packet_size, gen_b->data + frames_consumed * packet_size, packet_size) != 0)
                {
                    fprintf(stderr, "Generations do not match!\n");
                    print_ith_frame_ab(gen_a, gen_b, packet_size, frames_consumed);
                    print_pkt_diff(gen_a, gen_b, frames_consumed, packet_size);
                    free_everything(gen_a, gen_b, rlnc_block_a, rlnc_block_b);
                    return -1;
                }
                // the next packet could be decoded and is in gen_b
                frames_consumed++;
            }
        }
        // TODO: log rank of the matrix
    }
    update_statistics(frames_delivered, frames_dropped, frames_delivered_after_full_rank);
    size_t linear_dependent = frames_delivered - frames_delivered_after_full_rank - generation_size;
    return linear_dependent == 0;
}

/**
 * Validates the rlnc library according to the parameters. Returns 0 if successfull.
 *
 * @param iterations Amount of iteration to be executed
 * @param packet_size data length of the generation
 * @param generation_size size of the generations
 * @param loss_rate rate with which information might be lost in transmission
 * @param seed seed for the random generator
 * @param gftype underlying Galois Field.
 * @param csv True if intermediate statistics should be printed to a CSV file in the current working directory after each iteration
 * @param prefill True if validation should be done in so called prefill mode
 */
int validate(size_t iterations, size_t packet_size, size_t generation_size, float loss_rate, unsigned int seed, int gftype, bool csv, bool prefill)
{
    struct generation *gen_a;
    struct generation *gen_b;
    rlnc_block_t rlnc_block_a;
    rlnc_block_t rlnc_block_b;
    size_t i;
    size_t all_linear_independent = 0;
    bool linear_dependency;
    float r = 0.0;

    set_seed(seed);
    for (i = 0; i < iterations; i++)
    {
        logger("Starting iteration #%ld\n", i);
        gen_a = create_generation(packet_size, generation_size);
        gen_b = empty_generation(packet_size, generation_size);
        if (i == 0 || r < 0.5)
        {
            // block has been reset for r>=0.5
            // we only need to init the block in the beginning and not when it has been reset
            rlnc_block_a = rlnc_block_init((int)generation_size, packet_size, MEMORY_ALIGNMENT, gftype);
            rlnc_block_b = rlnc_block_init((int)generation_size, packet_size, MEMORY_ALIGNMENT, gftype);
        }
        // rlnc_block_set_seed(rlnc_block_a, i);
        // rlnc_block_set_seed(rlnc_block_b, i);

        if (prefill)
        {
            linear_dependency = pre_fill(i, packet_size, loss_rate, generation_size, gen_a, gen_b, rlnc_block_a, rlnc_block_b);
        }
        else
        {
            linear_dependency = random_order(i, packet_size, loss_rate, generation_size, gen_a, gen_b, rlnc_block_a, rlnc_block_b);
        }

        if (linear_dependency)
            all_linear_independent++;

        if (!cmp_gen(gen_a, gen_b))
        {
            fprintf(stderr, "Generations do not match! This should not happen as they are compared packet-wise before\n");
            free_everything(gen_a, gen_b, rlnc_block_a, rlnc_block_b);
            return -1;
        }

        free_gen(gen_a);
        free_gen(gen_b);
        // randomly reuse the buffer by resetting it instead of freeing and allocating again and again
        r = randf();
        if (i == iterations - 1 || r < 0.5)
        {
            rlnc_block_free(rlnc_block_a);
            rlnc_block_free(rlnc_block_b);
        }
        else
        {
            rlnc_block_reset(rlnc_block_a);
            rlnc_block_reset(rlnc_block_b);
        }

        logger("Iteration #%ld successfull.\n\n", i);
    }
    // TODO log all linear independent, mean std
    // average amount of frames needed
    logger("\n#### RESULTS: ####\ngf_type: %d, generation_size: %ld, packet_size: %ld, iterations: %ld\nExpected decoding prob.: %.4f\nMeasured decoding prob.: %ld/%ld = %.4f\n", gftype,
           generation_size, packet_size, iterations,
           prop_linear_independent(generation_size, gftype), all_linear_independent, iterations, all_linear_independent * 1.0 / iterations);
    if (!prefill)
        logger("Note: As you are running in random order mode you can ignore the expected decoding probability. For more details see the readme file.\n");
    // The generation at A slowly fills and thus there are many linear depent packets in the beginning in this mode.")
    return 0;
}