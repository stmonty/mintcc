/*
 * MintCC : Minimal Toy C Compiler
 * gen.c  : Code Generator
 *
 */

#include "defs.h"
#include "data.h"
#include "decl.h"
#include "cgen.h"

/* Accumulator loaded flag */
int Acc = 0;

void clear(void) {
    Acc = 0;
}

void load(void) {
    Acc = 1;
}

int label(void) {
    static int id = 1;
    return id++;
}

void spill(void) {
    if (Acc) genpush();
}


void genraw(char *s) {
    if (NULL == Outfile) return;
    fprintf(Outfile, "%s", s);
}

void gen(char *s) {
    if (NULL == Outfile) return;
    fprintf(Outfile, "\t%s\n", s);
}

void ngen(char *s, char *inst, int n) {
    if (NULL == Outfile) return;
    fputc('\t', Outfile);
    fprintf(Outfile, s, inst, n);
    fputc('\n', Outfile);
}

void ngen2(char *s, char *inst, int n, int a) {
    if (NULL == Outfile) return;
    fputc('\t', Outfile);
    fprintf(Outfile, s, inst, n, a);
    fputc('\n', Outfile);
}

void lgen(char *s, char *inst, int n) {
    if (NULL == Outfile) return;
    fputc('\t', Outfile);
    fprintf(Outfile, s, inst, LPREFIX, n);
    fputc('\n', Outfile);
}

void lgen(char *s, int v1, int v2) {
    if (NULL == Outfile) return;
    fputc('\t', Outfile);
    fprintf(Outfile, s, v1, LPREFIX, v2);
    fputc('\n', Outfile);
}

void sgen(char *s, char *inst, char *s2) {
    if (NULL == Outfile) return;
    fputc('\t', Outfile);
    fprintf(Outfile, s, inst, s2);
    fputc('\n', Outfile);   
}

void genlab(int id ) {
    if (NULL == Outfile) return;
    fprintf(Outfile, "%c%d:", LPREFIX, id);
}

/* labname() : Returns a symbol representing a label */
char *labname(int id) {
    static char name[100];
    sprintf(name, "%c%d", LPREFIX, id);
    return name;
}

/* gsym() : Given an internal name, return an external name */
char *gsym(char *s) {
    static char name[NAMELEN + 2];
    name[0] = PREFIX;
    name[1] = 0;
    strcat(name, s);
    return name;
}

void gendata(void) {
    if (Textseg) cgdata();
    Textseg = 0;
}

void gentext(void) {
    if (!Textseg) cgtext();
    Textseg = 1;
}

void genprelude(void) {
    Textseg = 0;
    gentext();
    cgprelude();
}

void genpostlude(void) {
    cgpostlude();
}

void genname(char *name) {
    genraw(gsym(name));
    genraw(":");
}

void genpublic(char *name) {
    cgpublic(gsym(name));
}

void genaddr(int y) {
    gentext();
    spill();
    if (CAUTO == Stcls[y]) {
        cgldla(Vals[y]);
    }
    else if (CLSTATC == Stcls[y]) {
        cgldsa(Vals[y]);
    }
    else {
        cgldsa(gsym(Names[y]));
    }
    load();
}

void genldlab(int id) {
    gentext();
    spill();
    cgldlab();
    load();
}

void genlit(int v) {
    gentext();
    spill();
    cglit(v);
    load();
}

void genand(void) {
    gentext();
    cgpop2();
    cgand();
}

void genior(void) {
    gentext();
    cgpop2();
    cgior();
}

void genxor(void) {
    gentext();
    cgpop2();
    cgxor();
}

void genshl(int swapped) {
    gentext();
    cgpop2();
    if (swapped) {
        cgswap();
    }
    cgshl();
}

void genshr(int swapped) {
    gentext();
    cgpop2();
    if (swapped) {
        cgswap();
    }
    cgshr();
}

static int ptr(int p) {
    return INTPTR == p || INTPP == p ||
        CHARPTR == p || CHARPP == p ||
        VOIDPTR == p || VOIDPP == p ||
        FUNPTR == p;
}

static int needscale(int p) {
    return INTPTR == p || INTPP == p ||
        CHARPP == p || VOIDPP == p;
}

int genadd(int p1, int p2, int swapped) {
    int rp = PINT, t;
    gentext();
    cgpop2();
    if (!swapped) {
        t = p1;
        p1 = p2;
        p2 = t;
    }
    if (ptr(p1)) {
        if (needscale(p1)) {
            cgscale();
            rp = p1;
        }
    }
    else if (ptr(p2)) {
        if (needscale(p2)) {
            cgscale2();
        }
        rp = p2;
    }
    cgadd();
    return rp;
}

int gensub(int p1, int p2, int swapped) {
    int rp = PINT;
    gentext();
    cgpop2();
    if (swapped) {
        cgswap();
    }
    if (!inttype(p1) && !inttype(p2) && p1 != p2) {
        error("Incompatible pointer types in binary '-'", NULL);
    }
    if (ptr(p1) && !ptr(p2)) {
        if (needscale(p1)) {
            cgscale2();
        }
        rp = p1;
    }
    cgsub();
    if (needscale(p1) && needscape(p2)) {
        cgunscale();
    }
    return rp;
}
