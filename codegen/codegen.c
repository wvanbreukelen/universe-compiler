#include "codegen.h"
#include "util/str.h"
#include <stdbool.h>
#include <ctype.h>
#include "snippets.h"

map_void_t m;

static bool increase_reg(struct tools* t) {
    if (strlen(t->avail_reg) <= 1) {
        cerror("Panic!!");
        return false;
    }

    if (t->avail_reg[0] == 'r' && isalpha(t->avail_reg[1]) && t->avail_reg[2] == 'x') {
        if (t->avail_reg[1] + 1 < 'd') {
            t->avail_reg[1] += 1;
        } else {
            free(t->avail_reg);
            t->avail_reg = strdup("r8");
        }
    } else if (t->avail_reg[0] == 'r' && is_digit_str(&t->avail_reg[1])) {
        if (atoi(t->avail_reg) + 1 < 16) {
            sprintf(t->avail_reg, "r%d", atoi(&t->avail_reg[1]) + 1);
        } else {
            cerror("Error: no more registers available, we should use the stack now...!\n");
            return false;
        }
    } else {
        cerror("Unknown error!\n");
        return false;
    }

    return true;
}


static char *rand_string(char *str, size_t size)
{
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJK...";
    if (size) {
        --size;
        for (size_t n = 0; n < size; n++) {
            int key = rand() % (int) (sizeof charset - 1);
            str[n] = charset[key];
        }
        str[size] = '\0';
    }
    return str;
}

bool startsWith(const char *pre, const char *str)
{
    size_t lenpre = strlen(pre),
           lenstr = strlen(str);
    return lenstr < lenpre ? false : memcmp(pre, str, lenpre) == 0;
}

void visitor_start(mpc_ast_t* tree, const char* filename) {
    __maybe_unused mpc_ast_trav_t* trav = mpc_ast_traverse_start(tree, mpc_ast_trav_order_pre);
    mpc_ast_t * ast_next = mpc_ast_traverse_next(&trav);

    struct tools* t = calloc(sizeof((*t)), 1);
    t->buf = calloc(10000, 1);
    t->rodata_end = calloc(10000, 1);
    t->rodata_end = strcat(t->rodata_end, "\nsection .rodata\n");
    t->data_end = calloc(10000, 1);
    t->data_end = strcat(t->data_end, "\nsection .data\n");

    t->templates = calloc(10000, 1);

    t->stack_offset = 0;
    t->avail_reg = strdup("r8");
    char* _filename = malloc(strlen(filename) + 8);
    strcpy(_filename, filename);
    _filename = strcat(_filename, ".asm");

    t->whandle = fopen(_filename, "w+");
    free(_filename);

    if (!t->whandle) {
        perror("Unable to open file handle!");
        exit(1);
    }

    map_init(&m);


    // for (int i = 0; i < ast_next->children_num; i++) {
    //     mpc_ast_t *func_candidate = ast_next->children[i];

    //     if (strcmp(func_candidate->tag, "func|>") == 0) {
    //         printf("%s\n", func_candidate->children[0]->contents);
    //         if (strcmp(func_candidate->children[0]->contents, "main") == 0) {
    //             visit_main(ast_next, trav, t);
    //         }
    //     }
    // }

    if (strcmp(ast_next->tag, ">") == 0) {
        ast_next = mpc_ast_traverse_next(&trav);
        if (strcmp(ast_next->tag, "regex") == 0) {
            ast_next = mpc_ast_traverse_next(&trav);
            
            if (strcmp(ast_next->tag, "main|>") == 0) {
                visit_main(ast_next, trav, t);
            }
        }
    }

    
    //if (strlen(t->buf) > 0)
        fputs(t->buf, t->whandle);
    
    fputs(t->templates, t->whandle);
    //if (strlen(t->rodata_end) > 0)
        fputs(t->rodata_end, t->whandle);
    
    //if (strlen(t->data_end) > 0)
        fputs(t->data_end, t->whandle);

    fclose(t->whandle);

    free(t->templates);
    free(t->buf);
    free(t->rodata_end);
    free(t->data_end);
    free(t);
    mpc_ast_traverse_free(&trav);

    map_deinit(&m);
}

