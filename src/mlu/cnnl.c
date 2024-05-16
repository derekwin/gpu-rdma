#include "cnnl.h"

#if HAVE_ROCM_RUNTIME
PFN_hsa_init pfn_hsa_init;
PFN_hsa_system_get_info pfn_hsa_system_get_info;
PFN_hsa_status_string pfn_hsa_status_string;
PFN_hsa_amd_portable_export_dmabuf pfn_hsa_amd_portable_export_dmabuf;
#endif

// rocm

bool LaunchBlocking = false;

status_t prepare_gpu_driver()
{
    int ret = STATUS_SUCCESS;
#if HAVE_ROCM_RUNTIME
    ret = rocm_init();
#endif
    return ret;
}

// load func

#if HAVE_ROCM_RUNTIME
static void initOnceFunc() {
  do {
    char* val = getenv("CNCL_LAUNCH_BLOCKING");
    LaunchBlocking = val!=NULL && val[0]!=0 && !(val[0]=='0' && val[1]==0);
  } while (0);

  bool dmaBufSupport = false;
  hsa_status_t res;

  /*
   * Load ROCr driver library
   */
  char path[1024];
  char *CNCLPath = getenv("RCCL_ROCR_PATH");
  if (CNCLPath == NULL)
    snprintf(path, 1024, "%s", "libhsa-runtime64.so");
  else
    snprintf(path, 1024, "%s%s", CNCLPath, "libhsa-runtime64.so");

  hsaLib = dlopen(path, RTLD_LAZY);
  if (hsaLib == NULL) {
    log_warn("Failed to find ROCm runtime library in %s (RCCL_ROCR_PATH=%s)", CNCLPath, CNCLPath);
    goto error;
  }

  /*
   * Load initial ROCr functions
   */

  pfn_hsa_init = (PFN_hsa_init) dlsym(hsaLib, "hsa_init");
  if (pfn_hsa_init == NULL) {
    log_warn("Failed to load ROCr missing symbol hsa_init");
    goto error;
  }

  pfn_hsa_system_get_info = (PFN_hsa_system_get_info) dlsym(hsaLib, "hsa_system_get_info");
  if (pfn_hsa_system_get_info == NULL) {
    log_warn("Failed to load ROCr missing symbol hsa_system_get_info");
    goto error;
  }

  pfn_hsa_status_string = (PFN_hsa_status_string) dlsym(hsaLib, "hsa_status_string");
  if (pfn_hsa_status_string == NULL) {
    log_warn("Failed to load ROCr missing symbol hsa_status_string");
    goto error;
  }

  res = pfn_hsa_system_get_info(HSA_SYSTEM_INFO_VERSION_MAJOR, &version_major);
  if (res != 0) {
    log_warn("pfn_hsa_system_get_info failed with %d", res);
    goto error;
  }
  res = pfn_hsa_system_get_info(HSA_SYSTEM_INFO_VERSION_MINOR, &version_minor);
  if (res != 0) {
    log_warn("pfn_hsa_system_get_info failed with %d", res);
    goto error;
  }

  log_info("ROCr version %d.%d", version_major, version_minor);

  //if (hsaDriverVersion < ROCR_DRIVER_MIN_VERSION) {
    // log_warn("ROCr Driver version found is %d. Minimum requirement is %d", hsaDriverVersion, ROCR_DRIVER_MIN_VERSION);
    // Silently ignore version check mismatch for backwards compatibility
    //goto error;
  //}

  /* DMA-BUF support */
  //ROCm support
//   if (paramDmaBufEnable() == 0 ) {
//     log_info("Dmabuf feature disabled without DMABUF_ENABLE=1");
//     goto error;
//   }
  res = pfn_hsa_system_get_info((hsa_system_info_t) 0x204, &dmaBufSupport);
  if (res != HSA_STATUS_SUCCESS || !dmaBufSupport) {
    log_info("Current version of ROCm does not support dmabuf feature.");
    goto error;
  }
  else {
    pfn_hsa_amd_portable_export_dmabuf = (PFN_hsa_amd_portable_export_dmabuf) dlsym(hsaLib, "hsa_amd_portable_export_dmabuf");
    if (pfn_hsa_amd_portable_export_dmabuf == NULL) {
      log_warn("Failed to load ROCr missing symbol hsa_amd_portable_export_dmabuf");
      goto error;
    }
    else {
      //check OS kernel support
      struct utsname utsname;
      FILE *fp = NULL;
      char kernel_opt1[28] = "CONFIG_DMABUF_MOVE_NOTIFY=y";
      char kernel_opt2[20] = "CONFIG_PCI_P2PDMA=y";
      char kernel_conf_file[128];
      char buf[256];
      int found_opt1 = 0;
      int found_opt2 = 0;

      //check for kernel name exists
      if (uname(&utsname) == -1) log_info("Could not get kernel name");
      //format and store the kernel conf file location
      snprintf(kernel_conf_file, sizeof(kernel_conf_file), "/boot/config-%s", utsname.release);
      fp = fopen(kernel_conf_file, "r");
      if (fp == NULL) log_info("Could not open kernel conf file");
      //look for kernel_opt1 and kernel_opt2 in the conf file and check
      while (fgets(buf, sizeof(buf), fp) != NULL) {
        if (strstr(buf, kernel_opt1) != NULL) {
          found_opt1 = 1;
          log_info("CONFIG_DMABUF_MOVE_NOTIFY=y in /boot/config-%s", utsname.release);
        }
        if (strstr(buf, kernel_opt2) != NULL) {
          found_opt2 = 1;
          log_info("CONFIG_PCI_P2PDMA=y in /boot/config-%s", utsname.release);
        }
      }
      if (!found_opt1 || !found_opt2) {
        dmaBufSupport = 0;
        log_info( "CONFIG_DMABUF_MOVE_NOTIFY and CONFIG_PCI_P2PDMA should be set for DMA_BUF in /boot/config-%s", utsname.release);
        log_info( "DMA_BUF_SUPPORT Failed due to OS kernel support");
      }

      if(dmaBufSupport) log_info( "DMA_BUF Support Enabled");
      else goto error;
    }
  }

  /*
   * Required to initialize the ROCr Driver.
   * Multiple calls of hsa_init() will return immediately
   * without making any relevant change
   */
  pfn_hsa_init();

  initResult = STATUS_SUCCESS;


error:
  initResult = STATUS_UNSUPPORTED;
}

