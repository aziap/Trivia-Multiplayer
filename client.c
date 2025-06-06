#ifndef DEBUG_ON
#define DEBUG_ON
#endif

#include "debug.h"

#include "costanti.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(int argc, char* argv[]) {
    if (argc != 2){
        printf("Numero di argomenti non corretto! Aspettati: 1, ricevuti: %d\n", argc -1);
        return 0;
    }
    
    int server_fd;
    char buffer[DIM_BUFFER] = {0};
    struct sockaddr_in server_addr;
    int port = atoi(argv[1]);

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

    while(1) {
        printf("Messaggio: ");
        fgets(buffer, DIM_BUFFER, stdin);
        int len = strcspn(buffer, "\n");
        buffer[len] = 0;

        // Invio il messaggio

        send(server_fd, buffer, len, 0);

        // Risposta 
        int numReceived = recv(server_fd, buffer, DIM_BUFFER, 0);
        if (numReceived <= 0) {
            printf("Connessione chiusa dal server\n");
            break;
        }
        buffer[numReceived] = '\0';
        printf("Risposta del server: %s\n", buffer);
    }
    close(server_fd);
    return 0;
}
