build/te: src/main.c
	mkdir -p build
	gcc src/main.c -l sds -o build/te 