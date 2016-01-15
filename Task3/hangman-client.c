/**
 * @file hangman-client.c
 * @author Edward Toth 0725631
 * @date 10.01.2016 
 * 
 * @brief The client program for playing hangman
 * 
 **/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <signal.h>
#include <ctype.h>

#include "hangman-common.h"

#define ABC (27)
#define ASCIIA (97)

struct client {
	int id;

	int win;
	int loss;

	int wrong_guess;
	char used[ABC];
	char word[BUFFER];
};

static char *cmd;
static struct client *this;

/**
 * Usage function
 * @brief If the server is started impropperly a usage message is printed and usually the program exits afterwards.
 **/
static void usage(void);

/**
 * Bail out function - used in case of emergency
 * @brief Prints error message and exits program if any problem occurs during execution.
 * @param msg contains the error message
 **/
static void bail_out(char* msg);

/**
 * Free semaphores 
 * @brief Will try to free the semaphores which have been allocated.
 * In case of an error it will print error messages.
 **/
static void free_sem(void);

/**
 * Free shared memory 
 * @brief Will try to free the shared memory which has been allocated.
 * In case of an error it will print error messages.
 **/
static void free_shm(void);

/**
 * Signal handler
 * @brief In case of a signal to end the program, the handler will try to release all allocated resources.
 * In case of an error it will print error messages.
 * @param signum the signal which was received.
 **/
static void handler(int signum);

/**
 * Finalize resources
 * @brief takes the final actions before exiting the program, 
 * like notifying the server, printing the score and closing the shared memory and semaphores
 **/
static void finalize(void);

/**
 * Normalize strings
 * @brief this function will remove any trailing newline or non ascii (except space).
 * It also sets all characters to lower case
 * @param str the string which is to be normalized
 **/
static void normalize(char *str);

/**
 * The main function
 * @brief Sets up the signal handler, shared memory and semaphores. 
 * @details It relies on the fact that the server already created all of the above (except signal handler).
 * If not, it will terminate with EXIT_FAILURE
 * It also manages the comunication to the server.
 * @param argc counts the number arguments
 * @param argv has a list of input arguments including the program itself.
 * @return returns 0/SUCCESS in case everything is ok, EXIT_FAILURE otherwise.
 **/
