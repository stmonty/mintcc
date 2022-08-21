/*
 * MintCC : Minimal Toy C Compiler
 * sym.c : Symbol table management
 *
 */
#include "defs.h"
#include "data.h"
#include "decl.h"

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
        }
    }
}
