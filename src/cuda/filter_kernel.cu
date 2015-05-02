#include "filter.h"

#include <cuda.h>

texture<unsigned char, 2> dataIn;
texture<unsigned char, 2> dataOut;

__constant__ int kernel_3x3[9];
__constant__ int kernel_sum[1];
__constant__ int total_planes[1];
__constant__ int current_plane[1];

__global__ void filter_3x3_kernel(unsigned char *data, bool dstOut) {
	
	int x = threadIdx.x + blockIdx.x * blockDim.x;
	int y = threadIdx.y + blockIdx.y * blockDim.y;
	int offset = x + y * gridDim.x * blockDim.x;
	
	int ul, u, ur, l, c, r, dl, d, dr;
	if(dstOut) {
		ul = tex2D(dataIn, x - 1, y - 1) * (kernel_3x3[0]);
		u = tex2D(dataIn, x, y - 1) * (kernel_3x3[1]);
		ur = tex2D(dataIn, x + 1, y - 1) * (kernel_3x3[2]);
		l = tex2D(dataIn, x - 1, y) * (kernel_3x3[3]);
		c = tex2D(dataIn, x, y) * (kernel_3x3[4]);
		r = tex2D(dataIn, x + 1, y) * (kernel_3x3[5]);
		dl = tex2D(dataIn, x - 1, y + 1) * (kernel_3x3[6]);
		d = tex2D(dataIn, x, y + 1) * (kernel_3x3[7]);
		dr = tex2D(dataIn, x + 1, y + 1) * (kernel_3x3[8]);
	} else {
		ul = tex2D(dataOut, x - 1, y - 1) * (kernel_3x3[0]);
		u = tex2D(dataOut, x, y - 1) * (kernel_3x3[1]);
		ur = tex2D(dataOut, x + 1, y - 1) * (kernel_3x3[2]);
		l = tex2D(dataOut, x - 1, y) * (kernel_3x3[3]);
		c = tex2D(dataOut, x, y) * (kernel_3x3[4]);
		r = tex2D(dataOut, x + 1, y) * (kernel_3x3[5]);
		dl = tex2D(dataOut, x - 1, y + 1) * (kernel_3x3[6]);
		d = tex2D(dataOut, x, y + 1) * (kernel_3x3[7]);
		dr = tex2D(dataOut, x + 1, y + 1) * (kernel_3x3[8]);
	}
	
	data[offset] = (unsigned char)((ul + u + ur + l + c + r + dl + d + dr)/(kernel_sum[0]));
}

__global__ void deinterleave_planes(unsigned char *original, unsigned char *data) {
	
	//each thread split one pixel
	
	int x = threadIdx.x + blockIdx.x * blockDim.x;
	int y = threadIdx.y + blockIdx.y * blockDim.y;
	int out_offset = x + y * gridDim.x * blockDim.x;
	int in_offset = x * total_planes[0] +  current_plane[0] + y * gridDim.x * blockDim.x * total_planes[0];
	
	data[out_offset] = original[in_offset]; 
}

__global__ void interleave_planes(unsigned char *data, unsigned char *out) {
	
	//each thread interleave one pixel
	
	int x = threadIdx.x + blockIdx.x * blockDim.x;
	int y = threadIdx.y + blockIdx.y * blockDim.y;
	int in_offset = x + y * gridDim.x * blockDim.x;
	int out_offset = x * total_planes[0] +  current_plane[0] + y * gridDim.x * blockDim.x * total_planes[0];
	
	out[out_offset] = data[in_offset];
}