int visit(mpc_ast_t* node, __maybe_unused mpc_ast_trav_t* trav, struct tools* t) {
    if (strcmp(node->tag, "funcdef|>") == 0) {
        return visit_funcdef(node, trav, t);
    }

    if (strcmp(node->tag, "stmt|>") == 0) {
        return visit_statement(node, trav, t);
    }

    if (strcmp(node->tag, "stmt|ifstmt|>") == 0) {
        return visit_if_statement(node, trav, t);
    }

    if (strcmp(node->tag, "funcall|>") == 0) {
        return visit_funcall(node, trav, t);
    }

    if (strcmp(node->tag, "return|>") == 0) {
        return visit_return(node, trav, t);
    }

    if (strcmp(node->tag, "assign|>") == 0) {
        return visit_assign(node, trav, t);
    }

    if (strcmp(node->tag, "reassign|>") == 0) {
        return visit_reassign(node, trav, t);
    }

    if (startsWith("lexp", node->tag)) {
        (void) visit_exp(node, trav, t);
    }

    return 0;
}

int visit_main(mpc_ast_t* node, __maybe_unused mpc_ast_trav_t* trav, struct tools* t) {
    t->buf = strcat(t->buf, "global _start\n\nsection .text\n\n_start:\n");

    for (int i = 0; i < node->children_num; i++) {
        visit(node->children[i], trav, t);
    }

    return 0;
}


int visit_funcdef(mpc_ast_t* node, __maybe_unused mpc_ast_trav_t* trav, struct tools* t) {
    for (int i = 0; i < node->children_num; i++) {
        if (strcmp(node->children[i]->tag, "body|>") == 0) {
            visit_body(node->children[i], trav, t);
        }

        if (strcmp(node->children[i]->tag, "args|typeident|>") == 0) {
            visit_main_params(node->children[i], trav, t);
        }
    }

    return 0;
}

int visit_main_params(__maybe_unused mpc_ast_t* node, __maybe_unused mpc_ast_trav_t* trav, __maybe_unused struct tools* t) {
    // FIXME...

    return 0;
}

int visit_body(mpc_ast_t* node, __maybe_unused mpc_ast_trav_t* trav, struct tools* t) {
    for (int i = 0; i < node->children_num; i++) {   
        visit(node->children[i], trav, t);
    }

    return 0;
}

int visit_statement(mpc_ast_t* node, __maybe_unused mpc_ast_trav_t* trav, struct tools* t) {    
    for (int i = 0; i < node->children_num; i++) {
        visit(node->children[i], trav, t);
    }

    return 0;
}

int visit_if_statement(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t) {


    struct value arg = visit_logic_exp(node->children[2], trav, t);


    return 0;
}

static void _unfold_math_exp_impl(mpc_ast_t* node, __maybe_unused mpc_ast_trav_t* trav, struct tools* t, int base_index, bool fold_mode) {
    if (!fold_mode) {
        (void) add_use(node->children[base_index], trav, t, "rax");
    }

    (void) add_use(node->children[base_index + 2], trav, t, "rbx");
    char *value = calloc(1024, sizeof(char));

    if (strcmp("+", node->children[base_index + 1]->contents) == 0) {
        const char* template1 = "add %s, %s\n";
        sprintf(value, template1, "rax", "rbx");
    } else if (strcmp("-", node->children[base_index + 1]->contents) == 0) {
        const char* template2 = "sub %s, %s\n";
        sprintf(value, template2, "rax", "rbx"); 
    } else if (strcmp(">", node->children[base_index + 1]->contents) == 0) {
        const char* template2 = "cmp %s, %s\n";
        sprintf(value, template2, "rax", "rbx");
    } else {
        cerror("Unknown operator %s\n", node->children[base_index + 1]->contents);
    }

    t->buf = strcat(t->buf, value);

    if (base_index + 3 < node->children_num) {
        base_index += 2;
       _unfold_math_exp_impl(node, trav, t, base_index, true);
    }

    free(value);
}

static void unfold_math_exp(mpc_ast_t* node, __maybe_unused mpc_ast_trav_t* trav, struct tools* t) {
    _unfold_math_exp_impl(node, trav, t, 0, false);
}

struct value visit_exp(mpc_ast_t* node, __maybe_unused mpc_ast_trav_t* trav, struct tools* t) {
    if (startsWith("lexp|term", node->tag)) {
        return visit_term(node, trav, t);
    }

    if (strcmp("lexp|>", node->tag) == 0) {
        unfold_math_exp(node, trav, t);

        return (struct value) {value_register, strdup("rax"), false };
    }
    
    return (struct value) { value_unknown, NULL, false };
}

