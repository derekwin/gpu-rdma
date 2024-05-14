#include "rocm_mem.h"
#include <pthread.h>

// rocm

// // Detect DMA-BUF support
// int is_dmabuf_supported()
// {
//     int dmabuf_supported = 0;

// #if HAVE_HSA_AMD_PORTABLE_EXPORT_DMABUF
//     const char kernel_opt1[] = "CONFIG_DMABUF_MOVE_NOTIFY=y";
//     const char kernel_opt2[] = "CONFIG_PCI_P2PDMA=y";
//     int found_opt1           = 0;
//     int found_opt2           = 0;
//     FILE *fp;
//     struct utsname utsname;
//     char kernel_conf_file[128];
//     char buf[256];

//     if (uname(&utsname) == -1) {
//         ucs_trace("could not get kernel name");
//         goto out;
//     }

//     ucs_snprintf_safe(kernel_conf_file, sizeof(kernel_conf_file),
//                       "/boot/config-%s", utsname.release);
//     fp = fopen(kernel_conf_file, "r");
//     if (fp == NULL) {
//         ucs_trace("could not open kernel conf file %s error: %m",
//                   kernel_conf_file);
//         goto out;
//     }

//     while (fgets(buf, sizeof(buf), fp) != NULL) {
//         if (strstr(buf, kernel_opt1) != NULL) {
//             found_opt1 = 1;
//         }
//         if (strstr(buf, kernel_opt2) != NULL) {
//             found_opt2 = 1;
//         }
//         if (found_opt1 && found_opt2) {
//             dmabuf_supported = 1;
//             break;
//         }
//     }
//     fclose(fp);
// #endif
// out:
//     return dmabuf_supported;
// }

status_t rocm_init(void)
{
    static pthread_mutex_t rocm_init_mutex = PTHREAD_MUTEX_INITIALIZER;
    static volatile int rocm_initialized = 0;
    status_t status;

    if (pthread_mutex_lock(&rocm_init_mutex) == 0) {
        if (rocm_initialized) {
            status =  STATUS_SUCCESS;
            goto end;
        }
    } else  {
        ucs_error("Could not take mutex");
        status = STATUS_ERROR;
        return status;
    }

    memset(&rocm_agents, 0, sizeof(rocm_agents));

    status = hsa_init();
    if (status != STATUS_SUCCESS) {
        ucs_debug("Failure to open HSA connection: 0x%x", status);
        goto end;
    }

    status = hsa_iterate_agents(uct_rocm_hsa_agent_callback, NULL);
    if (status != STATUS_SUCCESS) {
        ucs_debug("Failure to iterate HSA agents: 0x%x", status);
        goto end;
    }

    rocm_initialized = 1;

end:
    pthread_mutex_unlock(&rocm_init_mutex);
    return status;
}

static status_t uct_rocm_hsa_agent_callback(hsa_agent_t agent, void* data)
{
    const unsigned sys_device_priority = 10;
    hsa_device_type_t device_type;
    sys_device_t sys_dev;
    char device_name[10];
    status_t status;

    ucs_assert(rocm_agents.num < MAX_AGENTS);
    if (rocm_agents.num >= MAX_AGENTS) {
        return STATUS_ERROR;
    }

    hsa_agent_get_info(agent, HSA_AGENT_INFO_DEVICE, &device_type);
    if (device_type == HSA_DEVICE_TYPE_CPU) {
        log_info("found cpu agent %lu", agent.handle);
    }
    else if (device_type == HSA_DEVICE_TYPE_GPU) {
        rocm_agents.gpu_agents[rocm_agents.num_gpu] = agent;

        status = rocm_get_sys_dev(agent, &sys_dev);
        if (status == STATUS_SUCCESS) {
            log_info("found gpu agent %lu, name %s", rocm_agents.num_gpu, device_name);
            ucs_topo_sys_device_set_name(sys_dev, device_name,
                                         sys_device_priority);
        }
        log_info("found gpu agent %lu", agent.handle);
        rocm_agents.num_gpu++;
    }
    else {
        log_info("found unknown agent %lu", agent.handle);
    }

    rocm_agents.agents[rocm_agents.num] = agent;
    rocm_agents.num++;

    return STATUS_SUCCESS;
}

