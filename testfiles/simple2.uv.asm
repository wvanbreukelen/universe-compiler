global _start

section .text

_start:
mov r8, [x] ; load x
mov r9, r8 ; load y
mov r10, [z] ; load z
push r9; y
push r8; x
push r10; z
mov rax, 1
mov rdi, 1
mov rsi, z
mov rdx, 100
syscall
pop r10; z
pop r8; x
pop r9; y
mov rax, 60
mov rdi, r9 ; load y
syscall
section .rodata
z: db "Hello", 32, "World!"

section .data
x: dq 1
