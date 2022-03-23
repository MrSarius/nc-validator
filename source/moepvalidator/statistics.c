#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define PATH "./statistics.csv"

FILE *fptr;

void init_stats(){
    if ((fptr = fopen(PATH, "a")) == NULL){
       printf("Error opening statistics file\n");
       exit(1);
   }
   fprintf(fptr, "Attr1,Attr2,Attr3\n");
}

void close_stats(){
    fclose(fptr);
}

void update_statistics(){
    if (fptr){
        //TODO
    }
}