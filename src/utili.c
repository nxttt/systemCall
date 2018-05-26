#include "../include/utili.h"

#include <stdlib.h>		//malloc, exit
#include <sys/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int countCifre(int i){
	int c = 0;
	while (i!=0){
		c++;
		i=i/10;
	}
	return c;
}

void copiaStr(char *dest, const char *src){
   //char *save = dest;
   while(*dest++ = *src++);
}

void copiaStrArr2(char *dest, char **src){
   //char *save = dest;
   while(*dest++ = **src++);
}

void copiaStrArr1(char **dest, char *src){
   //char *save = dest;
   while(**dest++ = *src++);
}

char * intToChar(int i, int cif, char *c){
	while (i!=0){
		cif--;
		c[cif] = i%10 + '0';
		i=i/10;		
	}	
	return c;
}

char * hex(int n){
	// Perche' 100 caratteri? una chiave sara' al massimo 32 bit
	char * hex = (char *) malloc(sizeof(char) * 100);

	int i = 0;
	int temp = 0;
	while(n!=0){		
		temp = n%16;
		if(temp < 10){
			hex[i] = temp + '0';
			i++;
		}
		else{
			hex[i] = temp-10 + 'A';
			i++;
		}
		n = n/16;
	}
	
	// Inverte la stringa
	char * hexx = (char *) malloc(sizeof(char) * i);
	int j = i-1;
	int k =0;
	for(j, k; j >= 0; j--, k++){
		hexx[k] = hex[j]; 
	}
	free(hex);
	return hexx;
}

int fileSize(char * file){
	struct stat buf;
	int fd = open(file, O_RDWR, 0666);
	fstat(fd, &buf);
	close(fd);
	// size in byte?
	off_t size = buf.st_size; 
	return (int) size;
}
