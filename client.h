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
            debug("non ci sono più record da stampare\n");
            putchar('\n');
            continue;
        }
        debug("tema del prossimo record: %u\n", msgClassifica[offset]); 
        while (msgClassifica[offset] == i) {
            // Ho letto il tema, passo al campo successivo
            ++offset;
            // Copio il nome del giocatore in una stringa per la stampa
            strncpy(tmpNick, msgClassifica + offset, DIM_NICK);
            
            offset += DIM_NICK;
            printf("- %s %u\n", tmpNick, msgClassifica[offset++]);
        }
        putchar('\n');
    
    }

}

#endif
