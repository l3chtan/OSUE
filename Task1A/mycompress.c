#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LEN  80

// char* append(char* source, char c){
// 	char * tmp = realloc(source,strlen(source)+2);
// 	if(tmp == NULL) return NULL;
// 	source = tmp;
// 	int len = strlen(source);
// 	source[len] = c;
// 	source[len+1] = '\0';
// 	return source;
// }

char* compress(char *line){
	char * dest = (char*)malloc(1);
	int i = 0, cnt = 0, cx = 0;
	char c = line[0];
	char buffer[10];

	buffer[0] = line[0];
	if(dest == NULL) return NULL;
	while(line[i] != '\0'){
		if(line[i] == c){
			cnt++;
		} else {

			cx = snprintf(buffer,10,"%d",cnt);
			if(cx < 0 || cx > 10) return NULL;

			strncat(buffer,&line[i],1);
			if(buffer == NULL) return NULL;

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
	cx = snprintf(buffer,10,"%d",cnt);
	if(cx < 0 || cx > 10) return NULL;

	strncat(buffer,&line[i],1);
	if(buffer == NULL) return NULL;

	int len = strlen(dest)+strlen(buffer);
	char * t = realloc(dest,len);
	if(t == NULL) return NULL;

	strncat(t,buffer,len);
	if(dest == NULL) return NULL;
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

int main(int argc, char **argv){
			int i = 0;
			FILE *input, *output;
			char *orig, *compr;
			char *original_name;

	switch(argc){
		case 1: 
			printf("To compress the input enter Ctrl-D\n");
			original_name = "Stdin";
			if((orig = read_input(stdin) )== NULL){
				return 1;
			}

		default: 
			for(i = 1; i<argc;++i){
				input = fopen(argv[i],"r");

				original_name = argv[i];
				if((orig = read_input(input))== NULL){
					return 1;
				}
				fclose(input);
			}
			break;
	}

	compr = compress(orig);
	if(compr == NULL) return 1;

	char *new_name = (char*) malloc(strlen(original_name)+5);
	strcpy(new_name, original_name);
	strcat(new_name, ".comp");

	output = fopen(new_name,"w");

	fputs(compr,output);
	fclose(output);

	printf("%s:\t\t%d Zeichen\n",original_name,(int) strlen(orig));
	printf("%s.comp: \t%d Zeichen\n",original_name,(int) strlen(compr));	

	free(orig);
	free(compr);
	free(new_name);
	return 0;
}