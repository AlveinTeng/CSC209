#include <stdio.h>

int main() {
    char phone[11];
    int number;

    scanf("%s", phone);
    int num_error = 0;
    while (1)
    {
        if(scanf("%d", &number) == EOF){
            break;
        }
        if (number == -1){
        printf("%s\n", phone);
        
    } else if (number<= 9 && number >= 0){
        printf("%c\n", phone[number]);
        
    } else{
        printf("ERROR\n");
        num_error = 1;
    }
    
    }
    return num_error;
    

}