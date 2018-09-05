
all: main
	gcc -o bin main.o -pthread

main: main.c
	gcc -c main.c

clean:
	rm -r *.o bin
