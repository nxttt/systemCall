#ifndef PADRE_H
#define PADRE_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdlib.h>		//malloc, exit
#include <stdio.h>		//perror, printf, sprintf
//#include <wait.h>			//SIGUSR1, signal
#include <unistd.h>		//fork, pause, write

//wrapper del processo padre
void padre(char * file, char * fileOutput);

//crea segmento di memoria condivisa
struct Shmem* attach_segments(key_t key, int size);

//elimina segmento di memoria condivisa
void detach_segments(int descrittore);

//carica file di input
void load_file(char * file, void* addr);

//salva le chiavi sul file di output
void save_keys(char * fileOutput, void * addr);

//controlla che le chiavi siano corrette per tutta la lunghezza delle stringhe
int check_keys(int i, const int max, void * addr);

#endif
