
build: all

all:
	hipcc -c test.c -o test.o
	hipcc -c dlop.c -o dlop.o
	hipcc test.o dlop.o -libverbs -lrdmacm -o dlop

clean:
	rm -r *.o
	rm test dlop