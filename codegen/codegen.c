#include "codegen.h"
#include "util/str.h"
#include <stdbool.h>
#include <ctype.h>
#include "snippets.h"

map_str_t m;

static bool increase_reg(struct tools* t) {
    if (strlen(t->avail_reg) <= 1) {
        perror("Panic!!");
        exit(1);
    }

    if (t->avail_reg[0] == 'r' && isalpha(t->avail_reg[1]) && t->avail_reg[2] == 'x') {
        if (t->avail_reg[1] + 1 < 'd') {
            t->avail_reg[1] += 1;
            //printf("%s\n\n\n", t->avail_reg);
        } else {
            free(t->avail_reg);
            t->avail_reg = strdup("r8");
        }
    } else if (t->avail_reg[0] == 'r' && is_digit_str(&t->avail_reg[1])) {
        if (atoi(t->avail_reg) + 1 < 16) {
            sprintf(t->avail_reg, "r%d", atoi(&t->avail_reg[1]) + 1);
        } else {
            perror("No more registers available, we should use the stack now...!\n");
            exit(1);
        }
    } else {
        //printf("%s\n\n", t->avail_reg);
        perror("Unknown error!\n");
        exit(1);
    }

    // if (t->avail_reg[0] + 1 > 'd') {
    //     t->avail_reg = '8';
    // } else if (t->avail_reg + 1 > '')

    // if (t->avail_reg + 1 > 'd') {
    //     perror("No more registers available, we should use the stack now...!\n");
        
    //     t->stack_offset += sizeof(int);

    //     exit(1);

    //     return false;
    // }

    // t->avail_reg++;

    // return true;
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
    mpc_ast_trav_t* trav = mpc_ast_traverse_start(tree, mpc_ast_trav_order_pre);
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
    const char* template = "output.asm";
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
    free(t->buf);
    free(t->rodata_end);
    free(t->data_end);
    free(t);
    mpc_ast_traverse_free(&trav);

    map_deinit(&m);
}

