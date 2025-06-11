#include "server_game_logic.h"
#include "messaggi.h" 

#ifndef DEBUG_ON 
#define DEBUG_ON 1
#endif
#include "debug.h"

char buffer[DIM_BUFFER] = {0};

void stampaEsito(int e) {
    printf("esito gestione messaggio: %s\n", 
    e == ERROR ? "errore" 
    : e == DISCONNECT ? "disconnetti il giocatore"
    : "ok");
}

int main() {
    // Testo la registrazione di un giocatore
    char nickProva[][50] = {"nick valido", "nickname decisamente troppo lungo"};
	
    if(!pack(NICK_PROPOSITION_T, 50, 0, nickProva[0], buffer)) {
        printf("manco va la pack()...\n");
        return 0;
    }

    stampaEsito( gestisciMessaggio(1, buffer) );
    /*
    printf("Provo a registrare di nuovo il giocatore\n");

    pack(NICK_PROPOSITION_T, 50, 0, nickProva[0], buffer);

    stampaEsito( gestisciMessaggio(2, buffer) );
    printf("provo con un altro nick\n");
    
    */
    pack(NICK_PROPOSITION_T, 50, 0, "genoveffa", buffer);

    stampaEsito( gestisciMessaggio(3, buffer) );



    return 0;
}
