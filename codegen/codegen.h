#pragma once

#include "../mpc.h"

struct tools {
    FILE *whandle;
    char* buf;
    char* rodata_end;
};

int walk_tree(void);

void visitor_start(mpc_ast_t* tree);
int visit(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t);
int visit_main(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t);
int visit_funcdef(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t);
int visit_funcall(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t);
int visit_body(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t);
int visit_statement(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t);

char* visit_exp(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t);
char* visit_term(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t);
char* visit_factor(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t);
char* visit_string(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t);
char* visit_ident(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t);


int visit_return(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t);
int visit_assign(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t);

int move_rodata(const char* name, const char *str, struct tools* t);