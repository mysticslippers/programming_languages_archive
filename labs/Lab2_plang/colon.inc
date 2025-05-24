;Macro for dictionary
 %define next_word_pointer 0

 %macro colon 2

        %%tmp: dq next_word_pointer
        db %1, 0

        %2:
           %define next_word_pointer %%tmp
 %endmacro
