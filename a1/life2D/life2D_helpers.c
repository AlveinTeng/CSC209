#include <stdio.h>
#include <stdlib.h>


void print_state(int *board, int num_rows, int num_cols) {
    for (int i = 0; i < num_rows * num_cols; i++) {
        printf("%d", board[i]);
        if (((i + 1) % num_cols) == 0) {
            printf("\n");
        }
    }
    printf("\n");
}

void update_state(int *board, int num_rows, int num_cols) {
    int temp[num_rows * num_cols];
    
    for (int i = 0; i < num_rows * num_cols; i++){
        if(i < num_cols ||  i % num_cols == 0 || i % num_cols == num_cols - 1 || i>= (num_rows - 1 )*num_cols){
            temp[i] = board[i];
        }
        else{
            int count = (board[i-1] + board[i+1] + board[i - num_cols] + board[i- num_cols - 1] + board[i - num_cols +1] + board[i + num_cols] + board[i + num_cols -1] + board[i + num_cols + 1]);
            if(board[i] == 1 && (count > 3|| count < 2)){
                temp[i] = 0;
            }else if(board[i] == 0 && (count == 3 || count == 2)){
                temp[i] = 1;
            }else{
                temp[i] = board[i];
            }
            }
    
    }
    for(int i = 0; i < num_rows * num_cols; i++){
        board[i] = temp[i];
    }

    return;
}
