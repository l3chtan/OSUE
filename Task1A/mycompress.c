#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LEN 80

char* append(char* source, char c){
	int len = strlen(source);
	char * dest = (char*)malloc(len+2);
	strcpy(dest,source);
	dest[len] = c;
	dest[len+1] = '\0';
	free(source);
	return dest;
}

char* compress(char *line, char* buf){
	int i = 0, cnt = 0;
	char c = line[0];

	buf = append(buf,c);
	while(line[i] != '\0'){
		if(line[i] == c){
			cnt++;
		} else {
			buf = append(buf,cnt + '0');
			buf = append(buf,line[i]);
			cnt = 1;
		c = line[i];
		}
		i++;
	}
	buf = append(buf,cnt+'0');
	buf = append(buf,c);
	return buf;
}

int read_input(FILE *stream, char *original_name){

	int cnt_original = 0;
	int cnt_comp = 0;
	char *buf = (char *) malloc(MAX_LINE_LEN+1);
	char *updated = malloc(MAX_LINE_LEN+1);
	char *new_buf = (char *) malloc(1);
	FILE *output;


	fgets(updated,MAX_LINE_LEN+1,stream);
	while(fgets(buf,MAX_LINE_LEN+1,stream)){

		char *tmp = malloc(strlen(buf)+strlen(updated));

		strcpy(tmp,updated);
		strcat(tmp,buf);
		free(updated);
		updated = tmp;

		if(buf[0] == EOF) break;
	}
		new_buf = compress(updated,new_buf);

	char *new_name = (char*) malloc(strlen(original_name)+5);
	strcpy(new_name, original_name);
	strcat(new_name, ".comp");

	output = fopen(new_name,"w");

	fputs(new_buf,output);
	fclose(output);

	printf("%s:\t\t%d Zeichen\n",original_name,(int) strlen(updated));
	printf("%s.comp: \t%d Zeichen\n",original_name,(int) strlen(new_buf));	

	free(updated);
	free(new_name);
	free(new_buf);
	free(buf);
	return 0;
}

int main(int argc, char **argv){

	switch(argc){
		case 1: 
			if(read_input(stdin, "Stdin") != 0){
				return 1;
			}
			break;
		default: 
			int i = 0;
			FILE *input;

			for(i = 1; i<argc;++i){
				input = fopen(argv[i],"r");

				if(read_input(input,argv[i]) != 0){
					return 1;
				}
				fclose(input);
			}
			break;
	}

	return 0;
}