/**
 * @file rshutdown.c
 * @author Edward Toth 0725631
 * @date 22.11.2015
 * 
 * @brief A Program to simulate a shutdown
 * 
 * This program simulates the shutdown of a nuclear reactor, which has a 2/3 chance of exploding.
 * A successful shutdown is indicated by the output "SHUTDOWN COMPLETED", 
 * and a failed shutdown by "KaBOOM!"
 **/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

#define SHUTDOWN "SHUTDOWN COMPLETED"
#define KABOOM "KaBOOM!"

/**
 * Program entry point.
 * @brief The chance for success/failure and the output are generated here.
 * @details A random number generator creates numbers between 0..2. If the number is less than 1 (aka 0)
 * the shutdown was successful. Otherwise the reactor will explode.
 * @return returns EXIT_SUCCESS if the shutdown was successful, EXIT_FAILURE otherwise.
 **/
int main(void){

	srand(time(NULL));	
	int rn = rand() % 3;

	if(rn < 1){
		printf("%s\n",SHUTDOWN);
		return 0;
	}
	
	printf("%s\n",KABOOM);
	exit(EXIT_FAILURE);


	return EXIT_SUCCESS;
}
