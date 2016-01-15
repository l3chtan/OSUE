/**
 * @file hangman-common.c
 * @author Edward Toth 0725631
 * @date 10.01.2016 
 * 
 * @brief Common functions between client and server
 * 
 **/
 #include <string.h>
 

 #include "hangman-common.h"

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
