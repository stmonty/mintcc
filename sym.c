/*
 * MintCC : Minimal Toy C Compiler
 * sym.c : Symbol table management
 *
 */
#include "defs.h"
#include "data.h"
#include "decl.h"
#include <stdio.h>

int findglob(char *s) {
    int i;
    for (i = 0; i < Globs; i++) {
        if (Types[i] != TMACRO && *s == *Names[i] && !strcmp(s, Names[i])) {
            return i;
        }
    }
    return 0;
}

int findloc(char *s) {
    int i;
    for (i=Locs; i < NSYMBOLS; i++) {
        if (*s == *Names[i] && !strcmp(s, Names[i])) {
            return i;
        }
    }
    return 0;
}

int findmac(char *s) {
    int i;
    for (i = 0; i < Globs; i++) {
        if (TMACRO == Types[i] && *s == *Names[i] && !strcmp(s, Names[i])) {
            return i;
        }
    }
    return 0;
}

int newglob(void) {
    int p;
    if ((p = Globs++) >= Locs) {
        fatal("Symbol Table Overflow");
    }
    return p;
}

int newloc(void) {
    int p;
    if ((p = --Locs) >= Globs) {
        fatal("Symbol Table Overflow");
    }
    return p;
}

char *globname(char *s) {
    int p;
    int k;
    k = strlen(s) + 1;
    if (Nbot + k >= Ntop) {
        fatal("Name list overflow");
    }
    p = Nbot;
    Nbot += k;
    strcpy(&Nlist[p], s);
    return &Nlist[p];
}

char *locname(char *s) {
    int p;
    int k;
    k = strlen(s) + 1;
    if (Nbot + k >= Ntop) {
        fatal("Name list overflow");
    }
    Ntop -= k;
    p = Ntop;
    strcpy(&Nlist[p], s);
    return &Nlist[p];
}

static void defglob(char *name, int prim, int type, int size,
                    int val, int scls, int init) {
    if (TCONSTANT == type || TFUNCTION == type) {
        return;
    }
    gendata();
    if (CPUBLIC == scls) {
        genpublic(name);
    }
    if (init && TARRAY == type) {
        return;
    }
    if (TARRAY != type) {
        genname(name);
    }
    if (PCHAR == prim) {
        if (TARRAY == type) {
            genbss(gsym(name), size);
        } else {
            gendefb(val);
        }
    }
    else if (PINT == prim) {
        if (TARRAY == type) {
            genbss(gsym(name), size * INTSIZE);
        } else {
            gendefw(val);
        }
    } else {
        if (TARRAY == type) {
            genbss(gsym(name), size * PTRSIZE);
        } else {
            gendefp(val);
        }
    }
}

int addglob(char *name, int prim, int type, int scls, int size,
            int val, char *mtext, int init) {
    
    int y;
    if ((y = findglob(name)) != 0) {
        if (Stcls[y] != CEXTERN) {
            error("Redefiniton of: %s", name);
        } else if (CSTATIC == scls) {
            error("Extern symbol declared static: %s", name);
        }
        // Check this
        if (TFUNCTION == Types[y]) {
            mtext = Mtext[y];
        }
    }
    if (y == 0) {
        y = newglob();
        Names[y] = globname(name);
    }
    else if (TFUNCTION == Types[y] || TMACRO == Types[y]) {
        if (Prims[y] != prim || Types[y] != type) {
            error("Redefiniton does not match prior type: %s", name);
        }
    }
    if (CPUBLIC == scls || CSTATIC == scls) {
        defglob(name, prim, type, size, val, scls, init);
    }
    Prims[y] = prim;
    Types[y] = type;
    Stcls[y] = scls;
    Sizes[y] = size;
    Vals[y]  = val;
    Mtext[y] = mtext;
    return y;
}

static void defloc(int prim, int type, int size, int val, int init) {
    gendata();
    if (type != TARRAY) {
        genlab(val);
    }
    if (PCHAR == prim) {
        if (TARRAY == type) {
            genbss(labname(val), size);
        } else {
            gendefb(init);
        }
    }
    else if (PINT == prim) {
        if (TARRAY == type) {
            genbss(labname(val), size * INTSIZE);
        } else {
            gendefw(init);
        }
    } else {
        if (TARRAY == type) {
            genbss(labname(val), size * PTRSIZE);
        } else {
            gendefp(init);
        }
    }
}

int addloc(char *name, int prim, int type, int scls, int size,
           int val, int init)
{
    int y;
    if (findloc(name)) {
        error("Redefinition of: %s", name);
    }
    y = newloc();
    if (CLSTATC == scls) {
        defloc(prim, type, size, val, init);
    }
    Names[y] = locname(name);
    Prims[y] = prim;
    Types[y] = type;
    Stcls[y] = scls;
    Sizes[y] = size;
    Vals[y]  = val;
    return y;
}

/*
 * Clears the local symbol table and removes all local names from
 * the name list.
 */
void clrlocs(void) {
    Ntop = POOLSIZE;
    Locs = NSYMBOLS;
}

int objsize(int prim, int type, int size) {
    int k = 0;
    if (PINT == prim) {
        k = INTSIZE;
    }
    else if (PCHAR == prim) {
        k = CHARSIZE;
    }
    else if (INTPTR == prim || CHARPTR == prim || VOIDPTR == prim) {
        k = PTRSIZE;
    }
    else if (FUNPTR == prim) {
        k = PTRSIZE;
    }
    if (TFUNCTION == type || TCONSTANT == type || TMACRO == type) {
        return 0;
    }
    if (TARRAY == type) {
        k *= size;
    }
    return k;
}

/*
 * Returns a string describing the given primitive type
 */
static char *typename(int p) {
    return PINT == p ? "INT":
        PCHAR == p ? "CHAR":
        INTPTR == p ? "INT*":
        CHARPTR == p ? "CHAR*":
        VOIDPTR == p ? "VOID*":
        FUNPTR == p ? "FUN*":
        CHARPP == p ? "CHAR**":
        INTPP == p ? "INT**":
        VOIDPP == p ? "VOID**":
        PVOID == p ? "VOID": "NULL";
}

void dumpsyms(char *title, char *sub, int from, int to) {
    int i;
    char *p;
    printf("\n==== %s%s ====\n", title, sub);
    printf(
           "PRIM    TYPE    STCLS    SIZE    VALUE    NAME (MVAL)\n"
           "----    ----    -----    ----    -----    -----------\n");
    for (i = from; i < to; i++) {
        printf("%-6s  %s  %s  %5d  %s",
               typename(Prims[i]),
               TVARIABLE == Types[i] ? "VAR ":
               TARRAY == Types[i] ? "ARRY":
               TFUNCTION == Types[i] ? "FUN ":
               TCONSTANT == Types[i] ? "CNST ":
               TMACRO == Types[i] ? "MAC ":
               "Null",
               CPUBLIC == Stcls[i] ? "PUBLIC":
               CSTATIC == Stcls[i] ? "STATIC":
               CLSTATC == Stcls[i] ? "LSTATC":
               CAUTO   == Stcls[i] ? "AUTO":
               "NULL ",
               Sizes[i],
               Vals[i],
               Names[i]);
        if (TMACRO == Types[i]) {
            printf(" [\"%s\"]", Mtext[i]);
        }
        if (TFUNCTION == Types[i]) {
            printf(" (");
            for (p = Mtext[i]; *p; p++) {
                printf("%s", typename(*p));
                if (p[1]) {
                    printf(", ");
                }
            }
            putchar(')');
        }
        putchar('\n');
    }
}
