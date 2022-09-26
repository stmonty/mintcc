/*
 * MintCC : Minimal Toy C Compiler
 * cexpr.c : Constant Expression Parser
 *
 */

#include "defs.h"
#include "data.h"
#include "decl.h"
#include "prec.h"

/*
 * constfac() : Accepts a constant factor
 */
static int constfac(void) {
    int y;
    int v = Value;

    if (INTLIT == Token) {
        Token = scan();
        return v;
    }
    if (MINUS == Token) {
        Token = scan();
        return -constfac();
    }
    if (TILDE == Token) {
        Token = scan();
        return ~constfac();
    }
    if (LPAREN == Token) {
        Token = scan();
        v = constexpr();
        rparen();
        return v;
    }
    if (Token == IDENT) {
        y = findglob(Text);
        if (!y || Types[y] != TCONSTANT) {
            error("Not a constant: %s", Text);
        }
        Token = scan();
        return y ? Vals[y] : 0;
    }
    else {
        error("Constant expression expected at: %s", Text);
        Token = scan();
        return 1;
    }
}

/*
 * constop() : Applies the operation 'op' identified by its token
 * to the values v1 and v2 and returns the result
 */
static int constop(int op, int v1, int v2) {
    if ((SLASH == op || MOD == op) && 0 == v2) {
        error("Constant divide by 0", NULL);
        return 0;
    }
}
