build: all


IFLAGS += -I../../Common
CFLAGS += -DHAVE_HSA_AMD_PORTABLE_EXPORT_DMABUF=1

all:
	hipcc ${CFLAGS} ${CFLAGS} -c rdma_common.c -o rdma_common.o
	hipcc ${CFLAGS} ${CFLAGS} -c rdma_client.c -o rdma_client.o
	hipcc ${CFLAGS} ${CFLAGS} -c rdma_server.c -o rdma_server.o
	hipcc ${CFLAGS} ${CFLAGS} rdma_common.o rdma_client.o -libverbs -lrdmacm -o rocm_rdma_client
	hipcc ${CFLAGS} ${CFLAGS} rdma_common.o rdma_server.o -libverbs -lrdmacm -o rocm_rdma_server

clean:
	rm -r *.o
	rm rocm_rdma_client rocm_rdma_server