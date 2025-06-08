#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "input_check.h"


int main() {
    /*
    char tooLong[] = "nicknamedecisamentetroppolungo";
    char empty[] = "           \n  ";
    char withSpaces[] = "     padding      ";
    char gucci[] = "natalino";
    */

    // withSpaces[10] = 0;
    // printf("%s\n", withSpaces);

    // printf( "stringa prima: %s\n", empty);
    // printf("Stringa dopo: %s\n", trim(empty));
    // printf( "stringa prima: %s\n", withSpaces);
    // printf("Stringa dopo: %s\n", trim(withSpaces));

    /*
    printf("Input: %s\n", tooLong);
    printf("Esito: %s\n", (checkNicknameFormat(tooLong)) ? "Ok" : ">:(");
    printf("Input: %s\n", empty);
    printf("Esito: %s\n", (checkNicknameFormat(empty)) ? "Ok" : ">:(");
    printf("Input: %s\n", withSpaces);
    printf("Esito: %s\n", (checkNicknameFormat(withSpaces)) ? "Ok" : ">:(");
    printf("Input: %s\n", gucci);
    printf("Esito: %s\n", (checkNicknameFormat(gucci)) ? "Ok" : ">:(");
    */

    // Controllo sull'opzione
    for (int op = 0; op < 4; op++)
        printf("Opzione %d %s valida\n", op, checkOpzioneMenu(op) ? 
            "" : "non");

    // Controllo sul tema
    for (int t = 0; t < 4; t++)
        printf("Tema %d %s valido\n", t, checkSceltaTema(t) ? 
            "" : "non");
    // Controllo lunghezza risposta
    char r1[DIM_RISPOSTA] = {0};
    for (int i = 0; i < DIM_RISPOSTA - 1; i++)
        r1[i] = 'a';
    
    char r2[DIM_RISPOSTA + 1] = {0};
    for (int i = 0; i < DIM_RISPOSTA; i++)
        r2[i] = 'b';
    
    printf("Risposta %s di lunghezza %ld: %s accettata\n", 
        r1, strlen(r1),
        checkLunghezzaRisposta(r1) ? "" : "non");
    printf("Risposta %s di lunghezza %ld: %s accettata\n", 
        r2, strlen(r2),
        checkLunghezzaRisposta(r2) ? "" : "non");

    bool statoTemi[] = {true, false};
    // Controllo sul tema scelto 
    printf("Il tema %d %s è stato già svolto\n", 1, 
        checkTemaGiaScelto(1, statoTemi) ? 
            "" : "non");
        printf("Il tema %d %s è stato già svolto\n", 2, 
        checkTemaGiaScelto(2, statoTemi) ? 
            "" : "non");
    
    return 0;
}
