/**
 * @file hangman-common.h
 * @author Edward Toth 0725631
 * @date 10.01.2016 
 * 
 * @brief Header for common defines, structs and functions 
 * 
 **/

#ifndef HANGMAN
#define HANGMAN

#define BUFFER (64)
#define SHM_NAME "/hangman"
#define PERMISSION (0600)
#define SEM_1 "/sem_1"
#define SEM_2 "/sem_2"
#define SEM_3 "/sem_3"

#define REGISTER (-42)

#define NO_QUIT (0)
#define CLIENT_QUIT (1)


struct shm_game {
	char msg[BUFFER];

	int client_id;
	int lost;
	int wrong;
	int quit;

	char word[BUFFER];
	char guess[BUFFER]; 
};

sem_t *s1, *s2, *s3;
struct shm_game *game;

#endif
