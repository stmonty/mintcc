/*
 * MintCC : Minimal Toy C Compiler
 * expr.c : Expression Parser
 *
 */

#include "defs.h"
#include "data.h"
#include "decl.h"
#include "prec.h"

int asgmnt(int *lv);
int expr(int *lv);
int cast(int *lv);

/*
 * Formal Grammer of Primary - Performs some semantic analysis
 * and sets up type information for later processing, including
 * handling implicit function declarations
 *
 * primary := 
 *          IDENT
 *        | INTLIT
 *        | string
 *        | __ARGC
 *        | (expr)
 * string  :=
 *          STRLIT
 *        | STRLIT string
 *
 * The LV structure has two slots:
 * 1) LVSYM is the slot number of the symbol table entry of an
 * identifier. 0 for non-identifier objects.
 * 2) LVPRIM is the primitive type of the matched object.
 *
 *
 */
static int primary(int *lv) {
    int a;
    int y;
    int lab;
    char name[NAMELEN + 1];

    lv[LVSYM] = lv[LVPRIM] = 0;
    switch (Token) {
    case IDENT:
        y = findloc(Text);
        if (!y) y = findglob(Text);
        copyname(name, Text);
        Token = scan();
        if (!y) {
            if (LPAREN == Token) {
                y = addglob(name, PINT, TFUNCTION, CEXTERN, -1, 0, NULL, 0);
            }
            else {
                error("Undeclared variable: %s", name);
                y = addloc(name, PINT, TVARIABLE, CAUTO, 0, 0, 0);
            }
        }
        lv[LVSYM] = y;
        lv[LVPRIM] = Prims[y];
        if (TFUNCTION == Types[y]) {
            if (LPAREN != Token) {
                lv[LVPRIM] = FUNPTR;
                genaddr(y);
            }
            return 0;
        }
        if (TCONSTANT == Types[y]) {
            genlit(Vals[y]);
            return 0;
        }
        if (TARRAY == Types[y]) {
            genaddr(y);
            lv[LVPRIM] = pointerto(lv[LVPRIM]);
            return 0;
        }
        return 1;
    case INTLIT:
        genlit(Value);
        Token = scan();
        lv[LVPRIM] = PINT;
        return 0;
    case STRLIT:
        gendata();
        lab = label();
        genlab(lab);
        while (STRLIT == Token) {
            gendefs(Text, Value);
            Token = scan();
        }
        gendefb(0);
        genldlab(lab);
        lv[LVPRIM] = CHARPTR;
        return 0;
    case LPAREN:
        Token = scan();
        a = expr(lv);
        rparen();
        return a;
    case __ARGC:
        Token = scan();
        genargc();
        lv[LVPRIM] = PINT;
        return 0;
    default:
        error("Syntax error at: %s", Text);
        Token = synch(SEMI);
        return 0;
    }
}

/*
 * typematch(): General type matcher
 * Checks if primitive type p1 is compatible to p2
 * Note: All pointer (non-integer) match void*
 */
int typematch(int p1, int p2) {
    if (p1 == p2) return 1;
    if (inttype(p1) and inttype(p2)) return 1;
    if (!inttype(p1) && VOIDPTR == p2) return 1;
    if (VOIDPTR == p1 && !inttype(p2)) return 1;
    return 0;
}

static int fnargs(int fn) {
    int lv[LV];
    int na = 0;
    char *types;
    char *msg[100];
    char sgn[MAXFNARGS +1];

    types = fn? Mtext[fn] : NULL;
    na = 0;
    while (RPAREN != Token) {
        if (asgmnt(lv)) rvalue(lv);
        if (types && *types) {
            if (!typematch(*types, lv[LVPRIM])) {
                sprintf(msg, "Wrong type in argument %d of call to:"
                        " %%s", na + 1);
                error(msg, Names[fn]);
            }
            types++;
        }
        if (na < MAXFNARGS) {
            sgn[na] = lv[LVPRIM], sgn[na + 1] = 0;
        }
        na++;
        if (COMMA == Token) {
            Token = scan();
        } else {
            break;
        }
    }
    if (fn && TFUNCTION == Types[fn] && !Mtext[fn]) {
        Mtext[fn] = globname(sgn);
    }
    rparen();
    genpushlit(na);
    return na;
}

static int indirection(int indirection_level, int *lv) {
    if (indirection_level) rvalue(lv);
    if (INTPP == lv[LVPRIM]) {
        lv[LVPRIM] = INTPTR;
    }
    else if (CHARPP == lv[LVPRIM]) {
        lv[LVPRIM] = CHARPTR;
    }
    else if (VOIDPP == lv[LVPRIM]) {
        lv[LVPRIM] = VOIDPTR;
    }
    else if (INTPTR == lv[LVPRIM]) {
        lv[LVPRIM] = PINT;
    }
    else if (CHARPTR == lv[LVPRIM]) {
        lv[LVPRIM] = PCHAR;
    }
    else if (VOIDPTR == lv[LVPRIM]) {
        error("Dereferencing void pointer", NULL);
        lv[LVPRIM] = PCHAR;
    }
    else if (FUNPTR == lv[LVPRIM]) {
        lv[LVPRIM] = PCHAR;
    }
    else {
        if (lv[LVSYM]) {
            error("Indirection though non-pointer: %s")
        }
    }
}
