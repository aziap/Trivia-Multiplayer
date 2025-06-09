#ifndef SERVER_GAME_LOGIC_H
#define SERVER_GAME_LOGIC_H

#include "costanti.h"
#include "classifica.h"
#include "input_check.h"
#include "messaggi.h"


#ifndef DEBUG_ON
#define DEBUG_ON
#include "debug.h"
#endif

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

// Dato il socket descriptor, restituisce l'indice del giocatore
//      nell'array giocatori[]
static inline int getGiocatore(int sd) {
    int index = 0;
    while (index <= numGiocatori && index != giocatori[index]->sd) {
        ++index;
    }
    return index <= numGiocatori ? index : -1;
}


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

    // scelta di un nickname
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

        // return OK;
    }

    // Scelta di un tema
    else if (m->type == THEME_CHOICE_T) {
        // Controllo se il tema è già stato scelto precedentemente
        // ...

        // Aggiorno lo stato del giocatore
        // ...

        // Inserisco un nuovo record nella classifica
        // ...

        // Prelevo la prima domanda dal domande.txt
        // ...

        // Preparo il messaggio con la prima domanda nel buffer
            // Setto il flag FIRST_QST
            // ...

        // return OK;
    }

    // Risposta ad una domanda del quiz
    else if (m->type == ANSWER_T) {
        // TODO: per gestire questo caso, meglio fare una funzione apposita

        // Controlli di consistenza
        // ...

        // Aggiorno lo stato del giocatore
        // ...

        // Se non era l'ultima domanda, prelevo la prossima domanda
        // ...

        // Altrimenti setto il flag NO_QST
        // ...
        
        // Leggo la risposta in risposte.txt e controllo
        // ...

        // Preparo il messaggio con esito della risposta 
        //      e eventualmente prossima domanda
        // ...

        // return OK;
    }

    // Richiesta della classifica
    else if (m->type == SHOW_SCORE_T) {
        // Serializzo la classifica
        // ...

        // Impacchetto nel buffer
        // ...

        // return OK;
    }

    else if (m->type == ENDQUIZ) {
        // Dealloca le strutture dati
        // ...
        
        // Rimuovi tutti i record dalla classifica
        // ...

        // return DISCONNECT;
    }
}


#endif