static status_t rocm_get_sys_dev(hsa_agent_t agent, sys_device_t *sys_dev_p)
{
    hsa_status_t status;
    sys_bus_id_t bus_id;
    uint32_t bdfid;
    uint32_t domainid;

    status = hsa_agent_get_info(agent, (hsa_agent_info_t)HSA_AMD_AGENT_INFO_BDFID,
                                &bdfid);
    if (status != STATUS_SUCCESS) {
        return STATUS_UNSUPPORTED;
    }
    bus_id.bus  = (bdfid & (0xFF << 8)) >> 8;
    bus_id.slot = (bdfid & (0x1F << 3)) >> 3;

    status = hsa_agent_get_info(agent, (hsa_agent_info_t)HSA_AMD_AGENT_INFO_DOMAIN,
                                &domainid);
    if (status != STATUS_SUCCESS) {
        return STATUS_UNSUPPORTED;
    }
    bus_id.domain = domainid;

    /* function is always set to 0 */
    bus_id.function = 0;

    // return ucs_topo_find_device_by_bus_id(&bus_id, sys_dev_p);  // TODO: pass find_device_by_bus_id
    return STATUS_SUCCESS;
}


// ucs_status_t ucs_topo_find_device_by_bus_id(const ucs_sys_bus_id_t *bus_id,
//                                             ucs_sys_device_t *sys_dev)
// {
//     ucs_bus_id_bit_rep_t bus_id_bit_rep;
//     ucs_kh_put_t kh_put_status;
//     khiter_t hash_it;
//     char *name;

//     bus_id_bit_rep  = ucs_topo_get_bus_id_bit_repr(bus_id);

//     ucs_spin_lock(&ucs_topo_global_ctx.lock);
//     hash_it = kh_put(
//             bus_to_sys_dev /*name*/,
//             &ucs_topo_global_ctx.bus_to_sys_dev_hash /*pointer to hashmap*/,
//             bus_id_bit_rep /*key*/, &kh_put_status);

//     if (kh_put_status == UCS_KH_PUT_KEY_PRESENT) {
//         *sys_dev = kh_value(&ucs_topo_global_ctx.bus_to_sys_dev_hash, hash_it);
//     } else if ((kh_put_status == UCS_KH_PUT_BUCKET_EMPTY) ||
//                (kh_put_status == UCS_KH_PUT_BUCKET_CLEAR)) {
//         ucs_assert_always(ucs_topo_global_ctx.num_devices <
//                           UCS_TOPO_MAX_SYS_DEVICES);
//         *sys_dev = ucs_topo_global_ctx.num_devices;
//         ++ucs_topo_global_ctx.num_devices;

//         kh_value(&ucs_topo_global_ctx.bus_to_sys_dev_hash, hash_it) = *sys_dev;

//         /* Set default name to abbreviated BDF */
//         name = ucs_malloc(UCS_SYS_BDF_NAME_MAX, "sys_dev_bdf_name");
//         if (name != NULL) {
//             ucs_topo_bus_id_str(bus_id, 1, name, UCS_SYS_BDF_NAME_MAX);
//         }

//         ucs_topo_global_ctx.devices[*sys_dev].bus_id        = *bus_id;
//         ucs_topo_global_ctx.devices[*sys_dev].name          = name;
//         ucs_topo_global_ctx.devices[*sys_dev].name_priority = 0;
//         ucs_topo_global_ctx.devices[*sys_dev].numa_node     =
//                 ucs_topo_read_device_numa_node(bus_id);
//         ucs_debug("added sys_dev %d for bus id %s", *sys_dev, name);
//     }

//     ucs_spin_unlock(&ucs_topo_global_ctx.lock);
//     return STATUS_SUCCESS;
// }


