global _start

section .text

_start:
mov rax, 1
mov rdi, 1
mov rsi, xgrJHpAmjvcBktd
mov rdx, 16
syscall
mov rax, 1
mov rdi, 1
mov rsi, guAmqlihDwkwdBp
mov rdx, 2
syscall
mov rax, 1
mov rdi, 1
mov rax, 60
mov rdi, 0
syscall

section .rodata
xgrJHpAmjvcBktd: db "Initializing...", 10
guAmqlihDwkwdBp: db "h", 10
