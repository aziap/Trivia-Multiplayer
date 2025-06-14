#ifndef DEBUG_ON
#define DEBUG_ON
#endif

#include "debug.h"

#include "costanti.h"
#include "client.h"
#include "messaggi.h"
#include "input_check.h"

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

// Settata se il giocatore ha usato il comando endquiz
//      o se c'è stato un errore nella comunicazione con il server
bool ending = false;   
// Socket descriptor per la comunicazione col server
int sd;

void endquiz(char* buffer) {
    ending = true;
    printf("Chiusura della connessione con il server in corso...\n");
    pack(ENDQUIZ_T, 0, 0, "", buffer);
    send(sd, buffer, HEADER_LEN, 0);
    memset(buffer, 0, HEADER_LEN);
    close(sd);
} 

int connettiUtente(int port) {
    int server_fd;
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

static inline int attendiMessaggio(char* buffer) {
    int ricevuti = recv(sd, buffer, DIM_BUFFER, 0);
    if (ricevuti == 0) {
        printf("Connessione chiusa dal server\n");
        close(sd);
    } 
    if (ricevuti < 0) {
        perror("Errore nella comunicazione con il server");
        close(sd);
    }
    return ricevuti;
}

void showScore(char* buffer) {
    pack(SHOW_SCORE_T, 0, 0, "", buffer);
    send(sd, buffer, HEADER_LEN, 0);
    memset(buffer, 0, DIM_BUFFER);
    // TODO: eventualmente fare qualcosa con numRead
    int numRet = recv(sd, buffer, DIM_BUFFER, 0);
    struct Messaggio* m = unpack(buffer);
    memset(buffer, 0, DIM_BUFFER);
    stampaClassificaClient(m->payload, m->msgLen);
    free(m);
}


char sceltaMenu() {
    printf("Menù:\n");
    printFrame();
    for(int i = 1; i <= (sizeof(menu) / sizeof(char*)) ; i++) {
        printf("%d - %s\n", i, menu[i-1]);   
    }
    char opt = 0;
    while (opt != AVVIA_OPT && opt != ESCI_OPT) {
        printf("La tua scelta: ");
        opt = getchar();
        // Scarto eventuali caratteri successivi e il newline
        while (getchar() != '\n' && !feof(stdin));
    }
    return opt;
}

// Controlla la validità del formato del nickname.
// Se il formato è valido invia un messaggio al server per testare l'unicità.
// Da usare in loop finché il nickname non viene accettato dal server
// @return struct Messaggio* esito del controllo di unicità 
struct Messaggio* controlloNickname(char* buffer) {
    while(1) {
        printf("Scegli un nickname (deve essere univoco):\n");
        leggiStringa(buffer, DIM_NICK + 1);
        if (!checkNicknameFormat(buffer)) {
            printf("Formato non valido: il nickname deve contenere almeno un carattere non spazio e non può superare i 15 caratteri\n");
            continue;
        }
        char tmpNick[DIM_NICK] = {0};
        strncpy(tmpNick, buffer, DIM_NICK);
        // Il nick ha un formato valido, lo invio al server per testare
        //      l'unicità
        pack(NICK_PROPOSITION_T, DIM_NICK, 0, tmpNick, buffer);
        send(sd, buffer, DIM_NICK + HEADER_LEN, 0);
        memset(buffer, 0, DIM_BUFFER);
        if (attendiMessaggio(buffer) <= 0) return NULL; 
        // // Leggo la risposta del server
        struct Messaggio* m = unpack(buffer);
        return m;
    }
}

// Controlla la validità del formato e l'unicità del nickname
// Se termina con successo, all'uscita il buffer passato come parametro
//      contiene la lista dei temi sotto forma di stringa
// Può settare il flag ending
void scegliNickname(char* buffer) {
    struct Messaggio* esito;
    while (1) { 
        if ((esito = controlloNickname(buffer)) == NULL) {
            // La comunicazione col server è fallita, esco
            ending = true;
            return;
        }
        if (esito->type == NICK_UNAVAIL_T) {
            printf("Il nickname scelto è già preso da un altro giocatore");
            free(esito);
            continue;
        }
    	break;
    }// Ho ricevuto la lista dei temi, posso proseguire
    memcpy(buffer, esito->payload, esito->msgLen);
    free(esito);
}

// TODO: aggiornare giocatore

// TODO: Estrarre la lista dei temi dal payload del messaggio e formattarla in un array di stringhe

// Stampa la lista dei temi e attende la scelta del giocatore
// Può settare il flag ending
// Se all'uscita il flag ending non è settato, il buffer contiene la prima
//      domanda del quiz
int sceltaTema(char** temi, char* buffer) {
    int tema;

    printf("Quiz disponibili\n");
    printFrame();
    for (int i = 1; i <= NUM_TEMI; i++) {
        printf("%d - %s\n", i, temi[i - 1]);
    }
    printFrame();
    // Buffer di dimensione arbitraria, sufficiente a contenere un comando
    char scelta[20] = {0};
    while (1) {
        printf("La tua scelta:\n");
        if (checkValoreTema(
            tema = (uint8_t)atoi(leggiStringa(scelta, 20)))
        ) {
            if (giocatore.temiCompletati[tema - 1]) {
                printf("Hai già completato quel quiz\n");
                continue;
            } else break;
        }

        if (strcmp(ENDQUIZ, scelta) == 0) {
            endquiz(buffer);
            break;
        }
        else if (strcmp(SHOW_SCORE, scelta) == 0) {
            showScore(buffer);
            printFrame();
            continue;
        }
        else {
            printf("Scelta non valida\n");
            continue;
        }
    }
    // Il tema scelto era valido
    giocatore.temaCorrente = tema;
    scelta[0] = tema;
    
    // DEBUG
    // printf("tema scelto: %s\n", temi[tema-1]);
    
    pack(THEME_CHOICE_T, 1, 0, scelta , buffer);
    // TODO: che ci faccio?
    int numRet = send(sd, buffer, HEADER_LEN + 1, 0);
    memset(buffer, 0, HEADER_LEN + 1);
    if (attendiMessaggio(buffer) <= 0) {
        ending = true;
        return 0;
    }
    printFrame();
    printf("Quiz - %s\n", temi[tema - 1]);
    printFrame();
    return tema;
}

// Fa in loop le seguenti azioni: stampa la domanda ricevuta dal server, 
//      rimane in attesa di una risposta o di un comando del giocatore, invia 
//      la risposta al server e stampa l'esito. Termina quando il server
//      invia l'ultima domanda 
void svolgiQuiz(char* buffer, int tema) {
    giocatore.domandaCorrente = 1;
    char risposta[DIM_RISPOSTA] = {0};
    struct Messaggio* m;
    while(1) {
        // Il buffer contiene domanda inviata dal server
        m = unpack(buffer);
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
            free(m);
            break;
        }

        while(1) {    
            // Stampa la domanda, escludendo l'indice
            printf("%s\nRisposta:\n", m->payload + 4);
            // Leggi la risposta
            leggiStringa(risposta, DIM_RISPOSTA);
            // Fai il check per il comando
            if (strcmp(risposta, SHOW_SCORE) == 0) {
                showScore(buffer);
                printFrame();
                continue;
            }
            if (strcmp(risposta, ENDQUIZ) == 0) {
                endquiz(buffer);
                break;
            }
            // ImpacchettO la risposta e la mandao al server
            pack(ANSWER_T, DIM_RISPOSTA, 0, risposta, buffer);
            send(sd, buffer, DIM_RISPOSTA + HEADER_LEN, 0);
            memset(buffer, 0, DIM_RISPOSTA + HEADER_LEN);
            if (attendiMessaggio(buffer) <= 0) {
                ending = true;
                free(m);
            }
            break;  // Esco e stampo la domanda successiva
        } // END loop gestione risposta
        free(m);
        if (ending) break;
    } // END loop domanda - risposta
}



