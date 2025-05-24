 ;main.asm with _start and including lin.inc
 section .text
 global _start

 %include "lib.inc"

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
