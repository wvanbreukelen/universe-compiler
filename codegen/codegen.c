#include "codegen.h"
#include "util/str.h"
#include <stdbool.h>
#include <ctype.h>

map_str_t m;

static bool increase_reg(struct tools* t) {
    if (strlen(t->avail_reg) <= 1) {
        perror("Panic!!");
        exit(1);
    }

    if (t->avail_reg[0] == 'r' && isalpha(t->avail_reg[1]) && t->avail_reg[2] == 'x') {
        if (t->avail_reg[1] + 1 < 'd') {
            t->avail_reg[1] += 1;
            printf("%s\n\n\n", t->avail_reg);
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
        printf("%s\n\n", t->avail_reg);
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

void visitor_start(mpc_ast_t* tree) {
    mpc_ast_trav_t* trav = mpc_ast_traverse_start(tree, mpc_ast_trav_order_pre);
    mpc_ast_t * ast_next = mpc_ast_traverse_next(&trav);

    struct tools* t = calloc(sizeof((*t)), 1);
    t->buf = calloc(10000, 1);
    t->rodata_end = calloc(10000, 1);
    t->rodata_end = strcat(t->rodata_end, "\nsection .rodata\n");
    t->data_end = calloc(10000, 1);
    t->data_end = strcat(t->data_end, "\nsection .data\n");
    t->stack_offset = 0;
    t->avail_reg = strdup("r8");

    t->whandle = fopen("output.asm", "w+");

    if (!t->whandle) {
        printf("Unable to open file handle!");
        exit(1);
    }

    map_init(&m);

    if (strcmp(ast_next->tag, ">") == 0) {
        ast_next = mpc_ast_traverse_next(&trav);
        if (strcmp(ast_next->tag, "regex") == 0) {
            ast_next = mpc_ast_traverse_next(&trav);

            if (strcmp(ast_next->tag, "main|>") == 0) {
                visit_main(ast_next, trav, t);
            }
        }
    }

    // while(ast_next != NULL) {
    //     printf("Tag: %s; Contents: %s\n",
    //     ast_next->tag,
    //     ast_next->contents);
    //     ast_next = mpc_ast_traverse_next(&trav);
    // }
    
    fputs(t->buf, t->whandle);
    fputs(t->rodata_end, t->whandle);
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
    printf("visit funcdef\n");

    char* return_type = strdup(node->children[3]->contents);
    printf("Return type: %s\n", return_type);
    free(return_type);

    if (strcmp(node->children[5]->tag, "body|>") == 0) {
        visit_body(node->children[5], trav, t);
    }

    return 0;
}

int visit_body(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t) {
    printf("visit body\n");

    for (int i = 1; i < node->children_num - 1; i++) {
        visit(node->children[i], trav, t);
    }

    
    return 0;
}

int visit_statement(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t) {
    printf("visit statement\n");
    
    for (int i = 0; i < node->children_num; i++) {
        //printf("%s\n", node->children[i]->tag);
        visit(node->children[i], trav, t);
    }

    return 0;
}

char* visit_exp(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t) {
    if (startsWith("lexp|term", node->tag)) {
        return visit_term(node, trav, t);
    }

    if (strcmp("lexp|>", node->tag) == 0) {
        // Start folding??
        printf("We should fold here...\n");

        mpc_ast_t* op_node = node->children[1];

        if (strcmp("char", op_node->tag) == 0) {
            char *lhs = add_use(node->children[0], trav, t, "rax");
            char *rhs = add_use(node->children[2], trav, t, "rbx");

            char *value = calloc(1024, sizeof(char));

            if (strcmp("+", op_node->contents) == 0) {
                const char* template1 = "add %s, %s\n";
                sprintf(value, template1, "rax", "rbx");
            } else if (strcmp("-", op_node->contents) == 0) {
                const char* template2 = "sub %s, %s\n";
                sprintf(value, template2, "rax", "rbx"); 
            }

            //char *res = add_load(node->children[i - 1]->contents, "rax", trav, t);

            t->buf = strcat(t->buf, value);

            if (lhs) free(lhs);
            if (rhs) free(rhs);
            free(value);

            //free(res);
        }

        return strdup("rax");
    }
    
    return NULL;
}

char* visit_term(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t) {
    if (startsWith("lexp|term|factor", node->tag)) {
        return visit_factor(node, trav, t);
    }

    return NULL;
}
char* visit_factor(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t) {
    if (startsWith("lexp|term|factor|string", node->tag)) {
        return visit_string(node, trav, t);
    }

    if (startsWith("lexp|term|factor|number", node->tag)) {
        return visit_number(node, trav, t);
    }

    if (startsWith("lexp|term|factor|ident", node->tag)) {
        return visit_ident(node, trav, t);
    }

    return NULL;
}

char* visit_string(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t) {
    char* str = malloc(strlen(node->contents) - 1);
    str = strcpy(str, node->contents);

    str[strlen(str) - 1] = '\0';
  
    return ++str;
}

char* visit_number(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t) {
    return strdup(node->contents);
}

char* visit_ident(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t) {
    char **val = map_get(&m, node->contents);

    if (val) {
        printf("Variable found! Pointer offset: %s\n", *val);
    } else {
        fprintf(stderr, "Variable %s undefined!\n", node->contents);
        //exit(1);
    }

    return strdup(node->contents);
}


int visit_funcall(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t) {
    char* func_name = strdup(node->children[0]->contents);
    printf("Function call -> %s\n", func_name);

    // Check if builtin
    if (strcmp(func_name, "print") == 0) {
        t->buf = strcat(t->buf, "mov rax, 1\nmov rdi, 1\n");

        // Unwrap arguments.
        for (int i = 2; i < node->children_num - 1; i++) {
            char* arg = visit_exp(node->children[i], trav, t);

            if (arg) {
                char* random_string = malloc(16);
                int data_length = move_data(rand_string(random_string, 16), arg, t, true, section_rodata);

                const char* template = "mov rsi, %s\nmov rdx, %d\nsyscall\n";
                char* value = calloc(strlen(template) + 128, sizeof(char));
                sprintf(value, template, random_string, data_length);

                t->buf = strcat(t->buf, value);
                free(--arg);
                free(random_string);
                free(value);
            }
        }
    }

    printf("Number of arguments -> %d\n", node->children_num - 3);

    free(func_name);

    return 0;
}

int visit_return(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t) {
    printf("Return call!\n");

    if (node->children_num > 1) {
        char* return_value = visit_exp(node->children[1], trav, t);
        printf("Return value: %s\n", return_value);

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
        free(return_value);
    }

    return 0;
}

int visit_assign(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t) {
    printf("Visit assign!\n");

    if (strcmp(node->children[0]->contents, "var") == 0) {
        // We are dealing with a new variable assignment.

        char* var_name = strdup(node->children[1]->contents);
        printf("Var name: %s\n", var_name);

        // Parse expression.
        char* expr = visit_exp(node->children[5], trav, t);

        if (expr) {
            printf("Value: %s\n", expr);
            add_load(var_name, expr, trav, t);

            free(expr);
        }


        free(var_name);
    }

    return 0;
}

int move_data(const char* name, const char *str, struct tools* t, bool string_mode, enum t_section section) {
    const char* template = "%s: db %s\n";
    char* value = calloc(strlen(template) + 128, sizeof(char));

    if (string_mode) {
        char* temp = malloc(strlen(str) + 256);

        temp[0] = '\0';

        const char *it = str;
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
            } else {
                int length = strlen(temp);
                if (!in_string) {
                    if (in_newline) {
                        temp[length] = ','; ++length;
                        temp[length] = ' '; ++length;
                        in_newline = false;
                    }

                    temp[length] = '"'; ++length;
                }
                    
                
                temp[length] = *it;
                temp[length + 1] = '\0';
                in_string = true;
            }

            it++;
        }

        if (in_string) {
            int length = strlen(temp);
                        
            temp[length] = '"';
            temp[length + 1] = '\0';
        }

        sprintf(value, template, name, temp);
        free(temp);
    } else {
        sprintf(value, template, name, str);
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
    
    return strlen(str) - 1;
}

char* add_load(char* name, const char* value, mpc_ast_trav_t* trav, struct tools* t) {
    if (is_digit_str(value)) {
        move_data(name, value, t, false, section_data);

        const char* template = "mov %s, [%s]\n";
        char* val = calloc(strlen(template) + 24, sizeof(char));
        //int *val_offset = *map_get(&m, value);
        sprintf(val, template, t->avail_reg, name);
        t->buf = strcat(t->buf, val);


        map_set(&m, name, strdup(t->avail_reg));
        //t->stack_offset += sizeof(int);
        increase_reg(t);

        //free(val);
    } else if (strlen(value) >= 3 && value[0] == 'r' && value[2] == 'x') {
        const char* template = "mov %s, r%cx\n";
        char* val = calloc(strlen(template) + 24, sizeof(char));

        sprintf(val, template, t->avail_reg, value[1]);
        t->buf = strcat(t->buf, val);

        map_set(&m, name, strdup(t->avail_reg));
        //t->stack_offset += sizeof(int);
        //t->avail_reg++;
        increase_reg(t);
    } else {
        //move_rodata(name, value, t, false);
        //move_data(name, "0", t, false);

        // Not a constant, add a move based on init value.

        const char* template = "mov %s, %s\n";
        char* val = calloc(strlen(template) + 24, sizeof(char));
        char** reg = map_get(&m, value);

        sprintf(val, template, t->avail_reg, *reg);
        t->buf = strcat(t->buf, val);
       


        map_set(&m, name, strdup(t->avail_reg));
        //t->stack_offset += sizeof(int);
        //t->avail_reg++;
        
        increase_reg(t);

        //free(val);
    }
}

char* add_store(char* name, const char* value, mpc_ast_trav_t* trav, struct tools* t) {


    const char* template = "mov [%s], %s\n";
    char* val = calloc(strlen(template) + 24, sizeof(char));
    char** reg = map_get(&m, value);

    sprintf(val, template, name, *reg);
    t->buf = strcat(t->buf, val);

    map_set(&m, name, strdup(t->avail_reg));
        //free(val);
    //}
}


char* add_use(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t, const char* reg) {
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
        const char* template = "mov %s, %s\n";
        char* value = calloc(strlen(template) + 128, sizeof(char));

        sprintf(value, template, reg, node->contents);

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
        char** reg_use = map_get(&m, node->contents);

        if (!reg_use) {
            fprintf(stderr, "NO INSTANCE for %s\n", node->contents);
            exit(1);
            return 0;
        }

        sprintf(value, template, reg, *reg_use);

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
        //     //sprintf(output, "%s", t->avail_reg);
        //     //t->avail_reg++;
        // } else {
        //     sprintf(output, "%s", reg);
        // }

        // return output;
    }

    return NULL;
}



