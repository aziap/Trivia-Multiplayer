#ifndef CLIENT_H
#define CLIENT_H

#include "messaggi.h"
#include "costanti.h"

// Legge un input dell'utente, fino ad un numero massimo di caratteri.
// @param buffer: buffer in cui inserire l'input letto
// @param dim: quanti caratteri leggere al più
static inline char* leggiStringa(char* buffer, int dim) {
    fgets(buffer, dim, stdin);
    buffer[strcspn(buffer, "\n")] = 0; 
    return buffer;
}

// @param msgClassifica: buffer che contiene i record della classifica 
// @param len: lunghezza complessiva in byte della classifica
void stampaClassificaClient(char *msgClassifica, msgsize_t len) {
    int offset = 0;
    char tmpNick[DIM_NICK] = {0};
    for (int i = 1; i <= NUM_TEMI; i++) {
        printf("Punteggio tema %d\n", i);
        if (offset >= len) {
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
