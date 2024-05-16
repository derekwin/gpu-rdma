#include <hip/hip_runtime.h>
#include <stdio.h>

int test() {
	void *d_buf;
	hipMalloc(&d_buf, 2);
	printf("Hello, world!\n");
	hipFree(d_buf);
	return 1;
}