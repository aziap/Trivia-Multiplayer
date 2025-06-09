#ifndef SERVER_GAME_LOGIC_H
#define SERVER_GAME_LOGIC_H

#include "costanti.h"

// Valori di ritorno delle funzioni che gestiscono la logica di gioco.
//      Dicono al chiamante, in server.c, se procedere con 
//      il ciclo send - receive, o se chiudere la connessione 
//      con il client corrispondente al socket esaminato
#define ERROR -1        // Errore critico 
#define DISCONNECT 0    // L'utente deve essere disconnesso
#define OK 1            // Il buffer contiene un messaggio da inviare

// Contiene lo stato corrente di un giocatore
// @param nick: nickname con cui il giocatore si è registrato.
// @param temaCorrente: id del tema del quiz che il giocatore sta giocando attualmente.
// 		0 se non ne sta giocando nessuno 
// @param prossimaDomanda: id della prossima domanda da inviare al giocatore. 
// @param statoTemi: 1 se il giocatore ha completato un tema, 0 altrimenti.
struct Giocatore {
    int sd;     // Socket descriptor associato al giocatore
	char nick[DIM_NICK];
	uint8_t temaCorrente;
	uint8_t punteggioCorrente;
	uint8_t prossimaDomanda;
	bool statoTemi[NUM_TEMI];
};

int numGiocatori = 0;

struct Giocatore* giocatori[MAX_CLIENTS];

struct Giocatore* registraGiocatore(char* nick, int sd);

int gestisciMessaggio(int sd, char* buffer);

bool checkNickPreso(char* nick);

// Dato il socket descriptor, restituisce l'indice del giocatore
//      nell'array giocatori[]
static inline int getGiocatore(int sd) {
    int index = 0;
    while (index <= numGiocatori && index != giocatori[index]->sd) {
        ++index;
    }
    return index <= numGiocatori ? index : -1;
}

#endif