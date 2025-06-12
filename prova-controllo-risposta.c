#include "server_game_logic.h"


void stampaEsito(int e) {
    printf("esito gestione messaggio: %s\n", 
    e == ERROR ? "errore" 
    : e == DISCONNECT ? "disconnetti il giocatore"
    : "ok");
}

int main() {
	char buffer[DIM_BUFFER] = {0};
    struct Giocatore g;
    g.sd = 77;
    strncpy(g.nick, "roland", 7);
    g.statoTemi[0] = false;
    g.statoTemi[1] = false;
    g.temaCorrente = 1;
    g.domandaCorrente = 5;
    g.punteggioCorrente = 0;

	giocatori[0] = &g;
	++numGiocatori;

	inserisciInClassifica(g.nick, g.temaCorrente);

	char* risposta = "1";
	pack(ANSWER_T, DIM_RISPOSTA, 0, risposta, buffer);
	
	stampaEsito(gestisciMessaggio(77, buffer));
	
	struct Messaggio* m = unpack(buffer);
	
	printf("Prossima domanda: %s\n", m->payload);
	
	/*
	char test[][DIM_RISPOSTA] = {"barrito", "barri", "barritooo", "\tbarrito   ", "asbestos barrito"};
    printf("Esito della risposta: %s -> %d\n", 
        test[0], gestisciRispostaQuiz(test[0], &g)
    );
    printf("Esito della risposta: %s -> %d\n", 
        test[1], gestisciRispostaQuiz(test[1], &g)
    );
    printf("Esito della risposta: %s -> %d\n", 
        test[2], gestisciRispostaQuiz(test[2], &g)
    );

    printf("Esito della risposta: %s -> %d\n", 
        test[3], gestisciRispostaQuiz(test[3], &g)
    );
    printf("Esito della risposta: %s -> %d\n", 
        test[4], gestisciRispostaQuiz(test[4], &g)
    );
    */
    return 0;
}
