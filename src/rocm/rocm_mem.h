#include <hsa.h>
#include <hip/hip_runtime.h>
// hipError_t hipSuccess
#include <hsa_ext_amd.h>
#include "type.h"

#define MAX_AGENTS 127
static struct agents {
    int num;
    hsa_agent_t agents[MAX_AGENTS];
    int num_gpu;
    hsa_agent_t gpu_agents[MAX_AGENTS];
} rocm_agents;

int is_dmabuf_supported();
status_t rocm_init(void);
static status_t uct_rocm_hsa_agent_callback(hsa_agent_t agent, void* data);
static status_t rocm_get_sys_dev(hsa_agent_t agent, sys_device_t *sys_dev_p);


static void uct_rocm_base_dmabuf_export(const void *addr, const size_t length,
                                        memory_type_t mem_type,
                                        int *dmabuf_fd, size_t *dmabuf_offset);
// status_t uct_rocm_base_mem_query(uct_md_h md, const void *addr,
//                                      const size_t length,
//                                      uct_md_mem_attr_t *mem_attr_p)