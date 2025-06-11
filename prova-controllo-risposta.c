#include "server_game_logic.h"

int main() {
    struct Giocatore g;
    g.sd = 77;
    g.nick = "roland";
    g.statoTemi = {false, false};
    g.temaCorrente = 1;
    g.domandaCorrente = 1;
    g.punteggioCorrente = 0;

    printf("Esito della risposta: %d\n", 
        gestisciRispostaQuiz("ghepardo", &g)
    );

    return 0;
}