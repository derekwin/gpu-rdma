#include <hip/hip_runtime.h>
#include <stdio.h>

void main() {
	void *d_buf;
	hipMalloc(&d_buf, 2);
	printf("Hello, world!\n");
	hipFree(d_buf);
}