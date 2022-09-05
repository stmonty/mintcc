#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define W64
//#define HOSTLIB

#define PREFIX  'C'
#define LPREFIX 'L'

#ifndef SCCDIR
 #define SCCDIR "."
#endif

#ifdef W64
  #define LDCMD "ld -m elf_i386_fbsd -o %s %s/lib/crt0.o"
  #define SYSLIBC "/usr/lib32/libc.a"
#else
  #define LDCMD "ld -o %s %s/lib/crt0.o" // Invokes the system linker
  #define SYSLIBC "/usr/lib/libc.a"
// ld -o output-file SCCDIR/lib/crt0.o ... SCCLIBC SYSLIBC
#endif

#define ASCMD "as -32 -o %s %s" // Assemble the output of the compiler
#define SCCLIBC "%s/lib/libscc.a"

#ifndef HOSTLIB
  #undef SYSLIBC
  #define SYSLIBC  ""
#endif

#define INTSIZE 4
#define PTRSIZE INTSIZE
#define CHARSIZE 1
#define CHAROFF 0

#define TEXTLEN 512
#define NAMELEN 16

#define MAXFILES 32

#define MAXIFDEF 16 // Max nested #if(n)defs
#define MAXNMAC 32 // Max nested macros
#define MAXCASE 256 // Max number of cases in a switch
#define MAXBREAK 16 // Max number of nested break and continue contexts
#define MAXLOCINIT 32 // Max initalizers per local constant
#define MAXFNARGS 32 // Max formal function arguments

#define NSYMBOLS 1024
#define POOLSIZE 8192

/* Meta Types */
#define TVARIABLE 1
#define TARRAY 2
#define TFUNCTION 3
#define TCONSTANT 4
#define TMACRO 5

/* Primitive Types */
#define PCHAR 1 // char
#define PINT 2 // int
#define CHARPTR 3 // char *
#define INTPTR 4 // int *
#define CHARPP 5 // char **
#define INTPP 6 // int **
#define PVOID 7 // void
#define VOIDPTR 8 // void *
#define VOIDPP 9 // void **
#define FUNPTR 10 // int(*)()

/* Storage Classes */
#define CPUBLIC 1
#define CEXTERN 2
#define CSTATIC 3
#define CLSTATC 4
#define CAUTO 5

/* LV (lvalue) is a pseudo-struct represented through symbolic 
 * array subscripts. Therefore the LVstruct is defined by
 * int lv[LV];
 * Access members using lv[LVSYM] and lv[LVPRIM]
 */
#define LV 2
#define LVSYM 0 //Slot number of symbol table entry - 0 for non-identifier
#define LVPRIM 1 //Primitive type of the matched object

/* Debug Options */
#define D_LSYM 1
#define D_GSYM 2
#define D_STAT 4

/* Tokens */
enum {
SLASH, STAR, MOD, PLUS, MINUS, LSHIFT, RSHIFT, GREATER, GTEQ,
LESS, LTEQ, EQUAL, NOTEQ, AMPER, CARET, PIPE, LOGAND, LOGOR,

__ARGC, ASAND, ASXOR, ASLSHIFT, ASMINUS, ASMOD, ASOR, ASPLUS,
ASRSHIFT, ASDIV, ASMUL, ASSIGN, BREAK, CASE, CHAR, COLON,
COMMA, CONTINUE, DECR, DEFAULT, DO, ELLIPSIS, ELSE, ENUM,
EXTERN, FOR, IDENT, IF, INCR, INT, INTLIT, LBRACE, LBRACK,
LPAREN, NOT, QMARK, RBRACE, RBRACK, RETURN, RPAREN, SEMI,
SIZEOF, STATIC, STRLIT, SWITCH, TILDE, VOID, WHILE, XEOF, XMARK,

P_INCLUDE, P_DEFINE, P_ENDIF, P_ELSE, P_ELSENOT, P_IFDEF,
P_IFNDEF, P_UNDEF
};
