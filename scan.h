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
