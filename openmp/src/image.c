#include "image.h"

image_t* new_image(const char* filename, enum image_type type, int width, int height) {
	image_t *image;
	long image_size;
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

    switch(image->type){
    case GS :
	    image_size = image->width;
	    break;
	case RGB :
		image_size = image->width * 3;
		break;
	}
	
	image->data = malloc(sizeof(*image->data)*image->height);
	
	if(!image->data) {
		delete_image(image);
		perror("malloc");
		return NULL;
	}
	
	for(i=0; i < image->height; i++) {
		
		image->data[i] = malloc(sizeof(**image->data)*image_size);
		if(!image->data[i]) {
			perror("malloc data[i]");
			return NULL;
		}
	}

	return image;
}

void delete_image(image_t *image) {

    if(image && image->filename) 
        free(image->filename);
    if(image && image->data) {
    	int i;
    	for(i=0; i<image->height; i++) 
    		if(image->data[i])
	    		free(image->data[i]);
    	free(image->data);
    }    
    if(image)
	    free(image);
}

int load_image(image_t *image) {
	FILE *file;
	long image_size;
	int i;
	
	/* open file */
	file = fopen(image->filename, "rb");	

	if(!file) {
		perror(image->filename);
		return 0;
	}

	switch(image->type){
    case GS :
	    image_size = image->width;
	    break;
	case RGB :
		image_size = image->width * 3;
		break;
	}

	/* read file contents to data */
	for(i=0; i<image->height; i++) {
		if(fread(image->data[i], sizeof(**image->data), image_size, file) != image_size) {
			perror("fread");
			return 0;
		}
	}
	
	fclose(file);
	
	return 1;
}

int save_image(image_t *image) {
	FILE *file;
	long image_size;
	int i;

	file = fopen(image->filename, "wb");
	
	if(!file){
		perror("failed to open output image");
		return 0;
	}
	
	switch(image->type){
    case GS :
	    image_size = image->width;
	    break;
	case RGB :
		image_size = image->width * 3;
		break;
	}
	
	for(i=0; i<image->height; i++) {
		if(fwrite(image->data[i], sizeof(**image->data), image_size, file) != image_size) {
			fclose(file);
			perror("fwrite");
    	 	return 0;
		}
	}	
	
	fclose(file);

	return 1;
}

