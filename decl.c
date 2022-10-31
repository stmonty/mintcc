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

static int pmtrdecls(void) {
    char name[NAMELEN + 1];
    int prim;
    int type;
    int size;
    int na;
    int y;
    int addr;
    int dummy;

    if (RPAREN == Token) {
        return 0;
    }
    na = 0;
    for (;;) {
        if (na > 0 && ELLIPSIS == Token) {
            Token = scan();
            na = -(na + 1);
            break;
        }
        else if (IDENT == Token) {
            prim = PINT;
        }
        else {
            if (Token != CHAR && Token != INT && Token != VOID) {
                error("Type specifier expected at: %s", Text);
                Token = synch(RPAREN);
                return na;
            }
            name[0] = 0;
            prim = primtype(Token);
            Token = scan();
            if (RPAREN == Token && prim == PVOID && !na) {
                return 0;
            }
        }
        size = 1;
        type = declarator(1, CAUTO, name, &prim, &size, &dummy, &dummy);
        addloc(name, prim, type, CAUTO, size, 0, 0);
        na++;
        if (COMMA == Token) {
            Token = scan();
        }
        else {
            break;
        }
    }
    addr = INTSIZE * 2;
    for (y = Locs; y < NSYMBOLS; y++) {
        addr += INTSIZE;
        Vals[y] = addr;
    }
    return na;
}

/*
 * pointerto() : Promotes type prim to prim* (No more than two levels of indirecton)
 */
int pointerto(int prim) {
    if (CHARPP == prim || INTPP == prim || VOIDPP == prim || FUNPTR == prim) {
        error("Too many levels of indirection," NULL);
    }
    switch (prim) {
    case INTPTR:
      return PINT;
    case CHARPTR:
      return PCHAR;
    case VOIDPTR:
      return PVOID;
    case INTPP:
      return INTPTR;
    case CHARPP:
      return CHARPTR;
    default:
        return VOIDPP;
    }
}

int declarator(int pmtr, int scls, char *name, int *pprim, int *psize,
               int *pval, int *pinit) {
    
    int type = TVARIABLE;
    int ptrptr = 0;

    if (STAR == Token) {
        Token = scan();
        *pprim = pointerto(*pprim);
        if (STAR == Token) {
            Token = scan();
            *pprim = pointerto(*pprim);
            ptrptr = 1;
        }
    }
    else if (LPAREN == Token) {
        if (*pprim != PINT) {
            error("Function pointers are limited to 'int'", NULL);
        }
        Token = scan();
        *pprim = FUNPTR;
        match(STAR, "(*name) ()");
    }
    if (IDENT != Token) {
        error("Missing identifier at: %s", Text);
        name[0] = 0;
    }
    else {
        copyname(name, Text);
        Token = scan();
    }
    if (FUNPTR == *pprim) {
        rparen();
        lparen();
        rparen();
    }
    if (!pmtr && ASSIGN == Token) {
        Token = scan();
        *pval = constexpr();
        if (*pval && !inttype(*pprim)) {
            error("Non-null pointer initialization", NULL);
        }
        *pinit = 1;
    }
    else if (!pmtr && LPAREN == Token) {
        Token = scan();
        *psize = pmtrdecls();
        rparen();
        return TFUNCTION;
    }
    else if (LBRACK == Token) {
        if (ptrptr) {
            error("Too many levels of indirection," NULL);
        }
        Token = scan();
        if (RBRACK == Token) {
            Token = scan();
            if (pmtr) {
                *pprim = pointerto(*pprim);
            }
            else {
                type = TARRAY;
                *psize = 1;
                if (ASSIGN == Token) {
                    Token = scan();
                    if (!inttype(*pprim)) {
                        error("Initialization of pointer array not supported", NULL);
                    }
                    *psize = initlist(name, *pprim);
                    if (CAUTO == scls) {
                        error("Initialization of local array not supported: %s", name);
                    }
                    *pinit = 1;
                } else if (CEXTERN != scls) {
                    error("Automatically-sized array lacking initialization: %s",
                          name);
                }
            }
        }
        else {
            if (pmtr) {
                error("Array size not supported in parameters: %s", name);
            }
            *psize = constexpr();
            if (*psize <= 0) {
                error("Invalid array size", NULL);
                *psize = 1;
            }
            type = TARRAY;
            rbrack();
        }
    }
    if (PVOID == *pprim) {
        error("'void' is not a valid type: %s", name);
    }
    return type;
}

void signature(int fn, int from, int to) {
    char types[MAXFNARGS + 1];
    int i;

    if (to - from > MAXFNARGS) {
        error("Too many function parameters", Names[fn]);
    }
    for (i = 0; i < MAXFNARGS && from < to; i++) {
        types[i] = Prims[--to];
    }
    types[i] = 0;
    if (NULL == Mtext[fn]) {
        Mtext[fn] = globname(types);
    }
    else if (Sizes[fn] >= 0 && strcmp(Mtext[fn], types)) {
        error("Redefinition does not match prior type: %s," Names[fn]);
    }
}

