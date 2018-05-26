#ifndef NIPOTE_H
#define NIPOTE_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <stdio.h>		//perror, printf, sprintf
#include <stdlib.h>		//malloc, exit

// EXTRA



// EXTRA


//wrapper del processo nipote
void nipote(int shmid1, int shmid2, int nstrings, int x, int msgid);

// Ritorna una struttura con testo plain e testo criptato, prendendo in input l' id della stringa da esaminare e l'indirizzo della shared memory
struct Spek* load_string(int mystring, void * addr);

//blocca accesso esclusivo regione critica
int lock();

//sblocca accesso esclusivo regione critica
int unlock();

// Trova la chiave utilizzata per criptare plain_text in encoded_text
unsigned find_key(char * plain_text, char * encoded_text);

// Scrive in quanto tempo ha trovato la chiave
void send_timeelapsed(char * string, int coda);

// Scrive la chiave nella shared_memory2 (int mystring, int dhmid2, unsigned key)
void save_key(int mystring, void *shm2, unsigned key);

#endif
