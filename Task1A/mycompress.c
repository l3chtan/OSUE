#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LEN  80

char* append(char* source, char c){
	char * tmp = realloc(source,strlen(source)+2);
	if(tmp == NULL) return NULL;
	source = tmp;
	int len = strlen(source);
	source[len] = c;
	source[len+1] = '\0';
	return source;
}

char* compress(char *line, char* buf){
	char * dest = (char*)malloc(1);
	int i = 0, cnt = 0, ret = 0;
	char c = line[0];

	dest = append(dest,c);
	// if(ret != 0) return ret;
	while(line[i] != '\0'){
		if(line[i] == c){
			cnt++;
		} else {
			char *nums = (char*)malloc(10);
			int j = 0;
			while(cnt > 0){
				nums[j] = cnt % 10;
				nums[j] = nums[j] +48;
				cnt = cnt/10;
				j++;
			}
			nums[j] = '\0';
			char * t = realloc(dest,strlen(dest)+strlen(nums));
			if(t == NULL) return NULL;
			dest = t;
			strcat(dest,nums);
			free(nums);
			// dest = append(dest,cnt + '0');
			// if(ret != 0) return ret;
			dest = append(dest,line[i]);
			// if(ret != 0) return ret;
			cnt = 1;
		c = line[i];
		}
		i++;
	}
	dest = append(dest,cnt+'0');
	// if(ret != 0) return ret;
	dest = append(dest,c);
	// if(ret != 0) return ret;
	return dest;
}

int read_input(FILE *stream, char *original_name){
	int ret = 0;

	char *buf = (char *) malloc(MAX_LINE_LEN+1);
	char *updated = (char *) malloc(1);
	char *new_buf = (char *) malloc(1);
	FILE *output;


	while(fgets(buf,MAX_LINE_LEN+1,stream)){
		char * tmp = realloc(updated,strlen(updated)+strlen(buf)+1);
		if(tmp == NULL){
			return -1;
		}
		updated = tmp;
		strcat(updated,buf);

		if(buf[0] == EOF) break;
	}
	new_buf = compress(updated,new_buf);
	// if(ret != 0) return ret;

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
			int i = 0;
			FILE *input;

	switch(argc){
		case 1: 
			printf("To end the input press Ctrl-D\n");
			return read_input(stdin, "Stdin");

		default: 
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