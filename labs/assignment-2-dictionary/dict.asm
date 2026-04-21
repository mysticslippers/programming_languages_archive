; dict.asm
section .text
global find_word

%include "lib.inc"

; find_word(rdi, rsi)
; rdi -> искомый ключ
; rsi -> указатель на начало словаря
; return:
;   rax = адрес начала записи словаря, если ключ найден
;   rax = 0, если ключ не найден

find_word:
.loop:
    test rsi, rsi
    je .not_found

    push rdi
    push rsi

    lea rsi, [rsi + 8]    ; адрес ключа текущей записи
    call string_equals

    pop rsi
    pop rdi

    test rax, rax
    jnz .found

    mov rsi, [rsi]        ; следующий элемент списка
    jmp .loop

.found:
    mov rax, rsi
    ret

.not_found:
    xor eax, eax
    ret