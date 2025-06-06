#include "debug.h"
#include "costanti.h"
#include "messaggi.h"

int main() {
    char buffer[DIM_BUFFER] = {0};

    char *res = pack(SHOW_SCORE_T, 0, 0, "", buffer);

    printf("%u\n", res[0]);

    return 0;
}
