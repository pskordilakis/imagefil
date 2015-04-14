#ifndef FILTER_H
#define FILTER_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <omp.h>

#include "image.h"

typedef struct filter {
	int rows;
	int cols;
	int sum;
	int** kernel;
} filter_t;

filter_t *new_filter(int rows, int cols, int **kernel);
void delete_filter(filter_t *filter);
void apply_filter(image_t *in, filter_t *filter, image_t *out, int ntimes);
void print_filter(filter_t *filter);				

#endif
