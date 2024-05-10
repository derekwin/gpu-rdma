# gpu-rdma


rdma example:
https://github.com/animeshtrivedi/rdma-example


## cmake (host, rocm)
```
cmake -S . -B build
cmake -S . -B build -D GPU_RUNTIME=CUDA
cmake -S . -B build -DCMAKE_CUDA_COMPILER=/usr/local/cuda/bin/nvcc #cuda
cmake --build build
```

## make (cuda, cuda_example)
cd src/cuda
make

./cuda_rdma_server
./cuda_rdma_client -a 192.168.2.242 -s textstring