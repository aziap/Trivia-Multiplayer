#ifndef CLASSIFICA_H
#define CLASSIFICA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>	

#include "costanti.h"

// DEBUG

#include "debug.h"

// END DEBUG


// Contiene lo stato corrente di un giocatore
// @param nick: nickname con cui il giocatore si è registrato.
// @param temaCorrente: id del tema del quiz che il giocatore sta giocando attualmente.
// 		0 se non ne sta giocando nessuno 
// @param prossimaDomanda: id della prossima domanda da inviare al giocatore. 
// @param statoTemi: 1 se il giocatore ha completato un tema, 0 altrimenti.
struct Giocatore {
	char nick[DIM_NICK];
	uint8_t temaCorrente;
	uint8_t punteggioCorrente;
	uint8_t prossimaDomanda;
	bool statoTemi[NUM_TEMI];
};

// Elemento della classifica. Il tema è implicitamente indicato dall'indice dell'array "classificaTema"
// @param nick
// @param punti
// @param next: puntatore al prossimo elemento della lista.
struct RankGiocatore {
	char nick[DIM_NICK];
	uint8_t punti;
	struct RankGiocatore *next;
};

// Array che, per ogni tema, contiene la testa della lista che implementa la classifica 
struct RankGiocatore* classificaTema[NUM_TEMI];

// TODO: aggiornare
int contatoreRecord = 0;

// TODO: togliere?
// char tmpNick[DIM_NICK] = {0}; 

// NOTA: questa funzione assume che la stringa passata sia corretta secondo i parametri richiesti. 
//		 I controlli vanno fatti in un'altra funzione.
// TODO: deallocare la memoria quando il giocatore si disconnette
// TODO (ma da un'altra parte): chiudere il socket se la funzione fallisce
struct Giocatore* registraGiocatore(char* nick) {
	// TODO: inserire riferimento nella lista dei giocatori
	struct Giocatore* ptr = malloc(sizeof(struct Giocatore));
	if (ptr == NULL) {
		perror("errore in registraGiocatore()\nmalloc(): ");
		return NULL;
	}
	strcpy(ptr->nick, nick);
	ptr->temaCorrente = 0;
	ptr->punteggioCorrente = 0;
	ptr->prossimaDomanda = 0;
	for (int i = 0 ; i < NUM_TEMI ; i++) {
		ptr->statoTemi[i] = false;
	}
	return ptr;
}


// ********************************
// Funzioni per modificare le classifiche


// Estrae e restituisce un giocatore dalla classifica passata per riferimento
// ATTENZIONE: non decrementa il contatore dei record
struct RankGiocatore* estraiGiocatore( struct RankGiocatore** head, char* nick ) {
	// Lista vuota
	if ( *head == NULL ) {
		// DEBUG
		printf("La lista passata a estraiGiocatore() è vuota!\n");
		
		return NULL;
	}
	// Estrazione dalla testa
	if( !strcmp((*head)->nick, nick) ) {
		struct RankGiocatore* tmp = *head;
		*head = tmp->next;
		return tmp;
	}
	struct RankGiocatore *prev, *cur;
	prev = *head;
	cur = prev->next;
	while ( cur != NULL && strcmp(cur->nick, nick) != 0 ) {
		prev = cur;
		cur = cur->next;
	}
	if (cur == NULL) return NULL;
	prev->next = cur->next;
	return cur;
}


// 	Inserimento di un punteggio
bool inserisciInClassifica(char* nick, uint8_t tema) {
	if(tema > NUM_TEMI || tema == 0) {
		printf("Errore nell'inserimento del giocatore %s in classifica. Id del tema non valido: %u\n", nick, tema);
		return false;
	}
	struct RankGiocatore* elem = malloc( sizeof( struct RankGiocatore ) );
	// TODO: liberare la memoria quando il giocatore esce dal gioco
	if (elem == NULL) {
		perror("errore in inserisciInClassifica()\nmalloc()");
		return false;
	}
	strcpy(elem->nick, nick);
	elem->punti = 0;
	elem->next = NULL;

	struct RankGiocatore** head = &classificaTema[tema - 1]; 
	
	// Inserimento in testa
	if ( *head == NULL
	|| (*head)->punti < elem->punti
	|| ( (*head)->punti ==  elem->punti && strcmp(elem->nick, (*head)->nick) < 0 ) ) {
		// DEBUG
		printf("inserisco in testa\n");
		elem->next = *head;
		*head = elem;
		++contatoreRecord;
		return true;
	}
	
	struct RankGiocatore* prev = *head;
	struct RankGiocatore* cur = prev->next;
	while ( cur != NULL 
	&& ( cur->punti > elem->punti 
	|| ( cur->punti == elem->punti && strcmp(elem->nick, cur->nick) > 0 ) ) ) {
		prev = cur;
		cur = cur->next;
	}

	prev->next = elem;
	elem->next = cur;

	++contatoreRecord;
	return true;
}


