#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "image.h"
#include "filter.h"

int main(int argc, char** argv) {
	
	char *filein, *fileout;
	enum image_type type;
	int width, height, rows, cols, ntimes,counter;
	int** kernel;
	image_t *in, *out;
	filter_t *filter;
	
	/* read args */
	for(counter=1;counter<argc;counter++) {
		if(!strcmp(argv[counter], "-i")) {
			//input image path
			filein = malloc(strlen(argv[++counter]) + 1);
			
			if(!filein) {
				perror(argv[counter]);
				return -1;
			}
			
			if(strcpy(filein, argv[counter]) != filein) {
				perror(argv[counter]);
				return -1;
			}
		} else if(!strcmp(argv[counter], "-o")) {
			//output image path
			fileout = malloc(strlen(argv[++counter]) + 1);
			
			if(!fileout) {
				perror(argv[counter]);
				return -1;
			}
			
			if(strcpy(fileout, argv[counter]) != fileout) {
				perror(argv[counter]);
				return -1;
			}
		} else if(!strcmp(argv[counter], "-t")) {
			//images type
			if(!strcmp(argv[++counter], "GS")) {
				type = GS;
			} else if(!strcmp(argv[counter], "RGB")) {
				type = RGB;
			}
		
		} else if(!strcmp(argv[counter], "-w")) {
			//images width
			width = atoi(argv[++counter]);
		} else if(!strcmp(argv[counter], "-h")) {
			//images height
			height = atoi(argv[++counter]);
		} else if(!strcmp(argv[counter], "-f")) {
			int *kernel_buf;
		
			//filter
			rows = atoi(argv[++counter]);
			
			cols = atoi(argv[++counter]);
			
			kernel_buf = malloc(rows*cols*sizeof(*kernel_buf));
			
			int i;
			
			for(i=0; i<rows*cols; i++) {
				kernel_buf[i] = atoi(argv[++counter]);
			}
			
			kernel = malloc(rows*sizeof(*kernel));
			
			for(i=0; i<rows; i++) {
				kernel[i] = &kernel_buf[i*cols];
			}			
			 
		} else if(!strcmp(argv[counter],"-n")) {
			ntimes = atoi(argv[++counter]);
		} else {
			printf("%d\n", counter);
			printf("%s usage : \n%s -i image_path -o out_image_path -t image_type -w image_width -h image_width -f kernel_rows kernel_cols kernel -n times_to_filter\n",argv[0] , argv[0]);
			return -1;
		}
	}
	
	/* input image */
	in = new_image(filein, type, width, height);
	if(!in){
		puts("failed to create input image");
		return -1;
	}
	
	if(!load_image(in)) {
		delete_image(in);
		perror("failed to load input image");
		return -1;
	}
	/* output image */
	out = new_image(fileout, type, width, height);
	
	if(!out) {
		delete_image(in);
		perror("failed to create output image");
		return -1;
	}
	
	
	filter = new_filter(rows, cols, kernel);
	
	if(!filter) {
		perror("failed to create filter");
		return -1;
	}

	switch(in->type) {
	case GS :
		apply_filter_gs(in, filter, out, ntimes);
		break;
	case RGB : 
		apply_filter_rgb(in, filter, out, ntimes);
		break;
	}
	
	if(!save_image(out)) {
		perror("failed to save output image");
		delete_image(in);
		delete_image(out);
		delete_filter(filter);
		return -1;
	}
	
	delete_image(in);
	delete_image(out);
	delete_filter(filter);
	
	if(filein)
		free(filein);
	if(fileout)
		free(fileout);

	return 0;
}
