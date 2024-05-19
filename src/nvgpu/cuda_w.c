#include "cuda_w.h"

// cuda

status_t prepare_gpu_driver()
{
    int ret = STATUS_SUCCESS;
// #if HAVE_CUDA_RUNTIME
    ret = cuda_init();
// #endif
    return ret;
}

// cuda device init
status_t cuda_init(void)
{
    static pthread_mutex_t cuda_init_mutex = PTHREAD_MUTEX_INITIALIZER;
    static volatile int cuda_initialized = 0;
    status_t status = STATUS_SUCCESS;
    int count;

    //取得支持Cuda的装置的数目
    cudaGetDeviceCount(&count);

    //没有符合的硬件
    if (count == 0) {
        fprintf(stderr, "There is no device.\n");
        status = STATUS_ERROR;
        goto end;
    }

    int i;

    for (i = 0; i < count; i++) {
        struct cudaDeviceProp prop;
        if (cudaGetDeviceProperties(&prop, i) == cudaSuccess) {
            if (prop.major >= 1) {
                break;
            }
        }
    }

    if (i == count) {
        fprintf(stderr, "There is no device supporting CUDA 1.x.\n");
        status = STATUS_UNSUPPORTED;
        goto end;
    }

    cudaSetDevice(i);

    cuda_initialized = 1;

end:
    pthread_mutex_unlock(&cuda_init_mutex);
    return status;
}

// // cuda thread device init
// static status_t cuda_thread_init()
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

// cuda mem alloc
status_t cuda_mem_alloc(size_t length,
                                  memory_type_t mem_type,
                                  void **address_p)
{
    cudaError_t ret;

    if ((mem_type != MEMORY_TYPE_GPU) && (mem_type != MEMORY_TYPE_GPU_MANAGED)) {
        return STATUS_UNSUPPORTED;
    }
    
    ret = cudaMalloc(address_p, length);
    if (ret != cudaSuccess) {
        log_error("failed to allocate memory %d.", ret);
        return STATUS_ERROR;
    }

    return STATUS_SUCCESS;
}

status_t cuda_mem_free(void **address_p)
{
    cudaError_t ret;
    ret = cudaFree(address_p);
    if (ret != cudaSuccess) {
        log_error("failed to free memory");
        return STATUS_ERROR;
    }

    return STATUS_SUCCESS;
}