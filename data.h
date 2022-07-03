#ifndef _extern
 #define _extern extern
#endif

_extern FILE *Infile;
_extern FILE *Outfile;

_extern int Token;
_extern char Text[TEXTLEN+1];
_extern int Value;

/* Error Handling */
_extern int Line; // Current Input line number
_extern int Errors; // Error counter
_extern int Syntoken; // Scanner Synchronization Token

_extern int Putback;
_extern int Rejected;
_extern int Rejval;
_extern char Rejtext[TEXTLEN+1];

_extern char *File;
_extern char *Basefile;

/* A stack of structures for macro expansion */
_extern char *Macp[MAXNMAC];
_extern int Macc[MAXNMAC];
_extern int Mp;
/* 0 : Turn off macro expansion
 * Necessary when scanning macro names in `#define` */
_extern int Expandmac;

_extern int Ifdefstk[MAXIFDEF], Isp;
_extern int Inclev;

_extern int Textseg; // !0 : emit to text(code) : otherwise data

_extern char *Names[NSYMBOLS];
_extern char Prims[NSYMBOLS];
_extern char Types[NSYMBOLS];
_extern char Stcls[NSYMBOLS];
_extern int Sizes[NSYMBOLS];
_extern int Vals[NSYMBOLS];
_extern char *Mtext[NSYMBOLS];

_extern int Globs;
_extern int Locs;

_extern int Thisfn;

_extern char Nlist[POOLSIZE]; // Name List of Compiler
_extern int Nbot; // Free space between segments
_extern int Ntop; // Name of most recently inserted local symbol

_extern int Breakstk[MAXBREAK], Bsp;
_extern int Contstk[MAXBREAK], Csp;
_extern int Retlab;

_extern int LIaddr[MAXLOCINIT];
_extern int LIval[MAXLOCINIT];
_extern int Nli;

_extern char *Files[MAXFILES]; // Files for linker
_extern int Nf; // Number of files

/* Option Flags */
_extern int O_verbose; // -v
_extern int O_componly; // -c
_extern int O_asmonly; // -S
_extern int O_testonly; // -t
_extern int O_debug; // -d
_extern char *O_outfile; // -o
