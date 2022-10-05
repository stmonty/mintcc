/*
 * MintCC : Minimal Toy C Compiler
 * stmt.c : Statement Parser
 *
 */

#include "defs.h"
#include "data.h"
#include "decl.h"

void stmt(void);

void compount(int lbr) {
    if (lbr) {
        Token = scan();
    }
    while (RBRACE != Token) {
        if (eofcheck()) {
            return;
        }
        stmt();
    }
    Token = scan();
}

/*
 * pushbreak() : Pushes label onto break stack
 *
 */
static void pushbreak(int id) {
    if (Bsp >= MAXBREAK) {
        fatal("Too many nested loops/switches");
    }
    Breakstk[Bsp++] = id;
}

/*
 * pushcont() : Pushes label onto continue stack
 *
 */
static void pushcont(int id) {
    if (Csp >= MAXBREAK) {
        fatal("Too many nested loops/swtiches");
    }
    Contstk[Csp++] = id;
}

/*
 * break_stmt := BREAK;
 * continue_stmt := CONTINUE ;
 *
 */
static void break_stmt(void) {
    Token = scan();
    if (!Bsp) {
        error("'break' not in loop/switch context", NULL);
    }
    genjump(Breakstk[Bsp - 1]);
    semi();
}

static void continue_stmt(void) {
    Token = scan();
    if (!Csp) {
        error("'continue' not in loop/switch context", NULL);
    }
    genjump(Contstk[Csp - 1]);
    semi();
}

static void do_stmt(void) {
    int ls;
    int lb;
    int lc;

    Token = scan();
    ls = label();
    pushbreak(lb = label());
    pushcont(lc = label());
    genlab(ls);
    stmt();
    match(WHILE, "'while'");
    lparen();
    genlab(lc);
    rexpr();
    genbrtrue(ls);
    genlab(lb);
    rparen();
    semi();
    Bsp--;
    Csp--;
}

static void for_stmt(void) {
    int ls;
    int lbody;
    int lb;
    int lc;

    Token = scan();
    ls = label();
    lbody = label();
    pushbreak(lb = label());
    pushcont(lc = label());
    lparen();

    if (Token != SEMI) {
        rexpr();
        clear();
        genbrfalse(lb);
    }

    genjump(lbody);
    semi();
    genlab(lc);
    if (Token != RPAREN) {
        rexpr();
        clear();
    }
    genjump(ls);
    rparen();
    genlab(lbody);
    stmt();
    genjump(lc);
    genlab(lb);
    Bsp--;
    Csp--;
}

static void if_stmt(void) {
    int l1;
    int l2;

    Token = scan();
    lparen();
    rexpr();
    clear();
    rparen();
    l1 = label();
    genbrfalse(l1);
    stmt();

    if (ELSE == Token) {
        l2 = label();
        genjump(l2);
        genlab(l1);
        l1 = l2;
        Token = scan();
        stmt();
    }
    genlab(1);
}

static void return_stmt(void) {
    int lv[LV];

    Token = scan();
    if (Token != SEMI) {
        if (expr(lv)) {
            rvalue(lv);
        }
        if (!typematch(lv[LVPRIM], Prims[Thisfn])) {
            error("Incompatible type in 'return'", NULL);
        }
    }
    else {
        if (Prims[Thisfn] != PVOID) {
            error("Missing value after 'return'", NULL);
        }
    }
    genjump(Retlab);
    semi();
}

static void switch_block(void) {
    int lb;
    int ls;
    int ldflt = 0;
    int nc = 0;
    int cval[MAXCASE];
    int clab[MAXCASE];

    Token = scan();
    pushbreak(lb = label());
    ls = label();
    genjump(ls);

    while (RBRACE != Token) {
        if (eofcheck()) {
            return;
        }
        if ((CASE == Token || DEFAULT == Token && nc >= MAXCASE )) {
            error("Too many 'case's in switch statement", NULL);
            nc = 0;
        }
        if (CASE == Token) {
            
        }
    }
}
