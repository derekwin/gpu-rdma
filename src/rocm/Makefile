build: all

all:
	hipcc -c rdma_common.c -o rdma_common.o
	hipcc -c rdma_client.c -o rdma_client.o
	hipcc -c rdma_server.c -o rdma_server.o
	hipcc rdma_common.o rdma_client.o -libverbs -lrdmacm -o rocm_rdma_client
	hipcc rdma_common.o rdma_server.o -libverbs -lrdmacm -o rocm_rdma_server

clean:
	rm -r *.o
	rm rocm_rdma_client rocm_rdma_server