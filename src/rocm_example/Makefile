
build: all

all:
	hipcc -c test.c -o test.o
	hipcc test.o -libverbs -lrdmacm -o test

clean:
	rm -r *.o
	rm test