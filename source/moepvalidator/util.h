#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

bool verbose = false;

void setVerbose(bool v) { verbose = v; }

void logger(const char* format, ...) {
    if (verbose) {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }
}