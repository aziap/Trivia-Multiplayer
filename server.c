#include "costanti.h"
#include "messaggi.h"
#include "server_game_logic.h"
#include "stampe.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>

#define PORT 1919 
#define MAX_BACKLOG 128 // Numero massimo di connessioni in attesa 


// TODO: documentare meglio
static inline void disconnettiClient(int index, int *sdArr, int *nextSd, fd_set *master) {
    int sd = sdArr[index];
    close(sd);
    // Sposto l'ultimo sd nella posizione appena liberata
    //      e decremento il numero di client
    sdArr[index] = sdArr[--(*nextSd)];

    // Rimuovo l'sd dal set master
    FD_CLR(sd, master);
    rimuoviGiocatore(sd);
}

static inline void closeAll( int listenerSd, int* sdArr, int* nextSd) {
    while(*nextSd > 0) {
        rimuoviGiocatore(sdArr[--(*nextSd)]);
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

    // Inserisco il listener nel set master
    FD_SET(listener, &master);
    maxfd = listener;
    
    // Prelevo e stampo la lista dei temi
    leggiListaTemi(buffer);
    char listaTemi[NUM_TEMI][DIM_TEMA];
    formattaListaTemi(buffer, listaTemi);
    
    stampaTitolo();
    printFrame();
    printf("Temi:\n");
    for (int i = 0; i < NUM_TEMI; i++) {
    	printf("%d - %s\n", i + 1, listaTemi[i]);
    }
    printFrame();
    putchar('\n');
    stampaPartecipanti();
    printFrame();

    while (1) {
    	// Copio il master set
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
                closeAll(listener, client_socket, &numclient);
                exit(EXIT_FAILURE);
            }

		    if (numclient < MAX_CLIENTS) {
		        // Imposto il socket come non bloccante
		        fcntl(newfd, F_SETFL, O_NONBLOCK);
		        // Lo inserisco nell'array dei client socket e nel set master 
		        FD_SET(newfd, &master);
		        if (maxfd < newfd) maxfd = newfd;
		        client_socket[numclient++] = newfd;
		        pack(CONNECT_OK_T, 0, 0, "", buffer);
		        send(newfd, buffer, HEADER_LEN, 0);
				memset(buffer, 0, HEADER_LEN);
		    } 
            else {
				// Se ho raggiunto il numero massimo di giocatori, mando un messaggio
				//      all'utente per avvisarlo, e chiudo la connessione
				pack(MAX_CLI_REACH_T, 0, 0, "", buffer);
				send(newfd, buffer, HEADER_LEN, 0);
				close(newfd);
				FD_CLR(newfd, &master);
				memset(buffer, 0, HEADER_LEN);
			}
        } // END gestione nuove connessioni
        
        int sd;
        // Controllo se ci sono messaggi dai client connessi
        for (int i = 0; i < numclient; i++) {
            sd = client_socket[i];
         
            if (FD_ISSET(sd, &readfd)) {
                int numReceived = read(sd, buffer, DIM_BUFFER);
                
                if (numReceived == 0) {
                    // Client disconnesso
                    // Chiudo il socket e decremento il numero di giocatori
                    disconnettiClient(i, client_socket, &numclient, &master);
                    // Stampo le informazioni aggiornate sui giocatori
                    stampaPartecipanti();
                    stampaClassifica();
                    stampaQuizCompletati();
                    printFrame();
                    continue;
                }
				if (numReceived < 0 
                    && errno != EWOULDBLOCK // Il socket risulta pronto ma non ci sono ancora dati
                    && errno != EAGAIN      // Come sopra
                    && errno != EINTR       // System call interrotta da un segnale
                ) {
					// Errore critico
                    perror("Errore in recv()");
                    closeAll(listener, client_socket, &numclient);
                    exit(EXIT_FAILURE);
                }
                if (numReceived < 0) {
					// Errore non critico
					continue;
				}
                // C'è un nuovo messaggio
                int toSend = -1;
                int result = gestisciMessaggio(sd, buffer, &toSend);
                if (result == OK) {
                	int ret;
                    if ((ret = send(sd, buffer, toSend, 0)) == -1) {
                        perror("Errore nella send()");
                        disconnettiClient(i, client_socket, &numclient, &master);
                    }
                    memset(buffer, 0, toSend);
                }
                else if (result == DISCONNECT) {
                    disconnettiClient(i, client_socket, &numclient, &master);
                } else {
		            // C'è stato un errore critico
		            closeAll(listener, client_socket, &numclient);
		            exit(EXIT_FAILURE);
                }
                // Stampo le informazioni aggiornate dei giocatori
                putchar('\n');
                stampaPartecipanti();
                stampaClassifica();
                stampaQuizCompletati();
                printFrame();
            } // END gestione socket pronto
        } // END for(;;) - gestione nuovi messaggi dai client 
    } // END while() - main loop

    closeAll(listener, client_socket, &numclient);
    return 0;
}
