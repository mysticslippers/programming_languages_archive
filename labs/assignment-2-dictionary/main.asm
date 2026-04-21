; main.asm

%define SYS_WRITE    1
%define STDERR       2
%define buffer_size  256

%include "lib.inc"
%include "dict.inc"
%include "words.inc"

section .rodata
    no_word_err:       db "Not found!", 0
    out_of_range_err:  db "Word is too long for buffer!", 0

section .bss
    buffer: resb buffer_size

section .text
global _start

_start:
    ; read_word(buffer, buffer_size)
    mov rdi, buffer
    mov rsi, buffer_size
    call read_word

    test rax, rax
    jz .too_long_or_empty

    ; find_word(buffer, next_word_pointer)
    mov rdi, buffer
    mov rsi, next_word_pointer
    call find_word

    test rax, rax
    jz .not_found

    ; rax = адрес начала записи словаря
    ; сохраняем его, потому что string_length перезапишет rax
    push rax
    lea rdi, [rax + 8]         ; rdi -> key
    call string_length         ; rax = длина key
    pop rdx                    ; rdx = адрес начала записи

    ; значение начинается после dq next и key\0
    lea rdi, [rdx + 8 + rax + 1]
    call print_string
    call print_newline

    xor rdi, rdi
    jmp exit

.too_long_or_empty:
    mov rdi, out_of_range_err
    jmp .print_error

.not_found:
    mov rdi, no_word_err
    jmp .print_error

.print_error:
    push rdi
    call string_length
    pop rsi                    ; rsi = адрес строки ошибки
    mov rdx, rax               ; rdx = длина
    mov rdi, STDERR
    mov rax, SYS_WRITE
    syscall

    mov rdi, 1
    jmp exit