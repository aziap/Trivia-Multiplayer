#ifndef MESSAGGI_H
#define MESSAGGI_H

#ifndef DEBUG_ON
#define DEBUG_ON
#endif 
#include "debug.h"

#include "costanti.h"
#include <stdint.h>
#include <stdio.h>

typedef uint8_t msg_t;
// Tipi dei messaggi inviati dal server
const msg_t UNEXPECTED_ERR_T = 0;   // Generico per errori inaspettati
const msg_t MAX_CLI_REACH_T = 1;    // Il numero di utenti massimi è stato raggiunto
const msg_t THEME_LIST_T = 2;       // Lista dei temi
const msg_t QUESTION_T = 3;         // Domanda del quiz
const msg_t RANK_T = 4;             // Classifica
const msg_t NICK_UNAVAIL_T = 5;     // Nickname già preso (non mando conferme, mando direttamente lista dei temi)

// Tipi dei messaggi inviati dal client
// ... show_score, endquiz, answer, scelta tema, proposta nick
// THEME_LIST_T e RANK_T possono essere condivise e assumono
//      significato diverso a seconda del mittente (oppure meglio 
//      fare tipi diversi? tanto ho 256 possibili valori, avoja a daje) 
// ...
const msg_t NICK_PROPOSITION_T = 10;
const msg_t THEME_CHOICE_T = 11;
const msg_t ANSWER_T = 12;
const msg_t ENDQUIZ_T = 13;
const msg_t SHOW_SCORE_T = 14;

// Definiscono il range dei tipi validi
// Il firstServerType sarebbe 0, ma essendo unsigned non ce ne possono essere di minori
const msg_t lastSrvType = NICK_UNAVAIL_T;
const msg_t firstCliType = NICK_PROPOSITION_T;
const msg_t lastCliType = SHOW_SCORE_T;


// tipo per la dimensione in byte del messaggio contenente la classifica
typedef uint16_t rnksize_t;

/*
Struttura generica di un messaggio:
+-----------+-----------+-----------+-------------------------------+
|   TIPO    |   DIM*    |   FLAG*   |           PAYLOAD*            |        
+-----------+-----------+-----------+-------------------------------+

I campi marcati con * sono opzionali

TIPO: msg_t (1 Byte)
    Indica come interpretare il messaggio e quali sono le successive operazioni di packing/unpacking
        e i controlli da effettuare sul payload.
DIM: rnksize_t (2 Byte)
    Presente solo nel messaggio che contiene la classifica. Contiene la dimensione totale del pacchetto
        (inclusi Tipo, Dim e Flag).
FLAG: uint8_t (1 Byte)
    Presente nei messaggi che contengono le domande del quiz per dare al processo client
        quali informazioni aggiuntive stampare e se farlo. 
        Ad esempio, uno dei flag indica l'esito della risposta precedente del quiz;
        un altro indica se è la prima domanda, ecc (TODO: espandere).
PAYLOAD: unsigned char[] (0 - 2023 Byte)
    Contenuto del paccketto. Nei messaggi del client, contiene l'input dell'utente,
        più delle informazioni di controllo aggiunte dal processo client.
        Nei messaggi del server sono i contenuti richiesti dal client.

*/

struct Messaggio {
    msg_t type;
    uint8_t flags;  // Can be 0
    rnksize_t rankMsgLen;  // Can be 0
    char* payload;  // Can be NULL
};

char* pack(msg_t type, rnksize_t len, uint8_t flags, char* payload, char* buffer) {
    if ((type > lastSrvType && type < firstCliType)
    || type > lastCliType)
    {
        printf("Errore: tipo del messaggio non valido: %u\n", type);
        // Lascio la gestione al chiamante
        return NULL;
    }

    *buffer = '\0';
    if( type == UNEXPECTED_ERR_T 
        || type == MAX_CLI_REACH_T 
        || type == NICK_UNAVAIL_T 
        || type == ENDQUIZ_T 
        || type == SHOW_SCORE_T 
        ){
            // Mi interessa solo il tipo
            *buffer >> sizeof(msg_t)*8;
            *buffer = type;
            return buffer;
        }
        
        debug("Il tipo di messaggio %u non è stato gestito\n", type);
        return NULL;
    
}


#endif
