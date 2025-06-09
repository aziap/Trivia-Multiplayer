#include "server_game_logic.h"
#include "classifica.h"
#include "input_check.h"
#include "messaggi.h"


#ifndef DEBUG_ON
#define DEBUG_ON
#include "debug.h"
#endif



// NOTA: questa funzione assume che la stringa passata sia corretta secondo i parametri richiesti. 
//		 I controlli vanno fatti in un'altra funzione.
// TODO: deallocare la memoria quando il giocatore si disconnette
// TODO (ma da un'altra parte): chiudere il socket se la funzione fallisce
struct Giocatore* registraGiocatore(char* nick, int sd) {
	// Il controllo sul numero di giocatori massimo deve essere fatto
    //      dall chiamante, gestisco il caso come un errore critico
    if (numGiocatori >= MAX_CLIENTS) {
        debug("Errore in registraGiocatore: numero di giocatori massimo raggiunto\n");
        exit(EXIT_FAILURE);
    }
	struct Giocatore* g = malloc(sizeof(struct Giocatore));
	if (g == NULL) {
		perror("errore in registraGiocatore()\nmalloc(): ");
		return NULL;
	}
	strncpy(g->nick, nick, DIM_NICK);
    g->sd = sd;
	g->temaCorrente = 0;
	g->punteggioCorrente = 0;
	g->prossimaDomanda = 0;
	for (int i = 0 ; i < NUM_TEMI ; i++) {
		g->statoTemi[i] = false;
	}

    giocatori[numGiocatori++] = g;
	return g;
}

// Controllo se esiste un giocatore registrato con un certo nickname
// @returns true se esiste, false altrimenti
bool checkNickPreso(char* nick) {
    int i = 0;
    while (i <= numGiocatori && strcmp(nick, numGiocatori[i]->nick) != 0) {
        ++i;
    }
    return i <= numGiocatori;
}

int gestisciMessaggio(int sd, char* buffer) {

    struct Messaggio* m = unpack(buffer); 

    // I controlli devono essere fatti dal chiamante, 
    //      gestisco il caso come un errore critico
    if (m == NULL) {
        debug("Errore in gestisciMessaggio(): deserializzazione del messaggio fallita\n");
        return DISCONNECT;
    }

    // Il giocatore ha scelto un nickname
    if (m->type == NICK_PROPOSITION_T) {
        // Controllo la validità del formato.
        //      I controlli sono già stati fatti lato client, se il 
        //      formato non è valido, chiudo la connessione con il giocatore
        char nick[strlen(m->payload) + 1] = {0};
        if (!checkNicknameFormat(nick)) {
            debug("Errore in gestisciMessaggio(): il formato del nick non è valido: %s\n", 
                nick);
            return DISCONNECT;
        }

        // Controllo se esiste un giocatore con quel nickname
        if (checkNickPreso(nick)) {
            // Preparo il messaggio di risposta nel buffer
            if (!pack(NICK_UNAVAIL_T, 0, 0, "", buffer)) {
                 return ERROR;
            }
            // Comunico al chiamante che il buffer contiene un messaggio da inviare
            return OK;
        }
        
        // Registro il giocatore
        if (registraGiocatore(nick, sd) == NULL) {
            return DISCONNECT;
        }
        

        // Copio la lista dei temi nel buffer
        // Eventualmente se ci sono problemi 
        // return DISCONNECT
        // ...

        return OK;
    }
}
