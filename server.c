#include "costanti.h"
#include "messaggi.h"
#include "server_game_logic.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>

// DEBUG
#ifndef DEBUG_ON
#define DEBUG_ON
#endif
#include "debug.h"
// END DEBUG

#define PORT 1919 
#define MAX_BACKLOG 128 // Numero massimo di connessioni in attesa 


/**
 * read -> recv
 * send -> write
*/


static inline void disconnettiClient(int index, int *sdArr, int *nextSd, fd_set *master) {
    // DEBUG
    int sd = sdArr[index];
    
    close(sdArr[index]);
    // Sposto l'ultimo sd nella posizione appena liberata
    //      e decremento il numero di client
    sdArr[index] = sdArr[--(*nextSd)];

    // Rimuovo l'sd dal set master
    debug("Rimuovo il sd dal master set\n");
    FD_CLR(sd, master);

    rimuoviGiocatore(sd);
}

static inline void closeAll( int listenerSd, int* sdArr, int* nextSd) {
    while(*nextSd > 0) {
        rimuoviGiocatore(sdArr[--(*nextSd)])
        close(sdArr[*nextSd]);
    }
    close((listenerSd));
}


int main() {
    int listener, newfd, maxfd, ret; 
    // Numero massimo di connessioni in attesa di accept()
    int backlog = MAX_CLIENTS <= MAX_BACKLOG ? MAX_CLIENTS : MAX_BACKLOG;
    int numclient = 0; 
    // Array che contiene tutti i descrittori di socket dei
    //      client connessi
    int client_socket[MAX_CLIENTS] = {0};
    
    fd_set master;
    fd_set readfd;
    
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    char buffer[DIM_BUFFER] = {0};

    // Azzero i set
    FD_ZERO(&master);
    FD_ZERO(&readfd);

    // Creo il socket listener
    if((listener = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Creazione del socket del server fallita");
        exit(EXIT_FAILURE);
    }

    // Riutilizza il socket se...?
    // TODO: documentare meglio
    int yes = 1;
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    // Assegno indirizzo e porta
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Binding del listener
    if (bind(listener, (struct sockaddr *)&address, addrlen) < 0) {
        perror("Bind fallito");
        close(listener);
        exit(EXIT_FAILURE);
    }

    // Metto il socket in ascolto di richieste di connessione
    if (listen(listener, backlog) < 0) {
        perror("Listen fallito");
        exit(EXIT_FAILURE);
    }

    debug("Trivia Game Server in ascolto sulla porta %d\n", PORT);

    // Inserisco il listener nel set master
    FD_SET(listener, &master);
    maxfd = listener;

    while (1) {
        readfd = master;

        // Se la select viene interrotta da un segnale, riprovo
        do {
            ret = select(maxfd + 1, &readfd, NULL, NULL, NULL);
        } while (ret < 0 && errno == EINTR);

        if (ret < 0) {
            perror("Select fallito");
            closeAll(listener, client_socket, &numclient);
            exit(EXIT_FAILURE);
        }


        // Controllo se ci sono nuove richieste di connessione
        if (FD_ISSET(listener, &readfd)) {
            if ((newfd = accept(listener, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
                perror("Accept fallito");
                // TODO: close all
                exit(EXIT_FAILURE);
            }
            debug("Nuova connessione stabilita. Socket: %d\n", newfd);

            // TODO: gestire il caso di massimo numero di giocatori raggiunto
        }


        // Imposto il socket come non bloccante
        fcntl(newfd, F_SETFL, O_NONBLOCK);
        // Lo inserisco nell'array dei client socket e nel set master 
        FD_SET(newfd, &master);
        if (maxfd < newfd) maxfd = newfd;
        client_socket[numclient++] = newfd;

        // TODO: dire al glh di aggiungere una entry con un nuovo giocatore (per ora a NULL) nell'indice
        //  oppure no? registro un giocatore solo quando sceglie un nick

        int sd;
        // Controllo se ci sono messaggi dai client connessi
        for (int i = 0; i < numclient; i++) {
            
            debug("Controllo se ci sono nuovi messaggi\n");
            
            sd = client_socket[i];
         
            debug("esamino il socket %d\n", sd);
            
            if (FD_ISSET(sd, &readfd)) {
                int numReceived = read(sd, buffer, DIM_BUFFER);
                
                debug("ricevuti %d byte\n", numReceived);
                
                if (numReceived == 0) {
                    // Client disconnesso
                    debug("Client disconnesso, socket: %d\n", sd);
                    // Chiudo il socket e decremento il numero di giocatori
                    disconnettiClient(i, client_socket, &numclient, &master);
                    continue;
                }
                if (numReceived < 0 
                    && errno != EWOULDBLOCK 
                    && errno != EWOULDBLOCK 
                    && errno != EINTR ) 
                {
                    perror("recv() failed:");
                    closeAll(listener, client_socket, &numclient);
                    exit(EXIT_FAILURE);
                }
                if (numReceived < 0) continue;
                
                // C'è un nuovo messaggio!

                // DEBUG
                debug("Newline at %d\n", strcspn(buffer, "\n"));
                
                buffer[strcspn(buffer, "\n")] = 0;
                
                debug("Il client dice: %s\n", buffer);
                char* str ="Yo";
                send(sd, str, sizeof(str), 0);
                // END DEBUG
            }
        } // END for(;;) - handling ready client sockets 
    } // END while() - main loop

    closeAll(listener, client_socket, &numclient);
    return 0;
}
