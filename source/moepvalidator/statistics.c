#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "main.h"

FILE *fptr;
struct arguments args;

void init_stats(struct arguments a){
    if (!a.csv){
        return;
    }

    char filename[100];
    time_t t;
    struct tm *timeptr;
    t = time(NULL);
    timeptr = localtime(&t);

    strftime(filename, sizeof(filename), "statistics_%d-%m-%y-%H-%M.csv", timeptr);
     
    size_t strftime(char *s, size_t maxsize, const char *format, const struct tm *timeptr);
    if ((fptr = fopen(filename, "a")) == NULL){
       printf("Error opening statistics file\n");
       exit(1);
   }
    args = a;
    fprintf(fptr, "iteration,gf,gen_size,frame_size,frames_sent,frames_received,packets_dropped,linear_dependent,frames_dropped,loss_rate\n");
}

void close_stats(){
    if (fptr){
        fclose(fptr);
    }
}

void update_statistics(size_t i, size_t frames_sent, size_t frames_received, size_t frames_dropped){
    if (fptr){
        fprintf(fptr, "%ld,%d,%ld,%ld,%ld,%ld,%ld,%f\n", i, args.gftype, args.generation_size, args.packet_size, frames_sent, frames_received, frames_dropped, args.loss_rate);
        //TODO BEN Pass lost frames to update_statistics()
    }
}