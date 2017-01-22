/**
 * @file schedule.c
 * @author Edward Toth 0725631
 * @date 22.11.2015
 *
 * @brief Main program for this task
 *
 * This program checks in more or less regular intervals if the pressure in a nuclear reactor is ok.
 * If the pressure is too high, it will attempt to shut the reactor down. The whole checking and shutting down process is logged in a file.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <string.h>

#define READ 0
#define WRITE 1
#define EM_OK "EMERGENCY SUCCESSFUL"
#define EM_FAIL "EMERGENCY FAILED"
#define SHUT "SHUTDOWN COMPLETED"
#define SHUT_FAIL "KaBOOM!"

static int32_t opt_f = -1, opt_s = -1;
static char *command = "no command set";
static	int child_fd[2], parent_fd[2];
static	char *program, *emergency, *logfile;

/**
 * Option handler function
 * @brief This function parses the options given to the program and handles them accordingly.
 * @param argc is the number of arguments passed to the program (include the program itself)
 * @param argv holds the argument strings
 **/
static void opt_handle(int argc, char** argv);
/**
 * A function to create and use child processes
 * @brief In the created child processes programs, which were passed as arguments on program start,
 * are executed.
 * @details A new process is forked and a "program" executed in it. The ouptut of that program is passed to its parent process via a pipe.
 * Uses the global varialbes opt_f, opt_s, child_fd and parent_fd.
 * @param the program name which is to be called.
 * @return returns the status number of the executed program.
 **/
static int child_p(char *program);
/**
 * Usage function
 * @brief If schedule is started with the wrong arguments this function is called (usually from opt_handle)
 * and prints a usage message and terminates the program
 * @ return EXIT_FAILURE
 **/
static void usage(void);
/**
 * Bail out function - used in case of emergency
 * @brief Prints error message and exits program if any problem occurs during execution.
 * @param str contains the error message
 **/
 static void bail_out(char* str);

/**
 * Program entry point
 * @brief This function takes the program arguments, opens files and calls other functions.
 * @details sets command to the program name and calls opt_handle, to process the arguments.
 * Forks a child process which calls child_p in a loop where processes are created to call a program
 * which performs a certain task (e.g. check the pressure in a nuclear reactor).
 * After each call the loop pauses for a few seconds (the time window is specified by the arguments).
 * In case of a failure, another program in a different child process is executed and the program terminated afterwards.
 * All the output from the first executed program is logged in a file (which was also specified in the arguments list).
 * @param argc is the number of arguments passed to the program (include the program itself)
 * @param argv holds the argument strings
 * @ return returns EXIT_FAILURE if an error occured (e.g. process could not be spawned), EXIT_SUCCESS otherwise
 **/
int main(int argc, char** argv){

	command = argv[0];
	opt_handle(argc,argv);

	pid_t pid_child;
	int status_child = -1, status_program = 0;

	int log_fd = open(logfile,O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IRGRP|S_IROTH|S_IWUSR|S_IWGRP|S_IWOTH);

	if(pipe(parent_fd) != 0){
		bail_out("Could not create pipe");
	}

	/*First child process is created*/
	switch(pid_child = fork()){
		case -1:
			bail_out("Parent could not create child process");
			break;
		/*here is the code for the child process*/
		case 0:
			while(status_program == 0){
				int rn = 0;

				if(opt_f > 0){
					srand(time(NULL));
					rn = rand() % opt_f;
				}
				/*The process sleeps for a random amount of time between opt_s (default 1) and opt_f (default 0) seconds*/
				sleep(opt_s+rn); /*is this ok?*/

				/*if status_program is not 0 anymore (i.e. the process terminated with EXIT_FAILURE) the loop will stop*/
				status_program = child_p(program);
			}
			/*here is the call of the alternative program "emergency". It will be called if "program" terminates with a failure */
			(void) child_p(emergency);

			if(close(parent_fd[READ]) < 0) bail_out("Closing parent pipe failed");
			if(close(parent_fd[WRITE]) < 0) bail_out("Closing child pipe failed");
			break;
		/*this is the parent process*/
		default:
			if(close(parent_fd[WRITE]) < 0) bail_out("Closing parent pipe failed");

			char buffer[1024], tmp[64];
			memset(tmp,0,sizeof(tmp));

			/*check for the status of the child process and print the output from the pipes to stdout and to the log file.*/
			do{
				pid_child = waitpid(pid_child,&status_child,WNOHANG);

				fflush(stdout);
				memset(buffer,0,sizeof(buffer));
				if(read(parent_fd[READ],buffer,sizeof(buffer)) < 0) bail_out("Could not read from pipe to child");

				printf("%s",buffer);

				if(strncmp(buffer,SHUT,strlen(SHUT)) == 0){
					if(write(log_fd,EM_OK,strlen(EM_OK)) < 0) bail_out("Could not write to log file");
					printf("%s\n",EM_OK);

				} else if(strncmp(buffer,SHUT_FAIL,strlen(SHUT_FAIL)) == 0){
					if(write(log_fd,EM_FAIL,strlen(EM_FAIL)) < 0) bail_out("Could not write to log file");
					printf("%s\n",EM_FAIL);

				} else {
					if(write(log_fd,buffer,strlen(buffer)) < 0) bail_out("Could not write to log file");
				}

			} while(status_child < 0);

			if(close(log_fd) < 0) bail_out("Closing log file failed");
			if(close(parent_fd[READ]) < 0) bail_out("Closing parent pipe failed");
			break;
	}
	return EXIT_SUCCESS;
}

