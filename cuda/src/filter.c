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
	if(filter->kernel && filter->kernel[0])
		free(filter->kernel[0]);
	if(filter->kernel)
		free(filter->kernel);
	free(filter);
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

