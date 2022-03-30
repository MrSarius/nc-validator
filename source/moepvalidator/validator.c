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

struct generation
{
    size_t packet_size;
    size_t n_packets;
    size_t generation_size;
    // size: n_packets x packet_size
    uint8_t *data;
};

// TODO: use
struct validation_instance_args
{
    size_t packet_size;
    float loss_rate;
    size_t generation_size;
    struct generation *gen_a;
    struct generation *gen_b;
    rlnc_block_t rlnc_block_a;
    rlnc_block_t rlnc_block_b;
};

void free_gen(struct generation *gen)
{
    free(gen->data);
    free(gen);
}

void free_everything(struct generation *gen_a, struct generation *gen_b, rlnc_block_t rlnc_block_a, rlnc_block_t rlnc_block_b)
{
    free_gen(gen_a);
    free_gen(gen_b);
    rlnc_block_free(rlnc_block_a);
    rlnc_block_free(rlnc_block_b);
}

bool cmp_gen(const struct generation *gen_a, const struct generation *gen_b)
{
    if (gen_a->packet_size != gen_b->packet_size || gen_a->n_packets != gen_b->n_packets)
        return false;
    if (memcmp(gen_a->data, gen_b->data, gen_a->n_packets * gen_a->packet_size) == 0)
        return true;
    else
        return false;
}

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

int create_at_A(struct generation *gen_a, rlnc_block_t rlnc_block_a, size_t ith)
{
    size_t ps;

    ps = gen_a->packet_size;
    return rlnc_block_add(rlnc_block_a, (int)ith, (const uint8_t *)(gen_a->data + ith * ps), ps);
}

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
void print_ith_frame_ab(const struct generation *gen_a, const struct generation *gen_b, size_t packet_size, size_t i)
{
    char *a;
    char *b;

    a = malloc(sizeof(char) * packet_size + 1);
    b = malloc(sizeof(char) * packet_size + 1);
    memcpy(a, gen_a->data + i * packet_size, packet_size);
    memcpy(b, gen_b->data + i * packet_size, packet_size);
    a[packet_size] = '\0';
    b[packet_size] = '\0';
    fprintf(stderr, "Packet a: %s\n", a);
    fprintf(stderr, "Packet b: %s\n", b);
}

size_t pre_fill(size_t i, size_t packet_size, float loss_rate, size_t generation_size, struct generation *gen_a, struct generation *gen_b, rlnc_block_t rlnc_block_a, rlnc_block_t rlnc_block_b){
    // TODO: add bursts
    int re_val;
    size_t frames_created;
    size_t frames_consumed;
    size_t frames_delivered;
    size_t frames_dropped;

    frames_created = 0;
    frames_consumed = 0;
    frames_delivered = 0;
    frames_dropped = 0;

    // pre fill network buffer at A
    for (i = 0; i < generation_size; i++)
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
        }else if (re_val == -2)
        {
            frames_dropped++;
        }
    }

    for (i = 0; i < generation_size; i++)
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
    }
    update_statistics(i, frames_delivered, frames_dropped);
    return frames_delivered;
}

size_t random_order(size_t i, size_t packet_size, float loss_rate, size_t generation_size, struct generation *gen_a, struct generation *gen_b, rlnc_block_t rlnc_block_a, rlnc_block_t rlnc_block_b){
    int re_val;
    float r;
    size_t frames_created;
    size_t frames_consumed;
    size_t frames_delivered;
    size_t frames_dropped;
    size_t needed_transmissions;
    int tmp_rank;
    size_t failed_decodings;

    failed_decodings = 0;
    frames_created = 0;
    frames_consumed = 0;
    frames_delivered = 0;
    frames_dropped = 0;

    needed_transmissions = 0;

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
                if (tmp_rank < (int)generation_size)
                    needed_transmissions++;
            }else if (re_val == -2)
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
    update_statistics(i, frames_delivered, frames_dropped);
    return needed_transmissions;
}

int validate(size_t iterations, size_t packet_size, size_t generation_size, float loss_rate, unsigned int seed, int gftype, bool csv)
{
    struct generation *gen_a;
    struct generation *gen_b;
    // float r;
    rlnc_block_t rlnc_block_a;
    rlnc_block_t rlnc_block_b;
    // size_t frames_created;
    // size_t frames_consumed;
    // size_t frames_delivered;
    // size_t frames_dropped;
    // int re_val;
    size_t i;
    size_t all_linear_independent = 0;
    size_t needed_transmissions;
    // int tmp_rank;

    set_seed(seed);
    for (i = 0; i < iterations; i++)
    {
        logger("Starting iteration #%ld\n", i);
        gen_a = create_generation(packet_size, generation_size);
        gen_b = empty_generation(packet_size, generation_size);
        rlnc_block_a = rlnc_block_init((int)generation_size, packet_size, MEMORY_ALIGNMENT, gftype);
        rlnc_block_b = rlnc_block_init((int)generation_size, packet_size, MEMORY_ALIGNMENT, gftype);

        // TODO: let passed args decide which function we take
        needed_transmissions = pre_fill(i, packet_size, loss_rate, generation_size, gen_a, gen_b, rlnc_block_a, rlnc_block_b);

        if (needed_transmissions == generation_size)
            all_linear_independent++;
        

        if (!cmp_gen(gen_a, gen_b))
        {
            fprintf(stderr, "Generations do not match! This should not happen as they are compared packet-wise before\n");
            free_everything(gen_a, gen_b, rlnc_block_a, rlnc_block_b);
            return -1;
        }

        free_gen(gen_a);
        free_gen(gen_b);
        // TODO: maybe we can randomly reuse the buffer by resetting it instead of freeing and allocating again and again
        // rlnc_block_reset(rlnc_block_a);
        // rlnc_block_reset(rlnc_block_b);
        rlnc_block_free(rlnc_block_a);
        rlnc_block_free(rlnc_block_b);
        logger("Iteration #%ld successfull.\n\n", i);
    }
    logger("linear independet: %lu / %lu\n", all_linear_independent, iterations);
    logger("Chance of linear undependency: %.2f%%\n", (100.0 * all_linear_independent) / iterations);
    logger("Should be: %.2lf\n", 100.0 * prop_linear_independent(generation_size, gftype));
    return 0;
}