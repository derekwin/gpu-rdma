#include "cnrt.h"
#include "mlu_op.h"
#include "cn_api.h" // CNresult
#include <stdio.h>
#include <string.h>

int main() {
	CNaddr addr;
	CNresult ret;

	ret = cnInit(0);
    if (ret != CN_SUCCESS) {
        printf("failed to cnInit %d", ret);
        return -1;
    }

	CNcontext context;
	ret = cnCtxCreate(&context, 0, 0);
	if (ret != CN_SUCCESS) {
        printf("failed to create cnCtx %d.", ret);
        return -1;
    }

	ret = cnMalloc(&addr, 2);
    if (ret == CN_ERROR_NOT_INITIALIZED) {
        printf("CNDrv has not been initialized with cnInit or CNDrv fails to be initialized.");
        return -1;
    }
    if (ret == CN_ERROR_INVALID_VALUE) {
        printf("The parameters passed to this API are not within an acceptable value range.");
        return -1;
    }
    if (ret != CN_SUCCESS) {
        printf("failed to allocate memory %d.", ret);
        return -1;
    }
    char *test = "sssss";
	ret = cnMalloc(&addr, strlen(test));
	printf("Hello!\n");

    void *src_host = NULL;
    src_host = calloc(strlen(test) , 1);
    strncpy(src_host, test, strlen(test)); 
    cnMemcpy((CNaddr)addr, (CNaddr)src_host, strlen(test));

    printf("world! %s, %s\n", (char *)src_host, addr);
	ret = cnFree(addr);  // free的为什么是对象，不是传入指针
	if (ret != CN_SUCCESS) {
        printf("failed to free memory");
        return -1;
    }

	return 0;
}