#ifndef HANGMAN
#define HANGMAN

#define BUFFER (64)
#define SHM_NAME "/hangman"
#define PERMISSION (0600)
#define SEM_1 "/sem_1"
#define SEM_2 "/sem_2"
#define SEM_3 "/sem_3"


struct shm_game {
	char msg[BUFFER];

	int client_id;
	int lost;
	int wrong;

	char word[BUFFER];
	char guess[BUFFER]; 
};

#endif
