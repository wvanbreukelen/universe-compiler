#include "codegen.h"
#include <stdbool.h>

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
    t->buf = calloc(2048, 1);
    t->rodata_end = calloc(2048, 1);
    t->rodata_end = strcat(t->rodata_end, "\nsection .rodata\n");

    t->whandle = fopen("output.asm", "w+");

    if (!t->whandle) {
        printf("Unable to open file handle!");
        exit(1);
    }

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

    fclose(t->whandle);
    free(t->buf);
    free(t->rodata_end);
    free(t);
    mpc_ast_traverse_free(&trav);

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

    // for (int i = 0; i < node->children_num; i++) {
    //     printf("%s\n", node->children[i]->tag);
    // }

    // if (node->children_num > 0 && strcmp(node->children[0]->tag, "body|>") == 0) {
    //     visit_body(node->children[0], trav);
    // }

    // for (int i = 0; i < node->children_num; i++) {
    //     visit(node->children[i], trav);
    // }

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

char* visit_ident(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t) {
    return NULL;
}


int visit_funcall(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t) {
    

    char* func_name = strdup(node->children[0]->contents);
    printf("Function call -> %s\n", func_name);

    // Check if is builtin
    if (strcmp(func_name, "print") == 0) {
        t->buf = strcat(t->buf, "mov rax, 1\nmov rdi, 1\n");

        // Unwrap arguments.
        for (int i = 2; i < node->children_num - 1; i++) {
            char* arg = visit_exp(node->children[i], trav, t);

            if (arg) {
                char* random_string = malloc(16);
                int data_length = move_rodata(rand_string(random_string, 16), arg, t);

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
        char* return_value = strdup(node->children[1]->contents);
        printf("Return value: %s\n", return_value);

        const char* template = "mov rax, 60\nmov rdi, %s\nsyscall\n";
        char* value = calloc(strlen(template) + 24, sizeof(char));
        sprintf(value, template, return_value);

        t->buf = strcat(t->buf, value);

        free(value);


        free(return_value);
    }

    return 0;
}

int visit_assign(mpc_ast_t* node, mpc_ast_trav_t* trav, struct tools* t) {
    return 0;
}

int move_rodata(const char* name, const char *str, struct tools* t) {
    const char* template = "%s: db %s\n";
    char* value = calloc(strlen(template) + 128, sizeof(char));
    char* temp = malloc(strlen(str) + 256);

    temp[0] = '\0';

    const char *it = str;
    bool in_string = false;
    bool in_newline = false;

    while (*it != '\0') {
        if (*it == '\\' && *(it + 1) == 'n') {
            if (in_newline) strcat(temp, ", ");
            temp = (in_string) ? strcat(temp, "\", 10") : strcat(temp, "10");
            // if (in_newline) {
            //     strcat(temp, ", ");
            // }
            in_string = false;
            in_newline = true;
            it++;
        } else if (*it == '\\' && *(it + 1) == 't') {
            if (in_newline) strcat(temp, ", ");
            temp = (in_string) ? strcat(temp, "\", 9") : strcat(temp, "9");
            // if (in_newline) {
            //     strcat(temp, ", ");
            // }
            in_string = false;
            in_newline = true;
            it++;
        } else if (*it == '\\' && *(it + 1) == 'r') {
            if (in_newline) strcat(temp, ", ");
            temp = (in_string) ? strcat(temp, "\", 13") : strcat(temp, "13");
            // if (in_newline) {
            //     strcat(temp, ", ");
            // }
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
    t->rodata_end = strcat(t->rodata_end, value);

    

    free(value);
    free(temp);

    return strlen(str) - 1;
}


