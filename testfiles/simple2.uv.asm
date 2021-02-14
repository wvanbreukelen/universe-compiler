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
mov rdx, 7
syscall
pop r8; x
pop r9; y
push r9; y
push r8; x
mov rdi, r9
call _decprint
pop r8; x
pop r9; y
push r9; y
push r8; x
mov rax, 1
mov rdi, 1
mov rsi, guAmqlihDwkwdBp
mov rdx, 1
syscall
pop r8; x
pop r9; y
mov rax, 60
mov rdi, r9 ; load y
syscall
_decprint:
push    rbp
mov     rbp, rsp
sub     rsp, 16
mov     qword [rbp - 8], rdi
cmp     qword [rbp - 8], 9
jle     _decprint_finish
mov     rax, qword [rbp - 8]
cqo
mov     ecx, 10
idiv    rcx
mov     qword [rbp - 16], rax
imul    rax, qword [rbp - 16], 10
mov     rcx, qword [rbp - 8]
sub     rcx, rax
mov     qword [rbp - 8], rcx
mov     rdi, qword [rbp - 16]
call    _decprint

_decprint_finish:
mov     rax, qword [rbp - 8] 
add     rax, 48 
mov     qword [rbp - 8], rax 
mov rax, 1 
mov rdi, 1 
lea rsi, [rbp - 8] 
mov rdx, 1 
syscall 
add     rsp, 16
pop     rbp
ret                
section .rodata
xgrJHpAmjvcBktd: db "Value:", 9
guAmqlihDwkwdBp: db 10

section .data
x: dq 2342
