/*
 * MintCC : Minimal Toy C Compiler
 * prec.h : Binary Operator Precedence
 *
 */

/*
 * Order of precedence of most binary operators in C
 * Higher values in the array denote higher precedence 
 */
static int Prec[] = {
    7, // Slash
    7, // Star
    7, // Mod
    6, // Plus
    6, // Minus
    5, // LShift
    5, // RShift
    4, // Greater
    4, // GTEQ
    4, // Less
    4, // LTEQ
    3, // Equal
    3, // Noteq
    2, // Amper
    1, // Caret
    0, // Pipe
};

