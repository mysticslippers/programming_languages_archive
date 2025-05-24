 ;main.asm with _start and including lib.asm
 section .text
 global _start
 extern print_hex
 extern print_new_line
 extern exit

 _start:
        mov rdi, 12
        call print_hex
        call print_new_line
        mov rdi, 15
        call print_hex
        call print_new_line
        mov rdi, 10
        call print_hex
        call print_new_line
        call exit
