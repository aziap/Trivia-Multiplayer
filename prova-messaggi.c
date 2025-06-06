#include "debug.h"
#include "costanti.h"
#include "messaggi.h"

int main() {
    char buffer[DIM_BUFFER] = {0};

    *buffer = pack(SHOW_SCORE_T, 0, 0, "", &buffer);

    printf("%u\n", buffer[0]);

    return 0;
}