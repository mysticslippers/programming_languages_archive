ASM = nasm
ASMFLAGS = -f elf64
LD = ld
PY = python3

TARGET = lab
OBJS = main.o lib.o dict.o

%.o: %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

main.o: main.asm lib.inc dict.inc colon.inc words.inc
lib.o: lib.asm lib.inc
dict.o: dict.asm dict.inc lib.inc

$(TARGET): $(OBJS)
	$(LD) -o $@ $(OBJS)

.PHONY: all test clean

all: $(TARGET)

test: $(TARGET)
	$(PY) test.py

clean:
	rm -f *.o $(TARGET)