struct value visit_logic_exp(mpc_ast_t* node, __maybe_unused mpc_ast_trav_t* trav, struct tools* t) {
    if (strcmp("exp|>", node->tag) == 0) {
        
        struct value lhs = visit_exp(node->children[0], trav, t);

        if (lhs.type == value_register) {
            // FIXME add a push?
            //(void) add_load(lhs.value, lhs, trav, t);
        }

        struct value rhs = visit_exp(node->children[2], trav, t);


        if (rhs.type == value_register) {
            //cerror("%s\n", rhs.value);
        } else if (rhs.type == value_int64) {
            //(void) add_use(node->children[2], trav, t, "rbx");
        }
    
        const char* op = node->children[1]->contents;
        char *value = calloc(1024, sizeof(char));

        if (strcmp(op, ">") == 0 || strcmp(op, "<") == 0 || strcmp(op, ">=") == 0 || strcmp(op, "<=") == 0 || strcmp(op, "==") == 0 || strcmp(op, "!=") == 0) {
            const char* template1 = "cmp %s, %s\n";
            sprintf(value, template1, lhs.value, rhs.value);

            // TODO: set the jump targets? Maybe in the upper function?
        } else {
            cerror("Unknown logic operator `%s`\n", op);
        }

        t->buf = strcat(t->buf, value);
        free(value);
        


        //unfold_math_exp(node, trav, t);

        return (struct value) {value_register, NULL, false };
    }
    
    return (struct value) { value_unknown, NULL, false };
}

struct value visit_term(mpc_ast_t* node, __maybe_unused mpc_ast_trav_t* trav, struct tools* t) {
    
    if (startsWith("lexp|term|factor", node->tag)) {
       
        return visit_factor(node, trav, t);
    }

    return (struct value) { value_unknown, NULL, false };
}

struct value visit_factor(mpc_ast_t* node, __maybe_unused mpc_ast_trav_t* trav, __maybe_unused struct tools* t) {
    if (startsWith("lexp|term|factor|string", node->tag)) {
        return visit_string(node, trav, t);
    }

    if (startsWith("lexp|term|factor|number", node->tag)) {
        return visit_number(node, trav, t);
    }

    if (startsWith("lexp|term|factor|boolean", node->tag)) {
        return visit_boolean(node, trav, t);
    }

    if (startsWith("lexp|term|factor|ident", node->tag)) {
        return visit_ident(node, trav, t);
    }

    return (struct value) { value_unknown, NULL, false };
}

struct value visit_string(mpc_ast_t* node, __maybe_unused mpc_ast_trav_t* trav, __maybe_unused struct tools* t) {
    char* dup = strdup(&node->contents[1]);
    dup[strlen(dup)-1] = 0;

    return (struct value) { value_string, dup, true };
}

struct value visit_number(mpc_ast_t* node, __maybe_unused mpc_ast_trav_t* trav, __maybe_unused struct tools* t) {
    return (struct value) { value_int64, strdup(node->contents), false };
}

struct value visit_boolean(mpc_ast_t* node, __maybe_unused mpc_ast_trav_t* trav, __maybe_unused struct tools* t) {
    char* val = (strcmp(node->contents, "true") == 0) ? strdup("1") : strdup("0");

    return (struct value) { value_boolean, val, false };
}

struct value visit_ident(mpc_ast_t* node, __maybe_unused mpc_ast_trav_t* trav, __maybe_unused struct tools* t) {
    struct value **val = (struct value**) map_get(&m, node->contents);
 
    if (!val || !((*val)->value)) {
        cerror("Error: Variable `%s` undefined!\n", node->contents);
    }

    if ((*val)->type == value_string) {
        return (struct value) { value_register_data, strdup((*val)->value), (*val)->constant  };
    } else {
        return (struct value) { value_register, strdup((*val)->value), (*val)->constant };
    }    
}

