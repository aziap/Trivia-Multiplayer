#include <stdbool.h>
#include <string.h>
#include <stdio.h>

// Elimina gli spazi bianchi in testa e in coda a una stringa e il carriage return
char* trim(char *str) {
    printf("in trim\n");
    // Comincio dalla fine
    int i = strlen(str) - 1;
    while (i >= 0
        && (str[i] != ' ' || str[i] != '\n' || str[i] != 0) 
    ){
        --i;
    }
    // Stringa vuota, con soli spazi o senza terminatore
    if (i == 0) return "";
    str[i + 1] = 0;

    printf("Tolti gli spazi bianchi in coda\n");

    i = 0;
    int j = 0;

    // Cerco il primo carattere valido
    while (str[i] == ' ') {
        ++i;
    }

    // Shift dei caratteri validi 
    while (str[i] != 0) {
        str[j++] = str[i++];
    }
    printf("Tolti gli spazi bianchi in testa\n");
    return str;
}

int main() {
    // char *tooLong = "nicknamedecisamentetroppolungo";
    char *empty = "           \n  ";
    char *withSpaces = "     padding      ";

    printf( "stringa prima: %s\nStringa dopo: %s\n", empty, trim(empty));
    printf( "stringa prima: %s\nStringa dopo: %s\n", withSpaces, trim(withSpaces));

    return 0;
}