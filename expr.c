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
    if (inttype(p1) && inttype(p2)) return 1;
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
            error("Indirection through non-pointer: %s", Names[lv[LVSYM]]);
        }
        else {
            error("Indirection through non-pointer: %s", NULL);
        }
    }
    lv[LVSYM] = 0;
    return lv[LVPRIM];
}

/* badcall(): Reports a call to a non-function */
static void badcall(int *lv) {
    if (lv[LVSYM]) {
        error("Call of non-function: %s", Names[lv[LVSYM]]);
    }
    else {
        error("Call of non-function", NULL);
    }
}

static int argsok(int na, int nf) {
    return na == nf || nf < 0 && na >= -nf - 1;
}

/* postfix(): function accepts all unary postfix operators
 * postfix :=
 *           primary
 *         | postfix [ expr ]
 *         | postfix ( )
 *         | postfix ( fnargs )
 *         | postfix ++
 *         | postfix --
 *
 */
static int postfix(int *lv) {
    int a = primary(lv);
    int lv2[LV];
    int p;
    int na;
    for (;;) {
        switch (Token) {
        case LBRACK:
            while (LBRACK == Token) {
                p = indirection(a, lv);
                Token = scan();
                if (expr(lv2)) {
                    rvalue(lv2);
                }
                if (PINT != lv2[LVPRIM]) {
                    error("Non-integer subscript", NULL);
                }
                if (PINT == p || INTPTR == p || CHARPTR == p || VOIDPTR == p) {
                    genscale();
                }
                genadd(PINT, PINT, 1);
                rbrack();
                lv[LVSYM] = 0;
                a = 1;
            }
            break;
        case LPAREN:
            Token = scan();
            na = fnargs(lv[LVSYM]);
            if (lv[LVSYM] && TFUNCTION == Types[lv[LVSYM]]) {
                if (!argsok(na, Sizes[lv[LVSYM]])) {
                    error("Wrong number of arguments: %s", Names[lv[LVSYM]]);
                }
                gencall(lv[LVSYM]);
            }
            else {
                if (lv[LVPRIM] != FUNPTR) {
                    badcall(lv);
                }
                clear();
                rvalue(lv);
                gencalr();
                lv[LVPRIM] = PINT;
            }
            genstack((na + 1) * INTSIZE);
            a = 0;
            break;
        case INCR:
        case DECR:
            if (a) {
                if (INCR == Token) {
                    geninc(lv, 1, 0);
                }
                else {
                    geninc(lv, 0, 0);
                }
            }
            else {
                error("LValue required before %s", Text);
            }
            Token = scan();
            a = 0;
            break;
        default:
            return a;
        }
    }
}


/*
 * prefix :=
 *          postfix
 *        | ++ prefix
 *        | -- prefix
 *        | & cast
 *        | * cast
 *        | + cast
 *        | - cast
 *        | ~ cast
 *        | ! cast
 *        | SIZEOF ( primtype )
 *        | SIZEOF ( primtype * )
 *        | SIZEOF ( primtype * * )
 *        | SIZEOF ( INT ( * ) ( ) )
 *        | SIZEOF ( primtype * )
 *        | SIZEOF ( IDENT )
 *
 * primtype :=
 *            INT
 *          | CHAR
 *          | VOID
 *
 */

