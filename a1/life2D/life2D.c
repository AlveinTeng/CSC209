#include <stdio.h>
#include <stdlib.h>

#include "life2D_helpers.h"


int main(int argc, char **argv) {
  
    if (argc != 4) {
        fprintf(stderr, "Usage: life2D rows cols states\n");
        return 1;
    }

    int num_row = strtol(argv[1], NULL, 10);
    int num_cols = strtol(argv[2], NULL, 10);
    int print = strtol(argv[3], NULL, 10);
    int board[num_row * num_cols];
    
    int k = 0;
    int num;
    while(fscanf(stdin, "%d", &num) == 1){
        board[k] = num;
        k += 1;
    }

    do{
        print_state(board, num_row, num_cols);
        update_state(board, num_row, num_cols);
        print -= 1;
    } while(print > 0);
    // TODO: Implement.
    return 0;
    }
