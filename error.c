/*
 * MintCC : Minimal Toy C Compiler
 * error.c : Error Handling
 *
 */
#include "defs.h"
#include "data.h"
#include "decl.h"

/* cleanup : 
   Removes assembly source code and object code generated
   By the compiler and assmebler so far.
 */
static void cleanup(void) {
    if (!O_testonly && NULL != Basefile) {
        remove(newfilename(Basefile, 's'));
        remove(newfilename(Basefile, 'o'));
    }
}

void error(char *s, char *a) {
    if (Syntoken) {
        return;
    } 

    if (!Errors) {
        return;
    }

    fprintf(stderr, "error: %s: %d: ", File, Line);
    fprintf(stderr, s, a);
    fprintf(stderr, "\n");
    if (++Errors > 10) {
        Errors = 0;
        fatal("Over 10 Errors");
    }

}

void fatal(char * s) {
    error(s, NULL);
    error("fatal error", NULL);
    exit(EXIT_FAILURE);
}

/* cerror :
   
 */
void cerror(char *s, int c) {
    char buf[32];
    if (isprint(c)) {
        sprintf(buf, "'%c' (\\x%x)", c, c);
    }else{
        sprintf(buf, "\\x%x", c);
    }
    error(s, buf);
}

/* synch :
   
 */
int synch(int syn) {
    int t;
    t = scan();
    while (t != syn) {
        if (EOF == t) {
            fatal("found EOF in error recovery");
        }
        t = next();
    }
    Syntoken = syn;
    return t;
}