static void opt_handle(int argc, char **argv){

	int opt;
	// int32_t opt_s = -1, opt_f = -1;

	if(argc < 4 || argc > 8){
		usage();
		exit(EXIT_FAILURE);
	}

	while((opt = getopt(argc,argv,"s:f:"))!= -1){

		switch(opt){
			case 's':
				if(opt_s >= 0){
					usage();
					exit(EXIT_FAILURE);
					break;
				}
				opt_s = strtol(optarg,NULL,10);
				break;
			case 'f':
				if(opt_f >= 0){
					usage();
					exit(EXIT_FAILURE);
					break;
				}
				opt_f = strtol(optarg,NULL,10);
				break;
			case '?':
				fprintf(stderr,"%c is not a valid option\n",optopt);
				usage();
				exit(EXIT_FAILURE);
				break;
			default:
				usage();
				exit(EXIT_FAILURE);
				break;
		}
	}

	program = argv[optind];
	emergency = argv[++optind];
	logfile = argv[++optind];

	if(opt_s < 0){
		opt_s = 1;
	}
	if(opt_f < 0){
		opt_f = 0;
	}
}

static int child_p(char *program){
	pid_t pid_program;
	int status_program;
	char buffer[1024];

	if(pipe(child_fd) != 0){
		bail_out("Could not create child pipe");
	}

	switch(pid_program = fork()){
		case -1:
			bail_out("Could not create grandchild process");
			break;
		case 0:
			if(close(child_fd[READ]) < 0) bail_out("Closing read end from child pipe failed");
			if(close(parent_fd[READ]) < 0) bail_out("Closing read end from parent pipe failed");
			if(close(parent_fd[WRITE]) < 0) bail_out("Closing write end from parent pipe failed");

			if(dup2(child_fd[WRITE],WRITE)<0){
				bail_out("Could not change stdout file descriptor to child pipe write end");
			}
			if(close(child_fd[WRITE]) < 0) bail_out("Closing write end from child pipe failed");

			if(execl(program,program,(char*)0) == -1){
				bail_out("Execution of external program failed");
			}
			break;
		default:
			close(child_fd[WRITE]);

			memset(buffer,0,sizeof(buffer));

			pid_program = wait(&status_program);

			if(read(child_fd[READ],buffer,sizeof(buffer)) < 0) bail_out("Could not read from child pipe");
			if(write(parent_fd[WRITE],buffer,strlen(buffer)) < 0) bail_out("Could not write to buffer");

			if(close(child_fd[READ]) < 0) bail_out("Closing read end of child pipe failed");
			break;
	}
	return status_program;
}

static void usage(void){
	(void) fprintf(stderr, "SYNOPSIS:\n");
	(void) fprintf(stderr, "%s [-s <seconds>] [-f <seconds>] <program> <emergency> <logfile>\n\n",command);
	(void) fprintf(stderr, "-s <seconds>\tZeitfenster Anfang (Default: 1 Sekunde\n");
	(void) fprintf(stderr, "-f <seconds>\tmax. Zeitfenster Dauer (Default: 0 Sekunde\n");
	(void) fprintf(stderr, "<program>\tProgramm inkl. Pfad, das wiederholt ausgefuehrt werden soll\n");
	(void) fprintf(stderr, "<emergency>\tProgramm inkl. Pfad, das im Fehlerfall ausgefuehrt wird\n");
	(void) fprintf(stderr, "<logfile>\tPfad zu einer Datei, in der die Ausgabe von <program> sowie\n\t\tErfolg/Misserfolg von <emergency> protokolliert werden\n");
	exit(EXIT_FAILURE);
}

static void bail_out(char* str){
	(void) fprintf(stderr,"%s: %s\n",command,str);
	_exit(EXIT_FAILURE);
}
