;Lib with functions from 1st lab
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
 global read_line

    ; Принимает код возврата и завершает текущий процесс
    exit:
        xor rax, rax
        mov rax, 60                     ;код для syscall'a
        syscall ;system call
        ret

    ; Принимает указатель на нуль-терминированную строку, возвращает её длину
    string_length:
        xor rax, rax
        .counter:                       ;счётчик для длины строки
            cmp byte[rdi+rax], 0        ;в rdi содержится адрес строки и сравнение с 0, чтобы определить конец строки
            je .end
            inc rax                     ;обновляем счётчик
            jmp .counter
        .end:
            ret

    ; Принимает указатель на нуль-терминированную строку, выводит её в stdout
    print_string:
        xor rax, rax
        push rdi                        ;сохраняем rdi, т.к. rdi - caller-saved
        call string_length
        pop rdi                         ;возвращаем занчение rdi из стека
        mov rsi, rdi                    ;адрес строки из rdi в rsi
        mov rdx, rax                    ;длину строки после вызова string_length
        mov rdi, 1                      ;stdout дескриптор
        mov rax, 1
        syscall                         ;вызов write
        ret

    ; Принимает код символа и выводит его в stdout
    print_char:
        xor rax, rax
        push rdi                        ;Сохраняем rdi, т.к. rdi - caller-saved
        mov rsi, rsp                    ;переводим со стека адрес символа для syscall'a
        pop rdi                         ;возвращаем значение rdi из стека
        mov rdx, 1
        mov rdi, 1                      ;stdout дескриптор
        mov rax, 1
        syscall                         ;вызов write
        ret

    ; Переводит строку (выводит символ с кодом 0xA)
    print_newline:
        xor rax, rax
        mov rdi, 10                     ;10 - код ASCII символа \n
        jmp print_char                  ;переход (вызов) функции print_char

    ; Выводит беззнаковое 8-байтовое число в десятичном формате
    ; Совет: выделите место в стеке и храните там результаты деления
    ; Не забудьте перевести цифры в их ASCII коды.
    print_uint:
        xor rax, rax
        mov rax, rdi                    ;сохраняем в rax само число, rdi - аргумент
        mov r8, 10                      ;в r8 записываем основание СС
        mov r9, rsp                     ;сохраняем указатель стека до, чтобы потом записывать само число по одной цифре в стек и будем передавать нуль-терминированную строку в функцию print_string, передав адрес начала(указатель стека) в rdi
        push 0                          ;строка должна быть нуль-терминированной
        .counter:
            xor rdx,rdx                 ;после каждой итерации очищаем rdx
            div r8
            add rdx, 48                 ;конвертация символа в ASCII
            dec rsp
            mov byte[rsp], dl           ;dl is a register, smallest part of rdx (2 семинар)
            cmp rax, 0                  ;сравнение с 0
            je .end
            jmp .counter
        .end:
            mov rdi, rsp                ;адрес переведенного числа загружаем в rdi
            push r9                     ;сохраняем r9, т.к. он caller-saved
            call print_string
            pop r9
            mov rsp, r9                 ;возвращаем указатель стека до момента вызова функции print_uint
            ret

    ; Выводит знаковое 8-байтовое число в десятичном формате
    print_int:
        xor rax, rax
        cmp rdi, 0                      ;проверка числа на знак, если положительное число положительное, то вызов функции print_uint, иначе выводим сначала знак -, а потом neg самого числа и выводим print_uint
        jl .else                        ;установлены флаги OF и SF
        jmp print_uint
        .else:
            push rdi                    ;сохраняем аргумент
            mov rdi, 45                 ;записываем в rdi код знака минус ASCII
            call print_char             ;выводим минус
            pop rdi
            neg rdi                     ;обратный код отрицательного числа
            jmp print_uint

    ; Принимает два указателя на нуль-терминированные строки, возвращает 1 если они равны, 0 иначе
    string_equals:
        xor rax, rax
        xor r8, r8
        xor r9, r9
        xor rcx, rcx
        .counter:                       ;два указателя rdi и rsi, rcx - счётчик, r8b и r9b - регистры для i-ого элемента строк (lowest 8 bits of r8, r9)
            mov r8b, byte[rcx+rdi]
            mov r9b, byte[rcx+rsi]
            cmp r8b, r9b                ;сравнение двух символов
            jne .else                   ;если не равны
            cmp r8b, 0                  ;если все символы прошлы проверку и дошли до конца строки
            je .then
            inc rcx
            jmp .counter
        .then:
            mov rax, 1
            ret
        .else:
            mov rax,0
            ret

    ; Читает один символ из stdin и возвращает его. Возвращает 0 если достигнут конец потока
    read_char:
        xor rax, rax
        xor rdi, rdi                    ;stdin дескриптор
        mov rdx, 1
        push 0
        mov rsi, rsp                    ;загрузка адреса символа для syscall'a
        syscall
        pop rax
        ret

    ; Принимает: адрес начала буфера, размер буфера
    ; Читает в буфер слово из stdin, пропуская пробельные символы в начале, .
    ; Пробельные символы это пробел 0x20, табуляция 0x9 и перевод строки 0xA.
    ; Останавливается и возвращает 0 если слово слишком большое для буфера
    ; При успехе возвращает адрес буфера в rax, длину слова в rdx.
    ; При неудаче возвращает 0 в rax
    ; Эта функция должна дописывать к слову нуль-терминатор
    read_word:
        xor rax, rax
        xor rcx, rcx                    ;счётчик для итерации, а rdi - адрес начала, rsi - размера буфера
        .counter:
            push rdi
            push rsi
            push rcx
            call read_char              ;сохраняем регистры перед call read_char т.к. они caller-saved
            pop rcx
            pop rsi
            pop rdi
            cmp rax, 0                  ;сравнение с 0, чтобы определить конец строки
            je .end
            cmp rax, 0x20               ;проверка на пробельный символ, табуляцию, перевод строки
            je .spaces
            cmp rax, 0xa
            je .spaces
            cmp rax, 0x9
            je .spaces
            mov [rcx+rdi], rax          ;загрузка символа
            inc rcx
            cmp rcx, rsi                ;сравнение длины слова с размером буфера
            jge .outOfRange
            jmp .counter
        .spaces:
            cmp rcx, 0                  ;случай когда пробельный символ стоит в начале
            je .counter
            jmp .end                    ;случай когда пробельный символ стоит в конце
        .outOfRange:
            xor rax, rax
            xor rdx, rdx
            ret
        .end:
            xor rax, rax
            mov [rcx+rdi], rax          ;условие, что функция должна дописывать к слову нуль-терминатор
            mov rax, rdi                ;при успехе возвращает адрес буфера в rax, длину слова в rdx
            mov rdx, rcx
            ret

    ; Принимает указатель на строку, пытается
    ; прочитать из её начала беззнаковое число.
    ; Возвращает в rax: число, rdx : его длину в символах
    ; rdx = 0 если число прочитать не удалось
    parse_uint:
        xor rax, rax
        xor rcx, rcx                    ;счётчик для длины слова
        mov r8, 10                      ;в r8 записываем основание СС
        .counter:
            movzx r9, byte[rdi+rcx]     ;загружаем байт в r9 с расширением разрядности без учета знака
            cmp r9, 0                   ;проверка на конец строки
            je .end
            cmp r9b, 48                ;проверка на диапозазон от 0(48) до 9(57)
            jl .end
            cmp r9b, 57
            jg .end
            mul r8
            sub r9b, 48                ;получаем код цифры
            add rax, r9                ;добавляем цифру после сдвинутого разряда
            inc rcx
            jmp .counter
        .end
            mov rdx, rcx               ;условие, что возвращает в rax: число, rdx : его длину в символах
            ret

    ; Принимает указатель на строку, пытается
    ; прочитать из её начала знаковое число.
    ; Если есть знак, пробелы между ним и числом не разрешены.
    ; Возвращает в rax: число, rdx : его длину в символах (включая знак, если он был)
    ; rdx = 0 если число прочитать не удалось
    parse_int:
        xor rax, rax
        xor rdx, rdx
        mov rcx, rdi
        cmp byte[rcx], 45              ;проверка числа на положительный знак, если отрицательный, то выводим минус и вызываем print_uint
        je .then
        jmp .else
        .then:                         ;если отрицательное
            inc rcx                    ;сдвигаем адрес начала числа c минуса
            mov rdi, rcx
            push rcx                   ;сохраняем фывrcx перед вызовом parse_uint
            call parse_uint            ;считываем число
            pop rcx
            neg rax                    ;обратный код
            inc rdx
            ret
        .else:
            mov rdi, rcx
            jmp parse_uint             ;в случае положительного числа просто вызываем parse_uint

    ; Принимает указатель на строку, указатель на буфер и длину буфера
    ; Копирует строку в буфер
    ; Возвращает длину строки если она умещается в буфер, иначе 0
    string_copy:
        xor rax, rax
        xor rcx, rcx
        push rdx
        push rdi
        push rsi
        push rcx
        call string_length              ;находим длину строки и сохраняем caller-saved регистры rdi, rdx, rsi, rcx
        pop rcx
        pop rsi
        pop rdi
        pop rdx
        mov r8, rax                     ;помещаем в r8 длину строки
        cmp rdx, r8                     ;сравниваем размер буфера и длину строки, возвращаем 0.
        jl .outOfRange
        .counter:
            cmp rcx, r8                 ;если мы вышли счётчиком за пределы строки
            jg .end
            mov r10, [rcx+rdi]          ;копируем символ строки
            mov [rcx+rsi], r10          ;вставляем символ строки в буфер
            inc rcx
            jmp .counter
        .outOfRange:
            mov rax, 0                  ;условие возвращает длину строки если она умещается в буфер, иначе 0
            ret
        .end:
            mov rax, r8
            ret

   ; Принимает ссылку на буфер (rdi) и длину буфера (rsi).
; Считывает из стандартного потока ввода строку и записывает ее в буфер.
; Возвращает адрес буфера или 0, если строку не удалось прочитать (она длиннее буфера).
read_line:
    push r12
    push r13
    push rdi
    mov r12, rdi
    mov r13, rsi
    dec r13
    .begin:
        call read_char
        test al, al
        jz .finish
        cmp al, 0xA
        jz .finish
    .write_char:
        test r13, r13
        jz .size_error
        mov byte[r12], al
        inc r12
        dec r13
        jmp .begin
    .size_error:
        call read_char
        test al, al
        jz .handling
        cmp al, 0xA
        jnz .size_error
    .handling:
        pop rdi
        xor rax, rax
        jmp .end
    .finish:
        pop rax
        mov byte[r12], 0
        jmp .end
    .end:
        pop r13
        pop r12
    ret
