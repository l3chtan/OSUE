/**
 * @file rventgas.c
 * @author Edward Toth 0725631
 * @date 22.11.2015
 * 
 * @brief A Program to simulate the ventilation of a nuclear reactor, to lower the pressure inside.
 * 
 * This program simulates the process of an attempt to stabilize a nuclear reactor by ventig gas from it.
 * The ventilation has chance of 6/7 to work. If it didn't work, the reactor has to be shut down.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <stdint.h>

#define STATUS_OK "STATUS OK"
#define PRESSURE_TOO_HIGH "PRESSURE TOO HIGH - IMMEDIATE SHUTDOWN REQUIRED"

/**
 * Program entry point.
 * @brief The chance for success/failure and the output are generated here.
 * @details A random number generator creates numbers between 0..6. If the number is less than 6
 * the ventilation was successful. If it was not successful the reactor has to be shut down.
 * @return returns EXIT_SUCCESS if the shutdown was successful, EXIT_FAILURE otherwise.
 **/
int main(void){

	
	srand(time(NULL));	
	int rn = rand() % 7;

	if(rn < 6){
		printf("%s\n",STATUS_OK);
		return 0;
	}
	printf("%s\n",PRESSURE_TOO_HIGH);
	exit(EXIT_FAILURE);
}