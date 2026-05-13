#include <stdio.h>
#include <stdarg.h>
#include "error.h"

static int err_count = 0;

void error_report(int line, const char *fmt, ...) {
    va_list ap;
    err_count++;
    fprintf(stderr, "Error (line %d): ", line);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr, "\n");
}

int error_count(void) {
    return err_count;
}

int error_summary(void) {
    if (err_count > 0)
        fprintf(stderr, "%d error(s) found.\n", err_count);
    return err_count;
}
