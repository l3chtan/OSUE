/**
 * @file hangman-server.c
 * @author Edward Toth 0725631
 * @date 10.01.2016 
 * 
 * @brief The server program for playing hangman 
 * 
 * The server allocates the resources for the shared memory and the semaphores ,
 * which are used by both the server and the clients.
 * It also reads a list of words (either from stdin or a file) which can be guessed by the clients.
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

#define MAX_GUESS (9)

struct clients {
	int client_id;

	struct clients *next;

	int word_num;
	int  wrong_guess;
	char word[BUFFER];
	char guess[BUFFER]; 
};

static char* cmd;
static char **words;
static struct clients *head, *last, *current;


/**
 * 
 * @brief In the created child processes programs, which were passed as arguments on program start,
 * are executed.
 * @details A new process is forked and a "program" executed in it. The ouptut of that program is passed to its parent process via a pipe.
 * Uses the global varialbes opt_f, opt_s, child_fd and parent_fd.
 * @param the program name which is to be called.
 * @return returns the status number of the executed program.
 **/


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
 * Free array function
 * @brief Frees the 2D-array, which is used to hold the secret words.
 * @param mx mx is the array to be freed.
 **/
static void free_array(char** mx);

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

/*functions for the client*/

/**
 * Normalize strings
 * @brief this function will remove any trailing newline or non ascii (except space).
 * It also sets all characters to lower case
 * @param str the string which is to be normalized
 **/
static void normalize(char *str);

/**
 * Search for a client
 * @brief looks for a client in a linked list of clients which are currently playing.
 * @param id the id of a client is used to look for it (the id equals the process id of the client).
 * @return a pointer to the struct for the client with id 'id', or NULL in case it was not found.
 **/
static struct clients* find_client(int id);

/**
 * Set the found characters
 * @brief sets the all underscore '_' characters to the correct characters if they have been correctly guessed.
 * @param guess the character which is being tested if it appears in the word which is to be guessed.
 * @param unknown the hidden word which consits soley of underscores at the beginning of each round.
 * @param word the original word, against which 'guess' is being tested.
 * @return in case at least one match was found the function will return 0, otherwise it will return -1.
 **/
static int set_found(char guess, char* unknown, char *word);

/**
 * Hide a word
 * @brief A new string is created which is of the same length as the input, but all the characters are underscores.
 * @param str the original string which will be hidden
 * @return the hidden string
 **/
static char* hide_word(char *str);

/**
 * Free a not needed client
 * @brief Removes a client, who is not playing anymore, from the list of clients and free its resources.
 * @param current the client which is to be removed.
 **/
static void free_client(struct clients *current);

/**
 * Free all clients
 * @brief frees all clients currently in the linked list. This is only used in case the server shuts down. 
 * Internally the free_client function is used.
 **/
static void free_clients(void);

/**
 * Register a new client
 * @brief this function will create a new 'clients' object and add it to the linked list.
 * @param id the id the new client will have (it is the process id of the client)
 * @return the struct object containing the id of the new client and its initialized fields.
 **/
static struct clients* register_client(int id);
/*end*/

/**
 * Main function
 * @brief Reads all the words for the game and manages the clients
 * @details The main function takes at most one file and reads its contents. 
 * Each line in the file (or from stdin) will be treated as one word.
 * The main function also creates the shared memory and he semaphores.
 * I wonder if anyone will ever read this.
 * In a loop it also handles all the clients and computes answers for them.
 * @param argc counts the number arguments
 * @param argv has a list of input arguments including the program itself.
 * @return returns 0/SUCCESS in case everything is ok, EXIT_FAILURE otherwise.
 **/
