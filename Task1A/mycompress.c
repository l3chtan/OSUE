#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LEN  80
#define FILE_ENDING 5

char* compress(char *line){
	char *dest = malloc(1);
	dest[0] = 0;
	int i = 0, cnt = 0, cx = 0;
	char c = line[0];
	char buffer[10];

	if(dest == NULL) return NULL;
	while(line[i] != '\0'){
		if(line[i] == c){
			cnt++;
		} else {

			buffer[0] = c;

			cx = snprintf(buffer+1,10,"%d",cnt);
			if(cx < 0 || cx > 10) return NULL;

			int len = strlen(dest)+strlen(buffer);
			char * t = realloc(dest,len);
			if(t == NULL) return NULL;

			dest = t;
			strncat(dest,buffer,len);
			if(dest == NULL) return NULL;

			buffer[0] = '\0';
			cnt = 1;
			cx = 0;
			c = line[i];
		}
		i++;
	}
	buffer[0] = c;

	cx = snprintf(buffer+1,10,"%d",cnt);
	if(cx < 0 || cx > 10) return NULL;

	int len = strlen(dest)+strlen(buffer);
	char * t = realloc(dest,len);
	if(t == NULL) return NULL;

	strncat(t,buffer,len);
	if(t == NULL) return NULL;
	return t;
}

char* read_input(FILE *stream){

	char *buf = (char *) malloc(MAX_LINE_LEN+1);
	char *updated = (char *) malloc(1);

	while(fgets(buf,MAX_LINE_LEN+1,stream)){
		int len = strlen(updated)+strlen(buf);
		char * tmp = realloc(updated,len+1);
		if(tmp == NULL) return NULL;

		updated = tmp;
		strncat(updated,buf,len);

		if(buf[0] == EOF) break;
	}

	free(buf);

	return updated;
}

int print_result(char *original_name ,char *compr, size_t orig_len){

	FILE *output;

	char new_name[strlen(original_name)+FILE_ENDING];
	strcpy(new_name, original_name);
	strncat(new_name, ".comp",FILE_ENDING);

	output = fopen(new_name,"w");

	fputs(compr,output);
	fclose(output);

	printf("%s:\t\t%d Zeichen\n",original_name,(int) orig_len);
	printf("%s.comp: \t%d Zeichen\n",original_name,(int) strlen(compr));	

	free(compr);

	return 0;
}

int main(int argc, char **argv){
			int i = 0;
			FILE *input;
			char *orig, *compr;

	switch(argc){
		case 1: 
			printf("To compress the text enter Ctrl-D\n");
			if((orig = read_input(stdin) )== NULL) return 1;

			compr = compress(orig);
			if(compr == NULL) return 1;


			if(!print_result("Stdin",compr,strlen(orig))) return 1;
			free(orig);

		default: 
			for(i = 1; i<argc;++i){
				input = fopen(argv[i],"r");

				if((orig = read_input(input))== NULL){
					return 1;
				}
				fclose(input);

				compr = compress(orig);
				if(compr == NULL) return 1;

				if(!print_result(argv[i],compr,strlen(orig))) return 1;
				free(orig);
			}
			break;
	}
	return 0;
}