int main(int argc, char **argv){

/***Signal handler***/

	struct sigaction sa;

	sa.sa_handler = handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;

	if(sigaction(SIGINT, &sa, NULL) == -1){
		(void) fprintf(stderr,"Resource cleanup failed for %s\n",cmd);
		exit(EXIT_FAILURE);
	}

	if(sigaction(SIGTERM, &sa, NULL) == -1){
		(void) fprintf(stderr,"Resource cleanup failed for %s\n",cmd);
		exit(EXIT_FAILURE);
	}

/*Signal handler end*/



	cmd = argv[0];

	if(argc > 1) usage();

	char buffer[BUFFER];

	int shmfd = shm_open(SHM_NAME, O_RDWR|O_CREAT, PERMISSION);

	if(shmfd < 0){
		bail_out("Shared memory could not be opened.\n");
		exit(EXIT_FAILURE);
	}

	if(ftruncate(shmfd, sizeof *game) < 0){
		bail_out("Size of shared memory could not be set.\n");
		exit(EXIT_FAILURE);
	}

	game = mmap(NULL, sizeof *game, PROT_READ|PROT_WRITE, MAP_SHARED, shmfd, 0);

	if(game == MAP_FAILED){
		if (shm_unlink(SHM_NAME) < 0){
			bail_out("Shared memory object could not be removed\n");
		}
		exit(EXIT_FAILURE);
	}

	if(close(shmfd) < 0){
		if (munmap(game, sizeof *game) < 0){
			bail_out("Unmap of shared memory failed\n");
		}
		if (shm_unlink(SHM_NAME) < 0){
			bail_out("Shared memory object could not be removed\n");
		}
		exit(EXIT_FAILURE);
	}

	/***semaphore***/

	s1 = sem_open(SEM_1, 0);
	s2 = sem_open(SEM_2, 0);
	s3 = sem_open(SEM_3, 0);

	if(s1 == SEM_FAILED || s2 == SEM_FAILED || s3 == SEM_FAILED){
		(void) fprintf(stderr, "no server available\n");
		if(s1 != SEM_FAILED) {
			sem_close(s1);
			sem_unlink(SEM_1);
		}
		if(s2 != SEM_FAILED) {
			sem_close(s2);
			sem_unlink(SEM_2);
		}
		if(s3 != SEM_FAILED) {
			sem_close(s3);
			sem_unlink(SEM_3);
		}
		exit(EXIT_FAILURE);
	}

	/*semaphore end*/

	/*init struct*/	
	this = malloc(sizeof(struct client));
	this->id = game->client_id;
	this->win = 0;
	this->loss = 0;
	this->wrong_guess = 0;
	memset(this->used,' ',ABC);
	memset(this->word,0,BUFFER);
	/*end*/

	/****critical section****/

	/*This is extra to register the client to the server*/
	sem_wait(s3);
	sem_wait(s1);
	game->client_id = REGISTER;
	sem_post(s2);
	sem_wait(s1);
	game->client_id = getpid();
	// memset(game->guess,0,BUFFER);
	sem_post(s2);
	sem_wait(s1);
	this->id = game->client_id;
	printf("Your id: %d\n",this->id);
	sem_post(s1);
	sem_post(s3);


	printf("You started a new game!\n");
	printf("Guess: ");

	memset(buffer,0,BUFFER);
	while(fgets(buffer,BUFFER-1,stdin) != NULL){
		normalize(buffer);

		sem_wait(s3);
		sem_wait(s1);

		/*the client will quit if the server requests it*/
		if(game->quit == CLIENT_QUIT){
			game->quit = NO_QUIT;
			break;
		}

		game->client_id = this->id;

			int i=0,j=0;
			char *tmp = malloc(BUFFER);
			memset(tmp,0,BUFFER);
			/*write to the array which holds all characters that have already been used*/
			for(i=0;i<strlen(buffer);++i){
				int n =buffer[i] - ASCIIA;
				if(this->used[n] != buffer[i]){
					tmp[j++] = buffer[i];
					this->used[n] = buffer[i];
				}
			}

		memset(game->guess,0,BUFFER);
		strcpy(game->guess,tmp);
		free(tmp);

		sem_post(s2);
		sem_wait(s1);

		/*this section is only entered if the server sends an extra message in addition to
		 * the updated guessed word.*/
		if(strlen(game->msg) > 0){

			/*this part is only entered if the game is over (win or lose both)*/
			if(game->lost != 0){
				printf("The secret word was: %s\n",game->word);
				printf("%s\n",game->msg);
				memset(game->msg,0,BUFFER);
				memset(this->used,' ',ABC);

				if(game->lost > 0){
					this->loss++;
				} else {
					this->win++;	
				}

				int answer = 0;
				do{	
					if(fgets(buffer,BUFFER,stdin) == NULL) continue;
					answer = tolower(buffer[0]);
				} while(answer != 'n' && answer != 'y');

				game->msg[0] = answer;

				sem_post(s2);

				if(answer == 'n') break;
			} else {
				printf("%s\n",game->msg);
			}
			/*end - end of game*/
		} else {
			printf("The characters you already tried:\n> %s\n",this->used);
			printf("Your wrong guesses: %d\n",game->wrong);
			if(strlen(game->word) > 0){
				printf("%s\n",game->word);
			}
			sem_post(s1);
		}
		sem_post(s3);
		printf("Guess: ");
	}
	finalize();
	/**critical section end**/
	return 0;
}

/***Functions***/

static void bail_out(char* msg){
	(void) fprintf(stderr,"%s: %s\n",cmd,msg);
}

static void usage(){
	(void) fprintf(stderr, "\n USAGE: hangman-client\n");
	(void) fprintf(stderr, " Options and files are being ignored");
}


static void free_sem(){

		sem_close(s1); sem_close(s2); sem_close(s3);
}

static void free_shm(void){

	if(munmap(game, sizeof *game) < 0){
		bail_out("Unmap of shared memory failed\n");
		exit(EXIT_FAILURE);
	}
}

static void handler(int signum){
	if(signum == SIGINT || signum == SIGTERM){

		sem_wait(s3);
		sem_wait(s1);
		finalize();
	}
	exit(EXIT_SUCCESS);
}

static void finalize(void){
		printf("You lost %d times\n",this->loss);
		printf("You won %d times\n",this->win);

		game->client_id = this->id;
		game->quit = CLIENT_QUIT;
		sem_post(s2);
		sem_post(s1);
		sem_post(s3);
		free(this);
		free_sem();
		free_shm();
}


static void normalize(char *str){

	/*set all characters to lowercase*/
	int i,j;
	for(i=0;i<strlen(str);++i){
		str[i] = tolower(str[i]);
	}
	/*remove trailing newline*/
	int idx = strlen(str)-1;
	if(str[idx] == '\n'){
		str[idx] = 0;
	}
	/*remove all non ascii and non space characters*/
	char *tmp = malloc(BUFFER);
	j = 0;
	for(i=0;i<strlen(str);++i){
		if((str[i] >= 'a' && str[i] <= 'z') || str[i] == ' '){
			tmp[j++] = str[i];
		}	
	}
	memset(str,0,BUFFER);
	strcpy(str,tmp);
	free(tmp);
}

