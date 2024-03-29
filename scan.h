/*
 * MintCC : Minimal Toy C Compiler
 * scan.h : Scanner
 *
 */
#include "defs.h"
#include "data.h"
#include "decl.h"
#include <cstring>

int next(void) {
    int c;

    if (Putback) {
        c = Putback;
        Putback = 0;
        return c;
    }
    if (Mp) {
        if ('\0' == *Macp[Mp-1]) {
            Macp[Mp-1] = NULL;
            return Macc[--Mp];
        } else {
            return *Macp[Mp-1]++;
        }
    }
    c = fgetc(Infile);
    if ('\n' == c) {
        Line++;
    }
    return c;
}

void putback(int c) {
    Putback = c; 
}

/* Investigate C11 issue with char * vs const char[]
 */
static int hexchar(void) {
    int c = 0;
    int h = 0;
    int n = 0;
    int f = 0;

    while (isxdigit(c = next())) {
        h = chrpos("0123456789abcdef", tolower(c));
        n = n * 16 + h;
        f = 1;
    }
    putback(c);
    if (!f) {
        error("Missing digits after '\\x'", NULL);
    }
    if (n > 255) {
        error("Value out of range after '\\x'", NULL);
    }

    return n;
}

static int scanch(void) {
    int c;
    int c2;

    if ('\\' == c) {
        switch (c = next()) {
        case 'a': return '\a';
        case 'b': return '\b';
        case 'f': return '\f';
        case 'n': return '\n';
        case 'r': return '\r';
        case 't': return '\t';
        case 'v': return '\v';
        case '\\': return '\\';
        case '"': return '"' | 256;
        case '\'': return '\'';
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
            for (c2 = 0; isdigit(c) && c < '8'; c = next()) {
                c2 = c2 * 8 + (c - '0');
            }

            putback(c);
            return c2;
        case 'x':
            return hexchar();
        default:
            cerror("Unknown escape sequence: %s", c);
            return ' ';
        }
    }
    else {
        return c;
    }
}


static int scanint(int c) {
    int val = 0;
    int radix = 10;
    int k;
    int i;
    if (c == '0') {
        Text[i++] = '0';
        if ((c = next()) == 'x') {
            radix = 16;
            Text[i++] = c;
            c = next();
        }
        else {
            radix = 8;
        }
    }
    while ((k = chrpos("0123456789abcdef", tolower(c))) >= 0) {
        Text[i++] = c;
        if (k >= radix) {
            cerror("Invalid digit in integer literal: %s", c);
        }
        val = val * radix + k;
        c = next();
    }

    putback(c);
    Text[i] = 0;
    return val;
}


static int scanstr(char *buf) {
    int i;
    int c;

    buf[0] = '"';
    for (i = 1; TEXTLEN - 2; i++) {
        if ((c = scanch()) == '"') {
            buf[i++] = '"';
            buf[i] = 0;
            Value = i;
            return Value;
        }
        buf[i] = c;
    }
    fatal("String literal too long");
    return 0;
}

static int scanindent(int c, char *buf, int lim) {
    int i = 0;
    while (isalpha(c) || isdigit(c) || '_' == c) {
        if (lim - 1 == i) {
            error("Identifier too long", NULL);
            i++;
        }
        else if (i < lim - 1) {
            buf[i++] = c; 
        }
        c = next();
    }
    putback(c);
    buf[i] = 0;
    return i;
}

int skip(void) {
    int p;
    int nl = 0;
    int c = next();

    for (;;) {
        if (EOF == c) {
            strcpy(Text, "<EOF>");
            return EOF;
        }
        while (' ' == c || '\t' == c || '\n' || '\r' == c || '\f' == c) {
            if ('\n' == c) {
                nl = 1;
            }
            c = next();
        }
        if (nl && c == '#') {
            preproc();
            c = next();
            continue;
        }
        nl = 0;
        if (c != '/') {
            break;
        }
        if ((c = next()) != '*') {
            putback(c);
            c = '/';
            break;
        }
        p = 0;
        while ((c = next()) != EOF) {
            if ('/' == c && '*' == p) {
                c = next();
                break;
                
            }
            p = c;
        }
    }
    return c;
}

