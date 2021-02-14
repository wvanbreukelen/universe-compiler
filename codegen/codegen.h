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

enum t_value {
    value_register,
    value_string,
    value_int64,
    value_boolean
};

struct value {
    enum t_value type;
    char **value;
};

enum t_section {
    section_data,
    section_rodata
};

int walk_tree(void);

void visitor_start(mpc_ast_t* tree, const char* filename);
int visit(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t);
int visit_main(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t);
int visit_funcdef(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t);
int visit_funcall(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t);
int visit_body(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t);
int visit_statement(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t);

struct value visit_exp(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t);
struct value visit_term(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t);
struct value visit_factor(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t);
struct value visit_string(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t);
struct value visit_boolean(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t);
struct value visit_ident(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t);

char* visit_number(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t);


int visit_return(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t);
int visit_assign(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t);
int visit_reassign(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t);

int move_data(const char* name, const char *str, struct tools* t, bool string_mode, enum t_section section);

char* add_load(char* name, const char* value, mpc_ast_trav_t* trav, struct tools* t);
char* add_store(char* name, const char* value, mpc_ast_trav_t* trav, struct tools* t);
char* add_use(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t, const char* reg);
char* add_use_raw(char* content, mpc_ast_trav_t* trav, struct tools* t, const char* reg);

void push_active_regs(struct tools *t);
void pop_active_regs(struct tools *t);

bool var_exists(const char* name, struct tools *t);