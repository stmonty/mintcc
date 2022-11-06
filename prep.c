/*
 * MintCC : Minimal Toy C Compiler
 * prep.c : Built-in Preprocessor
 *
 */

#include "defs.h"
#include "data.h"
#include "decl.h"

/*
 * playmac() : Starts the replay of the macro 's'
 * Feeds 's' into the scanner
 *
 */
void playmac(char *s) {
    if (Mp >= MAXNMAC) {
        fatal("Too many nested macros");
    }
    Macc[Mp] = next();
    Macp[Mp++] = s;
}

static void defmac(void) {
    char name[NAMELEN + 1];
    char buf[TEXTLEN + 1];
    char *p;
    int k;
    int y;

    Token = scanraw();
    if (Token != IDENT) {
        error("Identifier expected after '#define': %s", Text);
    }
    copyname(name, Text);
    if ('\n' == Putback) {
        buf[0] = 0;
    }
    else {
        fgets(buf, TEXTLEN - 1, Infile);
    }
    k = strlen(buf);
    if (k) {
        buf[k - 1] = 0;
    }
    for (p = buf; isspace(*p); p++);
    if ((y = findmac(name)) != 0) {
        if (strcmp(Mtext[y], buf)) {
            error("Macro redefinition: %s", name);
        }
    }
    else {
        addglob(name, 0, TMACRO, 0, 0, 0, globname(p), 0);
    }
    Line++;
}

static void undef(void) {
    char name[NAMELEN + 1];
    int y;
    Token = scanraw();
    copyname(name, Text);
    if (IDENT != Token) {
        error("Identifier expected after '#undef': %s", Text);
    }
    if ((y = findmac(name)) != 0) {
        Names[y] = "#undef' d";
    }
}

static void include(void) {
    char file[TEXTLEN + 1];
    char path[TEXTLEN + 1];
    int c;
    int k;
    FILE *inc;
    FILE *oinfile;
    char *ofile;
    int oc;
    int oline;
    if ((c = skip()) == '<') {
        c = '>';
    }
    fgets(file, TEXTLEN - strlen(SCCDIR) - 9, Infile);
    Line++;
    k = strlen(file);
    file[k-1] = 0;
    if (file[k - 2] != c) {
        error("Missing delimiter in '#include'", NULL);
    }
    file[k-2] = 0;
    if (c == '"') {
        strcpy(path, file);
    }
    else {
        strcpy(path, SCCDIR);
        strcat(path, "/include/");
        strcat(path, file);
    }
    if ((inc = fopen(path, "r")) == NULL) {
        error("Cannot open include file: %s", path);
    } else {
        Inclev++;
        oc = next();
        oline = Line;
        ofile = File;
        oinfile = Infile;
        Line = 1;
        putback('\n');
        File = path;
        Infile = inc;
        Token = scan();
        while (XEOF != Token) top();
        Line = oline;
        File = ofile;
        Infile = oinfile;
        fclose(inc);
        putback(oc);
        Inclev--;
    }
}

static void ifdef(int expect) {
    char name[NAMELEN + 1];
    if (Isp >= MAXIFDEF) {
        fatal("Too many nested #ifdefs");
    }
    Token = scanraw();
    copyname(name, Text);
    if (IDENT != Token) {
        error("Identifier expected in '#ifdef'", NULL);
    }
    if (frozen(1)) {
        Ifdefstk[Isp++] = P_IFNDEF;
    }
    else if ((findmac(name) != 0) == expect) {
        Ifdefstk[Isp++] = P_IFDEF;
    }
    else {
        Ifdefstk[Isp++] = P_IFNDEF;
    }
}

static void p_else(void) {
    if (!Isp) {
        error("'#else' without matching '#ifdef'", NULL);
    }
    else if (frozen(2))
        ;
    else if (P_IFDEF == Ifdefstk[Isp - 1]) {
        Ifdefstk[Isp - 1] = P_ELSENOT;
    }
    else if (P_IFNDEF == Ifdefstk[Isp - 1]) {
        Ifdefstk[Isp - 1] = P_ELSE;
    }
    else {
        error("'#else' without matching '#ifdef'", NULL)
    }
}

static void endif(void) {
    if (!Isp)
        error("'#endif' without matching '#ifdef'", NULL);
    else
        Isp--;
}

/*
 * junkln : Reads a line from input file and throws it away
 * Used to discard preprocessor arguments if frozen.
 *
 */
static void junkln(void) {
    while (!feof(Infile) && fgetc(Infile) != '\n') {
        Line++;
    }
}

/*
 * frozen : Checks whether the depth'th element
 * is in a frozen context
 */
int frozen(int depth) {
    return Isp >= depth &&
        (P_IFNDEF == Ifdefstk[Isp-depth] ||
         P_ELSENOT == Ifdefstk[Isp-depth]);
}

void preproc(void) {
    putback('#');
    Token = scanraw();
    if (frozen(1) &&
        (P_DEFINE == Token ||
         P_INCLUDE == Token ||
         P_UNDEF == Token))
        {
            junkln();
            return;
        }
    switch (Token) {
    case  P_DEFINE: defmac(); break;
    case P_UNDEF:   undef(); break;
    case P_INCLUDE: include(); break;
    case P_IFDEF:   ifdef(1); break;
    case P_IFNDEF:  ifdef(0); break;
    case P_ELSE:    p_else(); break;
    case P_ENDIF:   endif(); break;
    default:        junkln(); break;
        break;
    }
}

