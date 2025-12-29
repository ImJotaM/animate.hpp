ifeq ($(OS),Windows_NT)
    SHELL := cmd.exe
    .SHELLFLAGS := /c
endif

bin/:
	mkdir bin

test.exe: test/test.cpp bin/
	g++ ./test/test.cpp -o ./bin/test.exe -I. -I./raylib/include/ -L./raylib/lib/ -lraylib -lwinmm -lgdi32

run: test.exe
ifeq ($(OS), Windows_NT)
	.\bin\test.exe
else
	./bin/test.exe
endif