int visit_funcall(mpc_ast_t* node, __maybe_unused mpc_ast_trav_t* trav, struct tools* t) {
    const char* func_name = node->children[0]->contents;

    // Check if builtin
    if (strcmp(func_name, "print") == 0) {
        push_active_regs(t);
        
        // Unwrap arguments.
        for (int i = 2; i < node->children_num - 1; i++) {
            struct value arg = visit_exp(node->children[i], trav, t);
            //printf("arg: %s, type: %s\n\n\n", arg.value, type_to_string(arg.type));

            if (arg.type == value_string && arg.value) {
                t->buf = strcat(t->buf, "mov rax, 1\nmov rdi, 1\n");

                char* random_string = malloc(16);
                int data_length = move_data(rand_string(random_string, 16), arg, t, true, section_rodata);

                const char* template = "mov rsi, %s\nmov rdx, %d\nsyscall\n";
                char* value = calloc(strlen(template) + 128, sizeof(char));
                sprintf(value, template, random_string, data_length);

                t->buf = strcat(t->buf, value);
                free(random_string);
                free(value);
            } else if (arg.type == value_register_data && var_exists(node->children[i]->contents)) {
                t->buf = strcat(t->buf, "mov rax, 1\nmov rdi, 1\n");

                const char* template = "mov rsi, %s\nmov rdx, %d\nsyscall\n";
                char* value = calloc(strlen(template) + 128, sizeof(char));
                // TODO find length.
                sprintf(value, template, node->children[i]->contents, 100);

                t->buf = strcat(t->buf, value);
                
                free(value);
            
            } else if (arg.type == value_register ) {
                const char* template = "mov rdi, %s\ncall _decprint\n";
                //t->buf = strcat(t->buf, "mov rax, 1\nmov rdi, 1\n");
                char* value = calloc(strlen(template) + 128, sizeof(char));
                sprintf(value, template, arg.value);
                t->buf = strcat(t->buf, value);
                free(value);

                preload_dec_print(t);
            } else {
                // fprintf(stderr, "Error: failed to unwrap argument of type `%s`!\n", type_to_string(arg.type));
                // exit(1);
            }

            free(arg.value);
        }

        pop_active_regs(t);
    }

    return 0;
}

int visit_return(mpc_ast_t* node, __maybe_unused mpc_ast_trav_t* trav, struct tools* t) {
    if (node->children_num > 1) {
        struct value return_value = visit_exp(node->children[1], trav, t);
        //printf("Return value: %s\n", return_value.value);

        t->buf = strcat(t->buf, "mov rax, 60\n");
        char* load = add_use(node->children[1], trav, t, "rdi");
        t->buf = strcat(t->buf, "syscall");

        if (load) {
            free(load);
        }
        
        free(return_value.value);
    }

    return 0;
}

int visit_assign(mpc_ast_t* node, __maybe_unused mpc_ast_trav_t* trav, struct tools* t) {

    if (strcmp(node->children[0]->contents, "var") == 0) {
        // We are dealing with a new variable assignment.
        char* var_name = strdup(node->children[1]->contents);

        // Parse expression.
        struct value val = visit_exp(node->children[5], trav, t);

        if (val.value) {
            add_load(var_name, val, trav, t);

            free(val.value);
        }

        free(var_name);
    }

    return 0;
}


int visit_reassign(mpc_ast_t* node, __maybe_unused mpc_ast_trav_t* trav, struct tools* t) {
    // Check if variable is within active set.
    if (!var_exists(node->children[0]->contents)) {
        cerror("Error: variable `%s` does not exists!\n", node->children[0]->contents);
    }

    // Parse expression.
    struct value val = visit_exp(node->children[2], trav, t);

    if (val.value) {
        add_load(node->children[0]->contents, val, trav, t);

        free(val.value);
    }

    return 0;
}

