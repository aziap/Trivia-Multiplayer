#ifndef CLIENT_H
#define CLIENT_H

#include "messaggi.h"

#ifndef DEBUG_ON
#define DEBUG_ON
#include "debug.h"
#endif

void stampaClassificaClient(char *msgClassifica, msgsize_t len) {
    int offset = 0;
    char tmpNick[DIM_NICK] = {0};
    for (int i = 1; i <= NUM_TEMI; i++) {
        printf("Punteggio tema %d\n", i);
        if (offset >= len) {
            putchar('\n');
            continue;
        }
        while(msgClassifica[offset++] == i) {
            // Copio il nome del giocatore in una stringa per la stampa
            strncpy(tmpNick, msgClassifica + offset, DIM_NICK);
            offset += DIM_NICK;
            printf("- %s %u\n", tmpNick, msgClassifica[offset++]);
        }
        putchar('\n');
    
    }

}

#endif