#ifndef INPUT_CHECK_H
#define INPUT_CHECK_H

#ifndef DEBUG_ON
#define DEBUG_ON
#endif
#include "debug.h"

#include "costanti.h"
#include <string.h>
#include <stdbool.h>

// Elimina gli spazi bianchi in testa e in coda a una stringa e il carriage return
char* trim(char* str) {
    // Comincio dalla fine
    int i = strlen(str);
    while (i >= 0
        && (str[i] == ' ' || str[i] == '\n' || str[i] == 0) 
    ){
        --i;
    }
    str[i + 1] = 0;

    // Tolgo gli spazi bianchi in coda
    i = 0;
    int j = 0;

    // Cerco il primo carattere valido
    while (str[i] == ' ') {
        ++i;
    }

    // Shift dei caratteri validi 
    while (str[j] != 0) {
        str[j++] = str[i++];
    }
    return str;
}

// Rimuovo gli spazi bianchi da testa e coda del nickname
//     e controllo che non sia vuoto o troppo lungo
static inline bool checkNicknameFormat(char* nick){
    if (strlen(trim(nick)) > DIM_NICK - 1) {
        debug("Nickname troppo lungo\n");
        return false;
    }
    if (strlen(nick) == 0) {
        debug("Nessun carattere valido trovato\n");
        return false;
    }
    return true;
}

// Controllo l'opzione scelta inserita al menù di avvio
static inline bool checkOpzioneMenu(int opt) {
    return !(opt < AVVIA_OPT || opt > ESCI_OPT);
}

// Controllo se l'input inserito è un tema valido
static inline bool checkValoreTema(int tema) {
    return !(tema < 1 || tema > NUM_TEMI);
}

// Controllo che la lunghezza dell risposta sia entro i parametri definiti 
static inline bool checkLunghezzaRisposta(char* risposta) {
    return strlen(trim(risposta)) < DIM_RISPOSTA;
}

// Controllo se il quiz è già stato completato
static inline bool checkTemaGiaScelto(int tema, bool completati[]) {
    return completati[tema - 1];
}

#endif
