build/te: src/main.c
	mkdir -p build
	gcc src/main.c -lsds -lm -obuild/te 