int move_data(const char* name, const struct value val, struct tools* t, bool string_mode, enum t_section section) {
    const char* template = (val.type == value_string) ? "%s: db %s\n" : "%s: dq %s\n";
    char* value = calloc(strlen(template) + 128, sizeof(char));
    int length = strlen(val.value) - 1;

    if (string_mode) {
        char* temp = malloc(strlen(val.value) + 256);

        temp[0] = '\0';

        const char *it = val.value;
        bool in_string = false;
        bool in_newline = false;

        while (*it != '\0') {
            if (*it == '\\' && *(it + 1) == 'n') {
                if (in_newline) strcat(temp, ", ");
                temp = (in_string) ? strcat(temp, "\", 10") : strcat(temp, "10");
                in_string = false;
                in_newline = true;
                it++;
            } else if (*it == '\\' && *(it + 1) == 't') {
                if (in_newline) strcat(temp, ", ");
                temp = (in_string) ? strcat(temp, "\", 9") : strcat(temp, "9");
                in_string = false;
                in_newline = true;
                it++;
            } else if (*it == '\\' && *(it + 1) == 'r') {
                if (in_newline) strcat(temp, ", ");
                temp = (in_string) ? strcat(temp, "\", 13") : strcat(temp, "13");
                in_string = false;
                in_newline = true;
                it++;
            } else if (*it == ' ') {
                if (in_newline) strcat(temp, ", ");
                temp = (in_string) ? strcat(temp, "\", 32") : strcat(temp, "32");
                in_string = false;
                in_newline = true;
            } else {
                int _length = strlen(temp);
                if (!in_string) {
                    if (in_newline) {
                        temp[_length] = ','; ++_length;
                        temp[_length] = ' '; ++_length;
                        in_newline = false;
                    }

                    temp[_length] = '"'; ++_length;
                }
                    
                
                temp[_length] = *it;
                temp[_length + 1] = '\0';
                in_string = true;
            }

            it++;
        }

        if (in_string) {
            int _length = strlen(temp);
                        
            temp[_length] = '"';
            temp[_length + 1] = '\0';
        }

        sprintf(value, template, name, temp);
        //length = strlen(temp) - 1;
        free(temp);
    } else {
        sprintf(value, template, name, val.value);
    }

    switch (section) {
        case section_data:
            t->data_end = strcat(t->data_end, value);
            break;
        case section_rodata:
            t->rodata_end = strcat(t->rodata_end, value);
            break;
        default:
            break;
    }
    
    free(value);
    return length;
}

char* add_load(char* name, const struct value val, __maybe_unused mpc_ast_trav_t* trav, struct tools* t) {
    if (is_digit_str(val.value)) {
        move_data(name, val, t, false, section_data);

        const char* template = "mov %s, [%s] ; load %s\n";
        char* calloc_val = calloc(strlen(template) + 24, sizeof(char));
        //int *val_offset = *map_get(&m, value);
        sprintf(calloc_val, template, t->avail_reg, name, name);
        t->buf = strcat(t->buf, calloc_val);

        struct value* map_val = malloc(sizeof(struct value));
        map_val->type = value_int64;
        map_val->value = strdup(t->avail_reg);
        map_val->constant = true;
        map_set(&m, name, map_val);

        //t->stack_offset += sizeof(int);
        increase_reg(t);

        //free(val);
    } else if (strlen(val.value) >= 3 && val.value[0] == 'r' && val.value[2] == 'x') {
        const char* template = "mov %s, r%cx ; load %s\n";
        char* calloc_val = calloc(strlen(template) + 24, sizeof(char));

        sprintf(calloc_val, template, t->avail_reg, val.value[1], name);
        t->buf = strcat(t->buf, calloc_val);

        struct value* map_val = malloc(sizeof(struct value));
        map_val->type = value_register;
        map_val->value = strdup(t->avail_reg);
        map_val->constant = true;
        map_set(&m, name, map_val);
        //t->stack_offset += sizeof(int);
        //t->avail_reg++;
        increase_reg(t);

    } else if (strlen(val.value) >= 2 && val.value[0] == 'r' && isdigit(val.value[1]))  {
        const char* template = "mov %s, r%c ; load %s\n";
        char* calloc_val = calloc(strlen(template) + 24, sizeof(char));

        sprintf(calloc_val, template, t->avail_reg, val.value[1], name);
        t->buf = strcat(t->buf, calloc_val);

        struct value* map_val = malloc(sizeof(struct value));
        map_val->type = value_register;
        map_val->value = strdup(t->avail_reg);
        map_val->constant = true;
        map_set(&m, name, map_val);

        increase_reg(t);
    } else if (val.type == value_string) {
        move_data(name, val, t, true, section_rodata);

        const char* template = "mov %s, [%s] ; load %s\n";
        char* calloc_val = calloc(strlen(template) + 24, sizeof(char));
        //int *val_offset = *map_get(&m, value);
        sprintf(calloc_val, template, t->avail_reg, name, name);
        t->buf = strcat(t->buf, calloc_val);

        struct value* map_val = malloc(sizeof(struct value));
        map_val->type = value_string;
        map_val->value = strdup(t->avail_reg);
        map_val->constant = true;
        map_set(&m, name, map_val);

        increase_reg(t);
    } else {
        //move_rodata(name, value, t, false);
        //move_data(name, "0", t, false);

        // Not a constant, add a move based on init value;

        const char* template = "mov %s, %s ; load %s\n";
        char* calloc_val = calloc(strlen(template) + 24, sizeof(char));
        struct value** reg = (struct value**) map_get(&m, val.value);

        sprintf(calloc_val, template, t->avail_reg, (*reg)->value, val.value);
        t->buf = strcat(t->buf, calloc_val);
       
        struct value* map_val = malloc(sizeof(struct value));
        map_val->type = value_unknown;
        map_val->value = strdup(t->avail_reg);
        map_val->constant = true;

        map_set(&m, name, map_val);

        increase_reg(t);

        free(calloc_val);
    }

    return NULL;
}

