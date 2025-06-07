#include "debug.h"
#include "costanti.h"
#include "messaggi.h"

int main() {
    char buffer[DIM_BUFFER] = {0};

    char* payload = "12 caratteri";
    printf("Quanti caratteri? %lld\n", strlen(payload));
    uint16_t len = strlen(payload) + 1 + sizeof(uint16_t) + sizeof(msg_t);
    
    pack(RANK_T, len, 0, payload, buffer);

    char str[20] = {0};
    strncpy(str, buffer + 4, strlen(payload) + 1);

    // printf("lunghezza di size_t?: %llu\n", sizeof(size_t));  // 8

    printf("Messaggio da inviare:\n\tTipo: %u\n\tLunghezza: %u\n\tFlag: %u\n\tPayload: %s\n", buffer[0], buffer[1], buffer[3], str);

    printf("Qui ci arrivo?\n");

    // MI FERMO QUI
    // WHY?

    struct Messaggio* m = unpack(buffer);
    printf("Se non compaio, we fucked up\n");

    printf("Messaggio ricevuto:\n\tTipo: %u\n\tLunghezza: %u\n\tFlag: %u\n\tPayload: %s\n", m->type, m->msgLen, m->flags, m->payload);

    return 0;
}
