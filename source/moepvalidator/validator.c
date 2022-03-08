
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
    // size: n_packets x packet_size
    char *data;
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
    return (float)r / (RAND_MAX);

}

int randint(int from, int to){
    long int r = random();
    double diff = to - from;
    return (int) (diff*r/(RAND_MAX) + from);
}
u8 randbyte(){
    return (u8) randint(0, 255);
}

void randbytes(size_t n, char *a){
    for (size_t i = 0; i < n; i++){
        a[i] = randbyte();
    }
}




bool assert_equal(struct generation *gen_a, struct generation *gen_b){
    if(gen_a->packet_size != gen_b->packet_size || gen_a->n_packets != gen_b->n_packets)
        return false;
    if(memcmp(gen_a->data, gen_b->data, gen_a->n_packets*gen_a->packet_size) == 0)
        return true;
    else
        return false;
}

void assert(bool exp, char *msg){
    // TODO: do some logging if exp is false -> should not happen
}

struct generation* empty_generation(u64 packet_size, u64 generation_size){
    size_t len = packet_size*generation_size;
    char *data = malloc(sizeof(char)*len);
    struct generation *gen_new = malloc(sizeof(struct generation));
    gen_new->data = data;
    gen_new->packet_size = packet_size;
    gen_new->generation_size = generation_size;
    gen_new->n_packets = 0;
    return gen_new;
}

struct generation* create_generation(u64 packet_size, u64 generation_size){
    size_t len = packet_size*generation_size;
    struct generation *gen_new = empty_generation(packet_size, generation_size);
    randbytes(len, gen_new->data);
    gen_new->n_packets = generation_size;
    return gen_new;

}

int create_at_A(struct generation *gen_a, rlnc_block_t rlnc_block_a, size_t ith){
    size_t ps = gen_a->packet_size;
    return rlnc_block_add(rlnc_block_a, (int)ith, (const uint8_t*) (gen_a->data + ith*ps), ps);
}

int transmit_A2B(float loss_rate, rlnc_block_t rlnc_block_a, rlnc_block_t rlnc_block_b, size_t packet_size){
    uint8_t *dst = malloc(sizeof(uint8_t)*packet_size);

    ssize_t re = rlnc_block_encode(rlnc_block_a, dst, packet_size, 0);
    assert(re == packet_size, "");
    float r = randf();
    if(r <= loss_rate){
        // simulated packet loss
        free(dst);
        return -1;
    }
    re = rlnc_block_decode(rlnc_block_b, dst, packet_size);
    // TODO check the return values

    // moeprln copies data so we have to free it
    free(dst);
    return 0;
}

int consume_at_B(rlnc_block_t rlnc_block_b, struct generation *gen_b, size_t packet_size, size_t consumed_packets){
    ssize_t	re = rlnc_block_get(rlnc_block_b, (int)consumed_packets, (uint8_t*) (gen_b->data + consumed_packets*packet_size), packet_size);
    if(re == 0){
        // no new packet could be decoded
        return -1;
    }
    assert(re == packet_size, "");
    return 0;
}


int validate(u64 iterations, u64 packet_size, u64 generation_size, float loss_rate, unsigned int seed){
    // TODO: add loop
    struct generation *gen_a;
    struct generation *gen_b;
    float r;
    rlnc_block_t rlnc_block_a;
    rlnc_block_t rlnc_block_b;
    size_t created_packets; 
    size_t consumed_packets; 
    // TODO: maybe randomly choose gftype
    enum MOEPGF_TYPE gftype = MOEPGF256;
    int re_val;

    set_seed(seed);
    for (size_t i = 0; i < iterations; i++)
    {
        created_packets = 0;
        consumed_packets = 0;
        gen_a = create_generation(packet_size, generation_size);
        gen_b = empty_generation(packet_size, generation_size);
        rlnc_block_a = rlnc_block_init((int)generation_size, (size_t)(generation_size*packet_size), (size_t) packet_size, gftype);
        rlnc_block_b = rlnc_block_init((int)generation_size, (size_t)(generation_size*packet_size), (size_t) packet_size, gftype);
        while (gen_a->n_packets != gen_b->n_packets)
        {
            r = randf();
            if (r < 1/3 && created_packets < generation_size)
            {
                re_val = create_at_A(gen_a, rlnc_block_a, created_packets++);
            }else if (r < 1/3 && r < 2/3)
            {
                re_val = transmit_A2B(loss_rate, rlnc_block_a, rlnc_block_b, packet_size);
            }else{
                re_val = consume_at_B(rlnc_block_b, gen_b, packet_size, consumed_packets);
                // TODO statistics
                if(re_val != 0){
                    // the next packet could not be decoded
                }else{
                    // the next packet could be decoded and is in gen_b
                    consumed_packets++;
                }
            }

            // TODO: log rank of the matrix
        }
        
        if(!assert_equal(gen_a, gen_b)){
            // error
            // TODO maybe use errno
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