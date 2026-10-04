#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>

int main() {
    int a, b;
    int result;

    puts("-------=[ BAD INTEGERS ]=-------");
    puts("|-< Enter two numbers to add >-|\n");

    printf("]>> ");
    scanf("%d", &a);

    printf("]>> ");
    scanf("%d", &b);

    result = a + b;

    if (a < 0 || b < 0) {
        puts("\n[o.O] Hmmm... that was a Good try!");
    }
    else if (result < 0) {
        printf("\n[*] C: %d", result);
        puts("\n[*] Popped Shell");
        puts("[*] Switching to interactive mode");
        system("cmd.exe");
    }
    else {
        printf("\n[*] ADDING %d + %d", a, b);
        printf("\n[*] RESULT: %d\n", result);
    }

    return 0;
}