static int prefix(int *lv) {
    int k;
    int y;
    int t;
    int a;

    switch(Token) {
    case INCR:
    case DECR:
        t = Token;
        Token = scan();
        if (prefix(lv)) {
            if (INCR == t) {
                geninc(lv, 1, 1);
            }
            else {
                geninc(lv, 0, 1);
            }
        }
        else {
            error("LValue expected after '%s'", t == INCR ? "++": "--");
        }
        return 0;
    case STAR:
        Token = scan();
        a = cast(lv);
        indirection(a, lv);
        return 1;
    case PLUS:
        Token = scan();
        if (cast(lv)) {
            rvalue(lv);
        }
        if (!inttype(lv[LVPRIM])) {
            error("Bad Operand to unary '+'", NULL);
        }
        return 0;
    case MINUS:
        Token = scan();
        if (cast(lv)) {
            rvalue(lv);
        }
        if (!inttype(lv[LVPRIM])) {
            error("Bad Operand to unary '-'", NULL);
        }
        genneg();
        return 0;
    case TILDE:
        Token = scan();
        if (cast(lv)) {
            rvalue(lv);
        }
        if (!inttype(lv[LVPRIM])) {
            error("Bad Operand to unary '~'", NULL);
        }
        gennot();
        return 0;
    case XMARK:
        Token = scan();
        if (cast(lv)) {
            rvalue(lv);
        }
        genlognot();
        lv[LVPRIM] = PINT;
        return 0;
    case AMPER:
        Token = scan();
        if (!cast(lv)) {
            error("LValue expected after unary '&'", NULL);
        }
        if (lv[LVSYM]) {
            genaddr(lv[LVSYM]);
        }
        lv[LVPRIM] = pointerto(lv[LVPRIM]);
        return 0;
    case SIZEOF:
        Token = scan();
        lparen();
        if(CHAR == Token || INT == Token || VOID == Token) {
            k = CHAR == Token ? CHARSIZE : INT == Token ? INTSIZE: 0;
            Token = scan();
            if (STAR == Token) {
                k = PTRSIZE;
                Token = scan();
                if (STAR == Token) {
                    Token = scan();
                }
            }
            else if (k == 0) {
                error("Cannot take sizeof(void)", NULL);
            }
        }
        else if (IDENT == Token) {
            y = findloc(Text);
            if (!y) {
                y = findglob(Text);
            }
            if (!y || !(k = objsize(Prims[y], Types[y], Sizes[y]))) {
                error("Cannot take sizeof object : %s", Text);
            }
            Token = scan();
        }
        else {
            error("Cannot take sizeof object : %s", Text);
            Token = scan();
        }
        genlit(k);
        rparen();
        lv[LVPRIM] = PINT;
        return 0;
    default:
        return postfix(lv);
    }
}

/* 
 * cast() : Handles type casts
 * 
 * cast :=
 *        prefix
 *      | ( primtype ) prefix
 *      | ( primtype * )  prefix
 *      | ( primtype * * )  prefix
 *      | ( INT ( * ) ( ) )  prefix
 *
 */
int cast(int *lv) {
    int t;
    int a;
    if (LPAREN == Token) {
        Token = scan();
        if (INT == Token) {
            t = PINT;
            Token = scan();
        }
        else if (CHAR == Token) {
            t = PCHAR;
            Token = scan();
        }
        else if (VOID == Token) {
            t = PVOID;
            Token = scan();
        }
        else {
            reject();
            Token = LPAREN;
            strcpy(Text, "(");
            return prefix(lv);
        }
        if (PINT == t && LPAREN == Token) {
            Token = scan();
            match(STAR, "int(*) ())");
            rparen();
            lparen();
            rparen();
            t = FUNPTR;
        }
        else if (STAR == Token) {
            t = pointerto(t);
            Token = scan();
            if (STAR == Token) {
                t = pointerto(t);
                Token = scan();
            }
        }
        rparen();
        a = prefix(lv);
        lv[LVPRIM] = t;
        return a;
    }
    else {
        return prefix(lv);
    }
}


static int binexpr(int *lv) {
    int ops[9];
    int prs[10];
    int sp = 0;

    int a = cast(lv);
    int a2 = 0;
    int lv2[LV];

    prs[0] = lv[LVPRIM];
    while (SLASH == Token || STAR == Token || MOD == Token ||
           PLUS == Token || MINUS == Token || LSHIFT == Token ||
           RSHIFT == Token || GREATER == Token || GTEQ == Token ||
           LESS == Token || LTEQ == Token || EQUAL == Token ||
           NOTEQ == Token || AMPER == Token || CARET == Token ||
           PIPE == Token) {
        if (a) {
            rvalue(lv);
        }
        if (a2) {
            rvalue(lv2);
        }
        while (sp > 0 && Prec[Token] <= Prec[ops[sp-1]]) {
            prs[sp - 1] = genbinop(ops[sp-1], prs[sp-1], prs[sp]);
            sp--;
        }
        ops[sp++] = Token;
        Token = scan();
        a2 = cast(lv2);
        prs[sp] = lv2[LVPRIM];
        a = 0;
    }
    if (a2) {
        rvalue(lv2);
    }
    while (sp > 0) {
        prs[sp - 1] = genbinop(ops[sp-1], prs[sp-1], prs[sp]);
        sp--;
    }
    lv[LVPRIM] = prs[0];
    return a;
}

