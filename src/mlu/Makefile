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
	${CC} ${IFLAGS} ${CFLAGS} -c rdma_common.c -o rdma_common.o
	${CC} ${IFLAGS} ${CFLAGS} -c rdma_client.c -o rdma_client.o
	${CC} ${IFLAGS} ${CFLAGS} -c rdma_server.c -o rdma_server.o
	${CC} ${IFLAGS} ${CFLAGS} ${LFLAGS} -c cnnl_w.c -o cnnl_w.o

	${CC} ${IFLAGS} ${CFLAGS} ${LFLAGS} cnnl_w.o rdma_common.o rdma_client.o -o cnnl_rdma_client
	${CC} ${IFLAGS} ${CFLAGS} ${LFLAGS} cnnl_w.o rdma_common.o rdma_server.o -o cnnl_rdma_server

clean:
	rm -r *.o
	rm cnnl_rdma_client cnnl_rdma_server