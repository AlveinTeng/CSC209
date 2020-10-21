#include <stdio.h>

#include "benford_helpers.h"

int count_digits(int num) {
    int i = 1;
    while ((num / BASE) >= BASE){
        i += 1;
        num = (num / BASE);
    }
    return i;
}

int get_ith_from_right(int num, int i) {
    //if(i < count_digits(num)){
        int j = 0;
        while(j <= i ){
            num = num / BASE;
            j += 1;
        //}
    }
    return num % BASE;
}

int get_ith_from_left(int num, int i) {
    
    return get_ith_from_right(num, count_digits(num) - i - 1);
}

void add_to_tally(int num, int i, int *tally) {
    int j = get_ith_from_left(num, i);
    tally[j] +=1;
    return;  
}
