#include <stdio.h>
#include <stdbool.h>

struct command{
    unsigned char is_powered : 1;
    unsigned char mode : 2;
    unsigned char speed : 5;
};

int main(){
    printf("%d", sizeof(struct command));
}