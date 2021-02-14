global _start

section .text

_start:
mov r8, [a]
mov r9, r8
mov r10, r9
mov r11, r10
mov rax, 60
mov rdi, r11
syscall
section .rodata

section .data
a: db 7
