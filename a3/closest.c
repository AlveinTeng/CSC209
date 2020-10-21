#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

#include "point.h"
#include "utilities_closest.h"
#include "serial_closest.h"
#include "parallel_closest.h"


void print_usage() {
    fprintf(stderr, "Usage: closest -f filename -d pdepth\n\n");
    fprintf(stderr, "    -d Maximum process tree depth\n");
    fprintf(stderr, "    -f File that contains the input points\n");

    exit(1);
}

int main(int argc, char **argv) {
    int n = -1;
    long pdepth = -1;
    char *filename = NULL;
    int pcount = 0;
    int fflag = 0;
    int dflag = 0;
    int c;

    // TODO: Parse the command line arguments
    while((c = getopt(argc, argv,"fd:"))!= -1){
        switch(c)
            {
            case 'f':// get the filename
                fflag = 1;
                break;
            case 'd':// get the depth
                dflag = 1;
                if ((pdepth =atoi(optarg))== 0){
                    print_usage();    
                }
                
                break;
            default:
                print_usage();
            }
    }
    if(fflag == 0 || dflag == 0){
        print_usage();
    }
    if(pdepth > 8){
        print_usage();
    }
   
    if (optind >=argc){// check if the input is valid
        fprintf(stderr, "Unexpected argument after options\n");
        exit(EXIT_FAILURE);
    }
    filename = argv[optind];// get the filename

    // Read the points
    n = total_points(filename);
    struct Point points_arr[n];
    read_points(filename, points_arr);

    // Sort the points
    qsort(points_arr, n, sizeof(struct Point), compare_x);

    // Calculate the result using the parallel algorithm.
    double result_p = closest_parallel(points_arr, n, pdepth, &pcount);
    printf("The smallest distance: is %.2f (total worker processes: %d)\n", result_p, pcount);

    exit(0);
}