extern "C" void apply_filter_rgb(image_t *in, filter_t *f, image_t *out, int ntimes) {
	
	int plane, planes, time;
	bool dstOut;
	
	planes = 3; 
	
	dim3 f_numBlocks(in->width*planes/30, in->height/30);
	dim3 numBlocks(in->width/30, in->height/30);
	dim3 threadsPerBlock(30, 30);

	unsigned char *original_src, *data1, *data2, *final_out;
	
	if(cudaMalloc((void **) &original_src, in->height*in->width*planes) != cudaSuccess) {
		perror("Could not allocate original_src");
		return;
	}
	
	if(cudaMalloc((void **) &data1, in->height*in->width) != cudaSuccess) {
		perror("Could not allocate data1");
		cudaFree(original_src);
		return;
	}
	
	if(cudaMalloc((void **) &data2, in->height*in->width) != cudaSuccess) {
		perror("Could not allocate data2");
		cudaFree(original_src);
		cudaFree(data1);
		return;
	}
	
	if(cudaMalloc((void **) &final_out, in->height*in->width*planes) != cudaSuccess) {
		perror("Could not allocate final_out");
		cudaFree(original_src);
		cudaFree(data1);
		cudaFree(data2);
		return;
	}
	
	if(cudaMemcpy(original_src, in->data[0], in->height*in->width*planes*sizeof(unsigned char), cudaMemcpyHostToDevice) != cudaSuccess) {
		perror("Could copy image to device");
		cudaFree(original_src);
		cudaFree(data1);
		cudaFree(data2);
		cudaFree(final_out);
		return;
	}
	
	cudaChannelFormatDesc desc = cudaCreateChannelDesc<unsigned char>();
	
	if(cudaBindTexture2D( NULL, dataIn, data1, desc, in->width, in->height, in->width*sizeof(unsigned char)) != cudaSuccess) {
		perror("Could not bind texture dataIn");
		cudaFree(original_src);
		cudaFree(data1);
		cudaFree(data2);
		cudaFree(final_out);
		return;
	}

	if(cudaBindTexture2D( NULL, dataOut, data2, desc, in->width, in->height, in->width*sizeof(unsigned char)) != cudaSuccess) {
		perror("Could not bind texture dataOut");
		cudaFree(original_src);
		cudaFree(data1);
		cudaFree(data2);
		cudaFree(final_out);
		return;
	}
	
	if(cudaMemcpyToSymbol(kernel_3x3, f->kernel[0], f->rows*f->cols*sizeof(int), 0, cudaMemcpyHostToDevice ) != cudaSuccess) {
		perror("Could not copy kernel to constant");
		cudaFree(original_src);
		cudaFree(data1);
		cudaFree(data2);
		cudaFree(final_out);
		return;
	}
	
	if(cudaMemcpyToSymbol(kernel_sum, &(f->sum), sizeof(int), 0, cudaMemcpyHostToDevice ) != cudaSuccess) {
		perror("Could not copy kernel sum to constant");
		cudaFree(original_src);
		cudaFree(data1);
		cudaFree(data2);
		cudaFree(final_out);
		return;
	}
	
	if(cudaMemcpyToSymbol(total_planes, &(planes), sizeof(int), 0, cudaMemcpyHostToDevice ) != cudaSuccess) {
		perror("Could not copy image height to constant");
		cudaFree(original_src);
		cudaFree(data1);
		cudaFree(data2);
		cudaFree(final_out);
		return;
	}
	
	for(plane = 0; plane < planes; plane++) {
		
		dstOut = true;
		
		if(cudaMemcpyToSymbol(current_plane, &(plane), sizeof(int), 0, cudaMemcpyHostToDevice ) != cudaSuccess) {
			perror("Could not copy image height to constant");
			cudaFree(original_src);
			cudaFree(data1);
			cudaFree(data2);
			cudaFree(final_out);
			return;
		}
		
		//deinterleave data
		deinterleave_planes<<<numBlocks, threadsPerBlock>>>(original_src, data1);
		
		if(cudaSuccess != cudaGetLastError()) {
			perror("deinteleave kernel error");
			break;
		}
		
		//filter
		for(time=0; time < ntimes; time++) {
		
			if(dstOut) {
				filter_3x3_kernel<<<numBlocks, threadsPerBlock>>>(data2, dstOut);
			} else { 
				filter_3x3_kernel<<<numBlocks, threadsPerBlock>>>(data1, dstOut);
			}
			
			if(cudaSuccess != cudaGetLastError()) {
				perror("filter kernel error");
				break;
			}
			
			dstOut = !dstOut;
		}
		
		//interleave out
		if(dstOut) {
			interleave_planes<<<numBlocks, threadsPerBlock>>>(data1, final_out);
		} else {
			interleave_planes<<<numBlocks, threadsPerBlock>>>(data2, final_out);
		}
		
		
		
		if(cudaSuccess != cudaGetLastError()) {
			perror("inteleave kernel error");
			break;
		}
	}
	
	if(cudaMemcpy(out->data[0], final_out, out->height*out->width*planes*sizeof(unsigned char), cudaMemcpyDeviceToHost) != cudaSuccess) {
		perror("cudaMemcpy error");
		cudaFree(original_src);
		cudaFree(data1);
		cudaFree(data2);
		cudaFree(final_out);
		return;
	}
	
	cudaUnbindTexture( dataIn );
	cudaUnbindTexture( dataOut );
	
	cudaFree(original_src);
	cudaFree(data1);
	cudaFree(data2);
	cudaFree(final_out);
	
}	

