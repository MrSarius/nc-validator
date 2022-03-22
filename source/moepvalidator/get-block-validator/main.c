#include <moeprlnc/rlnc.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ALLIGNMENT 32
#define GEN_SIZE 100
#define FRAME_SIZE 1500
#define GF_TYPE 3                // GF265
#define AMOUNT_TRANSMISSIONS 100 // transmissions per iteration
#define ITERATIONS 10

void iterate();
bool check_if_fully_decoded(rlnc_block_t b);

int main()
{
    size_t i;

    for (i = 0; i < ITERATIONS; i++)
    {
        printf("Start Iteration #%ld: ", i);
        iterate();
    }
}

void iterate()
{
    rlnc_block_t rlnc_block_a;
    rlnc_block_t rlnc_block_b;
    uint8_t *data_a;
    int i;
    int tmp;
    ssize_t re;
    size_t len;
    uint8_t *dst;

    // init Block A
    rlnc_block_a = rlnc_block_init(GEN_SIZE, FRAME_SIZE, ALLIGNMENT, GF_TYPE);

    // init Block B
    rlnc_block_b = rlnc_block_init(GEN_SIZE, FRAME_SIZE, ALLIGNMENT, GF_TYPE);

    // init data to be passed to Block A
    data_a = malloc(GEN_SIZE * FRAME_SIZE * sizeof(uint8_t));
    memset(data_a, 42,
           GEN_SIZE * FRAME_SIZE *
               sizeof(uint8_t)); // set all data to 42

    // pass data to Block A
    for (i = 0; i < GEN_SIZE; i++)
    {
        tmp = rlnc_block_add(rlnc_block_a, i, data_a + i * FRAME_SIZE,
                             FRAME_SIZE);
        if (tmp != 0)
        {
            printf("Error1\n");
            exit(-1);
        }
    }

    len = rlnc_block_current_frame_len(
        rlnc_block_a);
    len = (((len + ALLIGNMENT - 1) / ALLIGNMENT) * ALLIGNMENT);
    dst = malloc(sizeof(uint8_t) * len);

    i = 0;
    while (i < AMOUNT_TRANSMISSIONS)
    {
        re = rlnc_block_encode(rlnc_block_a, dst, len, 0);
        if (re == 0)
        {
            printf("Error while encoding\n");
            exit(-1);
        }

        re = rlnc_block_decode(rlnc_block_b, dst, len);
        if (re != 0)
        {
            printf("Error while decoding\n");
            exit(-1);
        }
        i++;
    }
    check_if_fully_decoded(rlnc_block_b);
}

bool check_if_fully_decoded(rlnc_block_t b)
{
    size_t sz;
    uint8_t *dst;
    size_t i;

    sz = (((FRAME_SIZE + ALLIGNMENT - 1) / ALLIGNMENT) * ALLIGNMENT);
    dst = malloc(sizeof(uint8_t) * sz);

    for (i = 0; i < GEN_SIZE; i++)
    {
        int tmp = rlnc_block_get(b, i, dst, sz);
        if (tmp == 0)
        { // if one is zero, not all pkts in generation have been decoded yet
            printf("Generation Could not be decoded.\n");
            return false;
        }
        if (tmp != FRAME_SIZE)
        {
            printf("Should Not Happen! Decoded frame bigger than original ones\n");
        }
    }
    printf("Generation could be decoded!\n");
    return true;
}