ASM=nasm
ASMFLAGS=-f elf64
LD=ld
PY=python3

%.o:%.asm
        $(ASM) $(ASMFLAGS) $< -o $@

main.o:main.asm lib.inc dict.inc colon.inc words.inc

lab:main.o lib.o dict.o
        $(LD) -o $@ $^

.PHONY: test clean

test:
        $(PY) test.py

clean:
        rm -f *.o lab
