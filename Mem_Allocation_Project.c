//  Found a formula for normal distribution and a more accurate random generator since libraries were not obtainable.
//  Link to formula: https://www.tutorialspoint.com/generate-random-numbers-following-a-normal-distribution-in-c-cplusplus
//  Credit for normal distribution formula and random generator: Karthikeya Boyini

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#define M_PI 3.14159265358979323846

//==================================Simulated Memory==================================//

struct PhysMem {
	int mem[20];
	int phy_mem;
} P;

//==================================Global Variables===================================//

static double cur_utl, avg_utl, avg_holes, sigma;
static int nI, size, block_size, count_requests;
FILE * s;

//======================================Arithmatic Functions======================================//

double standardDeviation() {
	int mean = 0, i, j;							//  local variables
	double stand_dev = 0.0;							//
	sigma = 0;								//
	
	for(i = 0; i < size; i++) {						//  Calculate mean
		mean += abs(P.mem[i]);						//
	}									//
	
	mean = mean/size;

	for(j = 0; j < size; j++) {						//  Sigma notation
		sigma += pow(abs(P.mem[j] - mean), 2);				//
	}
	
	stand_dev = sqrt(sigma/size);						//  Divide by population size and sqrt
	
	return stand_dev;
}

//***********************Reference to this section refered to at the top**************************//

double rand_gen() {
   // return a uniformly distributed random value
   return ( (double)(rand()) + 1. )/( (double)(RAND_MAX) + 1. );
}

double normalRandom() {
   // return a normally distributed random value
   double v1=rand_gen();
   double v2=rand_gen();
   return cos(2*3.14*v2)*sqrt(-2.*log(v1));
}

double normalDistribution(double o) {
	int u, i;
	double x;
	
	for(i = 0; i < size; i++) {							//  Calculate mean
		u += abs(P.mem[i]);							//
	}										//
											//
	u = u/size;
	x = normalRandom() * o + u;
	
	while(x < 1) {
		x = normalRandom() * o + u;
	}
	
	return abs(x);
}

//************************************************************************************************//

void initializeMemoryBlocks() {
	int ran = 0, i, j;								//  Local variables
	
	for(i = 0; i < size; i++) {							//  Generate random sizes of memory blocks
		ran = (double)rand()/(double)RAND_MAX * (block_size - 1) + 1;
		P.mem[i] = ran;								//
	}										//
	
	for(j = 0; j < size; j++) {							//  Convert all non-zeros to negative to simulate
		if(P.mem[j] > 0) {							// free memory sizes
			P.mem[j] = P.mem[j] * -1;					//
		}									//
	}										//
}

int releaseBlock() {
	int rb = 0;
	
	rb = rand() % size;								//  Randomize a block to pick to release
	P.mem[rb] = (-1 * block_size);							//  Revert it to full size to be used later
	
	return rb + 1;									//  Shifts for ease of reading
}

void computeMemoryUtilization(int r) {
	int i;
														
	for(i = 0; i < size; i++) {							//  Displays physical memory
		printf("[%i]", P.mem[i]);						//
											//
		if(P.mem[i] == 0) {							//  Recording current memory utilization
			cur_utl += block_size;						//
		}  else if(P.mem[i] > (-1 * block_size)) {				//
			cur_utl += (block_size + P.mem[i]);				//
		}									//
	}										//
		
	cur_utl = cur_utl/(size*block_size);						//  Calculate current average
	avg_utl += cur_utl;								//  Compile with total average for later
}

//==================================Memory Allocation Algorithms==================================//