// ###############################################################################
/*
查找分配注册等函数被调用的关键字：md->ops->
其中rocm_gpu_mem_query是其中rocm相关的一个ops中的mem_query的实现函数uct_rocm_base_mem_query，
对应需要该类显存时候，调用此函数分配显存
uct_rocm_copy_mem_reg_internal实现具体的显存注册到网卡attr的逻辑

https://github1s.com/openucx/ucx/blob/master/src/tools/perf/rocm/rocm_alloc.c 对应了
*/
// ###############################################################################

static inline status_t rocm_alloc(size_t length,
                                  memory_type_t mem_type,
                                  void **address_p)
{
    hipError_t ret;

    if ((mem_type != MEMORY_TYPE_GPU) && (mem_type != MEMORY_TYPE_GPU_MANAGED)) {
        return STATUS_UNSUPPORTED;
    }

    ret = ((mem_type == MEMORY_TYPE_GPU) ?
            hipMalloc(address_p, length) :
            hipMallocManaged(address_p, length, hipMemAttachGlobal));
    if (ret != hipSuccess) {
        ucs_error("failed to allocate memory");
        return STATUS_ERROR;
    }

    return STATUS_SUCCESS;
}

static void rocm_dmabuf_export(const void *addr, const size_t length,
                                        memory_type_t mem_type,
                                        int *dmabuf_fd, size_t *dmabuf_offset)
{
    int fd          = UCT_DMABUF_FD_INVALID;
    uint64_t offset = 0;
// #if HAVE_HSA_AMD_PORTABLE_EXPORT_DMABUF
    hsa_status_t status;

    if (mem_type == MEMORY_TYPE_GPU) {
        status = hsa_amd_portable_export_dmabuf(addr, length, &fd, &offset);
        if (status != HSA_STATUS_SUCCESS) {
            fd     = UCT_DMABUF_FD_INVALID;
            offset = 0;
            log_warn("failed to export dmabuf handle for addr %p / %zu", addr,
                     length);
        }

        log_info("dmabuf export addr %p %lu to dmabuf fd %d offset %zu\n",
                  addr, length, fd, offset);
    }
// #endif
    *dmabuf_fd     = fd;
    *dmabuf_offset = (size_t)offset;
}

// 是用来获取内存的dmabuf fd的
status_t rocm_gpu_mem_query(const void *addr,
                            const size_t length,
                            mem_attr_t *mem_attr_p)
{
    size_t dmabuf_offset       = 0;
    int is_exported            = 0;
    memory_type_t mem_type = MEMORY_TYPE_HOST;
    int dmabuf_fd;
    hsa_status_t status;
    hsa_device_type_t dev_type;
    hsa_amd_pointer_type_t hsa_mem_type;
    hsa_agent_t agent;
    sys_device_t sys_dev;
    status_t ucs_status;

    status = uct_rocm_base_get_ptr_info((void*)addr, length, NULL, NULL,
                                        &hsa_mem_type, &agent, &dev_type);
    if (status != HSA_STATUS_SUCCESS) {
        return status;
    }

    if ((hsa_mem_type == HSA_EXT_POINTER_TYPE_HSA) &&
        (dev_type == HSA_DEVICE_TYPE_GPU)) {
        mem_type = MEMORY_TYPE_GPU;
    }

    mem_attr_p->base_address = (void*) addr;
    mem_attr_p->alloc_length = length;
    if (mem_type == MEMORY_TYPE_GPU) {
        rocm_dmabuf_export(addr, length, mem_type, &dmabuf_fd,
                                &dmabuf_offset);
        mem_attr_p->dmabuf_fd = dmabuf_fd;
        mem_attr_p->dmabuf_offset = dmabuf_offset;
    }
    
    return STATUS_SUCCESS;
}


// cuda:https://github1s.com/openucx/ucx/blob/master/src/uct/cuda/cuda_copy/cuda_copy_md.c#L402-L528

