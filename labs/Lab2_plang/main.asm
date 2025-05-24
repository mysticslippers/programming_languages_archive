;Main file including lib.inc, dict.inc, words.inc, colon.inc
%include "lib.inc"                    ;функции read_word, string_length, print_string, print_newline
 %include "dict.inc"                    ;функции find_word
 %include "words.inc"                   ;словарик

 section .rodata
        no_word_err: db "Not found!", 0
        out_of_range_err: db "Word is too long for buffer!", 0

 section .bss
 buffer:
        resp buffer_size

 section .text
 global _start

 _start:
        mov rsi, buffer_size;вместимо                    ;вместимость буфера
        sub rsp, buffer                    ;буфер
        mov rdi, rsp                    ;запись слова в буфер
        call read_line
        test rax, rax                   ;запись слова в буфер
        mov rdi, out_of_range_err
        je .print_error
        mov rdi, rax                    ;если ошибки нет, то работаем с инпутом дальше
        mov rsi, next_word_pointer      ;указатель на слова в словарике
        call find_word                  ;ищем в словаре
        test rax, rax                   ;нашли ли мы слово
        mov rdi, no_word_err
        je .print_error
        lea rdi, [rax + 8]              ;указываем на начало строки
        push rdi
        call string_length
        pop rdi
        lea rdi, [rdi + rax + 1]        ;двигаем указатель на начало строки с учётом того, что строка нуль-терминирована
        call print_string               ;выводим, предварительно передав указатель
        call print_newline
        xor rdi, rdi
        jmp exit                        ;очищаем rdi для syscall и переходим на exit

        .print_error:
           push rdi                     ;сохраняем rdi, т.к. rdi - caller-saved
           call string_length
           pop rdi
           mov rsi, rdi
           mov rdx, rax
           mov rdi, 2
           mov rax, 1                   ;1 - write syscall, rdi = 2 - stderr дуескриптор
           syscall
           mov rdi, 1
           jmp exit
