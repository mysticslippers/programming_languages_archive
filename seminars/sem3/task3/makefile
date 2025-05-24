ASM=nasm
ASMFLAGS=-f elf64
LD=ld -o

%.o:%.asm
        $(ASM) $(ASMFLAGS) $< -o $@
task3:main.o lib.o
        $(LD) $@ $^
.PHONY:clean
clean:
        rm -f *.o task3
