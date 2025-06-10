#ifndef SERVER_GAME_LOGIC_H
#define SERVER_GAME_LOGIC_H

#include "costanti.h"
#include "classifica.h"
#include "input_check.h"
#include "messaggi.h"

#include <stdio.h>

#ifndef DEBUG_ON
#define DEBUG_ON 1
#endif
#include "debug.h"

// Valori di ritorno delle funzioni che gestiscono la logica di gioco.
//      Dicono al chiamante, in server.c, se procedere con 
//      il ciclo send - receive, o se chiudere la connessione 
//      con il client corrispondente al socket esaminato
#define ERROR -1        // Errore critico: lascio la gestione al chiamante
#define DISCONNECT 0    // Dico al chiamante di chiudere la connessione con l'utente
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

    debug("Parametri del giocatore:\n nick %s, socket %d, tema %u, punti %u, prossima domanda: %u\n",
        g->nick, g->sd, g->temaCorrente, g->punteggioCorrente, g->prossimaDomanda);

	for (int i = 0 ; i < NUM_TEMI ; i++) {
		g->statoTemi[i] = false;
	}

    giocatori[numGiocatori++] = g;
	return g;
}

// Controllo se esiste un giocatore registrato con un certo nickname
// @returns true se esiste, false altrimenti
bool checkNickPreso(char* nick) {
    // è il primo giocatore
    if (!numGiocatori) return false;
    int i = 0;
    while (i < numGiocatori && strcmp(nick, giocatori[i]->nick) != 0) {
        ++i;
    }
    return i < numGiocatori;
}

char* prelevaDomanda(uint8_t tema, uint8_t numero) {
    // fopen
    // Ogni domanda è scritta su una linea
    
    // fgets
    // 
}


int gestisciMessaggio(int sd, char* buffer) {
    struct Messaggio* m = unpack(buffer); 

    // I controlli devono essere fatti dal chiamante, 
    //      gestisco il caso come un errore critico
    if (m == NULL) {
        debug("Errore in gestisciMessaggio(): deserializzazione del messaggio fallita\n");
        return DISCONNECT;
    }

	    debug("in gestisciMessaggio()\ncampi del messaggio:\n- tipo: %u\n- flag: %u\n- len: %d\n- payload: %s\n",
    m->type, m->flags, m->msgLen, m->payload); 
    
    // scelta di un nickname
    if (m->type == NICK_PROPOSITION_T) {
    	debug("Il giocatore ha scelto un nickname: %s\n", m->payload);
    
        // Controllo la validità del formato.
        //      I controlli sono già stati fatti lato client, se il 
        //      formato non è valido, chiudo la connessione con il giocatore
        if (!checkNicknameFormat(m->payload)) {
            debug("Errore in gestisciMessaggio(): il formato del nick non è valido: %s\n", 
                m->payload);
            free(m);
            return DISCONNECT;
        }

        char nick[DIM_NICK] = {0};
        strncpy(nick, m->payload, DIM_NICK);

        // Il messaggio non mi serve più, posso deallocare la memoria
        free(m);

        // Controllo se esiste un giocatore con quel nickname
        if (checkNickPreso(nick)) {
	        debug("Il nick era già preso\n");
            // Preparo il messaggio di risposta nel buffer
            if (!pack(NICK_UNAVAIL_T, 0, 0, "", buffer)) {
                return ERROR;
            }
            // Comunico al chiamante che il buffer contiene un messaggio da inviare
            return OK;
        }
    	
    	debug("Il nick è disponibile\n");    
        
        // Registro il giocatore
        if (registraGiocatore(nick, sd) == NULL) {
            return DISCONNECT;
        }
	
		debug("in gestisciMessaggio(), sto per leggere temi.txt\n");

        FILE* temi; 
        if ((temi = fopen("./temi.txt","r")) == NULL) {
            perror("Errore nell'apertura di temi.txt");
            return ERROR;
        }
        
        debug("Apertura del file riuscita\n");
        
        // Copio la lista dei temi in un buffer temporaneo
        char listaTemi[DIM_TEMA * NUM_TEMI];
        size_t nread = fread(listaTemi, sizeof(char), DIM_TEMA * NUM_TEMI, temi);

        // Se non ho letto tutto il file o se c'è stato un problema nella lettura, restituisco il codice di errore
        if (ferror(temi) != 0) {
            perror("Errore nella lettura di temi.txt");
            fclose(temi);
            return ERROR;
        }
        
        if (feof(temi) == 0) {
            printf("Errore in gestisciMessaggio(): lo spazio allocato per la lettura dei temi è insufficiente\n");
            fclose(temi);
            return ERROR;
        }
        // La lettura ha avuto successo
        fclose(temi);
        
        listaTemi[nread] = 0;

        debug("lista dei temi: %s\n", listaTemi);
        return pack(THEME_LIST_T, nread, 0, listaTemi, buffer) ? OK : ERROR;
    }

    // Scelta di un tema
    else if (m->type == THEME_CHOICE_T) {
        // Controllo se il tema è già stato scelto precedentemente
        // ...
        uint8_t t = atoi(m->payload);
        free(m);
        if (!checkValoreTema(t)) {
            printf("Numero tema ricevuto non valido: %u\n", t);
            return DISCONNECT;
        }

        struct Giocatore* g = getGiocatore(sd);
        
        if (checkTemaGiaScelto(t, g->statoTemi)) {
            printf("%s ha già partecipato al quiz del tema %u\n", g->nick, t);
            return DISCONNECT;
        }

        if(g->temaCorrente != 0) {
            printf("%s sta già partecipando ad un altro quiz: %u\n", g->temaCorrente);
            return DISCONNECT;
        }

        // Aggiorno lo stato del giocatore
        g->temaCorrente = t;

        // Inserisco un nuovo record nella classifica
        if (!inserisciInClassifica(g->nick, t)) {
            // Errore nell'allocazione di memoria per il nuovo record
            return ERROR;
        }

        debug("Prelevo la prima domanda del tema %u\n", t);
        // Prelevo la prima domanda da domande.txt
        

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

        // Se non era l'ultima domanda, prelevo la prossima
        // ...

        // Altrimenti setto il flag NO_QST
        // ...
        
        // Leggo la risposta in risposte.txt e controllo
        // ...

        // Preparo il messaggio con esito della risposta 
        //      e eventualmente prossima domanda
        // ...
		// free(m);
        // return OK;
    }

    // Richiesta della classifica
    else if (m->type == SHOW_SCORE_T) {
        // Serializzo la classifica
        // ...

        // Impacchetto nel buffer
        // ...
		// free(m);
        // return OK;
    }

    else if (m->type == ENDQUIZ_T) {
        // Dealloca le strutture dati
        // ...
        
        // Rimuovi tutti i record dalla classifica
        // ...
		// free(m);
        // return DISCONNECT;
    }
    free(m);
    return ERROR;
}


#endif