int visit(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t) {
    if (strcmp(node->tag, "funcdef|>") == 0) {
        return visit_funcdef(node, trav, t);
    }

    if (strcmp(node->tag, "stmt|>") == 0) {
        return visit_statement(node, trav, t);
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

int visit_main(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t) {
    t->buf = strcat(t->buf, "global _start\n\nsection .text\n\n_start:\n");

    for (int i = 0; i < node->children_num; i++) {
        visit(node->children[i], trav, t);
    }

    return 0;
}
int visit_funcdef(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t) {
    //printf("visit funcdef\n");

    char* return_type = strdup(node->children[3]->contents);
    //printf("Return type: %s\n", return_type);
    free(return_type);

    if (strcmp(node->children[5]->tag, "body|>") == 0) {
        visit_body(node->children[5], trav, t);
    }

    return 0;
}

int visit_body(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t) {
    //printf("visit body\n");

    for (int i = 1; i < node->children_num - 1; i++) {
        visit(node->children[i], trav, t);
    }

    
    return 0;
}

int visit_statement(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t) {
    //printf("visit statement\n");
    
    for (int i = 0; i < node->children_num; i++) {
        visit(node->children[i], trav, t);
    }

    return 0;
}


static void _unfold_math_exp_impl(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t, int base_index, bool fold_mode) {
    char *lhs = NULL;

    if (!fold_mode) {
        lhs = add_use(node->children[base_index], trav, t, "rax");
    }

    
    char *rhs = add_use(node->children[base_index + 2], trav, t, "rbx");
    char *value = calloc(1024, sizeof(char));

    if (strcmp("+", node->children[base_index + 1]->contents) == 0) {
        const char* template1 = "add %s, %s\n";
        sprintf(value, template1, "rax", "rbx");
    } else if (strcmp("-", node->children[base_index + 1]->contents) == 0) {
        const char* template2 = "sub %s, %s\n";
        sprintf(value, template2, "rax", "rbx"); 
    }

    //char *res = add_load(node->children[i - 1]->contents, "rax", trav, t);

    t->buf = strcat(t->buf, value);

    if (base_index + 3 < node->children_num) {
        base_index += 2;
       _unfold_math_exp_impl(node, trav, t, base_index, true);
    }
}

static void unfold_math_exp(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t) {
    _unfold_math_exp_impl(node, trav, t, 0, false);
}


struct value visit_exp(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t) {
    if (startsWith("lexp|term", node->tag)) {
        return visit_term(node, trav, t);
    }

    if (strcmp("lexp|>", node->tag) == 0) {
        // Start folding??
        //printf("We should fold here...\n");

        unfold_math_exp(node, trav, t);

        return (struct value) {value_register, strdup("rax") };
    }
    
    return (struct value) { value_unknown, NULL };
}




struct value visit_term(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t) {
    if (startsWith("lexp|term|factor", node->tag)) {
        return visit_factor(node, trav, t);
    }

    return (struct value) { value_unknown, NULL };
}
struct value visit_factor(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t) {
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

    return (struct value) { value_unknown, NULL };
}

struct value visit_string(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t) {
    // char* str = malloc(strlen(node->contents) - 1);
    // str = strcpy(str, node->contents);

    // str[strlen(str) - 1] = '\0';
  
    //return ++str;

    char* dup = strdup(&node->contents[1]);
    dup[strlen(dup)-1] = 0;

    return (struct value) { value_string, dup };
}

struct value visit_number(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t) {
    return (struct value) { value_int64, strdup(node->contents) };
}

struct value visit_boolean(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t) {
    char* val = (strcmp(node->contents, "true") == 0) ? strdup("1") : strdup("0");

    return (struct value) { value_boolean, val };
}

struct value visit_ident(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t) {
    char **val = map_get(&m, node->contents);


    if (val) {
        //printf("Variable found! Pointer offset: %s\n", *val);
    } else {
        fprintf(stderr, "Error: Variable `%s` undefined!\n", node->contents);
        exit(1);
    }

    //return strdup(node->contents);

    return (struct value) { value_register, strdup(*val) };
}


int visit_funcall(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t) {
    char* func_name = strdup(node->children[0]->contents);
    //printf("Function call -> %s\n", func_name);

    // Check if builtin
    if (strcmp(func_name, "print") == 0) {
        push_active_regs(t);
        
        // Unwrap arguments.
        for (int i = 2; i < node->children_num - 1; i++) {
            struct value arg = visit_exp(node->children[i], trav, t);
            printf("arg: %s\n\n\n", arg.value);

            if (arg.type == value_string && arg.value) {
                t->buf = strcat(t->buf, "mov rax, 1\nmov rdi, 1\n");

                char* random_string = malloc(16);
                int data_length = move_data(rand_string(random_string, 16), arg, t, true, section_rodata);

                const char* template = "mov rsi, %s\nmov rdx, %d\nsyscall\n";
                char* value = calloc(strlen(template) + 128, sizeof(char));
                sprintf(value, template, random_string, data_length);

                t->buf = strcat(t->buf, value);
                free(arg.value);
                free(random_string);
                free(value);
            } else if (arg.type == value_register) {
                const char* template = "mov rdi, %s\ncall _decprint\n";
                //t->buf = strcat(t->buf, "mov rax, 1\nmov rdi, 1\n");
                char* value = calloc(strlen(template) + 128, sizeof(char));
                sprintf(value, template, arg.value);
                t->buf = strcat(t->buf, value);
                free(value);
                free(arg.value);

                preload_dec_print(t);
            } else {
                // fprintf(stderr, "Error: failed to unwrap argument of type `%s`!\n", type_to_string(arg.type));
                // exit(1);
            }
        }

        pop_active_regs(t);
    }

    //printf("Number of arguments -> %d\n", node->children_num - 3);

    free(func_name);

    return 0;
}

int visit_return(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t) {
    //printf("Return call!\n");

    if (node->children_num > 1) {
        struct value return_value = visit_exp(node->children[1], trav, t);
        printf("Return value: %s\n", return_value.value);

        // char *load = add_use(node->children[1], trav, t, 'a');

        // if (load) {
        //     //t->buf = strcat(t->buf, load);
        //     free(load);
        // }
        

        // if (startsWith(node->children[i]->tag))

        t->buf = strcat(t->buf, "mov rax, 60\n");
        char* load = add_use(node->children[1], trav, t, "rdi");
        t->buf = strcat(t->buf, "syscall");

        // const char* template = "mov rax, 60\nmov rdi, [%s]\nsyscall\n";
        // char* value = calloc(strlen(template) + 24, sizeof(char));
        // sprintf(value, template, return_value);

        // t->buf = strcat(t->buf, value);

        free(load);
        free(return_value.value);
    }

    return 0;
}

int visit_assign(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t) {

    if (strcmp(node->children[0]->contents, "var") == 0) {
        // We are dealing with a new variable assignment.

        char* var_name = strdup(node->children[1]->contents);
        printf("Visit assign!\n");

        

        // Parse expression.
        struct value val = visit_exp(node->children[5], trav, t);
        

        if (val.value) {
            printf("Var name: %s, value %s\n", var_name, val.value);
            add_load(var_name, val, trav, t);

            free(val.value);
        }

            

        free(var_name);
    }

    return 0;
}


int visit_reassign(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t) {
    // Check if variable is within active set.
    if (!var_exists(node->children[0]->contents, t)) {
        fprintf(stderr, "Error: variable `%s` does not exists!\n", node->children[0]->contents);
        exit(1);
    }

    // Parse expression.
    struct value val = visit_exp(node->children[2], trav, t);

    if (val.value) {
        //printf("Value: %s\n", expr);
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

char* add_load(char* name, const struct value val, mpc_ast_trav_t* trav, struct tools* t) {
    if (is_digit_str(val.value)) {
        move_data(name, val, t, false, section_data);

        const char* template = "mov %s, [%s] ; load %s\n";
        char* calloc_val = calloc(strlen(template) + 24, sizeof(char));
        //int *val_offset = *map_get(&m, value);
        sprintf(calloc_val, template, t->avail_reg, name, name);
        t->buf = strcat(t->buf, calloc_val);


        map_set(&m, name, strdup(t->avail_reg));
        //t->stack_offset += sizeof(int);
        increase_reg(t);

        //free(val);
    } else if (strlen(val.value) >= 3 && val.value[0] == 'r' && val.value[2] == 'x') {
        const char* template = "mov %s, r%cx ; load %s\n";
        char* calloc_val = calloc(strlen(template) + 24, sizeof(char));

        sprintf(calloc_val, template, t->avail_reg, val.value[1], name);
        t->buf = strcat(t->buf, calloc_val);

        map_set(&m, name, strdup(t->avail_reg));
        //t->stack_offset += sizeof(int);
        //t->avail_reg++;
        increase_reg(t);

    } else if (strlen(val.value) >= 2 && val.value[0] == 'r' && isdigit(val.value[1]))  {
        const char* template = "mov %s, r%c ; load %s\n";
        char* calloc_val = calloc(strlen(template) + 24, sizeof(char));

        sprintf(calloc_val, template, t->avail_reg, val.value[1], name);
        t->buf = strcat(t->buf, calloc_val);

        map_set(&m, name, strdup(t->avail_reg));
        //t->stack_offset += sizeof(int);
        //t->avail_reg++;
        increase_reg(t);
    } else if (val.type == value_string) {
        move_data(name, val, t, true, section_rodata);

        const char* template = "mov %s, [%s] ; load %s\n";
        char* calloc_val = calloc(strlen(template) + 24, sizeof(char));
        //int *val_offset = *map_get(&m, value);
        sprintf(calloc_val, template, t->avail_reg, name, name);
        t->buf = strcat(t->buf, calloc_val);


        map_set(&m, name, strdup(t->avail_reg));
        //t->stack_offset += sizeof(int);
        increase_reg(t);
    } else {
        //move_rodata(name, value, t, false);
        //move_data(name, "0", t, false);

        // Not a constant, add a move based on init value.

       

        const char* template = "mov %s, %s ; load %s\n";
        char* calloc_val = calloc(strlen(template) + 24, sizeof(char));
        char** reg = map_get(&m, val.value);

        sprintf(calloc_val, template, t->avail_reg, *reg, val.value);
        t->buf = strcat(t->buf, calloc_val);
       


        map_set(&m, name, strdup(t->avail_reg));
        //t->stack_offset += sizeof(int);
        //t->avail_reg++;
        
        increase_reg(t);

        //free(val);
    }
}

char* add_store(char* name, const struct value val, mpc_ast_trav_t* trav, struct tools* t) {


    const char* template = "mov [%s], %s ; store %s\n";
    char* val_calloc = calloc(strlen(template) + 24, sizeof(char));
    char** reg = map_get(&m, val.value);

    sprintf(val_calloc, template, name, *reg, name);
    t->buf = strcat(t->buf, val_calloc);

    map_set(&m, name, strdup(t->avail_reg));
        //free(val);
    //}
}


char* add_use(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t, const char* reg) {
    if (!node) {
        perror("null pointer\n");
        exit(1);
    }

    // FIXME ugly
    //if (is_digit_str(node->contents)) {
        // const char* template = "mov e%cx, [%s]\n";
        // char* value = calloc(strlen(template) + 128, sizeof(char));
        // sprintf(value, template, (reg == '\0') ? t->avail_reg : reg, node->contents);

        // t->buf = strcat(t->buf, value);
        // free(value);

        // char* output = malloc(4);

        // if (reg == '\0') {
        //     sprintf(output, "e%cx", t->avail_reg);
        //     t->avail_reg++;
        // } else {
        //     sprintf(output, "e%cx", reg);
        // }

        // return output;
    
    if (is_digit_str(node->contents) == 1) {
        const char* template = "mov %s, %s ; load int %s\n";
        char* value = calloc(strlen(template) + 128, sizeof(char));

        sprintf(value, template, reg, node->contents, node->contents);

        t->buf = strcat(t->buf, value);
        free(value);

        // char* output = malloc(4);
        // if (reg == NULL) {
        //     //sprintf(output, "%s", t->avail_reg);
        //     //t->avail_reg++;
        // } else {
        //     sprintf(output, "%s", reg);
        // }

        // return output;
    } else {
        const char* template = "mov %s, %s ; load %s\n";
        char* value = calloc(strlen(template) + 128, sizeof(char));
        char** reg_use = map_get(&m, node->contents);

        if (!reg_use) {
            fprintf(stderr, "NO INSTANCE for %s\n", node->contents);
            exit(1);
            return 0;
        }

        sprintf(value, template, reg, *reg_use, node->contents);

        t->buf = strcat(t->buf, value);
        free(value);

        // char* output = malloc(4);
        // if (reg == NULL) {
        //     //sprintf(output, "%s", t->avail_reg);
        //     //t->avail_reg++;
        // } else {
        //     sprintf(output, "%s", reg);
        // }

        // return output;
    }

    return NULL;
}

char* add_use_raw(char* content, mpc_ast_trav_t* trav, struct tools* t, const char* reg) {
    // FIXME ugly
    //if (is_digit_str(node->contents)) {
        // const char* template = "mov e%cx, [%s]\n";
        // char* value = calloc(strlen(template) + 128, sizeof(char));
        // sprintf(value, template, (reg == '\0') ? t->avail_reg : reg, node->contents);

        // t->buf = strcat(t->buf, value);
        // free(value);

        // char* output = malloc(4);

        // if (reg == '\0') {
        //     sprintf(output, "e%cx", t->avail_reg);
        //     t->avail_reg++;
        // } else {
        //     sprintf(output, "e%cx", reg);
        // }

        // return output;
    
    if (is_digit_str(content) == 1) {
        const char* template = "mov %s, %s\n";
        char* value = calloc(strlen(template) + 128, sizeof(char));

        sprintf(value, template, reg, content);

        t->buf = strcat(t->buf, value);
        free(value);

        // char* output = malloc(4);
        // if (reg == NULL) {
        //     //sprintf(output, "%s", t->avail_reg);
        //     //t->avail_reg++;
        // } else {
        //     sprintf(output, "%s", reg);
        // }

        // return output;
    } else {
        const char* template = "mov %s, %s\n";
        char* value = calloc(strlen(template) + 128, sizeof(char));
        char** reg_use = map_get(&m, content);

        if (!reg_use) {
            fprintf(stderr, "NO INSTANCE for %s\n", content);
            exit(1);
            return 0;
        }

        sprintf(value, template, reg, *reg_use);

        t->buf = strcat(t->buf, value);
        free(value);

        // char* output = malloc(4);
        // if (reg == NULL) {
        //     //sprintf    map_iter_t iter = map_iter(&m);
    }

    return NULL;
}

void push_active_regs(struct tools *t) {
    const char *key;
    map_iter_t iter = map_iter(&m);

    const char *template = "push %s; %s\n";
    char* value = calloc(strlen(template) + 128, sizeof(char));

    while ((key = map_next(&m, &iter))) {
        sprintf(value, template, *map_get(&m, key), key);
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

    while (i < 128) {
        if (keys[i]) {
            sprintf(value, template, *map_get(&m, keys[i]), keys[i]);
            t->buf = strcat(t->buf, value);
        }
        i++;
    }


    free(keys);
    free(value);
}

bool var_exists(const char* name, struct tools *t) {
    const char *key;
    map_iter_t iter = map_iter(&m);

    while ((key = map_next(&m, &iter))) {
        if (strcmp(key, name) == 0) {
            return true;
        }
    }

    return false;
}


