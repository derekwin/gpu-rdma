build: all


IFLAGS += -I../../Common
IFLAGS += -I/opt/dtk/include/hip
IFLAGS += -I/opt/dtk/include/hsa

CFLAGS += -DHAVE_HSA_AMD_PORTABLE_EXPORT_DMABUF=1
CFLAGS += -DHAVE_ROCM_RUNTIME=1
# CFLAGS += -DROCM_DMABUF_SUPPERTED

LFLAGS += -libverbs -lrdmacm
# LFLAGS += -L/opt/dtk/hsa/lib # libhsa-runtime64.so

all:
	hipcc ${IFLAGS} ${CFLAGS} -c rdma_common.c -o rdma_common.o
	hipcc ${IFLAGS} ${CFLAGS} -c rdma_client.c -o rdma_client.o
	hipcc ${IFLAGS} ${CFLAGS} -c rdma_server.c -o rdma_server.o
	hipcc ${IFLAGS} ${CFLAGS} ${LFLAGS}  -c rocm.c -o rocm.o

	hipcc ${IFLAGS} ${CFLAGS} ${LFLAGS} rocm.o rdma_common.o rdma_client.o -o rocm_rdma_client
	hipcc ${IFLAGS} ${CFLAGS} ${LFLAGS} rocm.o rdma_common.o rdma_server.o -o rocm_rdma_server

clean:
	rm -r *.o
	rm rocm_rdma_client rocm_rdma_server