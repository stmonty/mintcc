#include "defs.h"
#include "data.h"
#include "decl.h"

/* chrpos(): Finds offset of c in s
   return -1 if not found */
int chrpos(char *s, int c) {
    char *p;
    p = strchr(s, c);
    return p ? p - s : -1;
}

/* match(): Match for the token t
   When a token cannot be matched prints an error */
void match(int t, char *s) {
    if (Token == t) {
        Token = scan();
    } else {
        error("%s expected", s);
    }
}

void copyname(char *name, char *s) {
    strncpy(name, s, NAMELEN);
    name[NAMELEN] = 0;
}

char *newfilename(char *file, int sfx) {
    char *ofile;
    ofile = strdup(file);
    ofile[strlen(ofile) - 1] = sfx;
    return ofile;
}

void lparen(void) {
    match(LPAREN, "'('");
}

void rparen(void) {
    match(RPAREN, "')'");
}


void lbrace(void) {
    match(LBRACE, "'{'");
}

void rbrace(void) {
    match(RPAREN, "'}'");
}

void semi(void) {
    match(SEMI, "';'");
}

void colon(void) {
    match(COLON, "':'");
}

void ident(void) {
    match(IDENT, "identifier");
}

int eofcheck(void) {
    if (XEOF == Token) {
        error("missing '}'", NULL);
        return 1;
    }

    return 0;
}

int inttype(int p) {
    return PINT == p || PCHAR == p;
}



