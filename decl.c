/*
 * MintCC : Minimal Toy C Compiler
 * decl.c : Declaration Parser
 *
 */

#include "defs.h"
#include "data.h"
#include "decl.h"

/*
 * The Declaration Parser is the top level of the parser and accepts
 * the whole program as input
 */

int declarator(int pmtr, int scls, char *name, int *pprim, int *psize,
               int *pval, int *pinit);

static void enumdecl(void) {
    int v = 0;
    char name[NAMELEN + 1];

    Token = scan();
    if (IDENT == Token) {
        Token = scan();
    }
    lbrace();
    for (;;) {
        copyname(name, Text);
        ident();
        if (ASSIGN == Token) {
            Token = scan();
            v = constexpr();
        }

        addglob(name, PINT, TCONSTANT, 0, 0, v++, NULL, 0);
        if (Token != COMMA) {
          break;
        }
        Token = scan();
        if (eofcheck())
          return;
    }
    match(RBRACE, "'}'");
    semi();
}

/*
 * initlist() : Specifies the size of a list, returns the size of list
 *
 */
static int initlist(char *name, int prim) {
    int v;
    int n = 0;
    char buf[30];

    gendata();
    genname(name);
    if (STRLIT == Token) {
        if (PCHAR != prim) {
            error("Initializer type mismatch: %s", name);
        }
        gendefs(Text, Value);
        gendefb(0);
        Token = scan();
        return Value - 1;
    }
    lbrace();
    while (Token != RBRACE) {
        v = constexpr();
        if (PCHAR == prim) {
            if (v < 0 || v > 255) {
                sprintf(buf, "%d", v);
                error("Initializer out of range: %s", buf);
            }
            gendefb(v);
        }
        else {
            gendefw(v);
        }
        n++;
        if (COMMA == Token) {
            Token = scan();
        }
        else {
            break;
        }
        if (eofcheck())
            return 0;
    }
    Token = scan();
    if (!n) {
        error("Too few initializers", NULL);
    }
    return n;
}

int primtype(int t) {
    switch(t) {
    case CHAR:
        return PCHAR;
    case INT:
        return PINT;
    }
    return PVOID;
}