bool incrementaPunteggio(char* nick, uint8_t nuovoPunteggio, uint8_t tema) {

	struct RankGiocatore** head = &classificaTema[tema - 1]; 
	if(*head == NULL) {
		printf("Fatal error: la classifica del tema %u non contiene giocatori\n", tema);
		exit(EXIT_FAILURE);
	}

	// Il giocatore era già primo in classifica
	if( !strcmp((*head)->nick, nick) ) {

		//DEBUG
		printf("%s era già primo in classifica!\n", nick);

		(*head)->punti++;
		return true;
	}

	struct RankGiocatore *cur, *prev;
	struct RankGiocatore** newPos;
	prev = *head;

	cur = prev->next;
	
	if ( (*head)->punti < nuovoPunteggio 
	|| ((*head)->punti == nuovoPunteggio && strcmp(nick, (*head)->nick) < 0) ) {
		// Il giocatore va spostato in testa alla classifica
		
		// DEBUG
		printf("%s va spostato in testa alla classifica\n", nick);

		newPos = head;
	} else {
		// Scorro finché non trovo la nuova posizione in classifica del giocatore 
		while( cur != NULL 
			&& ( cur->punti > nuovoPunteggio 
			|| ( cur->punti == nuovoPunteggio && strcmp(nick, cur->nick) > 0 ))) {
				prev = cur;
				cur = cur->next;
		}

		// Il giocatore non è nella classifica
		if( cur == NULL ){
			printf("Fatal error: il giocatore %s non si trova nella classifica del tema %u\n", nick,  tema);
			exit(EXIT_FAILURE);
		} 
		
		if (!strcmp(nick, cur->nick)) {
			// Non c'è bisogno di spostare l'elemento
			cur->punti++;
			return true;
		}
		// Ho trovato la nuova posizione del giocatore
		newPos = &prev;

		// DEBUG
		printf("%s verrà inserito dopo %s\n", nick, (*newPos)->nick);

	}
	// Inizio la ricerca del rank del giocatore dal punto in cui mi ero fermata
	cur = estraiGiocatore(newPos, nick);

	// DEBUG
	printf("Giocatore trovato! Nome: %s, punti: %u\n", cur->nick, cur->punti);

	cur->punti++;
	cur->next = *newPos;
	*newPos = cur;

	return true;
}

void stampaClassifica(){
	for(int i = 0; i < NUM_TEMI; i++) {
		printf ( "Tema %d:\n", i + 1 );
		if(classificaTema[i] == NULL) {
			putchar('\n');
			continue;
		}
		struct RankGiocatore* cur = classificaTema[i];
		while (cur != NULL) {
			printf("- %s %u\n", cur->nick, cur->punti);
			cur = cur->next;
		}
	}	
}

// Elimina 
bool rimuoviRankGiocatore(char* nick, uint8_t tema) {
	// controllo sul numero del tema
	if ( tema > NUM_TEMI ) {
		printf("Il tema %u non esiste\n", tema);
		return false;
	}
	// estrai
	struct RankGiocatore* r; 
	if ( ( r = estraiGiocatore(&classificaTema[tema -1], nick) ) == NULL){
		// gestione errori
		printf("Errore: giocatore %s non trovato nella classifica del tema %u\n", nick, tema);
		return false;
	}
	// dealloca memo
	free(r);
	// decrem contatore
	--contatoreRecord;
	return true;
}

// Formatta la classifica in una stringa di caratteri per poterla inserire in un messaggio
char* serializzaClassifica(){
	int len = contatoreRecord * (DIM_NICK + 2); // 1 byte per il tema + 1 byte per il punteggio 
	
	if (len > MAX_DIM_PAYLOAD) {
		printf("La classifica contiene troppi record per essere inviata\n");
		return NULL;
	}
	
	char *msgClassifica;
	if ((msgClassifica = malloc(len)) == NULL) {
		perror("Allocazione della memoria per serializzare la classifica fallita");
		return NULL;
	}

	int offset = 0;
	for(int i = 0; i < NUM_TEMI; i++) {
		if (classificaTema[i] == NULL) continue; 
		struct RankGiocatore *cur = classificaTema[i]; 
		while(cur != NULL) {
			msgClassifica[offset++] = (uint8_t)(i + 1);	// Tema
			strncpy(msgClassifica + offset, cur->nick, DIM_NICK);	// Nickname del giocatore
			offset += DIM_NICK;
			msgClassifica[offset++] = cur->punti;	// Punteggio del giocatore
			cur = cur->next;
		}
	}
	return msgClassifica;
}

#endif