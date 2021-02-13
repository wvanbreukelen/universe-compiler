#pragma once

#include "../mpc.h"
#include <stdbool.h>
#include "util/map.h"

struct tools {
    FILE *whandle;
    char* buf;
    char* data_end, *rodata_end;
    size_t stack_offset;
    char* avail_reg;
};

enum t_section {
    section_data,
    section_rodata
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

char* visit_number(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t);


int visit_return(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t);
int visit_assign(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t);

int move_data(const char* name, const char *str, struct tools* t, bool string_mode, enum t_section section);

char* add_load(char* name, const char* value, mpc_ast_trav_t* trav, struct tools* t);
char* add_store(char* name, const char* value, mpc_ast_trav_t* trav, struct tools* t);
char* add_use(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t, const char* reg);
char* add_use_raw(char* content, mpc_ast_trav_t* trav, struct tools* t, const char* reg);