#ifndef CLIENT_H
#define CLIENT_H

#include "messaggi.h"

#include <stdbool.h>

#define AVVIA_OPT '1' // Comando per avviare il gioco nel menù iniziale
#define EXIT_OPT '2'  // Comando per uscire dal gioco nel menù iniziale
#define SHOW_SCORE "show score"
#define ENDQUIZ "endquiz"

#ifndef DEBUG_ON
// #define DEBUG_ON
#endif
#include "debug.h"

void stampaClassificaClient(char *msgClassifica, msgsize_t len) {
    int offset = 0;
    char tmpNick[DIM_NICK] = {0};
    for (int i = 1; i <= NUM_TEMI; i++) {
        printf("Punteggio tema %d\n", i);
        if (offset >= len) {
            debug("non ci sono più record da stampare\n");
            putchar('\n');
            continue;
        }
        while (msgClassifica[offset] == i) {
            // Ho letto il tema, passo al campo successivo
            ++offset;
            // Copio il nome del giocatore in una stringa per la stampa
            strncpy(tmpNick, msgClassifica + offset, DIM_NICK);
            
            // Stampo nome e punteggio
            offset += DIM_NICK;
            printf("- %s %u\n", tmpNick, msgClassifica[offset++]);
        }
        putchar('\n');
    }

}

// Elimina gli spazi bianchi in testa e in coda a una stringa e il carriage return
bool trim(char *str) {
    // Comincio dalla fine
    int i = strlen(str + 1);
    while (i >= 0
        && (str[i] != ' ' || str[i] != 'n' || str[i] != '\0') 
    ){
        --i;
    }
    // Stringa vuota, con soli spazi o senza terminatore
    if (i == 0 || strlen(str + 1)) return false;
    str[i + 1] = '/0';

    i = 0;
    int j = 0;

    // Cerco il primo carattere valido
    while (str[i] == ' ') {
        ++i;
    }

    // Shift dei caratteri validi 
    while (str[i] != '\0') {
        str[j++] = str[i++];
    }
}

// TODO: spostare in libreria per i controlli sugli input
// TODO: generalizzare togliendo le stampe in modo da poterlo usare sia lato client che server (magari ritornando un tipo d'errore)
bool checkNicknameFormat(char* nick){
    if(strlen(nick) > DIM_NICK - 1) {
        printf("Nickname troppo lungo\n");
        return false;
    }
    if(strlen(nick) == 0 || trim(nick) ) {
        printf("Inserire almeno un carattere diverso da ' '\n");
        return false;
    }
    return true;
}

#endif