status_t rocmLibraryInit() {
  pthread_once(&initOnceControl, initOnceFunc);
  return initResult;
}

#endif

// #################################################################

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
//         log_debug("could not get kernel name");
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

// rocm driver init
status_t rocm_init(void)
{
    static pthread_mutex_t rocm_init_mutex = PTHREAD_MUTEX_INITIALIZER;
    static volatile int rocm_initialized = 0;
    hsa_status_t hsa_status;
    status_t status = STATUS_SUCCESS;

    if (pthread_mutex_lock(&rocm_init_mutex) == 0) {
        if (rocm_initialized) {
            goto end;
        }
    } else  {
        log_error("Could not take mutex");
        status = STATUS_ERROR;
        return status;
    }

    memset(&rocm_agents, 0, sizeof(rocm_agents));

    hsa_status = hsa_init();
    if (hsa_status != HSA_STATUS_SUCCESS) {
        status = STATUS_ERROR;
        log_debug("Failure to open HSA connection: 0x%x", status);
        goto end;
    }

    hsa_status = hsa_iterate_agents(rocm_hsa_agent_callback, NULL);
    if (hsa_status != HSA_STATUS_SUCCESS) {
        status = STATUS_ERROR;
        log_debug("Failure to iterate HSA agents: 0x%x", status);
        goto end;
    }

#if ROCM_DMABUF_SUPPERTED
    status = rocmLibraryInit();
    if (status != STATUS_SUCCESS) {
        log_debug("Failure to initialize ROCm library: 0x%x", status);
        goto end;
    }
#endif

    rocm_initialized = 1;

end:
    pthread_mutex_unlock(&rocm_init_mutex);
    return status;
}

static hsa_status_t rocm_hsa_agent_callback(hsa_agent_t agent, void* data)
{
    const unsigned sys_device_priority = 10;
    hsa_device_type_t device_type;
    sys_device_t sys_dev;
    char device_name[10];
    status_t status;

    if (rocm_agents.num >= MAX_AGENTS) {
        return HSA_STATUS_ERROR;
    }

    hsa_agent_get_info(agent, HSA_AGENT_INFO_DEVICE, &device_type);
    if (device_type == HSA_DEVICE_TYPE_CPU) {
        log_info("found cpu agent %lu", agent.handle);
    }
    else if (device_type == HSA_DEVICE_TYPE_GPU) {
        rocm_agents.gpu_agents[rocm_agents.num_gpu] = agent;

        status = rocm_get_sys_dev(agent, &sys_dev);
        if (status == STATUS_SUCCESS) {
            log_info("found gpu agent %d, name %s", rocm_agents.num_gpu, device_name);
            // ucs_topo_sys_device_set_name(sys_dev, device_name,
            //                              sys_device_priority);
        }
        log_info("found gpu agent %lu", agent.handle);
        rocm_agents.num_gpu++;
    }
    else {
        log_info("found unknown agent %lu", agent.handle);
    }

    rocm_agents.agents[rocm_agents.num] = agent;
    rocm_agents.num++;

    return HSA_STATUS_SUCCESS;
}

