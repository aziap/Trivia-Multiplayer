#ifndef INPUT_CHECK_H
#define INPUT_CHECK_H

#ifndef DEBUG_ON
#define DEBUG_ON
#endif
#include "debug.h"

#include "costanti.h"
#include <string.h>
#include <stdbool.h>

// controllo che il nickname non sia vuoto o troppo lungo
static inline bool checkNicknameFormat(char* nick){
	// Controllo che ci sia almeno un carattere non spazio
	int i = 0;
	while (i <= strlen(nick)
		&& ( nick[i] == ' ' || nick[i] == '\n' || nick[i] == '\t' || nick[i] == '\0')
	) {
		++i;
	}
	if (i > strlen(nick)) return false; 
	
    if (strlen(nick) > DIM_NICK - 1) {
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
    return strlen(risposta) < DIM_RISPOSTA;
}

// Controllo se il quiz è già stato completato
static inline bool checkTemaGiaScelto(int tema, bool completati[]) {
    return completati[tema - 1];
}

#endif
