#include <stdio.h>

int main() {
    char phone[11];
    int num;

    scanf("%s %d", phone, &num);

    if (num == -1){
        printf("%s", phone);
        return 0;
    } else if (num <= 9 && num >= 0){
        printf("%c", phone[num]);
        return 0;
    } else{
        printf("ERROR");
        return 1;
    }
}
    
    

