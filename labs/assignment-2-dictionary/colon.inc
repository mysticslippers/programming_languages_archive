; colon.inc
; Макрос для создания записи словаря.
; Формат записи:
;   dq <pointer to previous entry>
;   db <key>, 0
; <label>:
;   db <value>, 0

%ifndef COLON_INC
%define COLON_INC

%define next_word_pointer 0

%macro colon 2
    %%entry:
        dq next_word_pointer
        db %1, 0
    %2:
        %define next_word_pointer %%entry
%endmacro

%endif