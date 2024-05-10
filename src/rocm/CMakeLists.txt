set(proj_name rocm_gpu)

cmake_minimum_required(VERSION 3.21 FATAL_ERROR)
project(${proj_name})

# GPU RUNTIME CHANGE
set(GPU_RUNTIME "HIP" CACHE STRING "Switches between HIP and CUDA")
set(GPU_RUNTIMES "HIP" "CUDA")
set_property(CACHE GPU_RUNTIME PROPERTY STRINGS ${GPU_RUNTIMES})

if(NOT "${GPU_RUNTIME}" IN_LIST GPU_RUNTIMES)
    set(ERROR_MESSAGE "GPU_RUNTIME is set to \"${GPU_RUNTIME}\".\nGPU_RUNTIME must be either HIP or CUDA.")
    message(FATAL_ERROR ${ERROR_MESSAGE})
endif()

enable_language(${GPU_RUNTIME})
set(CMAKE_${GPU_RUNTIME}_STANDARD 11)
set(CMAKE_${GPU_RUNTIME}_EXTENSIONS OFF)
set(CMAKE_${GPU_RUNTIME}_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Release) # Debug

set(CMAKE_CXX_COMPILER g++) # or clang++
#set(CMAKE_CXX_COMPILER clang) # or clang++
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Ofast -ffast-math -funroll-loops -march=native") # get crazy here

if(WIN32)
    set(ROCM_ROOT "$ENV{HIP_PATH}" CACHE PATH "Root directory of the ROCm installation")
else()
    set(ROCM_ROOT "/opt/dtk" CACHE PATH "Root directory of the ROCm installation")
endif()

list(APPEND CMAKE_PREFIX_PATH "${ROCM_ROOT}" "${ROCM_ROOT}/hip")

set(include_dirs "")
list(APPEND include_dirs "${ROCM_ROOT}/include")
# For examples targeting NVIDIA, include the HIP header directory.
if(GPU_RUNTIME STREQUAL "CUDA")
    list(APPEND include_dirs "${ROCM_ROOT}/include")
endif()

find_library(IBVERBS_LIBRARY ibverbs)
find_library(RDMACM_LIBRARY rdmacm)
# link_libraries(pthread ${IBVERBS_LIBRARY} ${RDMACM_LIBRARY})
find_package(hip)

add_executable(rocm_rdma_server ${PROJECT_SOURCE_DIR}/rdma_common.c ${PROJECT_SOURCE_DIR}/rdma_server.c)
add_executable(rocm_rdma_client ${PROJECT_SOURCE_DIR}/rdma_common.c ${PROJECT_SOURCE_DIR}/rdma_client.c)

target_include_directories(rocm_rdma_server PRIVATE ${include_dirs} ${PROJECT_SOURCE_DIR})
target_include_directories(rocm_rdma_client PRIVATE ${include_dirs} ${PROJECT_SOURCE_DIR})

message(${PROJECT_SOURCE_DIR})
message(${include_dirs})

target_link_libraries(rocm_rdma_server hip::host pthread ${IBVERBS_LIBRARY} ${RDMACM_LIBRARY})
target_link_libraries(rocm_rdma_client hip::host pthread ${IBVERBS_LIBRARY} ${RDMACM_LIBRARY})

set_source_files_properties(${PROJECT_SOURCE_DIR}/rdma_server.c PROPERTIES LANGUAGE ${GPU_RUNTIME})
set_source_files_properties(${PROJECT_SOURCE_DIR}/rdma_common.c PROPERTIES LANGUAGE ${GPU_RUNTIME})
set_source_files_properties(${PROJECT_SOURCE_DIR}/rdma_client.c PROPERTIES LANGUAGE ${GPU_RUNTIME})
