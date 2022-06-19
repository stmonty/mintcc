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

