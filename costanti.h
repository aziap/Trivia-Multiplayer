// Costanti e tipi condivisi tra server e client

#ifndef COSTANTI_H
#define COSTANTI_H

#include <stdint.h>

typedef uint16_t msgsize_t; // Per le variabili che contengono la dimensione del payload
typedef uint8_t msg_t;      // Per le variabili che contengono il tipo di un messaggio
typedef uint8_t flag_t;     // Per le variabili che contengono i flag di un messaggio

#define SERVER_ADDR "127.0.0.1"
#define DIM_DOMANDA 256
#define DIM_RISPOSTA 64 // TODO: togliere se si include la dimensione nel pacchetto
#define DIM_NICK 16    // Include il carattere di fine stringa 
#define DIM_TEMA 64     // Caratteri massimi dell'argomento di un tema
#define NUM_TEMI 2
#define NUM_DOMANDE 5
#define DIM_BUFFER 1024
#define MAX_DIM_PAYLOAD (DIM_BUFFER - sizeof(msg_t) - sizeof(flag_t) - sizeof(msgsize_t))
#define MAX_CLIENTS 100
#define SEP '~';    // Separatore

// Costatni usate solo lato client
#define AVVIA_OPT 1 // Comando per avviare il gioco nel menù iniziale
#define ESCI_OPT 2  // Comando per uscire dal gioco nel menù iniziale
#define SHOW_SCORE "show score"
#define ENDQUIZ "endquiz"

#endif