void firstFit() {
	
	double sd = standardDeviation();
	int g = normalDistribution(sd);									//  Creating the average request size for the request
	
	printf("\nRequest size for First-fit: %i\n", g);
	
	int hole_size = 0, hole_tracker = 0, j = 0, i;							//  Local variables
	
	for(i = 0; i < size; i++) {									//  Loop uses First-fit logic to run through the physical
		count_requests++;									// memory and attempts to satisfy request until it cannot
		
		if(P.mem[i] < 0) {									//  Condition checks if current block observed contains a
													// hole of any size
			avg_holes++;									//  Accumulates holes observed to calculate average later

			if(g <= abs(P.mem[i])) {							//  Condition first checks if block observed can satisfy
													// request itself
				printf("\n*Found a hole starting at block %i!*\n", i + 1);												
				
				P.mem[i] += g;									
				return;
			}  else if(g > abs(P.mem[i]) && (i-1) != -1) {					//  Condition moves on to check if block itself cannot satisfy 
													// request and must use adjacent hole, if any exist, to satisfy it
				hole_size += abs(P.mem[i]);
				j = i;
				
				while(g > hole_size) {
					hole_tracker += 1;
						
					if(g > hole_size && P.mem[j-1] == 0) {
						hole_size = 0;
						break;
					}
					
					if((j - 1) == -1) {
						hole_size += abs(P.mem[j]);
						break;
					}  
					
					j -= 1;
					hole_size += abs(P.mem[j]);
					
				}
				
				
				
				if(g <= hole_size) {							//  If adjacent holes exist AND they can satisfy request size, goes
													// to current position of those holes and calculates how much
					int xMIN, xMAX;							// space the request size will take up of each block and update them
					
					xMAX = j + hole_tracker;
					xMIN = j;
					
					for(xMAX = xMAX; xMAX >= xMIN; xMAX--) {			//  Exits function if request size is satisfied, does nothing if it
						if(g >= abs(P.mem[xMAX])) {				// cannot
							g = g - abs(P.mem[xMAX]);			//
							P.mem[xMAX] = 0;				//
						}  else {						//
							P.mem[xMAX] = g + P.mem[xMAX];			//
						}							//
					}								//
					
					printf("\n*Found a hole starting at block %i!*\n", xMIN + 1);
					
					return;
				}  else {
					hole_tracker = 0;
				}
			}
		}
	}
	printf("\n*Did not find a large enough hole...*\n");
}

