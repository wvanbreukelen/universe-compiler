global _start

section .text

_start:
mov r8, [x] ; load x
mov r9, [y] ; load y
mov rax, r8 ; load x
mov rbx, r9 ; load y
add rax, rbx
mov rbx, r9 ; load y
sub rax, rbx
mov r10, rax ; load z
mov rax, r10 ; load z
mov rbx, 1 ; load int 1
sub rax, rbx
mov r11, rax ; load p
push r9; y
push r11; p
push r8; x
push r10; z
mov rax, 1
mov rdi, 1
mov rsi, xgrJHpAmjvcBktd
mov rdx, 15
syscall
mov rax, 1
mov rdi, 1
mov rsi, guAmqlihDwkwdBp
mov rdx, 6
syscall
pop r10; z
pop r8; x
pop r11; p
pop r9; y
mov rax, 60
mov rdi, r11 ; load p
syscall
section .rodata
xgrJHpAmjvcBktd: db "Hello World!", 10, 10
guAmqlihDwkwdBp: db "hefff", 10

section .data
x: db 10
y: db 5
