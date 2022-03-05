
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
#include <moep/modules/rlnc.h>
#include "util.h"


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

rlnc_block_t create_generation(u64 packet_size, u64 generation_size){
    size_t len = packet_size*generation_size;
    char *a = malloc(sizeof(char)*len);
    randbytes((size_t)(packet_size*generation_size), a);
    return a;
    // this should return a rlnc block
    rlnc_block_t gen_a = empty_generation(packet_size, generation_size);
    rlnc_block_add(gen_a, pv, a, len);
}

void assert_equal(char *a, char *b, size_t n){
}

rlnc_block_t empty_generation(u64 packet_size, u64 generation_size){
    // TODO: field size as parameter
    return rlnc_block_init((int)generation_size, (size_t)(generation_size*packet_size), (size_t) packet_size, MOEPGF256);
}

void create(char *gen){

}

void transmit(float loss_rate){

}

void consume(){

}


void validate(u64 iterations, u64 packet_size, u64 generation_size, float loss_rate, unsigned int seed){
    // TODO: add loop
    char *gen_a;
    char *gen_b;
    float r;

    set_seed(seed);
    for (u64 i = 0; i < iterations; i++)
    {
        /* code */
        // TODO: maybe randomly choose gftype
        gen_a = create_generation(packet_size, generation_size);
        while (/* condition */)
        {
            r = randf();
            if (r < 1/3)
            {
                create(gen_a);
            }else if (r < 1/3 && r < 2/3)
            {
                transmit(loss_rate);
            }else{
                consume();
            }

        }
        

        // free(gen_a);
        // free(gen_b);
    }
    


}