 ;parse_uint.asm

 section .text

 parse_uint:    ;number, number parse_uint(rdi, argument) function
        xor rax, rax
        xor rdx, rdx
        mov rcx, 0

.counter:
                mov dl, byte[rdi + rcx]         ;load to dl(low byte of rdx) one sign of number with relative addressing
                sub dl, 48                      ;if code of sign between 0 and 10

                cmp dl, 0                       ;if code < 0 (sign flag set)
                js .exit_func

                cmp dl, 10                      ;if code >= 10 (sign flag not set)
                jns .exit_func

                inc rcx                         ;inc number length
                imul rax, 10                    ;add digit
                add rax, rdx
                jmp .counter
                ret

        .exit_func:
                mov rdx, rcx                    ;load number length to rdx
                ret
