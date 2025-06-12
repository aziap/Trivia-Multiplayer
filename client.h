#ifndef CLIENT_H
#define CLIENT_H

#include "messaggi.h"
#include "costanti.h"
#include <stdbool.h>

#ifndef DEBUG_ON
// #define DEBUG_ON
#endif
#include "debug.h"



static inline char* leggiStringa(char* buffer) {
        fgets(buffer, DIM_BUFFER, stdin);
        int len = strcspn(buffer, "\n");
        buffer[len] = 0;
        return buffer;
}

static inline void printFrame() {
    printf("+++++++++++++++++\n")
}

static inline void stampaTitolo() {
    printf("Trivia Quiz");
}



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


#endif
