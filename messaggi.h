#ifndef MESSAGGI_H
#define MESSAGGI_H

#ifndef DEBUG_ON
#define DEBUG_ON
#include "debug.h"
#endif 

#include "costanti.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <stdbool.h>

// Tipi dei messaggi inviati dal server
const msg_t UNEXPECTED_ERR_T = 0;   // Generico per errori inaspettati
const msg_t MAX_CLI_REACH_T = 1;    // Il numero di utenti massimi è stato raggiunto
const msg_t CONNECT_OK_T = 2;       // La connessione è stata accettata
const msg_t THEME_LIST_T = 3;       // Lista dei temi
const msg_t QUESTION_T = 4;         // Domanda del quiz
const msg_t RANK_T = 5;             // Classifica
const msg_t NICK_UNAVAIL_T = 6;     // Nickname già preso (in caso contratio, 
                                    //     non mando conferme, mando 
                                    //     direttamente lista dei temi)

// Tipi dei messaggi inviati dal client
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


/*
Struttura generica di un messaggio:
+-----------+-----------+-----------+-------------------------------+
|   TIPO    |   DIM     |   FLAG    |           PAYLOAD             |        
+-----------+-----------+-----------+-------------------------------+

TIPO: msg_t (1 Byte)
    Indica come interpretare il messaggio e quali sono le successive operazioni di packing/unpacking
        e i controlli da effettuare sul payload.
DIM: msgsize_t (2 Byte)
    Dimensione del payload.
FLAG: uint8_t (1 Byte)
    Significativo nei messaggi che contengono le domande del quiz per dare al processo client
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
    flag_t flags;  // Can be 0
    msgsize_t msgLen;  // Can be 0
    char* payload;  // Can be NULL
};

bool pack(msg_t type, msgsize_t len, flag_t flags, char* payload, char* buffer);

// Serializzo il messaggio da inviare e lo inserisco nel buffer passato
//      per rifereimento
// @param type: valore campo tipo del messaggio
// @param len: numero di byte del payload
// @param flags: eventuali flag da inserire nel campo flag del messaggio
// @param payload: dati da inviare
// @param buffer: dove voglio che il messaggio finale venga inserito 
// @returns bool: true se il messaggio è stato correttamente inserito 
//      nel buffer, false altrimenti. Se restituisce false, il buffer non è
//      stato modificato 
bool pack(msg_t type, msgsize_t len, flag_t flags, char* payload, char* buffer) {
    if ((type > lastSrvType && type < firstCliType)
    || type > lastCliType)
    {
        printf("Errore: tipo del messaggio non valido: %u\n", type);
        // Lascio la gestione al chiamante
        return false;
    }

    int offset = 0;
    
    // Inserisco il tippo
    buffer[offset] = type;
    offset += sizeof(msg_t);

    // Inserisco la dimensione del messaggio
    buffer[offset] = htons(len);
    offset += sizeof(uint16_t);

    // Inserisco i flag
    buffer[offset] = flags;
    offset += sizeof(flag_t);

    strncpy(buffer + offset, payload, len);
    buffer[offset + len] = '\0';

    return true;  
}

// Deserializzo il messaggio ricevuto 
struct Messaggio* unpack(char *buffer) {
    // Creo la struttura in cui salvare i campi
    struct Messaggio* m = (struct Messaggio*)malloc(sizeof(struct Messaggio));

    if (m == NULL) {
        perror("Allocazione della memoria per il messaggio fallita");
        return NULL;
    }

    // Azzero tutti i campi
    memset(m, 0, sizeof(struct Messaggio));

    int offset = 0;
    
    // Popolo i campi della struttura
    m->type = buffer[offset];
    offset += sizeof(m->type);
    
    m->msgLen = ntohs(buffer[offset]);
    offset += sizeof(m->msgLen);

    m->flags = buffer[offset];
    offset += sizeof(m->flags);

    if(m->msgLen) {
        if ((m->payload = malloc(m->msgLen)) == NULL) {
            perror("Allocazione della memoria per il payload fallita");
            exit(EXIT_FAILURE);
        }
        memcpy(m->payload, buffer + offset, m->msgLen);
    }

    // Ho salvato le informazioni del messaggio, posso svuotare il buffer
    memset(buffer, 0, DIM_BUFFER);
    return m;
}

#endif
