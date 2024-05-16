#ifndef TYPE_H
#define TYPE_H

#include <stdio.h>

// status
typedef enum {
    STATUS_ERROR,
    STATUS_SUCCESS,
    STATUS_UNSUPPORTED
} status_t;

// memory
typedef enum memory_type {
    MEMORY_TYPE_HOST,          /**< Default system memory */
    MEMORY_TYPE_GPU,          /**< NVIDIA CUDA memory */
    MEMORY_TYPE_GPU_MANAGED,  /**< NVIDIA CUDA managed (or unified) memory */
    UCS_MEMORY_TYPE_RDMA,          /**< RDMA device memory */
    UCS_MEMORY_TYPE_UNKNOWN
} memory_type_t;

typedef uint8_t sys_device_t;

typedef struct mem_attr {
    memory_type_t     mem_type;
    sys_device_t      sys_dev; //Index of the system device on which the buffer resides. eg: NUMA/GPU
    void              *base_address;
    size_t            alloc_length;
    int               dmabuf_fd;
    size_t            dmabuf_offset;
} mem_attr_t;

#define UCT_DMABUF_FD_INVALID      -1

// bus
typedef struct sys_bus_id {
    uint16_t domain;   /* range: 0 to ffff */
    uint8_t  bus;      /* range: 0 to ff */
    uint8_t  slot;     /* range: 0 to 1f */
    uint8_t  function; /* range: 0 to 7 */
} sys_bus_id_t;

// log
typedef enum {
    LOG_LEVEL_ERROR,        /* Error is returned to the user */
    LOG_LEVEL_WARN,         /* Something's wrong, but we continue */
    LOG_LEVEL_INFO,         /* Information */
    LOG_LEVEL_DEBUG,        /* Low-volume debugging */
} log_level_t;


static const char *ucm_log_level_names[] = {
    [LOG_LEVEL_ERROR] = "ERROR",
    [LOG_LEVEL_WARN]  = "WARN",
    [LOG_LEVEL_INFO]  = "INFO",
    [LOG_LEVEL_DEBUG] = "DEBUG"
};

#define __log(_level, _fmt, ...) \
    do { \
        printf("%s: " _fmt "\n", ucm_log_level_names[_level], ## __VA_ARGS__); \
    } while (0)

#define log_error(_fmt, ...)        __log(LOG_LEVEL_ERROR, _fmt, ## __VA_ARGS__)
#define log_warn(_fmt, ...)         __log(LOG_LEVEL_WARN, _fmt,  ## __VA_ARGS__)
#define log_info(_fmt, ...)         __log(LOG_LEVEL_INFO, _fmt, ## __VA_ARGS__)
#define log_debug(_fmt, ...)        __log(LOG_LEVEL_DEBUG, _fmt, ##  __VA_ARGS__)

#endif /* TYPE_H */