build: all

CC := clang

IFLAGS += -I../../Common
IFLAGS += -I/usr/local/neuware/include

CFLAGS += -DNEUWARE_HOME=/usr/local/neuware
CFLAGS += -DHAVE_CNNL_RUNTIME=1

LFLAGS += -libverbs -lrdmacm -lcnrt -lmluops -lcndrv
LFLAGS += -L/usr/local/neuware/lib64
LFLAGS += -L/usr/local/neuware/lib

all:
	${CC} ${IFLAGS} ${CFLAGS} -c test.c -o test.o
	${CC} ${IFLAGS} ${CFLAGS} ${LFLAGS} test.o -o test

clean:
	rm -r *.o
	rm test