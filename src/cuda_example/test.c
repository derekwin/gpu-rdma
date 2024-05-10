#include <cuda_runtime.h>
#include <stdio.h>

void main() {
	void *d_buf;
	cudaMalloc(&d_buf, 2);
	printf("Hello, world!\n");
	cudaFree(d_buf);
}