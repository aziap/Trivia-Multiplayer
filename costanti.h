// Costanti e tipi condivisi tra server e client

#include <stdint.h>

#ifndef COSTANTI_H
#define COSTANTI_H


#define SERVER_ADDR "127.0.0.1"

#define DIM_FLAG 1
#define DIM_DOMANDA 256
#define DIM_RISPOSTA 64
#define DIM_NICK 16    // Include il carattere di fine stringa 
#define DIM_TEMA 64
#define NUM_TEMI 2
#define DIM_BUFFER 1024

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


// tipo per la dimensione in byte del messaggio contenente la classifica
typedef uint32_t rnksize_t;


#endif
