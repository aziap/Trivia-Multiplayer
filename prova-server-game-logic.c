#include "server_game_logic.h"
#include "messaggi.h" 

char* buffer[DIM_BUFFER] = {0};

void stampaEsito(int e) {
    printf("esito gestione messaggio: %s\n", 
    e == ERROR ? "errore" 
    : e == DISCONNECT ? "disconnetti il giocatore"
    : "ok");
}

int main() {
    // Testo la registrazione di un giocatore
    char nickProva[2][50] = {0};
    nickProva[0] = "nick valido";
    nickProva[1] = "nickname decisamente troppo lungo";

    if(!pack(NICK_PROPOSITION_T, 50, 0, nickProva[0]), buffer) {
        printf("manco va la pack()...\n");
        return 0;
    }

    stampaEsito( gestisciMessaggio(1, buffer) );



    return 0;
}