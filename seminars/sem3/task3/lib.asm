 ;lib.asm (file for print_hex.asm and exit.asm)
 section .data
 newline_char: db 10
 codes: db '0123456789abcdef'

 section .text
 global print_hex
 global print_new_line
 global exit

 exit:          ;void exit() function
        mov rax, 60
        xor rdi, rdi
        syscall

 print_new_line:         ;void print_new_line() function
        mov rax, 1
        mov rdi, 1      ;stdout descriptor
        mov rsi, newline_char
        mov rdx, 1
        syscall
        ret

 print_hex:      ;void print_hex(rdi, argument) function
        mov rax, rdi    ;pass argument from register rdi to rax
        mov rdi, 1      ;stdout descriptor
        mov rdx, 1
        mov rcx, 64     ;64 -> 60 -> 56 -> ... -> 4, 0

        .loop:
                push rax        ;save the initial rax value
                sub rcx, 4      ;each 4 bits

                sar rax, cl     ;cl is a smallest part of rcx (4 bits)
                and rax, 0xf    ;clear high order bits

                lea rsi, [codes + rax]
                mov rax, 1

                push rcx
                syscall
                pop rcx

                pop rax
                test rcx, rcx   ;fastest 'is it zero?' check
                jnz .loop
        ret
