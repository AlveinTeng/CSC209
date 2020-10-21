#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "point.h"
#include "serial_closest.h"
#include "utilities_closest.h"


/*
 * Multi-process (parallel) implementation of the recursive divide-and-conquer
 * algorithm to find the minimal distance between any two pair of points in p[].
 * Assumes that the array p[] is sorted according to x coordinate.
 */

void *malloc_wrapper(int size) {
    void *ptr;
    if ((ptr = malloc(size)) == NULL) {
        perror("malloc");
        exit(1);}
    return ptr;}
double closest_parallel(struct Point *p, int n, int pdmax, int *pcount) {
    if( n<4 || pdmax == 0){
	double result = closest_serial(p, n);// base case
	return result;
    }else{
        //allocate space for first_half, second_half and an array containing those two
        struct Point *first_half = malloc(sizeof(struct Point)*(n/2));
        struct Point *second_half = malloc(sizeof(struct Point)*(n-n/2));
	    struct Point **divided = malloc(sizeof(struct Point *)*(2));

        //assinging values for first_half and second_half.
        for (int i = 0; i < n/2; i++){
	    first_half[i] = p[i];
        }
        for (int i = n/2; i < n; i++){
	    second_half[i -n/2] = p[i];
        }
        // Create an array identify the child
        int child[2];
        // Create pipe
        int fd[2][2];
        // Create an array to count steps
        int count[2];
        // Create an array to track the status of child
        int status[2];
        // Create the array to contain the result
        double result[2];
        // Create the array to contain the distance
	    double distance[2];
        //put the two parts into the array
	    divided[0] = first_half;
	    divided[1] = second_half;

        // create the pipe
        for(int i =0; i < 2; i++){
            if(pipe(fd[i]) == -1){
                perror("pipe");
                exit(1);
            }
	        child[i] = fork();// create child process
            if((child[i] < 0)){
                perror("fork");
                exit(1);
            }else if((child[i])==0){// child process
                close(fd[i][0]);
                distance[i]= closest_parallel(divided[i], i?(n-n/2):n/2, pdmax-1, pcount);// recursive step

                if(write(fd[i][1], &distance[i], sizeof(double))== -1){// write to the pipe
			perror("write to pipe");
			exit(1);
		}
                exit(*pcount);
            }else if((child[i]) > 0){// parent process
                close(fd[i][1]);
                if(wait(&status[i]) != -1){// wait until both child are done
                    if(WIFEXITED(status[i]) == 0){
                        perror("wait");
                        exit(1);
                    }
		    count[i] = WEXITSTATUS(status[i]);// count the steps
                    if(read(fd[i][0], &result[i], sizeof(double))== -1){// read from the pipe
			perror("read from pipe");
			exit(1);
		    }
		    
                }else{
                    perror("wait");

                    exit(1);
                }
                
                
		 
            }
        }
        // count the total steps
	    *pcount += count[0] + count[1] + 2; 
        // free space
	    free(first_half);
	    free(second_half);
	    free(divided);
        return min(result[0], result[1]);
    }
   
}

