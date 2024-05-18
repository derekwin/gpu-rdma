# gpu-rdma


rdma example:
https://github.com/animeshtrivedi/rdma-example

## env
- 寒武纪使用clang编译
```
apt install clang
```
- 每个品牌的动态库要配置好到环境变量生效
```
# mlu
export LD_LIBRARY_PATH=/usr/local/neuware/lib64:$LD_LIBRARY_PATH
```

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

```
./cnnl_rdma_server
./cnnl_rdma_client -a 192.168.2.245 -s textstring
```