#include "image.h"

image_t* new_image(const char* filename, enum image_type type, int width, int height) {
	image_t *image;
	int i, planes;
	unsigned char *memblock;

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

    switch(image->type){
    case GS :
	    planes = 1;
	    break;
	case RGB :
		planes = 3;
		break;
	}
	
	memblock = (unsigned char *)calloc(image->height*image->width*planes, sizeof(*memblock));
	
	if(!memblock) {
		delete_image(image);
		perror("malloc");
		return NULL;
	}
	
	image->data = malloc(sizeof(*image->data)*image->height);
	
	if(!image->data) {
		free(memblock);
		delete_image(image);
		perror("malloc");
		return NULL;
	}
	
	//get rows of memblock
	for(i=0; i<image->height; i++) {
		image->data[i] = memblock + i * image->width * planes;
	}

	return image;
}

void delete_image(image_t *image) {

    if(image && image->filename) 
        free(image->filename);
    if(image && image->data) {
		if(image->data[0])
	    	free(image->data[0]);
	    free(image->data);
    }    
    
    if(image)
	    free(image);
}

int load_image(image_t *image) {
	FILE *file;
	int i, planes;
	
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

	/* read file contents to data */

	if(fread(image->data[0], sizeof(**image->data), image->height*image->width*planes, file) != image->height*image->width*planes) {
		perror("fread");
		return 0;
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
	
	if(fwrite(image->data[0], sizeof(**image->data), image->height*image->width*planes, file) != image->height*image->width*planes) {
		fclose(file);
		perror("fwrite");
    	return 0;
	}
		
	fclose(file);

	return 1;
}

