#include <stdbool.h>
#include <string.h>
#include <stdio.h>


int main() {
    // char *tooLong = "nicknamedecisamentetroppolungo";
    char empty[] = "           \n  ";
    char withSpaces[] = "     padding      ";

    // withSpaces[10] = 0;
    // printf("%s\n", withSpaces);

    printf( "stringa prima: %s\n", empty);
    printf("Stringa dopo: %s\n", trim(empty));
    printf( "stringa prima: %s\n", withSpaces);
    printf("Stringa dopo: %s\n", trim(withSpaces));

    return 0;
}
