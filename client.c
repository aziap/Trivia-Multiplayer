#ifndef DEBUG_ON
#define DEBUG_ON
#endif

#include "debug.h"

#include "costanti.h"
#include "client.h"
#include "messaggi.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

char* menu[]= {
    "Comincia una sessione di Trivia", 
    "Esci"
};

struct GiocatoreClient {
    int temaCorrente;
    int domandaCorrente;
    bool temiCompletati[NUM_TEMI];
} giocatore;

bool endQuiz = false;
int sd; // Socket descriptor per la comunicazione col server

int connettiUtente(int port) {
    int server_fd;
    char buffer[DIM_BUFFER] = {0};
    struct sockaddr_in server_addr;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Errore nella creazione del socket");
        exit(EXIT_FAILURE);
    }

    // Configuro l'indirizzo del server
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, SERVER_ADDR, &server_addr.sin_addr) <= 0) {
        printf("Indirizzo del server non valido\n");
        exit(EXIT_FAILURE);
    }    
    
    // Connessione al server
    if (connect(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connect");
        exit(EXIT_FAILURE);
    }
    return server_fd;
}

void showScore(char* buffer) {
    pack(SHOW_SCORE_T, 0, 0, "", buffer);
    // TODO: eventualmente fare qualcosa con numRet
    int numRet = send(sd, buffer, HEADER_LEN, 0);
    memset(buffer, 0, DIM_BUFFER);
    numRet = recv(ds, buffer, DIM_BUFFER, 0);
    // TODO: eventualmente fare qualcosa con numRead
    struct Messaggio* m = unpack(buffer);
    stampaClassificaClient(m->payload, m->msgLen);
    free(m);
}


char sceltaMenu() {
    printf("Menù:\n");
    printFrame();
    for(int i = 1; i <= (sizeof(menu) / sizeof(char*)) ; i++) {
        printf("%d - %s\n", i, menu[i]);   
    }
    char opt ;
    while (1) {
        printf("La tua scelta:\n");
        opt = getchar();
        if (opt != AVVIA_OPT || opt != ESCI_OPT) break;
    }
    return opt;
}


struct Messaggio controlloNickname(char* buffer) {
    while(1) {
        printf("Scegli un nickname (deve essere univoco):\n");
        leggiStringa(buffer, DIM_NICK + 1);
        if (!checkNicknameFormat(buffer)) {
            fprintf("Formato non valido: il nickname deve contenere almeno un carattere non spazio e non può superare i 15 caratteri\n");
            continue;
        }
        char tmpNick[DIM_NICK] = {0};
        strncpy(tmpNick, buffer, DIM_NICK);
        // Il nick ha un formato valido, lo invio al server per testare
        //      l'unicità
        pack(NICK_PROPOSITION_T, DIM_NICK, 0, tmpNick, buffer);
        // TODO: c'è da farci qualcosa?
        int numRet = send(sd, buffer, DIM_NICK + HEADER_LEN, 0);
        memset(buffer, 0, DIM_BUFFER);
        numRet = recv(sd, buffer, DIM_BUFFER, 0);
        // Leggo la risposta del server
        struct Messaggio* m = unpack(buffer);
        // TODO: se NICK_UNAVAIL_T, rientro in questa funzione
        return m;
    }
}

void scegliNickname(char* buffer) {
    struct Messaggio* esito;
    while (1) { 
        esito = controlloNickname(buffer);
        if (esito->type == NICK_UNAVAIL_T) {
            printf("Il nickname scelto è già preso da un altro giocatore");
            continue;
        }
    }// Ho ricevuto la lista dei temi, posso proseguire
    memccpy(buffer, esito->payload, esito->msgLen);
    break;
}

// TODO: aggiornare giocatore

// TODO: Estrarre la lista dei temi dal payload del messaggio e formattarla in un array di stringhe

void sceltaTema(char** temi, char* buffer) {
    int tema;

    pritnf("Quiz disponibili\n");
    printFrame();
    for (int i = 1; i <= NUM_TEMI; i++) {
        printf("%d - %s\n", i, temi[i]);
    }
    printFrame();
    
    // Buffer di dimensione arbitraria, sufficiente a contenere un comando
    char scelta[20] = {0}
    while (1) {
        printf("La tua scelta:\n");
        if (checkValoreTema(
            tema = atoi(leggiStringa(scelta)))) 
        {
            if (giocatore.temiCompletati[tema - 1]) {
                printf("Hai già completato quel quiz\n");
                continue;
            } else break;
        }

        if (strcmp(ENDQUIZ, buffer) == 0) {
            // TODO: endquiz
            printf("endquiz: working on it...\n");
            endQuiz = true;
            break;
        }
        else if (strcmp(SHOW_SCORE, buffer) == 0) {
            // TODO: showScore()
            printf("showScore(): working on it...\n");
            continue;
        }
        else {
            printf("Scelta non valida\n");
            continue;
        }
    }

    // Il tema scelto era valido
    giocatore.temaCorrente = tema;
    pack(THEME_CHOICE_T, 0, 0, (uint8_t)tema, buffer);
    // TODO: che ci faccio?
    int numRet = send(sd, buffer, HEADER_LEN + 1, 0);
    memset(buffer, 0, HEADER_LEN + 1);
    if (numRet = recv(sd, buffer, DIM_BUFFER, 0) == 0) {
        printf("Gestione disconnessione: coming soon\n");
        return;
    }
    // TODO: fare l'unpack nella funzione per le domande
    printFrame();
    printf("Quiz - %s\n", temi[tema - 1]);
    printFrame();
}

