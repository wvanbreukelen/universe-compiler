#include "mpc.h"

mpc_parser_t* init_lexer(void) {
    mpc_parser_t* Ident     = mpc_new("ident");
    mpc_parser_t* Number    = mpc_new("number");
    mpc_parser_t* Character = mpc_new("character");
    mpc_parser_t* Returntype = mpc_new("returntype");
    mpc_parser_t* String    = mpc_new("string");
    mpc_parser_t* Factor    = mpc_new("factor");
    mpc_parser_t* Term      = mpc_new("term");
    mpc_parser_t* Lexp      = mpc_new("lexp");
    mpc_parser_t* Funcall      = mpc_new("funcall");
    mpc_parser_t* Stmt      = mpc_new("stmt");
    mpc_parser_t* Exp       = mpc_new("exp");
    mpc_parser_t* Type       = mpc_new("type");
    mpc_parser_t* Typeident = mpc_new("typeident");
    mpc_parser_t* Decls     = mpc_new("decls");
    mpc_parser_t* Args      = mpc_new("args");
    mpc_parser_t* Body      = mpc_new("body");
    mpc_parser_t* Procedure = mpc_new("procedure");
    mpc_parser_t* Funcdef      = mpc_new("funcdef");
    mpc_parser_t* Func      = mpc_new("func");
    mpc_parser_t* ReturnCall     = mpc_new("return");
    mpc_parser_t* Assign     = mpc_new("assign");
    mpc_parser_t* Main      = mpc_new("main");
    //mpc_parser_t* Includes  = mpc_new("includes");
    mpc_parser_t* Smallc    = mpc_new("smallc");

    mpc_err_t* err = mpca_lang(MPCA_LANG_DEFAULT,
        " ident     : /[a-zA-Z_][a-zA-Z0-9_]*/ ;                           \n"
        " number    : /[0-9]+/ ;                                           \n"
        " character : /'.'/ ;                                              \n"
        " returntype : \"->\" ;                                              \n"
        " string    : /\"(\\\\.|[^\"])*\"/ ;                               \n"
        " type     : (\"int\" | \"char\" | \"float\");                                \n"
        "                                                                  \n"
        " factor    : '(' <lexp> ')'                                       \n"
        "           | <number>                                             \n"
        "           | <character>                                          \n"
        "           | <string>                                             \n"
        "           | <ident> '(' <lexp>? (',' <lexp>)* ')'                \n"
        "           | <ident> ;                                            \n"
        "                                                                  \n"
        " term      : <factor> (('*' | '/' | '%') <factor>)* ;             \n"
        " lexp      : <term> (('+' | '-') <term>)* ;                       \n"

        "                                                                  \n"
        " stmt      : '{' <stmt>* '}'                                      \n"
        "           | <assign> ';'                                         \n"
        "           | \"while\" '(' <exp> ')' <stmt>                       \n"
        "           | \"if\"    '(' <exp> ')' <stmt>                       \n"
        "           | <ident> '=' <lexp> ';'                               \n"
        //"           | \"print\" '(' <lexp>? ')' ';'                        \n"
        "           | <return> ';'                               \n"
        "           | <funcall> ';'                                         \n"
        "           | <ident> '(' <ident>? (',' <ident>)* ')'  ;        \n"
        "                                                                  \n"
        " exp       : <lexp> '>' <lexp>                                    \n"
        "           | <lexp> '<' <lexp>                                    \n"
        "           | <lexp> \">=\" <lexp>                                 \n"
        "           | <lexp> \"<=\" <lexp>                                 \n"
        "           | <lexp> \"!=\" <lexp>                                 \n"
        "           | <lexp> \"==\" <lexp> ;                               \n"
        "                                                                  \n"
        " assign    : \"var\" <ident> ':' <type> '=' <lexp> ;             \n"
        " return    : \"return\" <lexp>? ;                             \n"
        " funcall   : <ident> '(' <lexp>?  (',' <lexp>)* ')' ;               \n"
        " typeident : <type> <ident> ;                                    \n"
        " decls     : (<typeident> ';')* ;                                 \n"
        " args      : <typeident>? (',' <typeident>)* ;                    \n"
        " body      : '{' <decls> <stmt>* '}' ;                            \n"
        " procedure : (\"int\" | \"char\") <ident> '(' <args> ')' <body> ; \n"
        " funcdef   : '(' <args> ')' (<returntype> <type>)? '=' <body> ; \n"
        " main      : \"main\" <funcdef> ;    \n"
        " func      : <ident> <funcdef> <decls> <procedure>*; "
        //" includes  : (\"#include\" <string>)* ;                           \n"
        " smallc    : /^/ <main>? <func>* <main>? /$/ ;     \n", // <main> 
        Ident, Number, Character, Returntype, String, Factor, Term, Lexp, Funcall, Stmt, Exp, Type, Typeident, Decls, Procedure,
         Args, Body, Smallc, Funcdef, Func, Main, ReturnCall, Assign, NULL);
         
    if (err != NULL) {
    mpc_err_print(err);
    mpc_err_delete(err);
    exit(1);
    }

    return Smallc;
}