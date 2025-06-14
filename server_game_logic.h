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
// @param domandaCorrente: id dell'ultima domanda inviata al giocatore. 
// @param statoTemi: 1 se il giocatore ha completato un tema, 0 altrimenti.
struct Giocatore {
    int sd;     // Socket descriptor associato al giocatore
	char nick[DIM_NICK];
	uint8_t temaCorrente;
	uint8_t punteggioCorrente;
	uint8_t domandaCorrente;
	bool statoTemi[NUM_TEMI];
};

int numGiocatori = 0;

struct Giocatore* giocatori[MAX_CLIENTS];


// Dato il socket descriptor, restituisce l'indice del giocatore
//      nell'array giocatori[]
static inline int getGiocatore(int sd) {
    int index = 0;
    while (index <= numGiocatori && sd != giocatori[index]->sd) {
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
	g->domandaCorrente = 0;

    debug("Parametri del giocatore:\n nick %s, socket %d, tema %u, punti %u, domanda corrente: %u\n",
        g->nick, g->sd, g->temaCorrente, g->punteggioCorrente, g->domandaCorrente);

	for (int i = 0 ; i < NUM_TEMI ; i++) {
		g->statoTemi[i] = false;
	}

    giocatori[numGiocatori++] = g;
	return g;
}

void rimuoviGiocatore(sd) {
    int index = getGiocatore(sd);
    // Il giocatore corrispondente al sd non era ancora registrato
    if (index == -1) return;
    struct Giocatore* g = giocatori[index]; 

    if (g->temaCorrente)
        rimuoviRankGiocatore(g->nick, g->temaCorrente);
    
    for(int i = 0; i < NUM_TEMI; i++) {
        if(!g->statoTemi[i])
            continue;
        rimuoviRankGiocatore(g->nick, i + 1);
    }
    
    free(g);

    giocatori[index] = giocatori[--numGiocatori];
    return;
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


// Preleva una riga indicizzata da "tema" e "numero" dal file "fileTarget"
// Usata per prelevare domande e risposte dai rispettivi file
static inline bool prelevaRiga(uint8_t tema, uint8_t numero, char* bufLinea, int len, char* fileTarget) {
    if (tema > NUM_TEMI || tema == 0 || numero > NUM_DOMANDE || numero == 0) {
    	printf("Errore in prelevaRiga(): l'id del tema deve essere compreso tra 0 e %d (valore passato: %u) e il numero della domanda/risposta deve essere compreso tra 0 e %d (valore passato: %u)\n", NUM_TEMI, tema, NUM_DOMANDE, numero);
    	return false;
    }
    
    FILE* target;
    if ((target = fopen(fileTarget, "r")) == NULL) {
        return false;
    }
    // Creo una stringa con "<tema><separatore><numero>"
    int daComparare = 3;	// Assumendo che tema e numero siano ad una cifra
    char indiceRiga[daComparare] = {'0' + tema, SEP, '0' + numero};
    while (fgets(bufLinea, len, target) != NULL && strncmp(bufLinea, indiceRiga, daComparare) != 0) {}
    if (ferror(target) != 0) {
        perror("Errore in prelevaRiga(), fgets()");
        bufLinea[0] = 0;
        return false;
    }
    fclose(target);
    bufLinea[strcspn(bufLinea, "\n")] = 0;
    return true;
}


// Controlla il contenuto di una risposta, aggiorna la struttura Giocatore
//      con il nuovo punteggio.
// Non modifica il tema e la domanda correnti
// @returns -1 in caso di errore, 0 se la risposta è errata, 1 altrimenti 
static inline int gestisciRispostaQuiz(char* rispostaGiocatore, struct Giocatore* g) {
    // Se la stringa è vuota, non c'è bisogno di controllare la risposta
    if (!checkStringaNonVuota(rispostaGiocatore)) {
        return 0;
    }
    char rispostaCorretta[DIM_RISPOSTA] = {0};
    // Leggo la risposta in risposte.txt e controllo
    if (!prelevaRiga(g->temaCorrente, g->domandaCorrente, rispostaCorretta, DIM_RISPOSTA, "./risposte.txt")) {
        return -1;
    }
    return strncasecmp(rispostaCorretta + 4, rispostaGiocatore, strlen(rispostaCorretta)) != 0 ? 0 : 1;
}


// ******************************
//      GESTIONE MESSAGGI CLIENT
// ******************************

// Esamina il contenuto del buffer che contiene un messaggio ricevuto dal client,
//      in base al tipo eseguo delle azioni e preparo un messaggio di risposta 
//      nello stesso buffer
// @param sd: descrittore del socket dal quale è stato ricevuto il messaggio dal client
// @param buffer: buffer che contiene il messaggio
// @param toSend: indirizzo di un intero in cui salvare numero di byte da inviare al client
// @returns int codice che il chiamante legge per determinare la
//      prossima azione: 
//      - ERROR (-1) se si è verificato un problema critico, come un bug,
//        un problema di allocazione di memoria o di apertura di un file
//      - DISCONNECT (0) se l'utente ha richiesto la disconnessione o se
//        il messaggio contiene un messaggio imprevisto, imputabile ad un errore nel client
//      - OK (1) altrimenti. In questo caso il buffer passato contiene il messaggio 
//        da inviare in risposta 
int gestisciMessaggio(int sd, char* buffer, int* toSend) {
    struct Messaggio* m = unpack(buffer); 

    // I controlli devono essere fatti dal chiamante, 
    //      gestisco il caso come un errore critico
    if (m == NULL) {
        debug("Errore in gestisciMessaggio(): deserializzazione del messaggio fallita\n");
        return DISCONNECT;
    }

	    debug("in gestisciMessaggio()\ncampi del messaggio:\n- tipo: %u\n- flag: %u\n- len: %d\n- payload: %s\n",
    m->type, m->flags, m->msgLen, m->payload); 
    
    // ******************************
    //      scelta di un nickname
    // ******************************
    if (m->type == NICK_PROPOSITION_T) {
    	debug("Il giocatore ha scelto un nickname: %s\n", m->payload);

        // Controllo di non avere già un giocatore associato allo stesso socket descriptor
        if(getGiocatore(sd) != -1) {
            free(m);
            printf("Il giocatore con sd = %d era già registrato\n", sd);
            return DISCONNECT;
        }
    
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
            *toSend = HEADER_LEN;
            return OK;
        }
    	
    	debug("Il nick è disponibile\n");
        
        // Registro il giocatore
        if (registraGiocatore(nick, sd) == NULL) {
            return DISCONNECT;
        }

        FILE* temi; 
        if ((temi = fopen("./temi.txt","r")) == NULL) {
            perror("Errore nell'apertura di temi.txt");
            return ERROR;
        }
        
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

    // ******************************
    //      Comando show score
    // ******************************
    else if (m->type == SHOW_SCORE_T) {
    	free(m);
        char* classifica;
        // Serializzo la classifica
        if ((classifica = malloc(MAX_DIM_PAYLOAD)) == NULL) return ERROR;
        uint16_t len = serializzaClassifica(classifica);

        // La impacchetto nel buffer
        if (pack(RANK_T, len, 0, classifica, buffer)) {
		    free(classifica);
            *toSend = HEADER_LEN + len;
		    return OK;
        }
        free(classifica);
        return ERROR;
    }

    int index = getGiocatore(sd); 
    if (index == -1) {
        printf("Errore in gestisciMessaggio(): giocatore corrispondente al socket %d non trovato\n", sd);
        free(m);
        return ERROR;
    }
    struct Giocatore* g = giocatori[index];
       
    // ******************************
    //     Scelta di un tema
    // ******************************
    if (m->type == THEME_CHOICE_T) {
        // Controllo se il tema è già stato scelto precedentemente
        uint8_t t = atoi(m->payload);
        free(m);
        if (!checkValoreTema(t)) {
            printf("Numero tema ricevuto non valido: %u\n", t);
            return DISCONNECT;
        }

        if (checkTemaGiaScelto(t, g->statoTemi)) {
            printf("%s ha già partecipato al quiz del tema %u\n", g->nick, t);
            return DISCONNECT;
        }

        if(g->temaCorrente != 0) {
            printf("%s sta già partecipando ad un altro quiz: %u\n", g->nick, g->temaCorrente);
            return DISCONNECT;
        }
        
        // Inserisco un nuovo record nella classifica
        if (!inserisciInClassifica(g->nick, t)) {
            // Errore nell'allocazione di memoria per il nuovo record
            return ERROR;
        }

        // Aggiorno lo stato del giocatore
        g->temaCorrente = t;
        g->domandaCorrente = 1;
        
        debug("Prelevo la prima domanda del tema %u\n", t);
        // Prelevo la prima domanda da domande.txt
        char domanda[DIM_DOMANDA] = {0};
        if (!prelevaRiga(t, 1, domanda, DIM_DOMANDA, "./domande.txt")) {
            return ERROR;
        }
        // Preparo il messaggio con la prima domanda nel buffer,
        //      con il flag FIRST_QST settato
        *toSend = HEADER_LEN + DIM_DOMANDA;
        return pack(QUESTION_T, DIM_DOMANDA, FIRST_QST, domanda, buffer) ?
            OK : ERROR; 
    }
    
    
    // *****************************************
    //      Risposta ad una domanda del quiz
    // *****************************************
    else if (m->type == ANSWER_T) {
        // TODO: per gestire questo caso, meglio fare una funzione apposita

        // Controlli di consistenza
        if (g->temaCorrente == 0) {
            free(m);
            return DISCONNECT;
        }

		// ret: -1 -> errore; 1 -> risposta corretta; 0 -> risposta sbagliata 
        int ret = gestisciRispostaQuiz(m->payload, g);
        free(m);

        if (ret == -1) return ERROR;

        flag_t flags = ret ? PREV_ANS_CORRECT : 0;
        
        // Aggiorno il punteggio in classifica
        if (ret && !incrementaPunteggio(g->nick, ++(g->punteggioCorrente), g->temaCorrente))
            return ERROR;
        
        // Se era l'ultima domanda, marco il tema come completato, 
        //      azzero tema e domanda corrente e preparo nel buffer 
        //      un messaggio con solo l'esito
        if (g->domandaCorrente == NUM_DOMANDE) {
            g->statoTemi[g->temaCorrente - 1] = true;
            g->temaCorrente = 0;
            g->domandaCorrente = 0;
            // Il flag NO_QST indica che il messaggio non contiene una domanda
            flags ^= NO_QST;
            debug("Era l'ultima domanda, campo flag: %u\n", flags);
            *toSend = HEADER_LEN;
            return pack(QUESTION_T, 0, flags, "", buffer) ? OK : ERROR;
        }
        
        g->domandaCorrente++;
        // Prelevo la prossima domanda e preparo il messaggio nel buffer
        char domanda[DIM_DOMANDA] = {0};
        if (!prelevaRiga(g->temaCorrente, g->domandaCorrente, domanda, DIM_DOMANDA, "./domande.txt")) {
            return ERROR;
        }
        *toSend = HEADER_LEN + DIM_DOMANDA;
        return pack(QUESTION_T, DIM_DOMANDA, FIRST_QST, domanda, buffer) ?
            OK : ERROR; 
    }

    // ******************************
    //      Comando endquiz
    // ******************************
    else if (m->type == ENDQUIZ_T) {
        free(m);
        // Sarà il chiamante a chiamare la funzione per deallocare le strutture dati
        return DISCONNECT;
    }
    
    // Se arrivo fin qui, il tipo del messaggio non è valido
    free(m);
    return ERROR;
}


#endif
