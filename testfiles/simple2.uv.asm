global _start

section .text

_start:
mov r8, [x] ; load x
mov r9, r8 ; load y
push r9; y
push r8; x
mov rax, 1
mov rdi, 1
mov rsi, xgrJHpAmjvcBktd
mov rdx, 1
syscall
pop r8; x
pop r9; y
mov rax, 60
mov rdi, r9 ; load y
syscall
section .rodata
xgrJHpAmjvcBktd: db "r9"

section .data
x: db 10