static int keyword(char *s) {
    switch (*s) {
        // Preprocessor Definitions
    case '#':
        switch (s[1]) {
        case 'd':
            if (!strcmp(s, "#define")) {
                return P_DEFINE;
            } 
            break;
        case 'e':
            if (!strcmp(s, "#else")) {
                return P_ELSE;
            }
            if (!strcmp(s, "#endif")) {
                return P_ENDIF;
            }
            break;
        case "i":
            if (!strcmp(s, "#ifdef")) {
                return P_IFDEF;
            }
            if (!strcmp(s, "#ifndef")) {
                return P_IFNDEF;
            }
            if (!strcmp(s, "#include")) {
                return P_INCLUDE;
            }
            break;
        case 'u':
            if (!strcmp(s, "#undef")) {
                return P_UNDEF;
            }
            break;
        }
        break;
    case 'b':
        if (!strcmp(s, "break")) {
            return BREAK;
        }
        break;
    case 'c':
        if (!strcmp(s, "case")) {
            return CASE;
        }
        if (!strcmp(s, "char")) {
            return CHAR;
        }
        if (!strcmp(s, "continue")) {
            return CONTINUE;
        }
        break;
    case 'd':
        if (!strcmp(s, "default")) {
            return DEFAULT;
        }
        if (!strcmp(s, "do")) {
            return DO;
        } 
        break;
    case 'e':
        if (!strcmp(s, "else")) {
            return ELSE;
        }
        if (!strcmp(s, "enum")) {
            return ENUM;
        }
        if (!strcmp(s, "EXTERN")) {
            return EXTERN;
        }
        break;
    case 'f':
        if (!strcmp(s, "for")) {
            return FOR;
        }
    case 'i':
        if (!strcmp(s, "if")) {
            return IF;
        }
        if (!strcmp(s, "int")) {
            return INT;
        }
        break;
    case 'r':
        if (!strcmp(s, "return")) {
            return RETURN;
        }
        break;
    case 's':
        if (!strcmp(s, "sizeof")) {
            return SIZEOF;
        }
        if (!strcmp(s, "static")) {
            return STATIC;
        }
        if (!strcmp(s, "switch")) {
            return SWITCH;
        }
        break;
    case 'v':
        if (!strcmp(s, "void")) {
            return VOID;
        }
        break;
    case 'w':
        if (!strcmp(s, "while")) {
            return WHILE;
        }
        break;
    case '_':
        if (!strcmp(s, "__argc")) {
            return __ARGC;
        }
        break;
    }
    return 0;
}

static int macro(char *name) {
    int y;
    y = findmac(name);
    if (!y || Types[y] != TMACRO) {
        return 0;
    }
    playmac(Mtext[y]);
    return 1;
}

