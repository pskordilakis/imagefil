#ifndef IMAGE_H
#define IMAGE_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

enum image_type {
    GS, //GRAYSCALE - 1 byte per pixel
    RGB //RGB - 3 byte per pixel
};

typedef struct image {
	char* filename;
    enum image_type type;
	int width;
	int height;
	unsigned char **data;
} image_t;

image_t* new_image(const char* filename, enum image_type type, int width, int heigth );
void delete_image(image_t *image);
int load_image(image_t *image);
int save_image(image_t *image);

#endif
