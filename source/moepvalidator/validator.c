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

#define MEMORY_ALIGNMENT 32

struct generation
{
    size_t packet_size;
    size_t n_packets;
    size_t generation_size;
    // size: n_packets x packet_size
    uint8_t *data;
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

int transmit_A2B(float loss_rate, rlnc_block_t rlnc_block_a, rlnc_block_t rlnc_block_b, size_t created_packets)
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
    assert(!(created_packets == 0 && re != -1), "Return of rlnc_block_encode in transmit_A2B was %i instead of -1 (no packets previously added)", re);
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
        return -1;
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

int validate(size_t iterations, size_t packet_size, size_t generation_size, float loss_rate, unsigned int seed, int gftype)
{
    struct generation *gen_a;
    struct generation *gen_b;
    float r;
    rlnc_block_t rlnc_block_a;
    rlnc_block_t rlnc_block_b;
    size_t created_packets;
    size_t consumed_packets;
    size_t transmitted_packets;
    int re_val;
    size_t i;
    size_t all_linear_independent = 0;

    set_seed(seed);
    for (i = 0; i < iterations; i++)
    {
        logger("Starting iteration #%i\n", (int)i);
        created_packets = 0;
        consumed_packets = 0;
        transmitted_packets = 0;
        gen_a = create_generation(packet_size, generation_size);
        gen_b = empty_generation(packet_size, generation_size);
        rlnc_block_a = rlnc_block_init((int)generation_size, packet_size, MEMORY_ALIGNMENT, gftype);
        rlnc_block_b = rlnc_block_init((int)generation_size, packet_size, MEMORY_ALIGNMENT, gftype);
        while (gen_a->n_packets != gen_b->n_packets)
        {
            r = randf();
            if (r < 1. / 3 && created_packets < generation_size)
            {
                re_val = create_at_A(gen_a, rlnc_block_a, created_packets++);
            }
            else if (r > 1. / 3 && r < 2. / 3)
            {
                re_val = transmit_A2B(loss_rate, rlnc_block_a, rlnc_block_b, created_packets);
                if (re_val == 0)
                {
                    transmitted_packets++;
                }
            }
            else
            {
                re_val = consume_at_B(rlnc_block_b, gen_b, packet_size, consumed_packets);
                // TODO statistics

                if (re_val != 0)
                {
                    // the next packet could not be decoded
                }
                else
                {
                    // check if generations match
                    if (memcmp(gen_a->data + consumed_packets * packet_size, gen_b->data + consumed_packets * packet_size, packet_size) != 0)
                    {
                        fprintf(stderr, "Generations do not match!\n");
                        print_ith_frame_ab(gen_a, gen_b, packet_size, i);
                        print_pkt_diff(gen_a, gen_b, consumed_packets, packet_size);
                        free_everything(gen_a, gen_b, rlnc_block_a, rlnc_block_b);
                        return -1;
                    }
                    // the next packet could be decoded and is in gen_b
                    consumed_packets++;
                }
            }
            // TODO: log rank of the matrix
        }
        if (transmitted_packets == generation_size)
        {
            all_linear_independent++;
        }

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
    }
    return 0;
}