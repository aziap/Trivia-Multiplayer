#include "server_game_logic.h"

int main() {
	
    char riga[DIM_DOMANDA] = {0};
    prelevaRiga(1, 5, riga, DIM_DOMANDA, "./domande.txt");
    printf("Riga prelevata: %s\n", riga);

    return 0;
}