static int localdecls(void) {
    char name[NAMELEN + 1];
    int prim;
    int type;
    int size;
    int addr = 0;
    int val;
    int ini;
    int stat;
    int pbase;
    int rsize;

    Nli = 0;
    while (STATIC == Token || INT == Token ||
           CHAR == Token || VOID == Token) {
        stat = 0;
        if (STATIC == Token) {
            Token = scan();
            stat = 1;
            if (INT == Token || CHAR == TOKEN || VOID == Token) {
              prim = primtype(Token);
              Token = scan();
            } else {
              prim = PINT;
            }
        }
        else {
            prim = primtype(Token);
            Token = scan();
        }
        pbase = prim;
        for (;;) {
            prim = pbase;
            if (eofcheck()) {
                return 0;
            }
            size = 1;
            ini = val = 0;
            type = declarator(0, CAUTO, name, &prim, &size, &val, &ini);
            rsize = objsize(prim, type, size);
            rsize = (rsize + INTSIZE - 1) / INTSIZE * INTSIZE;
            if (stat) {
                addloc(name, prim, type, CLSTATC, size, label(), val);
            }
            else {
                addr -= rsize;
                addloc(name, prim, type, CAUTO, size, addr, 0);
            }
            if (ini && !stat) {
                if (Nli >= MAXLOCINIT) {
                    error("Too many local initializers", NULL);
                    Nli = 0;
                }
            LIaddr[Nli] = addr;
            LIval[Nli++] = val;                
            }
            if (COMMA == Token) {
                Token = scan();
            }
            else {
                break;
            }
        }
        semi();
    }
    return addr;
}

/*
 * decl() : Accepts a top level declaration (without type specifier)
 * This can be either a function definition or a list of variable decls
 *
 * When the object being defined is a function, 'decl' accepts any local
 * declarations at the beginning of the function body and emits code to
 * perform the following actions at run time:
 * - Setup a local context
 * - Allocate space for local variables
 * - Initialize automatic local variables
 * - Deallocate space for local symbols
 * - Restore the caller's context and return
 *
 * It also compies the function body after emitting code for initializing
 * automatic locals but before emitting code for deallocating local space
 */
void decl(int clss, int prim) {
    char name[NAMELEN + 1];
    int pbase;
    int type;
    int size = 0;
    int val;
    int init;
    int lsize;

    pbase = prim;
    for (;;) {
        prim = pbase;
        val = 0;
        init = 0;
        type = declarator(0, clss, name, &prim, &size, &val, &init);
        if (TFUNCTION == type) {
            if (SEMI == Token) {
                Token = scan();
                clss = CEXTERN;
            }
            Thisfn = addglob(name, prim, type, clss, size, 0, NULL, 0);
            signature(Thisfn, Locs, NSYMBOLS);
            if (clss != CEXTERN) {
                lbrace();
                lsize = localdecls();
                gentext();
                if (CPUBLIC == clss)
                    genpublic(name);
                genname(name);
                genentry();
                genstack(lsize);
                genlocinit();
                Retlab = label();
                genstack(-lsize);
                genexit();
                if (O_debug & D_LSYM) {
                    dumpsyms("LOCALS: ", name, Locs, NSYMBOLS);
                }
            }
            clrlocs();
            return;
        }
        if (CEXTERN == clss && init) {
            error("Initialization of extern: %s", name);
        }
        addglob(name, prim, type, clss, size, val, NULL, init);
        if (COMMA == Token) {
            Token = scan();
        }
        else {
            break;
        }
    }
    semi();
}

/*
 * top() : Finally accepts top-level declaration and delegates
 * further processing to the other declaration parsers.
 *
 * top :=
 *        enumdecl
 *      | decl
 *      | primtype decl
 *      | storclass decl
 *      | storclass primtype decl
 *
 * storclass :=
 *             EXTERN
 *           | STATIC
 *
 */
void top(void) {
    int prim;
    int clss = CPUBLIC;

    switch (Token) {
    case EXTERN:
        clss = CEXTERN;
        Token = scan();
        break;
    case STATIC:
        clss = CSTATIC;
        Token = scan();
        break;
    }
    switch (Token) {
    case ENUM:
        enumdecl();
        break;
    case CHAR:
    case INT:
    case VOID:
        prim = primtype(Token);
        Token = scan();
        decl(clss, prim);
        break;
    case IDENT:
        decl(clss, PINT);
        break;
    default:
        error("Type specifier expected at: %s", Text);
        Token = synch(SEMI);
        break;
    }
}
