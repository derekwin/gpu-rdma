#ifndef CUDA_H
#define CUDA_H

#include <cuda_runtime.h>
// #include "driver_types.h"
#include "stdint.h"

#include "type.h"

#include <pthread.h>

status_t prepare_gpu_driver();

status_t cuda_init(void);
static status_t cuda_thread_init();

status_t cuda_mem_alloc(size_t length,
                                  memory_type_t mem_type,
                                  void **address_p);
status_t cuda_mem_free(void **address_p);

#endif /* CUDA_H */

