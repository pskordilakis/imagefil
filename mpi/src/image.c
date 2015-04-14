#include "image.h"

image_t* new_image(const char* filename, enum image_type type, int width, int height) {
	image_t *image;
	int i;

	image = malloc(sizeof(*image));
	
	if(!image) {
		perror("malloc");
		return NULL;
	}
	
	image->filename = malloc(strlen(filename) + 1);
	
	if(strcpy(image->filename, filename) != image->filename) {
		delete_image(image);
		perror("strcpy");
		return NULL;
	}
	
	image->type = type;
	image->width = width;
	image->height = height;
	image->data = NULL;

	return image;
}

image_t* create_empty_buffer(image_t* image) {
	int i, planes;
	unsigned char *memblock;
	
	switch(image->type){
	case GS :
		planes = 1;
		break;
	case RGB :
		planes = 3;
		break;
	}

	memblock = malloc(sizeof(*memblock) * image->height * image->width * planes);

	if(!memblock) {
		delete_image(image);
		perror("malloc");
		return NULL;
	}

	image->data = malloc(sizeof(*image->data) * image->height);

	//get rows of memblock
	for(i=0; i<image->height; i++) {
		image->data[i] = memblock + i * image->width * planes;
	}

	return image;
}

void delete_image(image_t *image) {

    if(image && image->filename) {
        free(image->filename);
	}
	
	if(image && image->data) {
		if(image->data[0])	
    		free(image->data[0]);
    		
    	free(image->data);
    }   
	
    if(image) {
	    free(image);
	}
}

int load_image(image_t *image) {
	FILE *file;
	int i, planes;
	unsigned char *memblock;
	
	/* open file */
	file = fopen(image->filename, "rb");	

	if(!file) {
		perror(image->filename);
		return 0;
	}

	switch(image->type){
    case GS :
	    planes = 1;
	    break;
	case RGB :
		planes = 3;
		break;
	}
	
	memblock = malloc(sizeof(*memblock) * image->height * image->width * planes);
	
	if(!memblock) {
		delete_image(image);
		perror("malloc");
		return NULL;
	}
	
	if(fread(memblock, sizeof(*memblock), image->height * image->width * planes, file) != image->height * image->width * planes) {
		perror("fread");
		return 0;
	}

	image->data = malloc(sizeof(*image->data) * image->height);
	
	if(!image->data) {
		//error
	}

	/*get rows of memblock*/
	for(i=0; i<image->height; i++) {
		image->data[i] = memblock + i * image->width * planes;
	}
	
	fclose(file);
	
	return 1;
}

int save_image(image_t *image) {
	FILE *file;
	int i, planes;

	file = fopen(image->filename, "wb");
	
	if(!file){
		perror("failed to open output image");
		return 0;
	}
	
	switch(image->type){
    case GS :
	    planes = 1;
	    break;
	case RGB :
		planes = 3;
		break;
	}
	
	if(fwrite(image->data[0], sizeof(**image->data), image->height * image->width * planes, file) != image->height * image->width * planes) {
		fclose(file);
		perror("fwrite");
    	 return 0;
	}
	
	fclose(file);

	return 1;
}

image_block_t* new_image_block(int width, int height) {
	int i;
	unsigned char *memblock;

	image_block_t* block = malloc(sizeof(*block));
	
	block->width = width;
	block->height = height;
	
	memblock = malloc(sizeof(*memblock)*height*width);
	
	block->data = malloc(sizeof(*block->data) * height);
	
	if(!block->data) {
		//error
	}
	
	for(i=0; i<height; i++) {
		block->data[i] = memblock + i * width;
	}
	
	return block;
}

void delete_image_block(image_block_t* block) {
	if(block->data[0])
			free(block->data[0]);
	free(block->data);
	free(block);
}

