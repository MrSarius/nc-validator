
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
#include "util.h"

struct generation{
    size_t packet_size;
    size_t n_packets;
    size_t generation_size;
    // old: 2D array: generation_size x packet_size
    char *data;
    // size_t packet_size;
    // size_t generation_size;
};

void free_gen(struct generation *gen){
    free(gen->data);
    free(gen);
}

void set_seed(unsigned int seed){
    srandom(seed);
}

float randf(){
    long int r = random();
    return (float)r / (RAND_MAX+1);

}

int randint(int from, int to){
    long int r = random();
    double diff = to - from;
    return (int) (diff*r/(RAND_MAX+1) + from);
}
u8 randbyte(){
    return (u8) randint(0, 255);
}

void randbytes(size_t n, char *a){
    for (size_t i = 0; i < n; i++){
        a[i] = randbyte();
    }
}

// struct generation* create_generation(u64 packet_size, u64 generation_size){
//     size_t len = packet_size*generation_size;
//     char *data = malloc(sizeof(char)*len);
//     randbytes((size_t)(packet_size*generation_size), data);
//     struct generation *gen_new = malloc(sizeof(struct generation));
//     gen_new->data = data;
//     gen_new->n_packets = generation_size;
//     gen_new->packet_size = packet_size;
//     gen_new->generation_size = generation_size;
//     // *gen_new = {packet_size, generation_size, data};
//     // *gen = gen_new;
//     return gen_new;

//     // return data;
//     // this should return a rlnc block
//     // rlnc_block_t gen_a = empty_generation(packet_size, generation_size);
//     // rlnc_block_add(gen_a, pv, a, len);
// }

struct generation* create_generation(u64 packet_size, u64 generation_size){
    size_t len = packet_size*generation_size;
    struct generation *gen_new = empty_generation(packet_size, generation_size);
    randbytes(len, gen_new->data);
    gen_new->n_packets = generation_size;
    return gen_new;

    // return data;
    // this should return a rlnc block
    // rlnc_block_t gen_a = empty_generation(packet_size, generation_size);
    // rlnc_block_add(gen_a, pv, a, len);
}

// void assert_equal(char *a, char *b, size_t n){
// }

bool assert_equal(struct generation *gen_a, struct generation *gen_b){
    if(gen_a->packet_size != gen_b->packet_size || gen_a->n_packets != gen_b->n_packets)
        return false;
    if(memcmp(gen_a->data, gen_b->data, gen_a->n_packets*gen_a->packet_size) == 0)
        return true;
    else
        return false;
}

struct generation* empty_generation(u64 packet_size, u64 generation_size){
    // TODO: field size as parameter
    // return rlnc_block_init((int)generation_size, (size_t)(generation_size*packet_size), (size_t) packet_size, MOEPGF256);
    size_t len = packet_size*generation_size;
    char *data = malloc(sizeof(char)*len);
    struct generation *gen_new = malloc(sizeof(struct generation));
    gen_new->data = data;
    gen_new->packet_size = packet_size;
    gen_new->generation_size = generation_size;
    gen_new->n_packets = 0;
    return gen_new;
}

void create_at_A(struct generation *gen_a, rlnc_block_t rlnc_block_a){

}

void transmit_A2B(float loss_rate, rlnc_block_t rlnc_block_a, rlnc_block_t rlnc_block_b){

}

void consume_at_B(rlnc_block_t rlnc_block_b){
    // todo statistics: log if new block could be decoded
    // TODO: how to return a decoded block

}


void validate(u64 iterations, u64 packet_size, u64 generation_size, float loss_rate, unsigned int seed){
    // TODO: add loop
    struct generation *gen_a;
    struct generation *gen_b;
    float r;
    rlnc_block_t rlnc_block_a;
    rlnc_block_t rlnc_block_b;

    set_seed(seed);
    for (u64 i = 0; i < iterations; i++)
    {
        /* code */
        // TODO: maybe randomly choose gftype
        gen_a = create_generation(packet_size, generation_size);
        gen_b = empty_generation(packet_size, generation_size);
        rlnc_block_a = rlnc_block_init((int)generation_size, (size_t)(generation_size*packet_size), (size_t) packet_size, MOEPGF256);
        rlnc_block_b = rlnc_block_init((int)generation_size, (size_t)(generation_size*packet_size), (size_t) packet_size, MOEPGF256);
        while (gen_a->n_packets != gen_b->n_packets)
        {
            r = randf();
            if (r < 1/3)
            {
                create(gen_a, rlnc_block_a);
            }else if (r < 1/3 && r < 2/3)
            {
                transmit(loss_rate);
            }else{
                consume(rlnc_block_b);
            }

        }
        
        if(!assert_equal(gen_a, gen_b)){
            // error
            // TODO maybe use errno
            return -1;
        }

        free_gen(gen_a);
        free_gen(gen_b);
    }
    return 0;
    
}