void nextFit() {
	
	double sd = standardDeviation();
	int g = normalDistribution(sd);									//  Creating the average request size for the request
	
	printf("\nRequest size for Next-fit: %i\n", g);
	
	int count_requests = 0, count_holes = 0, hole_size = 0, hole_tracker = 0, j = 0, i = nI;	//  Local variables
	
	if(i == (size - 1)) {										//  Checks worst condition, which is if algorithm starts
		count_requests++;									// at the last block. It would've just keep going instead of
													// looping back to the beginning otherwise.
		if(P.mem[i] < 0) {									//	Checks if hole exists at currently observed block
			avg_holes++;
			
			if(g <= abs(P.mem[i])) {							//  If hole exists in observed block AND satisfies request																		// size, it takes up memory and leaves function
				printf("\n*Found a hole starting at block %i!*\n", i + 1);
				
				P.mem[i] += g;
				nI = i;
				return;
			}
		}  else {
			i = 0;	
		}
	}  else if(i == nI) {										//  If block observed doesn't start at the end and block
		count_requests++;									// checks current block being observed satisfies request size
		
		if(P.mem[i] < 0) {
			avg_holes++;
		
			if(g <= abs(P.mem[i])) {
				printf("\n*Found a hole starting at block %i!*\n", i + 1);
				
				P.mem[i] += g;
				nI = i;
				return;
			}  else {
			}
		}
		i++;
	}
	
	while(i != nI) {										//  Traverses through the array to check for holes to satisfy		
		count_requests++;									// request
		
		if(P.mem[i] < 0) {									//  Checks if current block observed contains a hole
			
			avg_holes++;
			
			if(g <= abs(P.mem[i])) {							//  If hole in current observed block satisfies request, uses
				printf("\n*Found a hole starting at block %i!*\n", i + 1);
			
				P.mem[i] += g;
				nI = i;
				return;
			}  else if(g > abs(P.mem[i]) && (i-1) != -1) {					//  Request size is bigger than hole size in observed block
													// and has existing adjacent blocks with holes
				hole_size += abs(P.mem[i]);
				j = i;
				
				while(g > hole_size) {							//  Starts from current block and traverses backwards to check adjacent
					hole_tracker += 1;						// holes to find a hole big enough to satisfy request
						
					if(g > hole_size && P.mem[j-1] == 0) {				//  Checks if adjacent holes exist next to currently observed block
						hole_size = 0;						// that contains a hole
						//printf("\n*Hole size not big enough*\n");
						break;
					}
					
					j -= 1;
					hole_size += abs(P.mem[j]);					//  Build hole size to compare with request size to check if it can
				}									// fit request size in it
				
				if(g <= hole_size) {							//  Checks if hole satisfies request
					
					int xMIN, xMAX;
					
					xMAX = j + hole_tracker;
					xMIN = j;
					
					for(xMAX = xMAX; xMAX >= xMIN; xMAX--) {			//  Goes through contigous hole and adjusts memory accordingly to
						if(g >= abs(P.mem[xMAX])) {				// size of request
							g = g - abs(P.mem[xMAX]);
							P.mem[xMAX] = 0;
						}  else {
							P.mem[xMAX] = g + P.mem[xMAX];
						}
					}
					
					printf("\n*Found a hole starting at block %i!*\n", xMIN + 1);
					
					nI = i;
					return;
				}  else {								//  Function returns with nothing if request was not successfully
					hole_tracker = 0;						// satisfied
				}
			}
		}
		
		if(i == (size - 1)) {									//  If iteration of loop ends at the last block, this will update
			i = 0;										// i so that it does not get updated to +1 and continuously go up
		} else {										// to a huge number
			i++;
		}
	
	}
	printf("\n*Did not find a large enough hole...*\n");
	
	nI = i;												//  Set new block to start at to last block observed in the previous
}													// run

//==================================Helper Function===================================//

void runSimulation(char a) {
	
	int r, i, rel_block;															//  Local variables
	
	switch(a) {
			
		case 'f':															//  First-Fit case
		
		count_requests = 0;
		avg_holes = 0;
		avg_utl = 0;
		
			for(r = 1; r <= 20; r++) {
				printf("\n|===================Request #%i=====================|\n", r);
	
				printf("\nBefore First-Fit:\n");
				
				for(i = 0; i < size; i++) {											//  Prints out the physical memory before
					printf("[%i]", P.mem[i]);
				}
				
				printf("\n");
				
				firstFit();													//  Executes First-Fit algorithm
				
				printf("\nAfter First-fit:\n");
				computeMemoryUtilization(r);											//  Calulates current memory utilization
																		// and holes examined				
				rel_block = releaseBlock();											//  Releases a randomly chosen block
				
				printf("\n");
				printf("\nRelease block: %i\n", rel_block);
		
				//  Testing to physically see array
				for(i = 0; i < size; i++) {											//  Prints out the physical memory after
					printf("[%i]", P.mem[i]);
				}
				
				printf("\n");
			}
		
			avg_utl = avg_utl/r;													//	Calculates average memory utilization
			avg_holes = avg_holes/count_requests;											//	Caluclates average number of holes examined
			
			printf("\n|==================Final Results===================|\n");
			
			printf("\nAverage memory utilization = %0.2f\n", avg_utl);
			printf("\nAverage number of holes examined = %0.2f\n", avg_holes);
	
			printf("\n|==================================================|\n");
			
				break;
			
		case 'n':
		
		count_requests = 0;
		avg_holes = 0;
		avg_utl = 0;
		nI = 0;
				
			for(r = 1; r <= 20; r++) {
				printf("\n|===================Request #%i=====================|\n", r);
				
				printf("\nMemory Sample:\n");
				
				//  Testing to physically see array
				for(i = 0; i < size; i++) {											//  Prints out the physical memory before
					printf("[%i]", P.mem[i]);
				}
				printf("\n");
				
				nextFit();													//  Executes Next-Fit algorithm
				printf("\nAfter Next-fit:\n");
				computeMemoryUtilization(r);											//  Calculates current memory utilization
																		// and holes examined				
				rel_block = releaseBlock();											//  Releases a randomly chosen block
				
				printf("\n");
				printf("\nRelease block: %i\n", rel_block);
		
				//  Testing to physically see array
				for(i = 0; i < size; i++) {											//  Prints out the physical memory after
					printf("[%i]", P.mem[i]);
				}
				
				printf("\n");
			}
			
			avg_utl = avg_utl/r;													//  Calculates average memory utilization
			avg_holes = avg_holes/r;												//  Calculates average number of holes examined
			
			printf("\n|==================Final Results===================|\n");
			
			printf("\nAverage memory utilization = %0.2f\n", avg_utl);
			printf("\nAverage number of holes examined = %0.2f\n", avg_holes);
			
			printf("\n");
	
			printf("\n|==================================================|\n");
				break;
			
			default:
			
			printf("\nERROR: The letter you entered did not correspond to a memory allocation algorithm.\n");			//  Catches if incorrect input for an
		}																// algorithm choice
}

