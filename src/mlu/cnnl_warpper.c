#include "cnnl_warpper.h"

// cnnl

bool LaunchBlocking = false;

status_t prepare_gpu_driver()
{
    int ret = STATUS_SUCCESS;
#if HAVE_CNNL_RUNTIME
    ret = cnnl_init();
#endif
    return ret;
}

// cnnl device init

void initDevice(int *dev, cnrtQueue_t *queue, mluOpHandle_t *handle)
{
  CNRT_CHECK(cnrtGetDevice(dev));
  CNRT_CHECK(cnrtSetDevice(*dev));

  CNRT_CHECK(cnrtQueueCreate(queue));

  mluOpCreate(handle);
  mluOpSetQueue(*handle, *queue);
}

status_t cnnl_init(void)
{
    static pthread_mutex_t cnnl_init_mutex = PTHREAD_MUTEX_INITIALIZER;
    static volatile int cnnl_initialized = 0;
    status_t status = STATUS_SUCCESS;

    // init device
    int dev;
    mluOpHandle_t handle = NULL;
    cnrtQueue_t queue = NULL;
    initDevice(&dev, &queue, &handle);
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
status_t cnnl_mem_alloc(size_t length,
                                  memory_type_t mem_type,
                                  void **address_p)
{
    CNresult ret;

    if ((mem_type != MEMORY_TYPE_GPU) && (mem_type != MEMORY_TYPE_GPU_MANAGED)) {
        return STATUS_UNSUPPORTED;
    }

    ret = cnMallocPeerAble(*address_p, length);

    if (ret != CN_SUCCESS) {
        log_error("failed to allocate memory");
        return STATUS_ERROR;
    }

    return STATUS_SUCCESS;
}

status_t cnnl_mem_free(void **address_p)
{
    CNresult ret;
    ret = cnFree((CNaddr)*address_p);
    if (ret != CN_SUCCESS) {
        log_error("failed to free memory");
        return STATUS_ERROR;
    }

    return STATUS_SUCCESS;
}