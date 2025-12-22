ifeq ($(OS),Windows_NT)
    SHELL := cmd.exe
    .SHELLFLAGS := /c
endif

bin/:
	mkdir bin

main.exe: src/main.cpp bin/
	g++ ./src/main.cpp -o ./bin/main.exe -I./raylib/include/ -L./raylib/lib/ -lraylib -lwinmm -lgdi32

run: main.exe
	.\bin\main.exe