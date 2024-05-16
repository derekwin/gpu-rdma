#include <dlfcn.h>
#include <stdio.h>
#include <hip/hip_runtime.h>

typedef int (*PFN_test)();
static void *hsaLib;
extern PFN_test pfn_test;

PFN_test pfn_test;

int main() {
    char path[1024];
    snprintf(path, 1024, "%s", "./test.o");

    hsaLib = dlopen(path, RTLD_LAZY);
    if (hsaLib == NULL) {
        printf("Failed to find runtime library");
        goto error;
    }

    pfn_test = (PFN_test) dlsym(hsaLib, "test");
    if (pfn_test == NULL) {
        printf("Failed to load ROCr missing symbol hsa_init");
        goto error;
    }

    pfn_test();
    
    return 1;
error:
    return 0;
}