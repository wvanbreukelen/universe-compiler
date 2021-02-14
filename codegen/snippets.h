#pragma once

#include <string.h>
#include "codegen.h"

bool preloaded_dec_print = false;

void preload_dec_print(struct tools *t) {
    if (preloaded_dec_print)
        return;

    t->templates = strcat(t->templates, "\
    \n\n_decprint:\n\
    push    rbp\n\
    mov     rbp, rsp\n\
    sub     rsp, 16\n\
    mov     qword [rbp - 8], rdi\n\
    cmp     qword [rbp - 8], 9\n\
    jle     _decprint_finish\n\
    mov     rax, qword [rbp - 8]\n\
    cqo\n\
    mov     ecx, 10\n\
    idiv    rcx\n\
    mov     qword [rbp - 16], rax\n\
    imul    rax, qword [rbp - 16], 10\n\
    mov     rcx, qword [rbp - 8]\n\
    sub     rcx, rax\n\
    mov     qword [rbp - 8], rcx\n\
    mov     rdi, qword [rbp - 16]\n\
    call    _decprint\n\n\
    _decprint_finish:\n\
    mov     rax, qword [rbp - 8] \n\
    add     rax, 48 \n\
    mov     qword [rbp - 8], rax \n\
    mov rax, 1 \n\
    mov rdi, 1 \n\
    lea rsi, [rbp - 8] \n\
    mov rdx, 1 \n\
    syscall \n\
    add     rsp, 16\n\
    pop     rbp\n\
    ret\
    ");

    preloaded_dec_print = true;
}