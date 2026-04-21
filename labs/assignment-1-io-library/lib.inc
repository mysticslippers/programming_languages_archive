; Laboratory work #1

%define SYS_READ   0
%define SYS_WRITE  1
%define SYS_EXIT   60

%define STD_IN     0
%define STD_OUT    1

section .text

global exit
global string_length
global print_string
global print_char
global print_newline
global print_uint
global print_int
global string_equals
global read_char
global read_word
global parse_uint
global parse_int
global string_copy

; Принимает код возврата и завершает текущий процесс
exit:
    mov rax, SYS_EXIT
    syscall

; Принимает указатель на нуль-терминированную строку, возвращает её длину
string_length:
    xor rax, rax
.len_loop:
    cmp byte [rdi + rax], 0
    je .done
    inc rax
    jmp .len_loop
.done:
    ret

; Принимает указатель на нуль-терминированную строку, выводит её в stdout
print_string:
    push rdi
    call string_length
    pop rsi                ; rsi = адрес строки
    mov rdx, rax           ; rdx = длина
    mov rdi, STD_OUT
    mov rax, SYS_WRITE
    syscall
    ret

; Принимает код символа и выводит его в stdout
print_char:
    sub rsp, 8
    mov byte [rsp], dil
    mov rsi, rsp
    mov rdx, 1
    mov rdi, STD_OUT
    mov rax, SYS_WRITE
    syscall
    add rsp, 8
    ret

; Переводит строку (выводит символ с кодом 0xA)
print_newline:
    mov rdi, 10
    jmp print_char

; Выводит беззнаковое 8-байтовое число в десятичном формате
print_uint:
    sub rsp, 32
    lea rsi, [rsp + 31]
    mov byte [rsi], 0

    mov rax, rdi
    mov rcx, 10

    test rax, rax
    jne .convert

    dec rsi
    mov byte [rsi], '0'
    jmp .print

.convert:
.loop:
    xor rdx, rdx
    div rcx
    add dl, '0'
    dec rsi
    mov byte [rsi], dl
    test rax, rax
    jne .loop

.print:
    mov rdi, rsi
    call print_string
    add rsp, 32
    ret

; Выводит знаковое 8-байтовое число в десятичном формате
print_int:
    test rdi, rdi
    jns .positive

    push rdi
    mov rdi, '-'
    call print_char
    pop rdi

    mov rax, rdi
    neg rax
    mov rdi, rax
    jmp print_uint

.positive:
    jmp print_uint

; Принимает два указателя на нуль-терминированные строки, возвращает 1 если они равны, 0 иначе
string_equals:
.loop:
    mov al, [rdi]
    mov dl, [rsi]
    cmp al, dl
    jne .not_equal
    test al, al
    je .equal
    inc rdi
    inc rsi
    jmp .loop

.equal:
    mov rax, 1
    ret

.not_equal:
    xor rax, rax
    ret

; Читает один символ из stdin и возвращает его.
; Возвращает 0 если достигнут конец потока
read_char:
    sub rsp, 8
    xor eax, eax           ; SYS_READ
    mov edi, STD_IN
    mov rsi, rsp
    mov edx, 1
    syscall

    test rax, rax
    jz .eof

    movzx eax, byte [rsp]
    add rsp, 8
    ret

.eof:
    xor eax, eax
    add rsp, 8
    ret

; Принимает: адрес начала буфера, размер буфера
; Читает в буфер слово из stdin, пропуская пробельные символы в начале.
; Пробельные символы: 0x20, 0x9, 0xA.
; Останавливается и возвращает 0 если слово слишком большое для буфера.
; При успехе возвращает адрес буфера в rax, длину слова в rdx.
; При неудаче возвращает 0 в rax.
; Эта функция должна дописывать к слову нуль-терминатор.
read_word:
    push r12
    push r13
    push r14

    mov r12, rdi           ; buffer
    mov r13, rsi           ; buffer size
    xor r14, r14           ; length = 0

    test r13, r13
    jz .fail

.skip_spaces:
    call read_char
    test rax, rax
    jz .eof_before_word

    cmp al, ' '
    je .skip_spaces
    cmp al, 9
    je .skip_spaces
    cmp al, 10
    je .skip_spaces

.read_loop:
    cmp r14, r13
    jae .fail

    mov [r12 + r14], al
    inc r14

    cmp r14, r13
    jae .fail_if_no_room_for_zero

    call read_char
    test rax, rax
    jz .finish

    cmp al, ' '
    je .finish
    cmp al, 9
    je .finish
    cmp al, 10
    je .finish
    jmp .read_loop

.fail_if_no_room_for_zero:
    ; слово заняло весь буфер, места под нуль-терминатор нет
.fail:
    xor eax, eax
    xor edx, edx
    pop r14
    pop r13
    pop r12
    ret

.eof_before_word:
    xor eax, eax
    xor edx, edx
    pop r14
    pop r13
    pop r12
    ret

.finish:
    mov byte [r12 + r14], 0
    mov rax, r12
    mov rdx, r14
    pop r14
    pop r13
    pop r12
    ret

; Принимает указатель на строку, пытается
; прочитать из её начала беззнаковое число.
; Возвращает в rax: число, rdx: его длину в символах
; rdx = 0 если число прочитать не удалось
parse_uint:
    xor rax, rax
    xor rdx, rdx

.loop:
    movzx rcx, byte [rdi + rdx]
    cmp cl, '0'
    jb .done
    cmp cl, '9'
    ja .done

    imul rax, rax, 10
    sub rcx, '0'
    add rax, rcx
    inc rdx
    jmp .loop

.done:
    ret

; Принимает указатель на строку, пытается
; прочитать из её начала знаковое число.
; Если есть знак, пробелы между ним и числом не разрешены.
; Возвращает в rax: число, rdx: его длину в символах (включая знак, если он был)
; rdx = 0 если число прочитать не удалось
parse_int:
    mov al, [rdi]
    cmp al, '-'
    je .negative
    cmp al, '+'
    je .positive_with_sign
    jmp parse_uint

.negative:
    lea rdi, [rdi + 1]
    call parse_uint
    test rdx, rdx
    jz .fail
    neg rax
    inc rdx
    ret

.positive_with_sign:
    lea rdi, [rdi + 1]
    call parse_uint
    test rdx, rdx
    jz .fail
    inc rdx
    ret

.fail:
    xor eax, eax
    xor edx, edx
    ret

; Принимает указатель на строку, указатель на буфер и длину буфера
; Копирует строку в буфер
; Возвращает длину строки если она умещается в буфер, иначе 0
string_copy:
    push rdi
    push rsi
    push rdx

    call string_length        ; rax = длина строки

    pop rdx                   ; размер буфера
    pop rsi                   ; буфер
    pop rdi                   ; строка

    mov rcx, rax              ; длина строки
    inc rcx                   ; +1 для нуль-терминатора
    cmp rcx, rdx
    ja .fail

    xor rdx, rdx
.copy_loop:
    mov al, [rdi + rdx]
    mov [rsi + rdx], al
    inc rdx
    test al, al
    jne .copy_loop

    mov rax, rcx
    dec rax                   ; вернуть длину строки без нуля
    ret

.fail:
    xor eax, eax
    ret