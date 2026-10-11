build/te: src/main.c build/libctui.a
	gcc src/main.c -Iinclude -lsds -lctui -lm -obuild/te

build/libctui.a: build/general_terminal.o
	ar rcs build/libctui.a build/general_terminal.o

build/general_terminal.o: src/general_terminal.c include/general_terminal.h
	mkdir -p build
	gcc src/general_terminal.c -Iinclude -c -obuild/general_terminal.o