int main(int argc, char* argv[]) {
    if (argc < 2){
        printf("Numero di argomenti insufficiente. Attesa la porta in ascolto del server.\n");
        return 0;
    }

    while(1) {
        // Stampa menù iniziale
        char opzione = sceltaMenu();
        if (opzione == ESCI_OPT) {
            return 0;
        }
        // L'utente ha scelto di avviare il gioco

        int port = atoi(argv[1]);
        sd = connettiUtente(port);

        char buffer[DIM_BUFFER] = {0};

        // Aspetto il primo messaggio del server
        // ******************************
        //      Primo messaggio server ("max_cli" o "conn_ok")
        // ******************************
        struct Messaggio* m = attendiMessaggio(buffer);
        if (m == NULL) continue;    // Torno al menù di avvio
        if (m->type == MAX_CLI_REACH_T) {
            printf("Siamo spiacenti, ma numero massimo di utenti connessi è stato raggiunto.\nRiprova più tardi.\n");
            free(m);
            close(sd);
            continue;
        }

        scegliNickname(buffer);
        if (ending) continue;   // torno al manù iniziale

        // Il buffer contiene la lista dei temi, la estraggo e la elaboro
        char listaTemi[NUM_TEMI][DIM_TEMA];
        int daCopiare = 0;  // Numero di caratteri da copiare 
        int index = 0;
        for (int i = 0; i < NUM_TEMI; i++) {
            // Escludo il carattere che indica il numero del tema e il separatore
            daCopiare = strcspn(buffer+ index, "\n") - 2;   
            strncpy(listaTemi[i], buffer + 2 + index, daCopiare);
            index += (daCopiare + 3);  // Includo l'indice, il separatore, e il newline 
            listaTemi[i][daCopiare] = 0;
            debug("%s\n", listaTemi[i]);
        }

        memset(buffer, 0, NUM_TEMI * (DIM_TEMA + 2));

        // ******************************
        //      LOOP scelta tema
        // ******************************
        while(1) {
            giocatore.temaCorrente = sceltaTema(listaTemi, buffer);
            if (ending) break;
            svolgiQuiz(buffer, giocatore.temaCorrente);
            if (ending) break;
        } // END loop sessione quiz
    } // END main loop
    
    close(sd);
    return 0;

    /*       
    while(1) { 
        
        int numReceived = recv(server_fd, buffer, DIM_BUFFER, 0);
        if (numReceived <= 0) {
            printf("Connessione chiusa dal server\n");
            break;
        }
        buffer[numReceived] = '\0';
        

        // Gestisci risposta del server

        printf("Messaggio: ");
        leggiStringa(buffer, DIM_BUFFER);

        // Invio il messaggio

        send(server_fd, buffer, len, 0);
        break;
    }
    close(sd);
    return 0;
    */
}

