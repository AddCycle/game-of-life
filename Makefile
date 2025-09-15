all: build run

build:
	clang *.c -o game.exe -I "C:/SDL3/include" -L "C:/SDL3/lib/x64" -lSDL3

run:
	./game.exe

clean:
	rm -rf *.o *.exe