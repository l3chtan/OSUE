#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* compress(char *line){
	/*int size = sizeof(line)/ sizeof(char);*/
	int i = 1, j = 0, cnt = 1;
	char *buf;
	buf = (char *) malloc(81*sizeof(char*));
		char c = line[0];

	buf[j] = line[0];
	while(line[i] != '\0'){
		if(line[i] == c){
			cnt++;
		} else {
			j++;
			buf[j] = cnt + '0';
			cnt = 1;
			j++;
			buf[j] = line[i];
			c = line[i];
		}
		i++;
	}
	buf[++j] = '\0';
		/*printf("%d\n",j);*/
	return buf;
}

int read_input(FILE *stream, char *original_name){

	int cnt_original = 0;
	int cnt_comp = 0;
	char *buf = (char *) malloc(81*sizeof(char));
	FILE *output;


	while(fgets(buf,81,stream)){
		if(buf[0] == EOF) break;
		cnt_original += sizeof buf;
		buf = compress(buf);

		/*if(buf == NULL) return 1;*/
		cnt_comp += sizeof buf;

		char *new_name = (char*) malloc((sizeof(buf) /sizeof(char))+5);
		/*new_name[0] = '\0';*/
		strcat(new_name,original_name);
		strcat(new_name, ".comp");

		output = fopen(new_name,"a");
		fputs(buf,output);
		fclose(output);
		buf[0] = '\0';

	}

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