#ifndef CLIENT_GAME_LOGIC_H
#define CLIENT_GAME_LOGIC_H

#include "costanti.h"
#include "input_check.h"
#include "messaggi.h"

#include <stdio.h>

#ifndef DEBUG_ON
#define DEBUG_ON 1
#endif
#include "debug.h"

int gestisciMessaggioServer(char* buffer) {
    
    // ******************************
    //      Limite giocatori raggiunto
    // ******************************
        // Stampa messaggio 
        // ...

        // Torna al menù di avvio
        // ...

    // ******************************
    //      Connessione accettata
    // ******************************
        // Chiedi nickname (*1)
        // ...

        // Controlli formato nickname
        // ...

        // Invia nickname
        // ...


    // ******************************
    //      Nickname non disponibile
    // ******************************
        // Stampa messaggio
        // ...

        // Richiedi nickname (back to *1)
        // ...

    // ******************************
    //      Lista temi
    // ******************************
        // Stampa lista
        // ...

        // Leggi input
        // Controlla se "endquiz" o "show score" (*2)
        //      

    // ******************************
    //      Domanda
    // ******************************
        // Se non è la prima, stampa esito precedente
        // ...

        // Stampa domanda
        // ...

        // Leggi input
        // se "endquiz" o "show score" (*2)

    // ******************************
    //      Classifica
    // ******************************
        // Devo sapere se il giocatore era alla schermata
        //      scelta temi o ad una domanda del quiz

}

#endif