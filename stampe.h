#include "costanti.h"

static inline void printFrame() {
    printf("++++++++++++++++++++++++++\n");
}

static inline void stampaTitolo() {
    printf("Trivia Quiz\n");
}

static inline void formattaListaTemi(char* buffer, char listaTemi[NUM_TEMI][DIM_TEMA]) {
        // Il buffer contiene la lista dei temi, la estraggo e la elaboro
        int daCopiare = 0;  // Numero di caratteri da copiare 
        int index = 0;
        debug("Temi nel buffer: %s\n", buffer); 
        for (int i = 0; i < NUM_TEMI; i++) {
            // Escludo il carattere che indica il numero del tema e il separatore
            daCopiare = strcspn(buffer+ index, "\n") - 2;   
            strncpy(listaTemi[i], buffer + 2 + index, daCopiare);
            index += (daCopiare + 3);  // Includo l'indice, il separatore, e il newline 
            listaTemi[i][daCopiare] = 0;
            debug("%s\n", listaTemi[i]);
        }
        
        debug("copia lista temi ok\n");

        memset(buffer, 0, NUM_TEMI * (DIM_TEMA + 2));
}


