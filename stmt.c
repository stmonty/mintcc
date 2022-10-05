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
