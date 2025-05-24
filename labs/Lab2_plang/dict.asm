;File with function find_word including lib.inc with function string_equals
 section .text
 global find_word

 %include "lib.inc"

 find_word:                             ;boolean find_word(rdi, rsi), �, где rdi - указатель на нуль-терминированную строку, rsi - указатель на строку в словаре
        xor r14, r14
        xor r15, r15
        sub rsi, 8
        .counter:
                test rsi, rsi           ;проверка на кон�ец словаря
                je .else
                add rsi, 8              ;наичинаем перебор слов
                mov r14, rdi            ;сохроняем значение регистра rdi в callee-saved регистр r14
                mov r15, rsi            ;сохроняем значение регистра rsi в callee-saved регистр r15
                call string_equals      ;вызов boolean string_equals(rdi, rsi), затем смотрим на rax
                mov rdi, r14            ;восстанавливаем значение регистра rdi после вызова string_equals
                mov rsi, r15            ;восстанавливаем значение регистра rsi после вызова string_equals
                test rax, rax
                jnz .then                ;rax = 1, тог�да строки равны
                mov rsi, [rsi]          ;смотрим следующее слово
                jmp .counter
        .then:
                mov rax, rsi            ;сох�если нашли слово, то передаем �указатель на не�ёеё в rax
                ret
        .else:
                xor rax, rax            ;если не нашли слово, о передаем 0 в rax
                ret
