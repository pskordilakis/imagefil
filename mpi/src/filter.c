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
	
	//compute sum of kernel for normalization
	sum = 0;	
	for(i=0; i<rows; i++) {
		for(j=0; j<cols; j++) {
			sum += filter->kernel[i][j];
		}
	}
	
	/*
	 * if sum == 0 probably already normalized
	 * divide with 1
	 */
	filter->sum = (sum == 0)?1:sum;

	return filter;
}


void delete_filter(filter_t *filter) {
	/* free allocated resources */
	int i;
	for(i=0; i<filter->rows;i++) {
		free(filter->kernel[i]);
	}
	free(filter->kernel);
	free(filter);
}

void apply_filter(image_t *in, filter_t *f, image_t *out, int ntimes) {
	int i,j, p, q, n, m, sum, time, plane, planes, rank, size, grid_side;
	image_block_t *input, *result;
	MPI_Status status;
	MPI_Request requests[16];
	MPI_Datatype mpi_image_block;
	MPI_Datatype mpi_block_col;
	
	switch(in->type) {
	case GS :
		planes = 1;
		break;
	case RGB :
		planes = 3;
		break;
	}
	
	//get rank
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	//get number of processes
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	
	/*
	 * compute grid side
	 * grid(sqrt(N), sqrt(N))
	 * total N processes
	 */
	grid_side = sqrt(size);
	
	/*create image_block to store input */
	input = new_image_block((in->width/grid_side+2)*planes, in->height/grid_side+2);
	
	/*create image_block to store output*/
	result = new_image_block((in->width/grid_side+2)*planes, in->height/grid_side+2);

	if(rank == 0) {
		//master send initial
		
		/*
		 * register block mpi block type
		 * used to send block of image to other processes
		 */
		 
		int sizes[2] = {in->height, in->width*planes};
		int subsizes[2] = {in->height/grid_side, (in->width*planes)/grid_side};
		int starts[2] = {0, 0};
		
		MPI_Type_create_subarray(2, sizes, subsizes, starts, MPI_ORDER_C, MPI_UNSIGNED_CHAR, &mpi_image_block);
		MPI_Type_commit(&mpi_image_block);
		
		/* send block to other processes */
		for(i=1; i<size; i++) {
			int row, col;
			row = i/grid_side*in->height/grid_side;
			col = (i%grid_side*in->width/grid_side)*planes;
			
			MPI_Send(&(in->data[row][col]), 1, mpi_image_block, i, 0, MPI_COMM_WORLD); 
		}
		
		//master will compute first block
		for(j=0; j<in->height/grid_side; j++) {
			if(memcpy(&input->data[j+1][planes], in->data[j] ,sizeof(**in->data)*in->width*planes/grid_side) != &input->data[j+1][planes]) {
				perror("master error abort");
				return;
			}
		}
		
	} else {
	
		/*
		 * register block mpi block type
		 * used to receive block of image from master
		 */
		 
		int sizes[2] = {input->height, input->width};
		int subsizes[2] = {input->height-2, input->width-(2*planes)};
		int starts[2] = {1, planes};
		
		MPI_Type_create_subarray(2, sizes, subsizes, starts, MPI_ORDER_C, MPI_UNSIGNED_CHAR, &mpi_image_block);
		MPI_Type_commit(&mpi_image_block);
		//get block from master
		MPI_Recv(&(input->data[0][0]), 1, mpi_image_block, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	}
				
	/*
	 * register block cols
	 * used to exchange column data from neighbour processes
	 */
	int count = input->height-2;
	int blocklenght = planes;
	int stride = input->width;
	MPI_Type_vector(count, blocklenght, stride, MPI_UNSIGNED_CHAR, &mpi_block_col);
	MPI_Type_commit(&mpi_block_col);
	
	/* filter image ntimes */
	for(time = 0; time < ntimes; time++) {
		int dest;
	
		/*
		 * exchange data with other processes
		 */
		
		//send-recieve from up process
		if(rank/grid_side == 0) {
			//no up process
			memcpy(input->data[0], input->data[planes], input->width-(2*planes));
		} else {
			//send-recieve
			dest = rank - grid_side;
			MPI_Isend(&input->data[1][planes], input->width-(2*planes), MPI_UNSIGNED_CHAR, dest, 0, MPI_COMM_WORLD, &requests[0]);
			MPI_Irecv(&input->data[0][planes], input->width-(2*planes), MPI_UNSIGNED_CHAR, dest, MPI_ANY_TAG, MPI_COMM_WORLD, &requests[1]);
		}

		//send-recieve from left
		if(rank%grid_side == 0) {
			//no left process
			for(i=1; i<input->height-1; i++) {
				memcpy(&input->data[i][0], &input->data[i][planes], sizeof(**input->data)*planes);
			}
		} else {
			//send-recieve
			dest = rank - 1;
			MPI_Isend(&input->data[1][planes], 1, mpi_block_col, dest, 0, MPI_COMM_WORLD, &requests[2]);
			MPI_Irecv(&input->data[1][0], 1, mpi_block_col, dest, MPI_ANY_TAG, MPI_COMM_WORLD, &requests[3]);
		}

		//send-recive from down
		if(rank/grid_side+1 == grid_side) {
			//no down process
			memcpy(input->data[input->height-1], input->data[input->height-2], input->width-(2*planes));
		} else {
			//send-recieve
			dest = rank + grid_side;
			MPI_Isend(&input->data[input->height-2][planes], input->width-(2*planes), MPI_UNSIGNED_CHAR, dest, 0, MPI_COMM_WORLD, &requests[4]);
			MPI_Irecv(&input->data[input->height-1][planes], input->width-(2*planes), MPI_UNSIGNED_CHAR, dest, MPI_ANY_TAG, MPI_COMM_WORLD, &requests[5]);
		}

		//send-recieve from right
		if(rank%grid_side+1 == grid_side) {
			//no right process
			for(i=1; i<input->height-1; i++) {
				memcpy(&input->data[i][input->width-planes], &input->data[i][input->width-(2*planes)], sizeof(**input->data)*planes);
			}
		} else {
			//send-recieve
			dest = rank + 1;
			MPI_Isend(&input->data[1][input->width-(2*planes)], 1, mpi_block_col, dest, 0, MPI_COMM_WORLD, &requests[6]);
			MPI_Irecv(&input->data[1][input->width-planes], 1, mpi_block_col, dest, MPI_ANY_TAG, MPI_COMM_WORLD, &requests[7]);
		}

		//send-recieve up-left
		if(rank/grid_side == 0 || rank%grid_side == 0) {
			//no up-left process
			memcpy(&input->data[0][0], &input->data[1][planes], sizeof(**input->data)*planes);
		} else {
			//send-recieve
			dest = rank - grid_side - 1;
			MPI_Isend(&input->data[1][planes], planes, MPI_UNSIGNED_CHAR, dest, 0, MPI_COMM_WORLD, &requests[8]);
			MPI_Irecv(&input->data[0][0], planes, MPI_UNSIGNED_CHAR, dest, MPI_ANY_TAG, MPI_COMM_WORLD, &requests[9]);
		}

		//send-recieve up-right
		if(rank/grid_side == 0 || rank%grid_side+1 == grid_side) {
			//no up-right process
			memcpy(&input->data[0][input->width-planes], &input->data[1][input->width-(2*planes)], sizeof(**input->data)*planes);
		} else {
			//send-recieve
			dest = rank - grid_side + 1;
			MPI_Isend(&input->data[1][input->width-(2*planes)], planes, MPI_UNSIGNED_CHAR, dest, 0, MPI_COMM_WORLD, &requests[10]);
			MPI_Irecv(&input->data[0][input->width-planes], planes, MPI_UNSIGNED_CHAR, dest, MPI_ANY_TAG, MPI_COMM_WORLD, &requests[11]);
		}
		
		//send-recieve down-left
		if(rank/grid_side+1 == grid_side || rank%grid_side == 0) {
			//no down-left process
			memcpy(&input->data[input->height-1][0], &input->data[input->height-2][planes], sizeof(**input->data)*planes);
		} else {
			dest = rank + grid_side - 1;
			MPI_Isend(&input->data[input->height-2][planes], planes, MPI_UNSIGNED_CHAR, dest, 0, MPI_COMM_WORLD, &requests[12]);
			MPI_Irecv(&input->data[input->height-1][0], planes, MPI_UNSIGNED_CHAR, dest, MPI_ANY_TAG, MPI_COMM_WORLD, &requests[13]);
		}

		//send-recieve down-right
		if(rank/grid_side+1 == grid_side || rank%grid_side+1 == grid_side) {
			//no down-left process
			memcpy(&input->data[input->height-1][input->width-planes], &input->data[input->height-2][input->width-(2*planes)], sizeof(**input->data)*planes);
		} else {
			dest = rank + grid_side + 1;
			MPI_Isend(&input->data[input->height-2][input->width-(2*planes)], planes, MPI_UNSIGNED_CHAR, dest, 0, MPI_COMM_WORLD, &requests[14]);
			MPI_Irecv(&input->data[input->height-1][input->width-planes], planes, MPI_UNSIGNED_CHAR, dest, MPI_ANY_TAG, MPI_COMM_WORLD, &requests[15]);
		}
		
		//filter inner block
		for(i=2; i<input->height-2; i++) {
			//for each row
			for(j=2*planes; j<input->width-2*planes; j=j+planes) {
				//for each pixel
				for(plane=0;plane<planes;plane++) {
					//for each plane compute sum
					sum = 0;
					for(p=0; p<f->rows; p++) {
						n = i - f->rows/2 + p;
							
						for(q=0; q<f->cols; q++) {
							m = j - (f->cols/2 - q)*planes + plane;
							sum += ((int)input->data[n][m])*f->kernel[p][q];
						}
					}
					
					sum /= f->sum;
					
					if(sum < 0) {
						sum = 0;
					} else if(sum > 255) {
						sum = 255;
					}
					
					result->data[i][j+plane] = ((unsigned char)sum);
				}
			}
		}
		
		//wait for isend & irecv
		if(rank/grid_side != 0) {
			MPI_Wait(&requests[0], MPI_STATUS_IGNORE);
			MPI_Wait(&requests[1], MPI_STATUS_IGNORE);
		}
		
		if(rank%grid_side != 0) {
			MPI_Wait(&requests[2], MPI_STATUS_IGNORE);
			MPI_Wait(&requests[3], MPI_STATUS_IGNORE);
		}
		
		if(rank/grid_side+1 != grid_side) {
			MPI_Wait(&requests[4], MPI_STATUS_IGNORE);
			MPI_Wait(&requests[5], MPI_STATUS_IGNORE);
		}
		
		if(rank%grid_side+1 != grid_side) {
			MPI_Wait(&requests[6], MPI_STATUS_IGNORE);
			MPI_Wait(&requests[7], MPI_STATUS_IGNORE);
		}
		
		if(rank/grid_side != 0 && rank%grid_side != 0) {
			MPI_Wait(&requests[8], MPI_STATUS_IGNORE);
			MPI_Wait(&requests[9], MPI_STATUS_IGNORE);
		}

		if(rank/grid_side != 0 && rank%grid_side+1 != grid_side) {
			MPI_Wait(&requests[10], MPI_STATUS_IGNORE);
			MPI_Wait(&requests[11], MPI_STATUS_IGNORE);
		}
		
		if(rank/grid_side+1 != grid_side && rank%grid_side != 0) {
			MPI_Wait(&requests[12], MPI_STATUS_IGNORE);
			MPI_Wait(&requests[13], MPI_STATUS_IGNORE);
		}

		if(rank/grid_side+1 != grid_side && rank%grid_side+1 != grid_side) {
			MPI_Wait(&requests[14], MPI_STATUS_IGNORE);		
			MPI_Wait(&requests[15], MPI_STATUS_IGNORE);
		}
		
		//filter outer pixels
		for(i=1; i<input->height-1; i++) {
			//for each row
			for(j=planes; j<input->width-planes; j=j+planes) {
				
				//skip filtered pixels
				
				if((i != 1 && i != input->height-2) && (j != planes && j != input->width-2*planes)) { 
					continue;
				}
				
				//for each pixel
				for(plane=0;plane<planes;plane++) {
					//for each plane compute sum
					sum = 0;
					for(p=0; p<f->rows; p++) {
						n = i - f->rows/2 + p;
							
						for(q=0; q<f->cols; q++) {
							m = j - (f->cols/2 - q)*planes + plane;
							sum += ((int)input->data[n][m])*f->kernel[p][q];
						}
					}
					
					sum /= f->sum;
					
					if(sum < 0) {
						sum = 0;
					} else if(sum > 255) {
						sum = 255;
					}
					
					result->data[i][j+plane] = ((unsigned char)sum);
				}
			}
		}
			
		//copy out data to in for next filtering
		if(memcpy(input->data[0], result->data[0], sizeof(**out->data) * input->height * input->width) != input->data[0]) {
			perror("memcpy");
		}
		
		//synchronize processes
		MPI_Barrier(MPI_COMM_WORLD);
	}
	
	//gather results from all processes
	if(rank == 0) {
		//master get results
		for(i=1; i<size; i++) {
			//recieve blocks
			int row, col;
			
			row = i/grid_side*in->height/grid_side;
			col = (i%grid_side*in->width/grid_side)*planes;
			MPI_Recv(&(out->data[row][col]), 1, mpi_image_block, i, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		}
		
		//copy first block to output
		for(j=0; j<in->height/grid_side; j++) {
			memcpy(out->data[j], &input->data[j+1][planes] ,sizeof(**out->data)*out->width*planes/grid_side);
		}
	} else {
		//slaves send results
		MPI_Send(&(input->data[0][0]), 1, mpi_image_block, 0, 0, MPI_COMM_WORLD); 
	}
	
	/* free resources */
	delete_image_block(input);
	delete_image_block(result);
	
	MPI_Type_free(&mpi_block_col);
	MPI_Type_free(&mpi_image_block);
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

