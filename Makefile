build/te: src/main.c build/libctui.a
	gcc src/main.c -Iinclude -Lbuild -lsds -lctui -lm -lpthread -obuild/te

build/libctui.a: build/general_terminal.o build/io_buffers.o
	ar rcs build/libctui.a build/general_terminal.o build/io_buffers.o build/time_check.o

build/general_terminal.o: src/general_terminal.c include/ctui_general_terminal.h
	mkdir -p build
	gcc src/general_terminal.c -Iinclude -c -obuild/general_terminal.o

build/io_buffers.o: src/io_buffers.c include/ctui_io_buffers.h
	mkdir -p build
	gcc src/io_buffers.c -Iinclude -c -obuild/io_buffers.o

build/time_check.o: src/time_check.c include/ctui_time_check.h
	mkdir -p build
	gcc src/time_check.c -Iinclude -c -obuild/time_check.o

clean:
	rm -r build