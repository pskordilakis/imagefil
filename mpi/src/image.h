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

typedef struct image_block {
	int width;
	int height;
	unsigned char **data;
} image_block_t;

image_t* new_image(const char* filename, enum image_type type, int width, int heigth );
image_t* create_empty_buffer(image_t* image); //returns pointer to image
void delete_image(image_t *image);
int load_image(image_t *image);
int save_image(image_t *image);

image_block_t* new_image_block(int width, int height);
void delete_image_block(image_block_t* block);

#endif
