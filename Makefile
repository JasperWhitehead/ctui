build/te: src/main.c build/libctui.a
	gcc src/main.c -Iinclude -Lbuild -lctui -lsds -lm -lpthread -obuild/te

build/libctui.a: build/general_terminal.o build/io_control.o build/time_check.o
	ar rcs build/libctui.a build/general_terminal.o build/io_control.o build/time_check.o

build/general_terminal.o: src/general_terminal.c include/ctui_general_terminal.h
	mkdir -p build
	gcc src/general_terminal.c -Iinclude -c -obuild/general_terminal.o

build/io_control.o: src/io_control.c include/ctui_io_control.h
	mkdir -p build
	gcc src/io_control.c -Iinclude -c -obuild/io_control.o

build/time_check.o: src/time_check.c include/ctui_time_check.h
	mkdir -p build
	gcc src/time_check.c -Iinclude -c -obuild/time_check.o

clean:
	rm -r build