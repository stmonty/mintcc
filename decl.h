/*
 * MintCC : Minimal Toy C Compiler
 * decl.h : Function Declarations
 *
 */
int addglob(char *name, int prim, int ttype, int scls,
            int size, int val, char *mval, int init);

int addloc(char *name, int prim, int type, int scls,
            int size, int val, int init);

void cerror(char *s, int c);
int chrpos(char *s, int c);
void clear(void);
void clrlocs(void);
void colon(void);
void compound(int lbr);
int constexpr(void);
void copyname(char *name, char *s);
int declarator(int arg, int scls, char *name, int *pprim,
            int *psize, int *pval, int *pinit);
void dumpsyms(char *title, char *sub, int from, int to);
int eofcheck(void);
void error(char *s, char *a);
int expr(int *lv);
void fatal(char *s);
int findglob(char *s);
int findloc(char *s);
int findmac(char *s);
int frozen(int depth);
void gen(char *s);
int genadd(int p1, int p2, int swapped);
void genaddr(int y);
void genand(void);
void genargc(void);
void genbool(void);
void genasop(int op, int p1, int p2);
int genbinop(int op, int p1, int p2);
void genbrfalse(int dest);
void genbrtrue(int dest);
void genbss(char *name, int len);
void gencall(int y);
void gencalr(void);
void gencmp(char *inst);
void gendata(void);
void gendefb(int v);
void gendefp(int v);
void gendefs(char *s, int len);
void gendefw(int v);
void gendiv(int swap);
void genentry(void);
void genexit(void);
void geninc(int *lv, int inc, int pre);
void genind(int p);
void genior(void);
void genjump(int dest);
void genlab(int id);
void genldlab(int id);
void genlit(int v);
void genln(char *s);
void genlocinit(void);
void genlognot(void);
void genmod(int swap);
void genmul(void);
void genname(char *name);
void genneg(void);
void gennot(void);
void genpostlude(void);
void genprelude(void);
void genpublic(char *name);
void genpush(void);
void genpushlit(int n);
void genraw(char *s);
void genscale(void);
void genscale2(void);
void genshr(int swap);
void genshl(int swap);
void genstack(int n);
void genstore(int op, int *lv, int *lv2);
void genswitch(int *vals, int *labs, int nc, int dflt);
void gentext(void);
void genxor(void);
int gensub(int p1, int p2, int swap);
int inttype(int p);
int label(void);
char *globname(char *s);
char *gsym(char *s);
char *labname(int id);
char *newfilename(char *name, int sfx);
void lbrace(void);
void lgen(char *s, char *inst, int n);
void lgen2(char *s, int v1, int v2);
void load(void);
void lparen(void);
void match(int t, char *w);
void ngen(char *s, char *inst, int n);
void ngen2(char *s, char *inst, int n, int a);
void playmac(char *s);
int pointerto(int prim);
void preproc(void);
void putback(int t);
void rbrack(void);
void reject(void);
void rparen(void);
void rvalue(int *lv);
void semi(void);
void sgen(char *s, char *inst, char *s2);
void top(void);
int objsize(int prim, int type, int size);
int next(void);
int primtype(int t);
int rexpr(void);
int scan(void);
int scanraw(void);
int skip(void);
int synch(int syn);
int typematch(int p1, int p2);





