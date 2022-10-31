/*
 * MintCC : Minimal Toy C Compiler
 * prep.c : Built-In Preprocessor
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

