; words.inc
%include "colon.inc"

section .rodata

    colon "third", third_word
    db "world!", 0

    colon "second", second_word
    db "another", 0

    colon "first", first_word
    db "Hello", 0