/*
 * cond2() : Handles logical "&&" and "||"
 * Notably handles short-circuit evaluation
 *
 * logand :=
 *         binexpr
 *       | logand && binexpr
 *
 * logor  :=
 *         logand
 *       | logor || logand 
 *
 */
static int cond2(int *lv, int op) {
    int a;
    int a2 = 0;
    int lv2[LV];
    int lab = 0;

    a = op == LOGOR ? cond2(lv, LOGAND) : binexpr(lv);

    while (Token == op) {
        if (!lab) {
            lab = label();
        }
        if (a) {
            rvalue(lv);
        }
        if (a2) {
            rvalue(lv2);
        }
        if (op == LOGOR) {
            genbrtrue(lab);
        }
        else {
            genbrfalse(lab);
        }
        clear();
        Token = scan();
        a2 = op == LOGOR ? cond2(lv2, LOGAND) : binexpr(lv2);
        a = 0;
    }
    if (lab) {
        if (a2) {
            rvalue(lv2);
        }
        genlab(lab);
        genbool();
        load();
    }
    return a;
}

/*
 * cond3() : Handles ternary operator
 *
 * condexpr :=
 *           logor
 *         | logor ? expr : condexpr
 *
 */
static int cond3(int *lv) {
    int lv2[LV];
    int p = 0;
    int l1 = 0;
    int l2 = 0;

    int a = cond2(lv, LOGOR);
    while (QMARK == Token) {
        l1 = label();
        if (!l2) {
            l2 = label();
        }
        if (a) {
            rvalue(lv);
        }
        a = 0;
        genbrfalse(l1);
        clear();
        Token = scan();

        if (expr(lv)) {
            rvalue(lv);
        }
        if (!p) {
            p = lv[LVPRIM];
        }
        if (!typematch(p, lv[LVPRIM])) {
            error("Incompatible types in '?:'", NULL);
        }
        genjump(l2);
        genlab(l1);
        clear();
        colon();
        if (cond2(lv2, LOGOR)) {
            rvalue(lv2);
        }
        if (QMARK != Token) {
            if (!typematch(p, lv[LVPRIM])) {
                error("Incompatible types in '?:'", NULL);
            }
        }
    }
    if (l2) {
        genlab(l2);
        load();
    }
    return a;
}

/*
 * asgmnt() : Handles assignment of values
 *
 *
 */
int asgmnt(int *lv) {
    int lv2[LV];
    int op;

    int a = cond3(lv);
    if (ASSIGN == Token || ASDIV == Token || ASMUL == Token ||
        ASMOD == Token || ASPLUS == Token || ASPLUS == Token ||
        ASLSHIFT == Token || ASRSHIFT == Token || ASAND == Token ||
        ASXOR == Token || ASOR == Token) {
        op = Token;
        Token = scan();
        if (ASSIGN != op && !lv[LVSYM]) {
            genpush();
            genind(lv[LVPRIM]);
        }
        if (asgmnt(lv2)) {
            rvalue(lv2);
        }
        if (ASSIGN == op) {
            if (!typematch(lv[LVPRIM], lv2[LVPRIM])) {
                error("Incompatible types in assignment", NULL);
            }
        }
        if (a) {
            genstore(op, lv, lv2);
        }
        else {
            error("Lvalue expected in assignment", Text);
        }
        a = 0;
    }
    return a;
}

/*
 * expr() : Handles any C expression
 *
 * expr :=
 *        asgmnt
 *      | asgmnt ,  expr 
 *
 */
int expr(int *lv) {
    int a = asgmnt(lv);
    int a2 = 0;
    int lv2[LV];

    while (COMMA == Token) {
        Token = scan();
        clear();
        a2 = asgmnt(lv2);
        a = 0;
    }

    if (a2) {
        rvalue(lv2);
    }
    return a;
}

int rexpr(void) {
    int lv[LV];
    if (expr(lv)) {
        rvalue(lv);
    }
    return lv[LVPRIM];
}