//==================================Main===================================//

int main(void) {
	
	int i, run_times, avg_req;														//  Local variables
	char algo_choice = ' ';															//
	
	s = fopen("Simulation_Output.csv","w");													//  Output file initialized and ready to write to
	
	//  User input
	printf("\n\nWelcome to the Memory Allocation Simulation!\nHow many times would you like to run the simulation?\n");
	scanf("%i", &run_times);														//  Scans user input for number of simulation runs
	
	printf("What is your choice of average request size?\n");
	scanf("%i", &avg_req);															//  Scans user input for average request size
	
	srand(time(NULL));															//  Sets seed for rand()
	
	//  Setting sizes of blocks and memory
	size = sizeof(P.mem)/sizeof(int);														
	block_size = avg_req;
	P.phy_mem = size * block_size;														//  Sets total possible size for the array
	
	//  Simulation starts
	fprintf(s, "First-Fit\n");
	fprintf(s, "Simulation Run,Average Memory Utilization,Average Holes Observed,Average Request Size\n");
	
	//  Running the First-Fit algorithm
	for(i = 0; i < run_times; i++) {													//  Runs First-Fit simulation 10 times
		printf("\n              [First-Fit Simulation: %i]\n", i + 1);
		
		printf("\nSize of memory = %i\n", P.phy_mem);
		printf("Block size = %i\n", block_size);
		
		initializeMemoryBlocks();													//  Initializes memory and sets values to each block
		runSimulation('f');														//  Runs First-Fit through helper method
		fprintf(s, "%i,%0.2f,%0.2f,%i\n", i+1, avg_utl, avg_holes, avg_req);								//  Outputs data into output file
	}
	
	//  Running the Next-Fit algorithm
	fprintf(s, "\n");
	fprintf(s, "Next-Fit\n");
	fprintf(s, "Simulation Run,Average Memory Utilization,Average Holes Examined,Average Request Size\n");
	
	for(i = 0; i < run_times; i++) {
		printf("\n              [Next-Fit Simulation: %i]\n", i + 1);									//  Runs Next-Fit simulation 10 times
		
		printf("\nSize of memory = %i\n", P.phy_mem);
		printf("Block size = %i\n", block_size);
		
		initializeMemoryBlocks();													//  Initializes memory and sets values to each block
		runSimulation('n');														//  Runs Next-Fit through helper method
		fprintf(s, "%i,%0.2f,%0.2f,%i\n", i+1, avg_utl, avg_holes, avg_req);								//  Outputs data into output file
	}
	
	fclose(s);																//  Closes output file
		
	return 0;
}
