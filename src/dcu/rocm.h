#ifndef ROCM_H
#define ROCM_H

#include <hsa.h>
#include <hip/hip_runtime.h>
// hipError_t hipSuccess
#include <hsa_ext_amd.h>
#include "type.h"
#include <pthread.h>

#include <dlfcn.h>
#include <sys/utsname.h>

#define MAX_AGENTS 127
static struct agents {
    int num;
    hsa_agent_t agents[MAX_AGENTS];
    int num_gpu;
    hsa_agent_t gpu_agents[MAX_AGENTS];
} rocm_agents;

status_t prepare_gpu_driver();

int is_dmabuf_supported();
status_t rocm_init(void);
static hsa_status_t rocm_hsa_agent_callback(hsa_agent_t agent, void* data);
static status_t rocm_get_sys_dev(hsa_agent_t agent, sys_device_t *sys_dev_p);
static status_t rocm_thread_init();

status_t rocm_mem_alloc(size_t length,
                                  memory_type_t mem_type,
                                  void **address_p);
status_t rocm_mem_free(void **address_p);
static void rocm_dmabuf_export(const void *addr, const size_t length,
                                        memory_type_t mem_type,
                                        int *dmabuf_fd, size_t *dmabuf_offset);
status_t rocm_gpu_mem_query(const void *addr,
                            const size_t length,
                            mem_attr_t *mem_attr_p);
hsa_status_t rocm_get_ptr_info(void *ptr, size_t size, void **base_ptr,
                                        size_t *base_size,
                                        hsa_amd_pointer_type_t *hsa_mem_type,
                                        hsa_agent_t *agent,
                                        hsa_device_type_t *dev_type);


// load Rocr Library

#if HAVE_ROCM_RUNTIME
static void *hsaLib;
extern bool ncclCudaLaunchBlocking;
static uint16_t version_major, version_minor;
#define DECLARE_ROCM_PFN(symbol) PFN_##symbol pfn_##symbol = NULL
static status_t initResult;
static pthread_once_t initOnceControl = PTHREAD_ONCE_INIT;

typedef hsa_status_t (*PFN_hsa_init)();
typedef hsa_status_t (*PFN_hsa_system_get_info)(hsa_system_info_t attribute, void* value);
typedef hsa_status_t (*PFN_hsa_status_string)(hsa_status_t status, const char ** status_string);
typedef hsa_status_t (*PFN_hsa_amd_portable_export_dmabuf)(const void* ptr, size_t size, int* dmabuf, uint64_t* offset);

extern PFN_hsa_init pfn_hsa_init;
extern PFN_hsa_system_get_info pfn_hsa_system_get_info;
extern PFN_hsa_status_string pfn_hsa_status_string;
extern PFN_hsa_amd_portable_export_dmabuf pfn_hsa_amd_portable_export_dmabuf;
#endif /* HAVE_ROCM_RUNTIME */

#endif /* ROCM_H */

