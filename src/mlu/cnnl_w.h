#ifndef CNNL_H
#define CNNL_H

#include "cnrt.h"
#include "mlu_op.h"
#include "cn_api.h" // CNresult

#include "type.h"

#include <pthread.h>

#include <dlfcn.h>
#include <sys/utsname.h>

status_t prepare_gpu_driver();

void initDevice(int *dev, cnrtQueue_t *queue, mluOpHandle_t *handle);
status_t cnnl_init(void);
static status_t cnnl_thread_init();

status_t cnnl_mem_alloc(size_t length,
                                  memory_type_t mem_type,
                                  void **address_p);
status_t cnnl_mem_free(void **address_p);
status_t cnnl_mem_cpy(void *dst, void *src, uint64_t len);
#endif /* CNNL_H */