void svolgiQuiz(char* buffer, int tema) {
    giocatore.domandaCorrente = 1;
    char domanda[DIM_DOMANDA] = {0};
    char risposta[DIM_RISPOSTA] = {0};
    struct Messaggio* m;
    while(1) {
        // Il buffer contiene domanda inviata dal server
        m = unpack(buffer);
        memset(buffer, 0, DIM_BUFFER);
        // Leggere i flag:
        // Se non è la prima domanda, stampare l'esito
        if (!(m->flags & FIRST_QST)) {
            printf("Risposta %s\n", m->flags & PREV_ANS_CORRECT ? "corretta" : "sbagliata");
        }

        // Se è l'ultima domanda, aggiorno lo stato del giocatore 
        //      e torno alla scelta dei temi
        if (m->flags & NO_QST) {
            giocatore.temaCorrente = 0;
            giocatore.domandaCorrente = 0;
            giocatore.temiCompletati[tema - 1] = true;
            free(m)
            break;
        }

        while(1) {    
            // Stampa la domanda, escludendo l'indice
            printf("%s\nRisposta:\n", m->payload + 4);
            // Leggi la risposta
            leggiStringa(risposta, DIM_RISPOSTA);
            // Fai il check per il comando
            if (strcmp(risposta, SHOW_SCORE) == 0) {
                //  TODO: showScore();
                printf("showScore(): working on it...\n");
                continue;
            }
            if (strcmp(risposta, ENDQUIZ) == 0) {
                // Gestione endquiz
                // ...
                printf("endquiz(): working on it...\n");
                endQuiz = true;
                break;
            }
            // ImpacchettO la risposta e la mandao al server
            pack(ANSWER_T, DIM_RISPOSTA, 0, risposta, buffer);
            send(sd, buffer, DIM_RISPOSTA + HEADER_LEN, 0);
            memset(buffer, 0, DIM_RISPOSTA + HEADER_LEN);
            int numRet = recv(sd, buffer, DIM_BUFFER, 0);
            break;  // Esco e stampo la domanda successiva
        } // END loop gestione risposta
        free(m);
        if (endQuiz) break;
    } // END loop domanda - risposta
}


int main() {
    char scelta = sceltaMenu();
    printf("%s\n", scelta);
}


int veroMain(int argc, char* argv[]) {
    
    if (argc < 2){
        printf("Numero di argomenti insufficiente. Attesa la porta in ascolto del server.\n");
        return 0;
    }

    
    // Stampa menù iniziale (LOOP)

    // Leggi opzione
    //  se ESCI, return
    // se incorretta, ristampa menù
    // ...

    // Se AVVIO, comincia connessione
    
    int port = atoi(argv[1]);
    sd = connettiUtente(port);

    // Aspetto il primo messaggio del server
    // ******************************
    //      Primo messaggio server ("max_cli" o "conn_ok")
    // ******************************
    // Se limite giocatori raggiunto, 
        // Stampa messaggio
        // ...

        // close(sd)  
        //  BACK TO START

    // ******************************
    //      LOOP scelta nickname (condizione: nickValido == true)
    // ******************************
        // bool nickValido = false;
        // Leggi input utente
        // ...

        // Valida nickname
            // if not valido, stampa messaggio
            // ...
            // continue;

        // Invia nickname
            // If già preso, stampa messaggio
            // ... 
            // continue;

    // ******************************
    //     FINE LOOP scelta nickname
    // ******************************
    
    // !!!
    // da qui in poi, devo "salvare lo stato"
    // e controllare gli input per showscore e endquiz
    // !!!
    
    // ******************************
    //      LOOP scelta tema (condizione: temaValido == true)
    // ******************************
        

            
    while(1) { 
        /*
        int numReceived = recv(server_fd, buffer, DIM_BUFFER, 0);
        if (numReceived <= 0) {
            printf("Connessione chiusa dal server\n");
            break;
        }
        buffer[numReceived] = '\0';
        */

        // Gestisci risposta del server

        printf("Messaggio: ");
        leggiStringa(buffer, DIM_BUFFER);

        // Invio il messaggio

        send(server_fd, buffer, len, 0);
        break;
    }
    close(sd);
    return 0;
}
