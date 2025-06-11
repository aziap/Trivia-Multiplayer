#include "server_game_logic.h"

int main() {
    char* riga = prelevaRiga(1, 1, DIM_DOMANDA, "./domande.txt");
    printf("Riga prelevata: %s\n", riga);

    return 0;
}