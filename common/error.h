#ifndef ERROR_H
#define ERROR_H

/* Unified error reporting for all compiler phases.
   All errors go to stderr; stdout is reserved for TAC/result output. */

/* Report a single error. Increments the internal error count. */
void error_report(int line, const char *fmt, ...);

/* Return the total number of errors reported so far. */
int error_count(void);

/* Print a summary line and return the count (convenience). */
int error_summary(void);

#endif /* ERROR_H */
