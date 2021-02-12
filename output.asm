global _start

section .text

_start:
push qword [rel x]
mov eax, [x]
mov rax, 60
mov rdi, [x]
syscall

section .rodata
x: db 5
