// #ifndef DEBUG_ON
// #define DEBUG_ON
// #endif

#include "classifica.h"
#include "client.h"
#include "server_game_logic.h"

char buffer[DIM_BUFFER] = {0};

int main() {
	// Inizializzo la classifica
	for (int i = 0 ; i < NUM_TEMI ; i++ ) {
		classificaTema[i] = NULL;
	}

	// DEBUG: creo dei giocatori

	struct Giocatore * mockPlayer1 = registraGiocatore("Cispolo",1);
	struct Giocatore * mockPlayer3 = registraGiocatore("Zaziki",2);
	struct Giocatore * mockPlayer2 = registraGiocatore("Bartolomeo",3);

	mockPlayer1->temaCorrente = 2;
	mockPlayer2->temaCorrente = 2;
	mockPlayer3->temaCorrente = 2;

	// END DEBUG

	// DEBUG: registro un giocatore

	// printf("Scegli un nickname (massimo 23 caratteri): ");
	// fgets(tmpNick, DIM_NICK, stdin);
	// // TODO: togliere spazi bianchi all'inizio e fine della stringa
	// // TODO: controllo su dimensione non nulla
	// tmpNick[strcspn(tmpNick, "\n")] = 0;


	// if ( ( giocatori[0] = registraGiocatore(tmpNick) ) != NULL ){
	// 	printf("Nickname: %s\nTema: %u\nProssima domanda: %u\n", giocatori[0]->nick, giocatori[0]->temaCorrente, giocatori[0]->domandaCorrente);
	// 	for (int i = 0 ; i < NUM_TEMI ; i++) 
	// 		printf( (giocatori[0]->statoTemi[i] ? "Tema %d completato\n" : "Tema %d non completato\n"), i + 1 );
	// }

	// END DEBUG
	

	// DEBUG inserisciInClassifica()
	// 		creo e inserisco in classifica alcuni giocatori

	inserisciInClassifica(mockPlayer1->nick, mockPlayer1->temaCorrente);
	inserisciInClassifica(mockPlayer3->nick, mockPlayer3->temaCorrente);
	inserisciInClassifica(mockPlayer2->nick, mockPlayer2->temaCorrente);

	// printf("Estratto %s dalla classifica\n", estraiGiocatore(&classificaTema[mockPlayer1->temaCorrente - 1],  "Zaziki")->nick);

	incrementaPunteggio(mockPlayer1->nick, mockPlayer1->punteggioCorrente, mockPlayer1->temaCorrente);
	incrementaPunteggio(mockPlayer2->nick, mockPlayer2->punteggioCorrente, mockPlayer2->temaCorrente);

	printf("Stampa classifica lato server\n");
	stampaClassifica();
	
	
	pack(SHOW_SCORE_T, 0, 0, "", buffer);
	printf("Esito invio messaggio: %d\n", gestisciMessaggio(99, buffer));
	struct Messaggio* m = unpack(buffer);
	printf("classifica lato client:\n");
	stampaClassificaClient(m->payload, m->msgLen);
	
	printf("Un giocatore si è disconnesso\n");
	rimuoviGiocatore(1);

	pack(SHOW_SCORE_T, 0, 0, "", buffer);
	printf("Esito invio messaggio: %d\n", gestisciMessaggio(99, buffer));
	struct Messaggio* m = unpack(buffer);
	printf("classifica lato client:\n");
	stampaClassificaClient(m->payload, m->msgLen);
	

	/*
	char classifica[MAX_DIM_PAYLOAD] = {0};
	serializzaClassifica(classifica);
	int len = (DIM_NICK + 2) * 3;	// classifica in questo caso 
	stampaClassificaClient(classifica, len);
	*/
	

	// END DEBUG

	return 0;
}

// bool rimuoviRankGiocatore(struct Giocatore* g) {
// 	// Rimuovi il rank del giocatore dalle classifiche dei temi che ha completato
// 	for (int i = 0; i < NUM_TEMI; i++) {
// 		if( !g->statoTemi[i] ) continue;
// 		struct RankGiocatore** head = &classificaTema[i];
// 		if ( !strcmp( (*head)->nick, g->nick ) ) ;
// 	}
// }