int main(int argc, char** argv){

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

/*end*/

/*Create and/or initialize variables for use throught the function*/
	cmd = argv[0];	

	int max_words = 5, cnt = 0;
	FILE *input = stdin;
	words = malloc(max_words*sizeof(char *));
	char buffer[BUFFER];

	current = NULL;
	head = NULL;
	last = NULL;

/*end*/

	if(argc > 1){
		input = fopen(argv[1],"r");
	} else {
		printf("Enter one word per line.\n");
		printf("When you are done hit Ctrl-D to stop adding new words.\n");
	}

	if(input == NULL){
		bail_out("No such file exists");
		usage();
	}

/*Read input from stin or file and save each line into a row in a matrix*/
	memset(buffer,0,BUFFER);
	while(fgets(buffer,BUFFER-1,input) != NULL){
		normalize(buffer);
		if(cnt >= max_words-1){
			max_words += max_words;
			char** chk = realloc(words, max_words*sizeof(char *));

			if(chk == NULL){
				free_array(words);
				bail_out("Memory allocation failed\n");
				exit(EXIT_FAILURE);
			}

			words = chk;
		}
		words[cnt] = malloc(strlen(buffer+1));
		(void) strcpy(words[cnt],buffer);
		cnt++;

		(void) memset(buffer,0,BUFFER);
	}
	printf("The secret words have been read\n");

/**************************done reading**************************/


/*create the shared memory*/

	int shmfd = shm_open(SHM_NAME, O_RDWR|O_CREAT, PERMISSION);

	if(shmfd < 0){
		free_array(words);
		bail_out("Shared memory could not be opened.\n");
		exit(EXIT_FAILURE);
	}

	if(ftruncate(shmfd, sizeof *game) < 0){
		free_array(words);
		bail_out("Size of shared memory could not be set.\n");
		exit(EXIT_FAILURE);
	}

	game = mmap(NULL, sizeof *game, PROT_READ|PROT_WRITE, MAP_SHARED, shmfd, 0);

	if(game == MAP_FAILED){
		if (shm_unlink(SHM_NAME) < 0){
			bail_out("Shared memory object could not be removed\n");
		}
		free_array(words);
		exit(EXIT_FAILURE);
	}

	if(close(shmfd) < 0){
		if (munmap(game, sizeof *game) < 0){
			bail_out("Unmap of shared memory failed\n");
		}
		if (shm_unlink(SHM_NAME) < 0){
			bail_out("Shared memory object could not be removed\n");
		}
		free_array(words);
		exit(EXIT_FAILURE);
	}

/*end*/

	/***semaphore***/

	s1 = sem_open(SEM_1, O_CREAT|O_EXCL, 0600, 0);
	s2 = sem_open(SEM_2, O_CREAT|O_EXCL, 0600, 1);
	s3 = sem_open(SEM_3, O_CREAT|O_EXCL, 0600, 0);

	/*semaphore end*/

	/****critical section****/

	/*init game struct*/
	sem_wait(s2);
	memset(game->msg,0,BUFFER);
	game->client_id = 0;
	game->lost = 0;
	game->wrong = 0;
	game->quit = 0;
	memset(game->word,0,BUFFER);
	memset(game->guess,0,BUFFER); 
	sem_post(s1);
	sem_post(s3);
	/*end*/

	do{
		printf("Waiting for client...\n");
		sem_wait(s2);

		/***register new client***/
		if(game->client_id == REGISTER){
			sem_post(s1);
			sem_wait(s2);
			printf("Registering new client...\n");
			current = register_client(game->client_id);

			printf("id: %d\n",current->client_id);
			game->client_id = current->client_id;
			sem_post(s1);
			printf("New client registered.\n");
			continue;
		}
		/***********end***********/


		/***get active client***/
		current = find_client(game->client_id);
		if(current == NULL){
			bail_out("Request from not registered client\n");
			sem_post(s1);
			continue;
		}
		/*********end*********/

		/*remove client if it left*/
		if(game->quit == CLIENT_QUIT){
			game->quit = NO_QUIT;
			printf("Client id %d is leaving\n",current->client_id);
			free_client(current);
			current = head;
			continue;
		}
		/*end*/

		/*set variables for client*/
		printf("clientID: %d\n",current->client_id);
		memset(game->msg,0,BUFFER);

		memset(current->guess,0,BUFFER);
		strcpy(current->guess,game->guess);
		/*end*/

		/*give client new word if available and it doesn't have one*/
		if(strlen(current->word) == 0){

			if(current->word_num < cnt){
				memset(current->word,0,BUFFER);
				memset(game->word,0,BUFFER);
				char *tmp = hide_word(words[current->word_num]);
				strcpy(game->word,tmp);
				strcpy(current->word,tmp);
				free(tmp);
				game->lost = 0;
			}
		}
		/*end*/

		/***compare guess to word***/
			int len = strlen(game->guess); 
			if(len > strlen(current->word)){
				len = strlen(current->word); /*cut the guess, in case it was longer than the word itself*/
			}
			/*loop through all the characters from the guess and match them to the word*/
				int i = 0;
				for(i=0;i<len;++i){
					printf("loop: %c\n",game->guess[i]);
					int tmp = set_found(game->guess[i],current->word,words[current->word_num]);
					/*increase a wrong guess counter each time a guessed character was not found in the word*/
					if(tmp == -1){
						current->wrong_guess++;
						game->wrong++;

						if(current->wrong_guess >= MAX_GUESS){
							game->lost = 1;	
							memset(game->msg,0,BUFFER);
							strcpy(game->msg,"Game over. Would you like to play another one? (y/n)");
							break;
						}
					}
					strcpy(game->word,current->word);
				}
				/*end loop*/
				if(strcmp(current->word,words[current->word_num]) == 0){
					game->lost = -1;
					memset(game->msg,0,BUFFER);
					strcpy(game->msg,"You Win! Would you like to play another one? (y/n)");
				}	

		if(game->lost != 0){
			strcpy(game->word,words[current->word_num]);

			sem_post(s1);
			sem_wait(s2);

			/*handle the cases if the player wants to play another game or not*/
			if(game->msg[0] == 'n'){
				printf("freeing resources...\n");
				continue;
			}	else {
				current->word_num++;
				/*if no words are left to play, quit the client and remove him from the list*/
				if(current->word_num == cnt){
					if(kill(current->client_id,SIGTERM) == -1) bail_out("Could not quit client\n");
					free_client(current);
					if(current != head) current = head;
				} else {
					game->lost = 0;
					memset(current->word,0,BUFFER);
					memset(current->guess,0,BUFFER);
					current->wrong_guess = 0;
				}
			}
		}
	/************end************/
		sem_post(s1);

	} while(1);

	/**critical section end**/

	free_sem();
	free_shm();
	free_array(words);
	return 0;
}









