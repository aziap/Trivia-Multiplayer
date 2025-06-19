#ifndef MESSAGGI_H
#define MESSAGGI_H

#include "costanti.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <stdbool.h>

// Tipi dei messaggi inviati dal server
const msg_t MAX_CLI_REACH_T = 0;    // Il numero di utenti massimi è stato raggiunto
const msg_t CONNECT_OK_T = 1;       // La connessione è stata accettata
const msg_t THEME_LIST_T = 2;       // Lista dei temi
const msg_t QUESTION_T = 3;         // Domanda del quiz
const msg_t RANK_T = 4;             // Classifica
const msg_t NICK_UNAVAIL_T = 5;     // Nickname già preso (in caso contratio, 
                                    //     non mando conferme, mando 
                                    //     direttamente lista dei temi)

// Tipi dei messaggi inviati dal client
const msg_t NICK_PROPOSITION_T = 10;    // Nick scelto dal giocatore
const msg_t THEME_CHOICE_T = 11;        // Tema del quiz a cui il giocatore vuole partecipare
const msg_t ANSWER_T = 12;              // Risposta ad una domanda del quiz
const msg_t ENDQUIZ_T = 13;         
const msg_t SHOW_SCORE_T = 14;

// Definiscono il range dei tipi validi
// Il firstServerType sarebbe 0, ma essendo unsigned non ce ne possono essere di minori
const msg_t lastSrvType = NICK_UNAVAIL_T;
const msg_t firstCliType = NICK_PROPOSITION_T;
const msg_t lastCliType = SHOW_SCORE_T;

// Flag
const flag_t FIRST_QST = 0b0001;    // Prima domanda
const flag_t PREV_ANS_CORRECT = 0b0010;     // La risposta precedente era corretta (sbagliata se non settato)
const flag_t NO_QST = 0b0100;   // Il messaggio non contiene domande, solo l'esito dell'ultima risposta


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
FLAG: flag_t (1 Byte)
    Significativo nei messaggi che contengono le domande del quiz per dare al processo client
        quali informazioni aggiuntive stampare e se farlo. 
        Ad esempio, uno dei flag indica l'esito della risposta precedente del quiz;
        un altro indica se è la prima domanda, ecc.
PAYLOAD: char[] (0 - 1020 Byte)
    Contenuto del pacchetto. Nei messaggi del client, contiene l'input dell'utente,
        eventualmente elaborato dall'applicazione client.
        Nei messaggi del server sono i contenuti richiesti dal client.

*/

struct Messaggio {
    msg_t type;
    flag_t flags;  // Can be 0
    msgsize_t msgLen;  // Can be 0
    char* payload;  // Can be NULL
};

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
    uint16_t netlen = htons(len); 
    memcpy(buffer + offset, &netlen, sizeof(uint16_t));
    offset += sizeof(uint16_t);

    // Inserisco i flag
    buffer[offset] = flags;
    offset += sizeof(flag_t);

    if (len != 0) {    
        // Copio il payload
        memcpy(buffer + offset, payload, len);
        buffer[offset + len] = '\0';
    }
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
    
    uint16_t netlen; 
    memcpy(&netlen, buffer + offset, sizeof(uint16_t));
    m->msgLen = ntohs(netlen);
    offset += sizeof(m->msgLen);

    m->flags = buffer[offset];
    offset += sizeof(m->flags);

    if(m->msgLen) {
        if ((m->payload = malloc(m->msgLen)) == NULL) {
            perror("Allocazione della memoria per il payload fallita");
            exit(EXIT_FAILURE);
        }
        memcpy(m->payload, buffer + offset, m->msgLen);
    } else {
        m->payload = "";
    }

    // Ho salvato le informazioni del messaggio, posso pulire il buffer
    memset(buffer, 0, HEADER_LEN + m->msgLen);
    return m;
}

#endif