static status_t rocm_get_sys_dev(hsa_agent_t agent, sys_device_t *sys_dev_p)
{
    hsa_status_t status;
    sys_bus_id_t bus_id;
    uint32_t bdfid;
    uint32_t domainid;

    status = hsa_agent_get_info(agent, (hsa_agent_info_t)HSA_AMD_AGENT_INFO_BDFID,
                                &bdfid);
    if (status != HSA_STATUS_SUCCESS) {
        return STATUS_UNSUPPORTED;
    }
    bus_id.bus  = (bdfid & (0xFF << 8)) >> 8;
    bus_id.slot = (bdfid & (0x1F << 3)) >> 3;

    status = hsa_agent_get_info(agent, (hsa_agent_info_t)HSA_AMD_AGENT_INFO_DOMAIN,
                                &domainid);
    if (status != HSA_STATUS_SUCCESS) {
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

// rocm thread device init
static status_t rocm_thread_init()
{
    hipError_t ret;
    int num_gpus;
    int gpu_index;

    ret = hipGetDeviceCount(&num_gpus);
    if (ret != hipSuccess) {
        return STATUS_ERROR;
    }

    gpu_index = num_gpus-1; // 后续实现集合同学需要引入group_id，组的概念

    ret = hipSetDevice(gpu_index);
    if (ret != hipSuccess) {
        return STATUS_UNSUPPORTED;
    }

    return STATUS_SUCCESS;
}

// rocm mem alloc
status_t rocm_mem_alloc(size_t length,
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
        log_error("failed to allocate memory");
        return STATUS_ERROR;
    }

    return STATUS_SUCCESS;
}

status_t rocm_mem_free(void **address_p)
{
    hipError_t ret;
    ret = hipFree(address_p);
    if (ret != hipSuccess) {
        log_error("failed to free memory");
        return STATUS_ERROR;
    }

    return STATUS_SUCCESS;
}

// rocm dma buf export
static void rocm_dmabuf_export(const void *addr, const size_t length,
                                        memory_type_t mem_type,
                                        int *dmabuf_fd, size_t *dmabuf_offset)
{
    int fd          = UCT_DMABUF_FD_INVALID;
    uint64_t offset = 0;
#if HAVE_HSA_AMD_PORTABLE_EXPORT_DMABUF
    hsa_status_t status;
    
    if (mem_type == MEMORY_TYPE_GPU) {
        status = pfn_hsa_amd_portable_export_dmabuf(addr, length, &fd, &offset);
        if (status != HSA_STATUS_SUCCESS) {
            fd     = UCT_DMABUF_FD_INVALID;
            offset = 0;
            log_warn("failed to export dmabuf handle for addr %p / %zu", addr,
                     length);
        }

        log_info("dmabuf export addr %p %lu to dmabuf fd %d offset %zu\n",
                  addr, length, fd, offset);
    }
#endif
    *dmabuf_fd     = fd;
    *dmabuf_offset = (size_t)offset;
}

// 获取内存的dmabuf fd
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

    status = rocm_get_ptr_info((void*)addr, length, NULL, NULL,
                                        &hsa_mem_type, &agent, &dev_type);
    if (status != HSA_STATUS_SUCCESS) {
        return STATUS_UNSUPPORTED;
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

// uct_rocm_base_get_ptr_info
hsa_status_t rocm_get_ptr_info(void *ptr, size_t size, void **base_ptr,
                                        size_t *base_size,
                                        hsa_amd_pointer_type_t *hsa_mem_type,
                                        hsa_agent_t *agent,
                                        hsa_device_type_t *dev_type)
{
    hsa_status_t status;
    hsa_amd_pointer_info_t info;

    info.size = sizeof(hsa_amd_pointer_info_t);
    status = hsa_amd_pointer_info(ptr, &info, NULL, NULL, NULL);
    if (status != HSA_STATUS_SUCCESS) {
        log_error("get pointer info fail %p", ptr);
        return status;
    }

    if (hsa_mem_type != NULL) {
        *hsa_mem_type = info.type;
    }
    if (agent != NULL) {
        *agent = info.agentOwner;
    }
    if (base_ptr != NULL) {
        *base_ptr = info.agentBaseAddress;
    }
    if (base_size != NULL) {
        *base_size = info.sizeInBytes;
    }
    if (dev_type != NULL) {
        if (info.type == HSA_EXT_POINTER_TYPE_UNKNOWN) {
            *dev_type = HSA_DEVICE_TYPE_CPU;
        } else {
            status = hsa_agent_get_info(info.agentOwner, HSA_AGENT_INFO_DEVICE,
                                        dev_type);
        }
    }

    return status;
}