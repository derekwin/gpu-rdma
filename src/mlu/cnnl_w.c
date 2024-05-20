#include "cnnl_w.h"

// cnnl
bool LaunchBlocking = false;

status_t prepare_gpu_driver()
{
    int ret = STATUS_SUCCESS;
// #if HAVE_CNNL_RUNTIME
    ret = cnnl_init();
// #endif
    return ret;
}

// cnnl device init
status_t cnnl_init(void)
{
    static pthread_mutex_t cnnl_init_mutex = PTHREAD_MUTEX_INITIALIZER;
    static volatile int cnnl_initialized = 0;
    status_t status = STATUS_SUCCESS;
    CNresult ret;
    
    // cnInit Driver
    ret = cnInit(0);
    if (ret != CN_SUCCESS) {
        log_error("failed to cnInit %d", ret);
        return STATUS_ERROR;
    }

    // need create context first
    CNcontext context;
	ret = cnCtxCreate(&context, 0, 0);
	if (ret != CN_SUCCESS) {
        log_error("failed to create cnCtx %d.", ret);
        return -1;
    }

    cnnl_initialized = 1;

end:
    pthread_mutex_unlock(&cnnl_init_mutex);
    return status;
}

// // cnnl thread device init
// static status_t cnnl_thread_init()
// {
//     hipError_t ret;
//     int num_gpus;
//     int gpu_index;

//     ret = hipGetDeviceCount(&num_gpus);
//     if (ret != hipSuccess) {
//         return STATUS_ERROR;
//     }

//     gpu_index = num_gpus-1; // 后续实现集合通信需要引入group_id，组的概念

//     ret = hipSetDevice(gpu_index);
//     if (ret != hipSuccess) {
//         return STATUS_UNSUPPORTED;
//     }

//     return STATUS_SUCCESS;
// }

// cnnl mem alloc

//https://github1s.com/NVIDIA/nccl/blob/master/src/include/alloc.h
// 参考下上面nccl的分配存储的逻辑，是否需要增加各种其他设置分配显存才可以被使用

status_t cnnl_mem_alloc(size_t length,
                                  memory_type_t mem_type,
                                  void **address_p)
{
    CNresult ret;
    CNaddr addr;
    cn_uint64_t len = length;

    if ((mem_type != MEMORY_TYPE_GPU) && (mem_type != MEMORY_TYPE_GPU_MANAGED)) {
        return STATUS_UNSUPPORTED;
    }
    log_info("alloc len %zu.", len);
    ret = cnMallocPeerAble(&addr, len);
    // ret = cnMallocPeerAble(addr, len);
    // ret = cnMalloc(&addr, len);
    if (ret == CN_ERROR_NOT_INITIALIZED) {
        log_error("CNDrv has not been initialized with cnInit or CNDrv fails to be initialized.");
        return STATUS_ERROR;
    }
    if (ret == CN_ERROR_INVALID_VALUE) {
        log_error("The parameters passed to this API are not within an acceptable value range.");
        return STATUS_ERROR;
    }
    if (ret != CN_SUCCESS) {
        log_error("failed to allocate memory %d.", ret);
        return STATUS_ERROR;
    }
    
    void *addr_p = &addr;
    *address_p= addr_p;
    log_info("allocate device memory &addr %p, address_p %p.", addr_p, address_p);
    return STATUS_SUCCESS;
}

status_t cnnl_mem_free(void **address_p)
{
    CNresult ret;
    CNaddr* addr = (CNaddr*)*address_p;
    ret = cnFree(*addr);
    if (ret != CN_SUCCESS) {
        log_error("failed to free memory");
        return STATUS_ERROR;
    }

    return STATUS_SUCCESS;
}

status_t cnnl_mem_cpy(void *dst, void *src, uint64_t len)
{   
    log_info("cp memory to %p from %p.", dst, src);
    CNaddr dst_a = (CNaddr)dst;
    CNaddr src_a = (CNaddr)src;
    // CNresult ret = cnMemcpy(*dst_a, *src_a, len);
    CNresult ret = cnMemcpy(dst_a, src_a, (cn_uint64_t)len+1); // 结束符
    if (ret != CN_SUCCESS) {
        log_error("failed to cp memory %d", ret);
        return STATUS_ERROR;
    }
    return STATUS_SUCCESS;
}
