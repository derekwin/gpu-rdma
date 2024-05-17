build: all

CC := gcc

IFLAGS += -I../../Common
IFLAGS += -I/usr/local/neuware/include

CFLAGS += -g -O0

LFLAGS += -libverbs -lrdmacm
LFLAGS += -L/usr/local/lib64/

all:
	${CC} ${IFLAGS} ${CFLAGS} -c rdma_common.c -o rdma_common.o
	${CC} ${IFLAGS} ${CFLAGS} -c rdma_client.c -o rdma_client.o
	${CC} ${IFLAGS} ${CFLAGS} -c rdma_server.c -o rdma_server.o
	${CC} ${IFLAGS} ${CFLAGS} ${LFLAGS} -c cnnl_warpper.c -o cnnl_warpper.o

	${CC} ${IFLAGS} ${CFLAGS} ${LFLAGS} cnnl_warpper.o rdma_common.o rdma_client.o -o cnnl_rdma_client
	${CC} ${IFLAGS} ${CFLAGS} ${LFLAGS} cnnl_warpper.o rdma_common.o rdma_server.o -o cnnl_rdma_server

clean:
	rm -r *.o
	rm cnnl_rdma_client cnnl_rdma_server