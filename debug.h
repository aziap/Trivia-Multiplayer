#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>
#include <stdarg.h>

#ifdef DEBUG_ON

// Stampa un messaggio di debug
static inline void debug(const char *format, ...) {
    printf("[DEBUG] ");
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

#else

// Se DEBUG_ON non è definita, non fa nulla
static inline void debug(const char *format, ...) {}

#endif // DEBUG_ON

#endif // DEBUG_H
