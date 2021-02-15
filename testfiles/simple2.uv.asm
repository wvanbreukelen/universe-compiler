global _start

section .text

_start:
mov r8, [x] ; load x
cmp r8, 10
push r8; x
mov rax, 1
mov rdi, 1
mov rsi, xgrJHpAmjvcBktd
mov rdx, 15
syscall
pop r8; x
mov rax, 60
mov rdi, 0 ; load int 0
syscall
section .rodata
xgrJHpAmjvcBktd: db "Hello", 32, "World!", 10, 10

section .data
x: dq 10
