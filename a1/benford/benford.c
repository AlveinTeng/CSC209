#include <stdio.h>
#include <stdlib.h>

#include "benford_helpers.h"

/*
 * The only print statement that you may use in your main function is the following:
 * - printf("%ds: %d\n")
 *
 */
int main(int argc, char **argv) {
    int total[BASE];
    for(int i =0; i< BASE; i++){
        total[i] = 0;
    }
    int num;
    FILE *f;
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "benford position [datafile]\n");
        return 1;
    }
    int position = strtol(argv[1], NULL, BASE);
    if(argc == 2){
        while(fscanf(stdin, "%d", &num) == 1){
            add_to_tally(num, position, total);
        }

    }else{
        f = fopen(argv[2], "r");
        if(f == NULL){
            fprintf(stderr, "Error opening input file\n");
            return 1;
        }
        while(fscanf(f, "%d", &num) == 1){
            add_to_tally(num, position, total);
        }
        
    }
    for(int i = 0; i< BASE; i++){
        printf("%ds: %d\n", i, total[i]);
    }
    return 0;
}
