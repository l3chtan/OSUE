#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
	char *buf = (char *) malloc(81);
	char *updated = malloc(81);
	char *new_buf = (char *) malloc(1);
	char  *comp;
	FILE *output;


	fgets(updated,81,stream);
	while(fgets(buf,81,stream)){

		char *tmp = malloc(strlen(buf)+strlen(updated));

		strcpy(tmp,updated);
		strcat(tmp,buf);
		free(updated);
		updated = tmp;

		if(buf[0] == EOF) break;
		cnt_original += sizeof buf;

		cnt_comp += sizeof comp;


		buf[0] = '\0';
	}
		new_buf = compress(updated,new_buf);

	char *new_name = (char*) malloc((sizeof(buf) /sizeof(char))+5);
	strcpy(new_name, original_name);
	strcat(new_name, ".comp");

	output = fopen(new_name,"w");

	fputs(new_buf,output);
	fclose(output);
	free(new_buf);
	free(buf);
	printf("%s:\t\t%d Zeichen\n",original_name,cnt_original);
	printf("%s.comp: %d Zeichen\n",original_name,cnt_comp);	
	return 0;
}

int files_input(int argc, char **argv){

	int i = 0;
	FILE *input;

	for(i = 1; i<argc;++i){
		input = fopen(argv[i],"r");

		if(read_input(input,argv[i]) != 0){
			return 1;
		}
		fclose(input);
	}

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
			if(files_input(argc, argv) != 0){
				return 1;
			}
			break;
	}

	return 0;
}