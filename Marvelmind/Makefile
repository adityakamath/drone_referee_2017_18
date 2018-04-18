all: marvelmind_example

marvelmind_example:
	gcc -g -pthread -c marvelmind.c -o marvelmind.o
	gcc -g -pthread -c example.c -o example.o
	gcc -o marvelmind_example example.o marvelmind.o -pthread

clean:
	rm -rf *.o

