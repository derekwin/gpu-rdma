# gpu-rdma


rdma example:
https://github.com/animeshtrivedi/rdma-example


## cmake (host)
```
cmake -S . -B build
cmake --build build
```

## make (rocm, rocm_example, cuda, cuda_example)
```
cd src/cuda
make all
make clean
```

## clean
```
bash clean.sh
```

## run
```
./cuda_rdma_server
./cuda_rdma_client -a 192.168.2.242 -s textstring
```

```
./rocm_rdma_server
./rocm_rdma_client -a 192.168.2.245 -s textstring
```