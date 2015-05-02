#include "filter.h"

filter_t *new_filter(int rows, int cols, int **kernel) {

	filter_t *filter;
	int i, j, sum;

	filter = malloc(sizeof(*filter));
	
	if(!filter) {
		perror("filter");
		return NULL;
	}
	
	filter->kernel = malloc(cols*sizeof(*filter->kernel));
		
	if(!filter->kernel) {
		perror("filter->kernel");
		free(filter);
		return NULL;
	}
	
	for(i=0; i<rows; i++){
		filter->kernel[i] = malloc(rows*sizeof(**filter->kernel));
	}
	
	filter->rows = rows;
	filter->cols = cols;

	filter->kernel = kernel;
	

	sum = 0;	
	for(i=0; i<rows; i++) {
		for(j=0; j<cols; j++) {
			sum += filter->kernel[i][j];
		}
	}
	
	filter->sum = (sum == 0)?1:sum;

	return filter;
}

void delete_filter(filter_t *filter) {
	int i;
	for(i=0; i<filter->rows;i++) {
		free(filter->kernel[i]);
	}
	free(filter->kernel);
	free(filter);
}

void apply_filter(image_t *in, filter_t *f, image_t *out, int ntimes) {
	int num_threads, rows_per_thread, planes;
	
	/* by default, but not necessary, thread will be as many as cores of cpu */
	num_threads = omp_get_num_procs();
	omp_set_num_threads(num_threads);
	
	/* each thread will filter a number of rows*/
	rows_per_thread = out->height/num_threads;
	
	switch(in->type) {
	case GS :
		planes = 1;
		break;
	case RGB :
		planes = 3;
		break;
	}
	
	int time;
	
	for(time = 0; time < ntimes; time++) {
		#pragma omp parallel
		{		
			int i,j, p, q, n, m, sum, plane, id;
			
			id = omp_get_thread_num();
			
			//for each row
			for(i=id*rows_per_thread; i<(id+1)*rows_per_thread; i++) {
				//for each pixel
				for(j=0; j<out->width*planes; j=j+planes) {
					//for each plane
					for(plane=0;plane<planes;plane++) {
						sum = 0;
						//compute sum
						for(p=0; p<f->rows; p++) {
							n = i - f->rows/2 + p;
						
							if(n < 0 ) {
								n = 0;
							} else if( n >= in->height ) {
								n=in->height-1;
							}

							for(q=0; q<f->cols; q++) {
								m = j- (f->cols/2 - q)*planes + plane;
						
								if(m < 0 ) {
									m = 0;
								} else if ( m >= in->width*planes ) {
									m = in->width*planes-1;
								}
								sum += ((int)in->data[n][m])*f->kernel[p][q];
							}
						}
						
						sum /= f->sum;
						
						if(sum < 0) {
							sum = 0;
						} else if(sum > 255) {
							sum = 255;
						}
						
						out->data[i][j+plane] = ((unsigned char)sum);
					}
				}
			}
		}
		
		int i;
		//copy out data to in for next filtering
		for(i=0; i<in->height; i++){
			if(memcpy(in->data[i], out->data[i], sizeof(**out->data)*out->width*planes) != in->data[i]) {
				//error
			}
		}
	}
}			

void print_filter(filter_t *filter) {
	int i, j;
	
	printf("rows : %d\ncols : %d\nsum %d\n", filter->rows, filter->cols, filter->sum);

	for(i = 0; i < filter->rows; i++) {
		for(j = 0; j < filter->cols; j++) {
			printf("%d\t", filter->kernel[i][j]);
		}
		puts("");
	}
		
}

