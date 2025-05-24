SRCCOMPILE=-d classes
SRCJAVA=-cp "src"
MAIN=src/Story.java
JARFLAGS=-cfm
MF=src/MANIFEST.mf
SRCBUILD=-C ./classes .
RUNJAR=java -jar Lab4_prog.jar

.PHONY:compile build run clean

compile:
        javac $(SRCCOMPILE) $(SRCJAVA) $(MAIN)

build:
        jar $(JARFLAFS) Lab4_prog.jar $(MF) $(SRCBUILD)

run:
        $(RUNJAR)

clean:
        rm -f ./classes/**/*.class ./Lab4_prog.jar
