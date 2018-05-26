#include <stdio.h>
#include <unistd.h>

#include "../include/padre.h"
#include "../include/types.h"
#include "../include/nipote.h"
#include "../include/figlio.h"
#include "../include/logger.h"

int main(int argc, char **argv){
	
	if(argc != 3){
		printf("errore argomenti\n");
	}else{
		padre((char*)argv[2], (char*)argv[3]);
	}	
	
	return 0;
}
