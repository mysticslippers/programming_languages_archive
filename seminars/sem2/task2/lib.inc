 ;print_string.asm and string_length.asm
 section .data
 message: db 'hello, world!', 0

 section .text
 global _start

 exit:  ;void exit() function
        mov rax, 60
        xor rdi, rdi
        syscall

 string_length:         ;number string_length(rax, rdi) function
        mov rax, rdi
        .counter:
                cmp byte [rax], 0
                je .end
                inc rax
                jmp .counter
        .end:
                sub rax, rdi
                ret

 print_string:           ;void print_string(rdx, rax) function
        push rdi
        call string_length
        pop rdi
        mov rdx, rax
        mov rsi, rdi
        mov rax, 1
        mov rdi, 1      ;stdout descriptor
        syscall
        ret