extern "C" void apply_filter_gs(image_t *in, filter_t *f, image_t *out, int ntimes) {
	
	int time;
	bool dstOut = true; 
	
	dim3 numBlocks(in->width/30, in->height/30);
	dim3 threadsPerBlock(30, 30);
	
	unsigned char *data1, *data2;
	
	if(cudaMalloc((void **) &data1, in->height*in->width) != cudaSuccess) {
		perror("Could not allocate data1");
		return;
	}
	
	if(cudaMalloc((void **) &data2, in->height*in->width) != cudaSuccess) {
		perror("Could not allocate data2");
		cudaFree(data1);
		return;
	}
	
	cudaChannelFormatDesc desc = cudaCreateChannelDesc<unsigned char>();
	
	if(cudaBindTexture2D( NULL, dataIn, data1, desc, in->width, in->height, in->width*sizeof(unsigned char)) != cudaSuccess) {
		perror("Could not bind texture dataIn");
		cudaFree(data1);
		cudaFree(data2);
		return;
	}
	
	if(cudaMemcpy(data1, in->data[0], in->height*in->width*sizeof(unsigned char), cudaMemcpyHostToDevice) != cudaSuccess) {
		perror("Could copy image to device");
		cudaFree(data1);
		cudaFree(data2);
		return;
	}
	
	if(cudaBindTexture2D( NULL, dataOut, data2, desc, in->width, in->height, in->width*sizeof(unsigned char)) != cudaSuccess) {
		perror("Could not bind texture dataOut");
		cudaFree(data1);
		cudaFree(data2);
		return;
	}
	
	if(cudaMemcpyToSymbol(kernel_3x3, f->kernel[0], f->rows*f->cols*sizeof(**f->kernel), 0, cudaMemcpyHostToDevice ) != cudaSuccess) {
		perror("Could not copy kernel to constant");
		cudaFree(data1);
		cudaFree(data2);
		return;
	}
	
	int sum = f->sum;
	if(cudaMemcpyToSymbol(kernel_sum, &(sum), sizeof(int), 0, cudaMemcpyHostToDevice ) != cudaSuccess) {
		perror("Could not copy kernel sum to constant");
		cudaFree(data1);
		cudaFree(data2);
		return;
	}
	
	//filter
	for(time=0; time < ntimes; time++) {
	
		if(dstOut) {
			filter_3x3_kernel<<<numBlocks, threadsPerBlock>>>(data2, dstOut);
		} else { 
			filter_3x3_kernel<<<numBlocks, threadsPerBlock>>>(data1, dstOut);
		}
		
		if(cudaSuccess != cudaGetLastError()) {
			perror("filter kernel error");
			break;
		}
		
		dstOut = !dstOut;
	}
	
	if(dstOut) {
		if(cudaMemcpy(out->data[0], data1, out->height*out->width*sizeof(unsigned char), cudaMemcpyDeviceToHost) != cudaSuccess) {
			perror("cudaMemcpy error");
			cudaFree(data1);
			cudaFree(data2);
			return;
		}	
	} else {
		if(cudaMemcpy(out->data[0], data2, out->height*out->width*sizeof(unsigned char), cudaMemcpyDeviceToHost) != cudaSuccess) {
			perror("cudaMemcpy error");
			cudaFree(data1);
			cudaFree(data2);
			return;
		}
	}

	cudaUnbindTexture( dataIn );
	cudaUnbindTexture( dataOut );
	
	cudaFree(data1);
	cudaFree(data2);
}		

