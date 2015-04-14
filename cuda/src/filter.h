#ifndef FILTER_H
#define FILTER_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "image.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct filter {
	int rows;
	int cols;
	int sum;
	int** kernel;
} filter_t;

filter_t *new_filter(int rows, int cols, int **kernel);
void delete_filter(filter_t *filter);
void apply_filter_gs(image_t *in, filter_t *filter, image_t *out, int ntimes);
void apply_filter_rgb(image_t *in, filter_t *filter, image_t *out, int ntimes);
void print_filter(filter_t *filter);				
#ifdef __cplusplus
}
#endif

#endif
