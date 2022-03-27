#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "statistics.h"

FILE *fptr;
struct arguments myargs;

void init_stats(struct arguments a){
    if (!a.csv){
        return;
    }

    char filename[100];
    time_t t;
    struct tm *timeptr;
    t = time(NULL);
    timeptr = localtime(&t);

    strftime(filename, sizeof(filename), "statistics_%d-%m-%y-%H-%M-%S.csv", timeptr);
     
    size_t strftime(char *s, size_t maxsize, const char *format, const struct tm *timeptr);
    if ((fptr = fopen(filename, "a")) == NULL){
       printf("Error opening statistics file\n");
       exit(1);
   }
    myargs = a;
    fprintf(fptr, "iteration,gf,gen_size,frame_size,frames_sent,frames_delivered,frames_dropped,loss_rate,linear_dependent,percentage_linear_dependent\n");
}

void close_stats(){
    if (fptr){
        fclose(fptr);
    }
}

void update_statistics(size_t i, size_t frames_delivered, size_t frames_dropped){
    if (fptr){
        size_t frames_sent = frames_delivered + frames_dropped;
        size_t linear_dependent = frames_delivered - myargs.generation_size;
        float percentage_linear_dependent = (linear_dependent*1.0/frames_delivered)*100;
        fprintf(fptr, "%ld,%d,%ld,%ld,%ld,%ld,%ld,%f,%ld,%f\n", i, myargs.gftype, myargs.generation_size, myargs.packet_size, frames_sent, frames_delivered, frames_dropped, myargs.loss_rate, linear_dependent, percentage_linear_dependent);
    }
}