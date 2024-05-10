# gpu-rdma


rdma example:
https://github.com/animeshtrivedi/rdma-example

cmake -S . -B build
cmake -S . -B build -D GPU_RUNTIME=CUDA
cmake --build build

./build/src/host/rdma_server
./build/src/host/rdma_client -a 192.168.2.244 -s textstring