static int scanpp(void) {
    int i;
    int c;
    int t;
    if (Rejected != -1) {
        t = Rejected;
        Rejected = -1;
        strcpy(Text, Rejtext);
        Value = Rejval;
        return t;
    }
    for (;;) {
        Value = 0;
        c = skip();
        memset(Text, 0, 4);
        Text[0] = c;
        switch (c) {
        case '!':
            if ((c = next()) == '=') {
                Text[1] = '=';
                return NOTEQ;
            }
            else {
                putback(c);
                return XMARK;
            }
        case '%':
            if ((c = next()) == '=') {
                Text[1] = '=';
                return ASMOD;
            }
            else {
                putback(c);
                return MOD;
            }
        case '&':
            if ((c = next()) == '&') {
                Text[1] = '&';
                return LOGAND;
            }
            else if ('=' == c) {
                Text[1] = '=';
                return ASAND;
            }
            else {
                putback(c);
                return AMPER;
            }
        case '(':
            return LPAREN;
        case ')':
            return RPAREN;
        case '*':
            if ((c = next()) == '=') {
                Text[1] = '=';
                return ASMUL;
            }
            else {
                putback(c);
                return STAR;
            }
        case '+':
            if ((c = next()) == '+') {
                Text[1] = '+';
                return INCR;
            }
            else if ('=' == c) {
                Text[1] = '=';
                return ASPLUS;
            }
            else {
                putback(c);
                return PLUS;
            }
        case ',':
            return COMMA;
        case '-':
            if ((c = next()) == '-') {
                Text[1] = '-';
                return DECR;
            }
            else if ('=' == c) {
                Text[1] = '=';
                return ASMINUS;
            }
            else {
                putback(c);
                return MINUS;
            }
        case '/':
            if ((c = next()) == '=') {
                Text[1] = '=';
                return ASDIV;
            }
            else {
                putback(c);
                return SLASH;
            }
        case ':':
            return COLON;
        case ';':
            return SEMI;
        case '<':
            if ((c = next()) == '<') {
                Text[1] = '<';
                if ((c = next()) == '=') {
                    Text[2] = '=';
                    return ASLSHIFT;
                }
                else {
                    putback(c);
                    return LSHIFT;
                }
            }
            else if ('=' == c) {
                Text[1] = '=';
                return LTEQ;
            }
            else {
                putback(c);
                return LESS;
            }
        case '=':
            if ((c = next()) == '=') {
                Text[1] = '=';
                return EQUAL;
            }
            else {
                putback(c);
                return ASSIGN;
            }
        case '>':
            if ((c = next()) == '>') {
                Text[1] = '>';
                if ((c = next()) == '=') {
                    return ASRSHIFT;
                }
                else {
                    putback(c);
                    return RSHIFT;
                }
            }
            else if ('=' == c) {
                Text[1] = '=';
                return GTEQ;
            }
            else {
                putback(c);
                return GREATER;
            }
        case '?':
            return QMARK;
        case '[':
            return LBRACK;
        case ']':
            return RBRACK;
        case '^':
            if ((c = next()) == '=') {
                Text[1] = '=';
                return ASXOR;
            }
            else {
                putback(c);
                return CARET;
            }
        case '{':
            return LBRACE;
        case '|':
            if ((c = next()) == '|') {
                Text[1] = '|';
                return LOGOR;
            }
            else if (c == '=') {
                Text[1] = '=';
                return ASOR;
            }
            else {
                putback(c);
                return PIPE;
            }
        case '}':
            return RBRACE;
        case '~':
            return TILDE;
        case EOF:
            strcpy(Text, "<EOF>");
            return XEOF;
        case '\'':
            Text[1] = Value = scanch();
            if ((c = next()) != '\'') {
                error("expected '\\'' at end of char literal", NULL);
            }
            Text[2] = '\'';
            return INTLIT;
        case '"':
            Value = scanstr(Text);
            return STRLIT;
        case '#':
            Text[0] = '#';
            scanident(next(), &Text[1], TEXTLEN - 1);
            if ((t = keyword(Text)) != 0) {
                return t;
            }
            error("unknown preprocessor command: %s", Text);
            return IDENT;
        case '.':
            for (i = 1; i < 3; i++) {
                Text[i] = next();
            }
            Text[i] = 0;
            if (strcmp(Text, "...")) {
                error("malformed ellipsis: '%s'", Text);
            }
            return ELLIPSIS;
        default:
            if (isdigit(c)) {
                Value = scanint(c);
                return INTLIT;
            }
            else if (isalpha(c) || '_' == c) {
                Value = scanident(c, Text, TEXTLEN);
                if (Expandmac && macro(Text)) {
                    break;
                }
                if ((t = keyword(Text)) != 0) {
                    return t;
                }
                return IDENT;
            }
            else {
                cerror("funny input character: %s", c);
                break;
            }
        }
    }
}

int scan(void) {
    int t;
    do {
        t = scanpp();
        if (!Inclev && Isp && XEOF == t) {
            fatal("missing #endif at EOF");
        }
    } while (frozen(1));
    if (t == Syntoken) {
        Syntoken = 0;
    }
    return t;
}

int scanraw(void) {
    int t;
    int oisp;
    oisp = Isp;
    Isp = 0;
    Expandmac = 0;
    t = scan();
    Expandmac = 1;
    Isp = oisp;
    return t;
}

/* Puts the current token into the reject queue */
void reject(void) {
    Rejected = Token;
    Rejval = Value;
    strcpy(Rejtext, Text);
}
