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

