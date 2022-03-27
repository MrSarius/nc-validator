#include <math.h>
#include <moeprlnc/rlnc.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ALIGNMENT 32
#define GEN_SIZE 100
#define FRAME_SIZE 100
#define GF_TYPE 0                   
#define AMOUNT_TRANSMISSIONS GEN_SIZE  // transmissions per iteration
#define ITERATIONS 10000

bool iterate(unsigned int seed);
bool check_if_fully_decoded(rlnc_block_t b);
size_t aligned_length(size_t len, size_t alignment);
double calc_expected_prob();

double calc_expected_prob() {
    size_t N = GEN_SIZE;
    size_t q = -1;
    if (GF_TYPE == 0) {
        q = 2;
    } else if (GF_TYPE == 1) {
        q = 4;
    } else if (GF_TYPE == 2) {
        q = 16;
    } else if (GF_TYPE == 3) {
        q = 256;
    }

    double res = 1.0;
    for (size_t i = 1; i <= N; i++) {
        res *= 1 - (pow(1, i - 1) / pow(q, N));
    }
    return res;
}

size_t aligned_length(size_t len, size_t alignment) {
    return (((len + alignment - 1) / alignment) * alignment);
}

int main() {
    size_t i;
    size_t succ_decoded = 0;
    for (i = 0; i < ITERATIONS; i++) {
        printf("Start Iteration #%ld: ", i);
        if (iterate(i)) {
            succ_decoded += 1;
            printf("success\n");
        } else {
            printf("failure\n");
        }
    }
    double expected = calc_expected_prob();
    printf("\n#### RESULTS: ####\ngf_type: %d\nExpected: %f\nMeasured: %ld/%d = %f\n", GF_TYPE,
           expected, succ_decoded, ITERATIONS, succ_decoded * 1.0 / ITERATIONS);
}

// returns true if generation could be decoded after the iteration
bool iterate(unsigned int seed) {
    rlnc_block_t rlnc_block_a;
    rlnc_block_t rlnc_block_b;
    uint8_t *data_a;
    int i;
    int tmp;
    ssize_t re;
    size_t len;
    uint8_t *dst;

    // init Block A
    rlnc_block_a = rlnc_block_init(GEN_SIZE, FRAME_SIZE, ALIGNMENT, GF_TYPE);

    // init Block B
    rlnc_block_b = rlnc_block_init(GEN_SIZE, FRAME_SIZE, ALIGNMENT, GF_TYPE);

    // set same seeds for both blocks
    rlnc_block_set_seed(rlnc_block_a, seed); //ATTENTION. THIS FUNCTION HAS BEEN MANUALLY ADDED.
    rlnc_block_set_seed(rlnc_block_a, seed);

    // init data to be passed to Block A
    data_a = malloc(GEN_SIZE * FRAME_SIZE * sizeof(uint8_t));
    memset(data_a, 42,
           GEN_SIZE * FRAME_SIZE * sizeof(uint8_t));  // set all data to 42

    // pass data to Block A
    for (i = 0; i < GEN_SIZE; i++) {
        tmp = rlnc_block_add(rlnc_block_a, i, data_a + i * FRAME_SIZE,
                             FRAME_SIZE);
        if (tmp != 0) {
            printf("Error1\n");
            exit(-1);
        }
    }

    len = rlnc_block_current_frame_len(rlnc_block_a);
    len = aligned_length(len, ALIGNMENT);
    dst = malloc(sizeof(uint8_t) * len);

    i = 0;
    while (i < AMOUNT_TRANSMISSIONS) {
        re = rlnc_block_encode(rlnc_block_a, dst, len, 0);
        if (re == 0) {
            printf("Error while encoding\n");
            exit(-1);
        }

        re = rlnc_block_decode(rlnc_block_b, dst, re);
        if (re != 0) {
            printf("Error while decoding\n");
            exit(-1);
        }
        i++;
    }
    bool check_decode = check_if_fully_decoded(rlnc_block_b);

    rlnc_block_free(rlnc_block_a);
    rlnc_block_free(rlnc_block_b);

    return check_decode;
}

// returns true if b can be fully decoded
bool check_if_fully_decoded(rlnc_block_t b) {
    size_t sz;
    uint8_t *dst;
    size_t i;

    sz = aligned_length(FRAME_SIZE, ALIGNMENT);
    dst = malloc(sizeof(uint8_t) * sz);

    for (i = 0; i < GEN_SIZE; i++) {
        int tmp = rlnc_block_get(b, i, dst, sz);
        if (tmp == 0) {  // if one is zero, not all pkts in generation have been
                         // decoded yet
            return false;
        }
        if (tmp != FRAME_SIZE) {
            printf(
                "Should Not Happen! Decoded frame bigger than original ones\n");
        }
    }
    return true;
}