char* add_store(char* name, const struct value val, __maybe_unused mpc_ast_trav_t* trav, struct tools* t) {
    const char* template = "mov [%s], %s ; store %s\n";
    char* val_calloc = calloc(strlen(template) + 24, sizeof(char));
    struct value **reg = (struct value**) map_get(&m, val.value);

    sprintf(val_calloc, template, name, (*reg)->value, name);
    t->buf = strcat(t->buf, val_calloc);

    struct value* map_val = malloc(sizeof(struct value));
    map_val->type = val.type;
    map_val->value = strdup(t->avail_reg);
    map_val->constant = true;

    map_set(&m, name, map_val);
    
    free(val_calloc);

    return NULL;
}


char* add_use(mpc_ast_t* node, __maybe_unused mpc_ast_trav_t* trav, struct tools* t, const char* reg) {
    if (!node) {
        cerror("null pointer\n");
    }
    
    if (is_digit_str(node->contents) == 1) {
        const char* template = "mov %s, %s ; load int %s\n";
        char* value = calloc(strlen(template) + 128, sizeof(char));

        sprintf(value, template, reg, node->contents, node->contents);

        t->buf = strcat(t->buf, value);
        free(value);
    } else {
        const char* template = "mov %s, %s ; load %s\n";
        char* value = calloc(strlen(template) + 128, sizeof(char));
        struct value** map_val = (struct value** )map_get(&m, node->contents);

        if (map_val && !(*map_val)->value) {
            cerror("No instance for `%s`\n", node->contents);
        }

        sprintf(value, template, reg, (*map_val)->value, node->contents);

        t->buf = strcat(t->buf, value);
        free(value);
    }

    return NULL;
}

char* add_use_raw(char* content, __maybe_unused mpc_ast_trav_t* trav, struct tools* t, const char* reg) {    
    if (is_digit_str(content) == 1) {
        const char* template = "mov %s, %s\n";
        char* value = calloc(strlen(template) + 128, sizeof(char));

        sprintf(value, template, reg, content);

        t->buf = strcat(t->buf, value);
        free(value);
    } else {
        const char* template = "mov %s, %s\n";
        char* value = calloc(strlen(template) + 128, sizeof(char));
        struct value** map_val = (struct value**) map_get(&m, content);

        if (map_val && !(*map_val)->value) {
            cerror("No instance for `%s`\n", content);
            return 0;
        }

        sprintf(value, template, reg, (*map_val)->value);

        t->buf = strcat(t->buf, value);
        free(value);
    }

    return NULL;
}

void push_active_regs(struct tools *t) {
    const char *key;
    map_iter_t iter = map_iter(&m);

    const char *template = "push %s; %s\n";
    char* value = calloc(strlen(template) + 128, sizeof(char));

    while ((key = map_next(&m, &iter))) {
        sprintf(value, template, (*((struct value**) map_get(&m, key)))->value, key);
        t->buf = strcat(t->buf, value);
    }

    free(value);
}

void pop_active_regs(struct tools *t) {
    const char *key;
    map_iter_t iter = map_iter(&m);

    const char *template = "pop %s; %s\n";
    char* value = calloc(strlen(template) + 128, sizeof(char));
    const char** keys = calloc(128 * sizeof(const char*), 1);
    size_t i = 127;

    while ((key = map_next(&m, &iter)) && i > 0) {
        keys[i] = key;
        i--;
    }

    i = 0;
    struct value** entry;

    while (i < 128) {
        if (keys[i]) {
            entry = (struct value**) map_get(&m, keys[i]);

            sprintf(value, template, (*entry)->value, keys[i]);
            t->buf = strcat(t->buf, value);
        }
        i++;
    }


    free(keys);
    free(value);
}

bool var_exists(const char* name) {
    const char *key;
    map_iter_t iter = map_iter(&m);

    while ((key = map_next(&m, &iter))) {
        if (strcmp(key, name) == 0) {
            return true;
        }
    }

    return false;
}


