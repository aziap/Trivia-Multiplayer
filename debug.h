#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>
#include <stdarg.h>

#ifdef DEBUG_ON

// Print a debug message with [DEBUG] prefix
static inline void debug(const char *format, ...) {
    printf("[DEBUG] ");
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

#else

// If DEBUG_ON is not defined, debug() does nothing
static inline void debug(const char *format, ...) {
    // nothing
}

#endif // DEBUG_ON

#endif // DEBUG_H