/***Functions***/

static void usage(void){
	(void) fprintf(stderr,"\n USAGE: hangman-server [input_file]\n\n");
	(void) fprintf(stderr," - input_file\teach line is treated as one word;\n\t\tnon ASCII characters will be ignored\n\n");
	exit(EXIT_FAILURE);
}

static void bail_out(char* msg){
	(void) fprintf(stderr,"%s: %s\n",cmd,msg);
}





static void free_array(char** mx){
	int i=0;
	for(i=0;mx[i] != NULL;i++){
		free(mx[i]);
	}
	free(mx);
}

static void free_sem(void){
	sem_close(s1); 
	sem_close(s2); 
	sem_close(s3);
	sem_unlink(SEM_1); sem_unlink(SEM_2); sem_unlink(SEM_3);
}

static void free_shm(void){

	if(munmap(game, sizeof *game) < 0){
		bail_out("Unmap of shared memory failed\n");
		free_array(words);
		exit(EXIT_FAILURE);
	}

	if(shm_unlink(SHM_NAME) < 0){
		bail_out("Shared memory object could not be removed\n");
		free_array(words);
		exit(EXIT_FAILURE);
	}
}

static void handler(int signum){
	if(signum == SIGINT || signum == SIGTERM){
		free_clients();
		free_sem();
		free_shm();
		free_array(words);
	}
	exit(EXIT_SUCCESS);
}







/*functions to handle the client*/

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

static struct clients* find_client(int id){
	struct clients *tmp;
	for(tmp = head; tmp != NULL;tmp = tmp->next){
		if(tmp->client_id == id){
			return tmp;
		}
	}
	return NULL;
}

static int set_found(char guess, char* unknown, char *word){
	int i=0, cnt=0;

	for(i=0;i<strlen(word);++i){
		if(word[i] == guess){
			unknown[i] = guess;
			cnt++;
		}
	}
	if(cnt == 0) return -1;
	return 0;
}

static char* hide_word(char *str){
	char *ret = malloc(BUFFER);
	memset(ret,0,BUFFER);
	int i;
	for(i=0;i<strlen(str);++i){
		if(str[i] == ' ') ret[i] = ' ';
		else ret[i] = '_';
	}
	return ret;
}

/*the function goes through the list and looks for the specified client
 * it will also check if the client is at the head of the list*/
static void free_client(struct clients *cl){
	printf("Removing client id: %d\n",cl->client_id);
	if(cl == NULL) return;

	struct clients *tmp;
	for(tmp = head; tmp->next != NULL;tmp = tmp->next){
	}

	if(head == cl) {
		if(head->next != NULL) head = head->next;
		else head = NULL;
		free(cl);
		return;
	}

	for(tmp = head;tmp->next != NULL;tmp = tmp->next){
		printf("tmp: %d\n",tmp->client_id);
		if(tmp->next == cl){
			tmp->next = cl->next;
			free(cl);
			return;
		}
	}

}

static void free_clients(void){
	if(head == NULL) return;
	struct clients *tmp = head;
	struct clients *n;

	 while(tmp != NULL){
	 	n = tmp->next;
	 	kill(tmp->client_id,SIGTERM);
	 	free_client(tmp);	
	 	tmp = n;
	 }
}

static struct clients* register_client(int id){
	struct clients *tmp = find_client(id);
	if(tmp != NULL) return tmp; 

	struct clients *new_client = malloc(sizeof(struct clients));
	new_client->client_id = id;
	new_client->next=NULL;
	new_client->word_num = 0;
	new_client->wrong_guess = 0;
	memset(new_client->word,0,BUFFER);
	memset(new_client->guess,0,BUFFER);

	if(head == NULL){
		head = new_client;
		last = head;
	} else {
		last->next = new_client;
		// current->prev = last;
		last = new_client;
	}
	return new_client;
}